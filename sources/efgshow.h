//
// FILE: efgshow.h -- Declarations of classes for extensive form display code
//
//  $Id$
//

#ifndef EFGSHOW_H
#define EFGSHOW_H

#include "wx/listctrl.h"
#include "wx/sashwin.h"

#include "efgnfgi.h"
#include "gambit.h"
#include "accels.h"
#include "efgsolng.h"
#include "efgconst.h"
#include "solnlist.h"

class EfgProfileList;
class EfgToolbar;
class EfgCursorWindow;
class TreeWindow;
class TreeZoomWindow;

typedef SolutionList<BehavSolution> BehavSolutionList;

const int idEFG_SOLUTION_LIST = 900;

class EfgShow : public wxFrame,
		public EfgNfgInterface, public EfgShowInterface {
private:
  wxFrame *parent;
  FullEfg &m_efg;
  TreeWindow *m_treeWindow;
  TreeZoomWindow *m_treeZoomWindow;

  // Solution routines
  struct StartingPoints {
    BehavSolutionList profiles;
    int last;
    StartingPoints() : last(-1) { }
  } starting_points;
  
  int cur_soln;

  gList<EFSupport *> m_supports;
  EFSupport *m_currentSupport;

  EfgProfileList *m_solutionTable;
  gList<Accel>    accelerators;
  gText   filename;
  EfgToolbar *m_toolbar;
  wxSashWindow *m_treeSashWindow, *m_nodeSashWindow, *m_toolSashWindow;
  wxSashWindow *m_solutionSashWindow;

  // Private functions
  gArray<AccelEvent> MakeEventNames(void);
  EfgCursorWindow *m_cursorWindow;

  void NodeInspect(bool insp);

  struct es_features {
    bool iset_hilight;
    es_features(void) : iset_hilight(FALSE) { }
  } features;

  void MakeMenus(void);
  void AdjustSizes(void);
  
  // Event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnFocus(wxFocusEvent &);
  void OnSize(wxSizeEvent &);
  void OnSashDrag(wxSashEvent &);

  void OnSolutionSelected(wxListEvent &);

  // Menu event handlers
  void OnFileOutput(wxCommandEvent &);
  void OnFileSave(wxCommandEvent &);
  
  void OnEditNodeAdd(wxCommandEvent &);
  void OnEditNodeDelete(wxCommandEvent &);
  void OnEditNodeInsert(wxCommandEvent &);
  void OnEditNodeLabel(wxCommandEvent &);
  void OnEditNodeSetMark(wxCommandEvent &);
  void OnEditNodeGotoMark(wxCommandEvent &);

  void OnEditActionDelete(wxCommandEvent &);
  void OnEditActionInsert(wxCommandEvent &);
  void OnEditActionAppend(wxCommandEvent &);
  void OnEditActionLabel(wxCommandEvent &);
  void OnEditActionProbs(wxCommandEvent &);

  void OnEditInfosetMerge(wxCommandEvent &);
  void OnEditInfosetBreak(wxCommandEvent &);
  void OnEditInfosetSplit(wxCommandEvent &);
  void OnEditInfosetJoin(wxCommandEvent &);
  void OnEditInfosetLabel(wxCommandEvent &);
  void OnEditInfosetPlayer(wxCommandEvent &);
  void OnEditInfosetReveal(wxCommandEvent &);

  void OnEditOutcomesAttach(wxCommandEvent &);
  void OnEditOutcomesDetach(wxCommandEvent &);
  void OnEditOutcomesLabel(wxCommandEvent &);
  void OnEditOutcomesPayoffs(wxCommandEvent &);
  void OnEditOutcomesNew(wxCommandEvent &);
  void OnEditOutcomesDelete(wxCommandEvent &);

  void OnEditTreeDelete(wxCommandEvent &);
  void OnEditTreeCopy(wxCommandEvent &);
  void OnEditTreeMove(wxCommandEvent &);
  void OnEditTreeLabel(wxCommandEvent &);
  void OnEditTreePlayers(wxCommandEvent &);
  void OnEditTreeInfosets(wxCommandEvent &);

  void OnSubgamesMarkAll(wxCommandEvent &);
  void OnSubgamesMark(wxCommandEvent &);
  void OnSubgamesUnMarkAll(wxCommandEvent &);
  void OnSubgamesUnMark(wxCommandEvent &);
  void OnSubgamesCollapseAll(wxCommandEvent &);
  void OnSubgamesCollapse(wxCommandEvent &);
  void OnSubgamesExpandAll(wxCommandEvent &);
  void OnSubgamesExpandBranch(wxCommandEvent &);
  void OnSubgamesExpand(wxCommandEvent &);

  void OnSupportUndominated(wxCommandEvent &);
  void OnSupportNew(wxCommandEvent &);
  void OnSupportEdit(wxCommandEvent &);
  void OnSupportDelete(wxCommandEvent &);
  void OnSupportSelectFromList(wxCommandEvent &);
  void OnSupportSelectPrevious(wxCommandEvent &);
  void OnSupportSelectNext(wxCommandEvent &);
  void OnSupportReachable(wxCommandEvent &);

  void OnSolveStandard(wxCommandEvent &);
  void OnSolveCustom(wxCommandEvent &);
  void OnSolveNormalReduced(wxCommandEvent &);
  void OnSolveNormalAgent(wxCommandEvent &);

  void OnInspectSolutions(wxCommandEvent &);
  void OnInspectCursor(wxCommandEvent &);
  void OnInspectInfosets(wxCommandEvent &);
  void OnInspectZoom(wxCommandEvent &);
  void OnInspectGameInfo(wxCommandEvent &);

  void OnPrefsZoomIn(wxCommandEvent &);
  void OnPrefsZoomOut(wxCommandEvent &);
  void OnPrefsLegend(wxCommandEvent &);
  void OnPrefsFontsAboveNode(wxCommandEvent &);
  void OnPrefsFontsBelowNode(wxCommandEvent &);
  void OnPrefsFontsAfterNode(wxCommandEvent &);
  void OnPrefsFontsAboveBranch(wxCommandEvent &);
  void OnPrefsFontsBelowBranch(wxCommandEvent &);
  void OnPrefsDisplayLayout(wxCommandEvent &);
  void OnPrefsDisplayFlashing(wxCommandEvent &);
  void OnPrefsDisplayDecimals(wxCommandEvent &);
  void OnPrefsColors(wxCommandEvent &);
  void OnPrefsSave(wxCommandEvent &);
  void OnPrefsLoad(wxCommandEvent &);
  void OnPrefsAccels(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesClone(wxCommandEvent &);
  void OnProfilesRename(wxCommandEvent &);
  void OnProfilesEdit(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);

public:
  // CONSTRUCTOR AND DESTRUCTOR
  EfgShow(FullEfg &p_efg, EfgNfgInterface *p_nfg, wxFrame *p_parent);
  virtual ~EfgShow();

  // Solution routines
  void RemoveSolutions(void);
  void ChangeSolution(int soln);
  BehavSolution CreateSolution(void);
  int CurrentSolution(void) const { return cur_soln; }

  void OnSelectedMoved(const Node *n);

  // Solution interface to the algorithms
  void PickSolutions(const Efg &, Node *, gList<BehavSolution> &);
  BehavProfile<gNumber> CreateStartProfile(int how);
  void RemoveStartProfiles(void);

  // Solution interface to normal form
  void SolutionToEfg(const BehavProfile<gNumber> &s, bool set = false);
  const Efg *InterfaceObjectEfg(void) { return &m_efg; }
  FullEfg *Game(void) { return &m_efg; }
  wxFrame *Frame(void);

  gNumber ActionProb(const Node *n, int br);

  // Reset the supports.
  void GameChanged(void);

  // Interface for infoset hilighting between the tree and solution display
  void HilightInfoset(int pl, int iset, int who);

  // Accelerators allow for platform-indep handling of hotkeys
  int CheckAccelerators(wxKeyEvent &ev);

  gText UniqueSupportName(void) const;

  // Used by TreeWindow
  virtual gText AsString(TypedSolnValues what, const Node *n, int br = 0) const;

  // Currently used support
  const EFSupport *GetSupport(void);
  TreeWindow *GetTreeWindow(void) const { return m_treeWindow; }
  
  void UpdateMenus(void);
  int NumDecimals(void) const;

  // File name
  void SetFileName(const gText &s);
  const gText &Filename(void) const;

  Node *Cursor(void) const;

  DECLARE_EVENT_TABLE()
};


// Solution constants
typedef enum {
  EFG_NO_SOLUTION = -1, EFG_QRE_SOLUTION, EFG_LIAP_SOLUTION,
  EFG_LCP_SOLUTION, EFG_PURENASH_SOLUTION, EFG_CSUM_SOLUTION,
  EFG_NUM_SOLUTIONS
} EfgSolutionT;

#endif // EFGSHOW_H

