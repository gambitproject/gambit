//
// FILE: efgshow.h -- Declarations of classes for extensive form display code
//
//  $Id$
//

#ifndef EFGSHOW_H
#define EFGSHOW_H

#include "wx/listctrl.h"
#include "wx/sashwin.h"
#include "wx/printdlg.h"

#include "gambit.h"
#include "efgconst.h"
#include "behavsol.h"

class EfgProfileList;
class EfgNavigateWindow;
class EfgOutcomeWindow;
class EfgSupportWindow;
class TreeWindow;
class TreeZoomWindow;

const int idEFG_SOLUTION_LIST = 900;

class EfgShow : public wxFrame, public EfgClient {
private:
  GambitFrame *m_parent;
  FullEfg &m_efg;
  TreeWindow *m_treeWindow;
  TreeZoomWindow *m_treeZoomWindow;

  int cur_soln;

  gList<EFSupport *> m_supports;
  EFSupport *m_currentSupport;

  EfgProfileList *m_solutionTable;
  gText m_filename;
  wxSashWindow *m_treeSashWindow, *m_nodeSashWindow, *m_toolSashWindow;
  wxSashWindow *m_solutionSashWindow;

  wxNotebook *m_infoNotebook;
  EfgNavigateWindow *m_navigateWindow;
  EfgOutcomeWindow *m_outcomeWindow;
  EfgSupportWindow *m_supportWindow;

  wxPageSetupData m_pageSetupData;
  wxPrintData m_printData;
  
  wxString m_script;

  void NodeInspect(bool insp);

  struct es_features {
    bool iset_hilight;
    es_features(void) : iset_hilight(FALSE) { }
  } features;

  void MakeMenus(void);
  void MakeToolbar(void);
  void AdjustSizes(void);
  
  // Event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnFocus(wxFocusEvent &);
  void OnSize(wxSizeEvent &);
  void OnSashDrag(wxSashEvent &);
  void OnActivate(wxActivateEvent &);

  void OnSetZoom(wxCommandEvent &);

  // Menu event handlers
  void OnFileSave(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &);

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

  void OnEditMassAll(wxCommandEvent &);
  void OnEditMassAction(wxCommandEvent &);

  void OnSubgamesMarkAll(wxCommandEvent &);
  void OnSubgamesMark(wxCommandEvent &);
  void OnSubgamesUnMarkAll(wxCommandEvent &);
  void OnSubgamesUnMark(wxCommandEvent &);
  void OnSubgamesCollapseAll(wxCommandEvent &);
  void OnSubgamesCollapse(wxCommandEvent &);
  void OnSubgamesExpandAll(wxCommandEvent &);
  void OnSubgamesExpandBranch(wxCommandEvent &);
  void OnSubgamesExpand(wxCommandEvent &);
  void OnSubgamesView(wxCommandEvent &);

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
  void OnSolveWizard(wxCommandEvent &);

  void OnViewProfiles(wxCommandEvent &);
  void OnViewCursor(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewInfosets(wxCommandEvent &);
  void OnViewGameInfo(wxCommandEvent &);
  void OnViewScript(wxCommandEvent &);

  void OnPrefsZoomIn(wxCommandEvent &);
  void OnPrefsZoomOut(wxCommandEvent &);
  void OnPrefsLegend(wxCommandEvent &);
  void OnPrefsFontsAboveNode(wxCommandEvent &);
  void OnPrefsFontsBelowNode(wxCommandEvent &);
  void OnPrefsFontsAfterNode(wxCommandEvent &);
  void OnPrefsFontsAboveBranch(wxCommandEvent &);
  void OnPrefsFontsBelowBranch(wxCommandEvent &);
  void OnPrefsDisplayLayout(wxCommandEvent &);
  void OnPrefsDisplayDecimals(wxCommandEvent &);
  void OnPrefsSave(wxCommandEvent &);
  void OnPrefsLoad(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesClone(wxCommandEvent &);
  void OnProfilesRename(wxCommandEvent &);
  void OnProfilesEdit(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);

  void OnInfoNotebookPage(wxNotebookEvent &);

  // EfgClient members
  void OnTreeChanged(bool, bool);

public:
  // CONSTRUCTOR AND DESTRUCTOR
  EfgShow(FullEfg &p_efg, GambitFrame *p_parent);
  virtual ~EfgShow();

  // Solution routines
  void RemoveSolutions(void);
  void ChangeSolution(int soln);
  int CurrentSolution(void) const { return cur_soln; }

  void OnSelectedMoved(const Node *n);

  // Solution interface to the algorithms
  void PickSolutions(const Efg::Game &, Node *, gList<BehavSolution> &);

  // Solution interface to normal form
  void AddSolution(const BehavSolution &, bool map);
  FullEfg *Game(void) { return &m_efg; }

  gNumber ActionProb(const Node *n, int br);

  // Interface for infoset hilighting between the tree and solution display
  void HilightInfoset(int pl, int iset, int who);

  gText UniqueSupportName(void) const;

  // Used by TreeWindow
  virtual gText AsString(TypedSolnValues what, const Node *n, int br = 0) const;

  // Currently used support
  EFSupport *GetSupport(void);
  const gList<EFSupport *> &Supports(void) const { return m_supports; }
  void SetSupportNumber(int p_number);
  void OnSupportsEdited(void);

  TreeWindow *GetTreeWindow(void) const { return m_treeWindow; }
  
  void UpdateMenus(void);
  int NumDecimals(void) const;

  void OnOutcomesEdited(void);

  // File name
  void SetFilename(const gText &s);
  const gText &Filename(void) const { return m_filename; }

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

