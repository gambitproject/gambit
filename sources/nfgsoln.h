// File: NfgSoln.h -- declaration of classes for displaying the normal
// form solutions.
//
// $Id$
#ifndef NfgSoln_H
#define NfgSoln_H
#include "spread.h"
#include "msolnsf.h"

//****************************************************************************
//                       MIXED SOLUTION SHOW
//****************************************************************************
#define		MSOLN_O_OPTIONS		1
#define		MSOLN_O_EDIT			2
#define		MSOLN_O_SORTFILT	4
#define		MSOLN_O_EFGNFG		16

class NfgShow;
class NfgSolnShow: public SpreadSheet3D
{
private:
	static void settings_button(wxButton &ob,wxEvent &ev);
	static void extensive_button(wxButton &ob,wxEvent &ev);
	static void add_button(wxButton &ob,wxEvent &ev);
	static void edit_button(wxButton &ob,wxEvent &ev);
	static void delete_button(wxButton &ob,wxEvent &ev);
	static void delete_all_button(wxButton &ob,wxEvent &ev);
	static void sortfilt_button(wxButton &ob,wxEvent &ev);
protected:
	NfgShow *parent;
	gSortList<MixedSolution> &solns;
	int 	num_players,cur_soln,num_solutions;
	NormalDrawSettings	&norm_draw_settings;
	gBlock<Bool> features;
	static char *feature_names[];
	static int   feature_width[];
	MSolnSortFilterOptions &sf_options;
	void		UpdateSoln(int row,int col);
	int		FeaturePos(int feature);
	int		SolnNum(int row);
	int		SolnPos(int soln_num);
	void		UpdateValues(void);
	virtual void	OnRemove(bool all);
	virtual void	OnAdd(void);
	virtual void	OnEdit(void);
	virtual void	SortFilter(bool inter=true);
	virtual void	SetOptions(void);
public:
	NfgSolnShow(gSortList<MixedSolution> &soln,int num_players,int max_strats,
						int cur_soln_,NormalDrawSettings	&ds,
						MSolnSortFilterOptions &sf_options,
						NfgShow *parent_=0,wxFrame *parent_frame=0,
						unsigned int opts=MSOLN_O_OPTIONS|MSOLN_O_EDIT|MSOLN_O_SORTFILT|MSOLN_O_EFGNFG);
   // Allow solution transfer to an EFG
	void SolutionToExtensive(void);
	// Double clicking on a solution will update the parent
	void OnDoubleClick(int row,int col,int level,const gText &value);
	// Moving a cell updates the parent, if the dynamic option is set
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	// OnOk must be defined to inform parent that I am killed
	void OnOk(void);
	// Overide help system
	void OnHelp(int help_type=0);
	// Take care of some options changes
	void OnOptionsChanged(unsigned int options=0);
  // OnClose calls OnOk
  virtual Bool OnClose(void); 
};

//****************************************************************************
//                       MIXED SOLUTION PICKER (single)
//****************************************************************************

class Nfg1SolnPicker: public NfgSolnShow
{
private:
	int picked;
	void PickSoln(int row);
protected:
	virtual void OnRemove(bool all);
public:
	Nfg1SolnPicker(gSortList<MixedSolution> &soln,int num_players,int max_strats,
						int cur_soln_,NormalDrawSettings	&ds,
						MSolnSortFilterOptions &sf_options,
						NfgShow *parent_=0,wxFrame *parent_frame=0);
	// Double clicking on a solution will select it
	void OnDoubleClick(int row,int col,int level,const gText &value);
	// Override parent class feature
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	// OnOk must be defined to inform parent that I am killed
	void OnOk(void);
	// Override the help system
	virtual void OnHelp(int help_type=0);
	// Return the pick
	int Picked(void) const;
};

#endif
