//
// File: normshow.h -- header declarations for classes used in Normal Form GUI
//
// $Id$
//

#ifndef NORMSHOW_H
#define NORMSHOW_H

#include "wx.h"
#include "wxmisc.h"

#include "spread.h"
#include "nfgdraw.h"
#include "accels.h"

#include "gmisc.h"

#include "nfg.h"
#include "nfgiter.h"
#include "mixedsol.h"
#include "nfgsolvd.h"
#include "msolnsf.h"

#include "efgnfgi.h"
#include "paramsd.h"

class NfgSolnShow;
class NormalSpread;
class NFSupportInspectDialog;
class NfgOutcomeDialog;

template <class T> class SolutionList: public gSortList<T>
{
private:
	unsigned int max_id;
public:
	SolutionList(void):gSortList<T>(),max_id(1) {}
	SolutionList(const gList<T> &l): gSortList<T>(l),max_id(1) { }
	virtual int Append(const T &a)
	{(*this)[gSortList<T>::Append(a)].SetId(max_id++);return Length();}
};
typedef SolutionList<MixedSolution> MixedSolutionList;

class NfgShow: public EfgNfgInterface, public ParametrizedGame
{
private:
	Nfg &nf;
	NfgIter nf_iter;
	gList<NFSupport *> supports;
  	MixedSolutionList solns;
	struct StartingPoints
	{
		MixedSolutionList profiles;
		int last;
		StartingPoints() : last(-1) { }
	} starting_points;
	int cur_soln;
   ParameterSetList param_sets;
   ParameterDialog *params_dialog;
 // we can display NF for one support, while working on a different support
 // disp_sup always corresponds to the support currently displayed.  cur_sup
 // corresponds to the support that will be operated upon by solution algs.
	NFSupport *cur_sup,*disp_sup;
	wxFrame		*pframe;
	NormalSpread 	*spread;
	NFSupportInspectDialog *support_dialog;
	NfgOutcomeDialog *outcome_dialog;
   ParameterDialog *param_dialog;
	int pl1,pl2;
	int rows,cols;
	NfgSolnShow *soln_show;	// need to keep track of this to kill at the end
	NormalDrawSettings	draw_settings;
	MSolnSortFilterOptions sf_options;
	gList<Accel>	accelerators;
	gString filename;
	// Private functions
	void UpdateSoln(void);
	void UpdateDom(void);
	void UpdateContingencyProb(const gArray<int> &profile);
	void DumpAscii(Bool all_cont);
	// Make a support based on the elimdom options
	NFSupport *MakeSolnSupport(void);
	// process accelerators
	gArray<AccelEvent> MakeEventNames(void);
public:
	// Constructor
	NfgShow(Nfg &N,EfgNfgInterface *efg=0,wxFrame *pframe=0);
	void SetPlayers(int _pl1,int _pl2,bool first_time=false);
	// Print takes care of ALL output needs.  Currently it supports
	// output in the following formats: Printer(win),
	// PostScript,Metafile/Clipboard(win),PrintPreview(win),Ascii
	void Print(void);
	void SetOptions(void);
	void SetColors(void);
	void SetLabels(int what); // what==0: game, what==1: strats, what==2: players
	// Saving the game in native (.nfg) format
	void Save(void);
	// Supports and domination
	int SolveElimDom(void);
	void ChangeSupport(int what);
	NFSupport *MakeSupport(void);
	void	DominanceSetup(void);
	// Outcomes
	void SetOutcome(int outc,int x=-1, int y=-1);
	void ChangeOutcomes(int what);
	void OutcomeOptions(void) {draw_settings.OutcomeOptions();UpdateVals();}
   void ChangeParameters(int what);

   void UpdateVals(void);
	void UpdateProfile(gArray<int> &profile);
	// OnOK clean up
	void OnOk(void);
	// Now come the solution functions
	void Solve(void);
	void SolveSetup(int what);
	void InspectSolutions(int what);
	void ClearSolutions(void);
	void RemoveSolutions(void);
	MixedSolution CreateSolution(void);
	void ChangeSolution(int sol);
	MixedProfile<gNumber> CreateStartProfile(int how);
	// Project solutions to EF.
	void SolutionToExtensive(const MixedSolution &mp,bool set=false);

   //
  	bool	 SolveLiap(const NFSupport *sup);
	//

	void ChangePayoffs(int st1,int st2,bool next=false);
	// Filename support
	void SetFileName(const gString &s);
	gString GetFileName(void) const;
	// Display some inherent game properties
	void ShowGameInfo(void);
   // Process Accelerator Keys
	void EditAccelerators(void);
	int  CheckAccelerators(wxKeyEvent &ev);
   ParameterSetList &Parameters(void);
};

class NormalSpread : public SpreadSheet3D
{
private:
	gArray<wxChoice *> strat_profile;
	wxChoice *row_choice,*col_choice;
	wxSlider *slider;
	int inspect_item;
	NfgShow *parent;
	int pl1,pl2;
	gArray<int> dimensionality;
	struct ns_features_struct
	{
	int prob,dom,val; /* these are actually int, not bool 0 or 1 */
	Bool verbose;
	ns_features_struct(void) :prob(0),dom(0),val(0),verbose(TRUE) { }
	ns_features_struct(const ns_features_struct &s): prob(s.prob),dom(s.dom),
													val(s.val),verbose(s.verbose) { }
	} features;
protected:
	wxMenuBar *MakeMenuBar(long menus);
public:
	// Constructor
	NormalSpread(const NFSupport *sup,int _pl1,int _pl2,NfgShow *p,wxFrame *pframe=0);
	// Takes care of updating the player's strategy choice boxes
	void UpdateProfile(void)
	{
		gArray<int> profile(strat_profile.Length());
		for (int i=1;i<=strat_profile.Length();i++)
			profile[i]=strat_profile[i]->GetSelection()+1;
		parent->UpdateProfile(profile);
		SetCurRow(profile[pl1]);SetCurCol(profile[pl2]);
	}
	// Takes care of updating what players are used for row/col
	void UpdatePlayers(void)
	{parent->SetPlayers(row_choice->GetSelection()+1,col_choice->GetSelection()+1);}
	// Allows the user to set the entire profile at one time
	void SetProfile(const gArray<int> &profile)
	{
		for (int i=1;i<=strat_profile.Length();i++) strat_profile[i]->SetSelection(profile[i]-1);
		SetCurRow(profile[pl1]);SetCurCol(profile[pl2]);
	}
	// Returns the current profile
	gArray<int> GetProfile(void)
	{
	gArray<int> profile(strat_profile.Length());
	for (int i=1;i<=strat_profile.Length();i++)
		profile[i]=strat_profile[i]->GetSelection()+1;
	return profile;
	}
	// Set Dimensionality.  This is needed for elimdom stuff
	void SetDimensionality(const NFSupport *sup);
	void SetLabels(const NFSupport *sup,int what); // what==0: game, what==1: strats, what==2: players
	// Functions to display the row/col players
	void SetRowPlayer(int pl) {row_choice->SetSelection(pl-1);pl1=pl;}
	void SetColPlayer(int pl) {col_choice->SetSelection(pl-1);pl2=pl;}
	// Functions to create an extra row&col to display probs, dominators, values
	void MakeProbDisp(void);
	void RemoveProbDisp(void);
	int  HaveProbs(void) const {return features.prob;}
	void MakeDomDisp(void);
	void RemoveDomDisp(void);
	int  HaveDom(void) const {return features.dom;}
	void MakeValDisp(void);
	void RemoveValDisp(void);
	int  HaveVal(void) const {return features.val;}
	// Functions to enable/disable menuiterms
	void EnableInspect(Bool enable) {GetMenuBar()->Enable(inspect_item,enable);}
	// Callback for double clicking on a cell.  Currently this edits the cell
	void OnDoubleClick(int row,int col,int ,const gString &) {parent->ChangePayoffs(row,col);}
	// Callback for moving the selected cell.  Currently this updates the pl1,pl2 choice boxes
	void OnSelectedMoved(int row,int col,SpreadMoveDir /*how*/=SpreadMoveJump)
	{strat_profile[pl1]->SetSelection(row-1);strat_profile[pl2]->SetSelection(col-1);}
	// Callback for changing the options function
	void OnOptions(void) {parent->SetOptions();}
	// Callback function for all output
	void OnPrint(void);
	// Callback for the OK button, deletes this sheet
	void OnOk(void) {Show(FALSE); parent->OnOk();}
	// Overide menu handling
	void OnMenuCommand(int id);
	// Overide character input handling
	Bool OnCharNew(wxKeyEvent &ev);
	// Overide help system
	void OnHelp(int help_type=0);
	// Process some options changes
	void OnOptionsChanged(unsigned int options=0);
	// Static Callbacks.
	static void normal_strat_func(wxChoice &ob,wxEvent &)
	{
		NormalSpread *ns=(NormalSpread *)ob.GetClientData();
		ns->UpdateProfile();
		ns->CanvasFocus();
	}
	static void normal_player_func(wxChoice &ob,wxEvent &)
	{
		NormalSpread *ns=(NormalSpread *)ob.GetClientData();
		ns->UpdatePlayers();
		ns->CanvasFocus();
	}
	static void normal_solve_func(wxButton &ob,wxEvent &)
	{
		NormalSpread *ns=(NormalSpread *)ob.GetClientData();
		ns->parent->Solve();
		ns->CanvasFocus();
	}
	static void normal_options_func(wxButton &ob,wxEvent &)
	{
		NormalSpread *ns=(NormalSpread *)ob.GetClientData();
		ns->OnOptions();
		ns->CanvasFocus();
	}

};

#endif
