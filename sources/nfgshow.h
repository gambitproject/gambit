//
// FILE: nfgshow.h -- header declarations for classes used in Normal Form GUI
//
// $Id$
//

#ifndef NFGSHOW_H
#define NFGSHOW_H

#include "wx.h"
#include "wxmisc.h"

#include "spread.h"
#include "nfgdraw.h"
#include "accels.h"

#include "gmisc.h"

#include "nfg.h"
#include "nfgiter.h"
#include "mixedsol.h"
#include "nfgsolng.h"

#include "msolnsf.h"

#include "efgnfgi.h"

class NfgSolnShow;
class NormalSpread;
class dialogNfgSupportInspect;

template <class T> class SolutionList: public gSortList<T>
{
private:
    unsigned int max_id;

public:
    SolutionList(void):gSortList<T>(), max_id(1) { }
    SolutionList(const gList<T> &l): gSortList<T>(l), max_id(1) { }

    virtual int Append(const T &a)
    {
        (*this)[gSortList<T>::Append(a)].SetId(max_id++);
        return Length();
    }
};

typedef SolutionList<MixedSolution> MixedSolutionList;

class NfgShow: public EfgNfgInterface, public NfgShowInterface {
private:
  Nfg &nf;
  NfgIter nf_iter;
  gList<NFSupport *> supports;
  MixedSolutionList solns;

  struct StartingPoints {
    MixedSolutionList profiles;
    int last;
    StartingPoints() : last(-1) { }
  } starting_points;

  int cur_soln;

  NFSupport *cur_sup;
  wxFrame     *pframe;
  NormalSpread    *spread;
  int pl1, pl2;
  int rows, cols;
  NfgSolnShow *soln_show; // need to keep track of this to kill at the end
  NormalDrawSettings  draw_settings;
  MSolnSortFilterOptions sf_options;
  gList<Accel>    accelerators;
  gText filename;
  
  // Private functions
  void UpdateContingencyProb(const gArray<int> &profile);
  void DumpAscii(Bool all_cont);

  gText UniqueSupportName(void) const;

  // process accelerators
  gArray<AccelEvent> MakeEventNames(void);

public:
  // Constructor
  NfgShow(Nfg &N, EfgNfgInterface *efg = 0, wxFrame *pframe = 0);
  virtual ~NfgShow();

  void SetPlayers(int _pl1, int _pl2, bool first_time = false);

  // Print takes care of ALL output needs.  Currently it supports
  // output in the following formats: Printer(win),
  // PostScript, Metafile/Clipboard(win), PrintPreview(win), Ascii
  void Print(void);
  void SetColors(void);

  void EditLabel(void);
  void SetStrategyLabels(void);
  void SetPlayerLabels(void);

  // Saving the game in native (.nfg) format
  Bool Save(void);

  // Supports and domination
  int SolveElimDom(void);

  void SupportNew(void);
  void SupportEdit(void);
  void SupportDelete(void);
  void SupportSelect(void);
  NFSupport *MakeSupport(void);
  
  void OutcomeNew(void);
  void OutcomeDelete(void);
  void OutcomeAttach(void);
  void OutcomeDetach(void);
  void OutcomeLabel(void);
  void OutcomePayoffs(int st1, int st2, bool next = false);
  void OutcomeOptions(void)
    { draw_settings.SetOutcomeDisp(1 - draw_settings.OutcomeDisp());
      UpdateVals();
    }

  void PrefsDisplayRows(void);
  void PrefsDisplayColumns(void);
  void PrefsDisplayDecimals(void);
  void PrefsFontsLabels(void);
  void PrefsFontsCells(void);

  void UpdateVals(void);
  void UpdateSoln(void);
  void UpdateProfile(gArray<int> &profile);
  
  // OnOK clean up
  void OnOk(void);

  // Now come the solution functions
  void Solve(int);
  void SolveStandard(void);
  void InspectSolutions(int what);
  void ClearSolutions(void);
  void RemoveSolutions(void);
  MixedSolution CreateSolution(void);
  void ChangeSolution(int sol);
  MixedProfile<gNumber> CreateStartProfile(int how);

  int NumSolutions(void) const { return solns.Length(); }

  // Project solutions to EF.
  void SolutionToExtensive(const MixedSolution &mp, bool set = false);

  // Filename support
  void SetFileName(const gText &s);
  const gText &Filename(void) const { return filename; }

  const Nfg &Game(void) const { return nf; }  
  // Display some inherent game properties
  void ShowGameInfo(void);
  bool GameIsDirty(void) const { return nf.IsDirty(); }

  // Process Accelerator Keys
  void EditAccelerators(void);
  int  CheckAccelerators(wxKeyEvent &ev);

  // Access to the actual window
  wxFrame *Frame(void) { return pframe; }

  // Access to the draw settings.
  const NormalDrawSettings& getNormalDrawSettings() { return draw_settings; }

  // Access to the underlying spreadsheet.
  const NormalSpread& getNormalSpread() { return *spread; }
};


class NormalSpread : public SpreadSheet3D {
private:
  gArray<wxChoice *> strat_profile;
  wxChoice *row_choice, *col_choice;
  wxSlider *slider;
  NfgShow *parent;
  int pl1, pl2;
  gArray<int> dimensionality;

  struct ns_features_struct {
    int prob, dom, val; /* these are actually int, not bool 0 or 1 */
    Bool verbose;
    ns_features_struct(void) :prob(0), dom(0), val(0), verbose(TRUE) { }
    ns_features_struct(const ns_features_struct &s): prob(s.prob), dom(s.dom),
      val(s.val), verbose(s.verbose) { }
  } features;

protected:
  wxMenuBar *MakeMenuBar(long menus);
  void UpdateMenus(void);

public:
  // Constructor
  NormalSpread(const NFSupport *sup, int _pl1, int _pl2, NfgShow *p, 
           wxFrame *pframe = 0);

  // Takes care of updating the player's strategy choice boxes
  void UpdateProfile(void);

  // Takes care of updating what players are used for row/col
  void UpdatePlayers(void)
    { parent->SetPlayers(row_choice->GetSelection()+1, col_choice->GetSelection()+1); }
  
  // Allows the user to set the entire profile at one time
  void SetProfile(const gArray<int> &profile);
  // Returns the current profile
  gArray<int> GetProfile(void);

  // Set Dimensionality.  This is needed for elimdom stuff
  void SetDimensionality(const NFSupport *sup);

  void SetStrategyLabels(const NFSupport *);
  void SetPlayerLabels(const NFSupport *);

  // Functions to display the row/col players
  void SetRowPlayer(int pl)
    {
      row_choice->SetSelection(pl-1);
      pl1 = pl;
    }

  void SetColPlayer(int pl)
    {
      col_choice->SetSelection(pl-1);
      pl2 = pl;
    }

  // Functions to create an extra row&col to display probs, dominators, values
  void MakeProbDisp(void);
  void RemoveProbDisp(void);
  int  HaveProbs(void) const { return features.prob; }
  void MakeDomDisp(void);
  void RemoveDomDisp(void);
  int  HaveDom(void) const { return features.dom; }
  void MakeValDisp(void);
  void RemoveValDisp(void);
  int  HaveVal(void) const { return features.val; }
  
  // Callback for double clicking on a cell.
  void OnDoubleClick(int row, int col, int, const gText &) 
    { parent->OutcomePayoffs(row, col); }

  // Callback for moving the selected cell.  Currently this updates the pl1, pl2 choice boxes
  void OnSelectedMoved(int row, int col, SpreadMoveDir /*how*/ = SpreadMoveJump)
    {
      strat_profile[pl1]->SetSelection(row-1);
      strat_profile[pl2]->SetSelection(col-1);
      UpdateMenus();
    }

  // Callback function for all output
  void OnPrint(void);

  // Callback for the OK button, deletes this sheet
  void OnOk(void)
    {
      Show(FALSE);
      parent->OnOk();
    }

  Bool OnClose(void);
  
  // Override menu handling
  void OnMenuCommand(int id);
  
  // Override character input handling
  Bool OnCharNew(wxKeyEvent &ev);
  
  // Override help system
  void OnHelp(int help_type = 0);

  // Process some options changes
  void OnOptionsChanged(unsigned int options = 0);

  // Static Callbacks.
  static void normal_strat_func(wxChoice &ob, wxEvent &)
    {
      NormalSpread *ns = (NormalSpread *)ob.GetClientData();
      ns->UpdateProfile();
      ns->CanvasFocus();
    }

  static void normal_player_func(wxChoice &ob, wxEvent &)
    {
      NormalSpread *ns = (NormalSpread *)ob.GetClientData();
      ns->UpdatePlayers();
      ns->CanvasFocus();
    }
};

#endif  // NFGSHOW_H
