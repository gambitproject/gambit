//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to class representing extensive form viewing window
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
  FullEfg &m_efg;
  TreeWindow *m_treeWindow;
  TreeZoomWindow *m_treeZoomWindow;

  int m_currentProfile;
  gList<EFSupport *> m_supports;
  EFSupport *m_currentSupport;

  EfgProfileList *m_profileTable;
  wxString m_filename;
  wxSashWindow *m_treeSashWindow, *m_nodeSashWindow, *m_toolSashWindow;
  wxSashWindow *m_solutionSashWindow;

  wxNotebook *m_infoNotebook;
  EfgNavigateWindow *m_navigateWindow;
  EfgOutcomeWindow *m_outcomeWindow;
  EfgSupportWindow *m_supportWindow;

  wxPageSetupData m_pageSetupData;
  wxPrintData m_printData;
  
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

  // Menu event handlers
  void OnFileNew(wxCommandEvent &);
  void OnFileOpen(wxCommandEvent &);
  void OnFileSave(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &);
  void OnFileExit(wxCommandEvent &);
  void OnFileMRUFile(wxCommandEvent &);

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

  void OnViewProfiles(wxCommandEvent &);
  void OnViewCursor(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewInfosets(wxCommandEvent &);
  void OnViewZoomIn(wxCommandEvent &);
  void OnViewZoomOut(wxCommandEvent &);
  void OnViewGameInfo(wxCommandEvent &);

  void OnFormatLegend(wxCommandEvent &);
  void OnFormatFontsAboveNode(wxCommandEvent &);
  void OnFormatFontsBelowNode(wxCommandEvent &);
  void OnFormatFontsAfterNode(wxCommandEvent &);
  void OnFormatFontsAboveBranch(wxCommandEvent &);
  void OnFormatFontsBelowBranch(wxCommandEvent &);
  void OnFormatDisplayLayout(wxCommandEvent &);
  void OnFormatDisplayDecimals(wxCommandEvent &);
  void OnFormatSave(wxCommandEvent &);
  void OnFormatLoad(wxCommandEvent &);

  void OnToolsSubgamesMarkAll(wxCommandEvent &);
  void OnToolsSubgamesMark(wxCommandEvent &);
  void OnToolsSubgamesUnMarkAll(wxCommandEvent &);
  void OnToolsSubgamesUnMark(wxCommandEvent &);
  void OnToolsSubgamesCollapseAll(wxCommandEvent &);
  void OnToolsSubgamesCollapse(wxCommandEvent &);
  void OnToolsSubgamesExpandAll(wxCommandEvent &);
  void OnToolsSubgamesExpandBranch(wxCommandEvent &);
  void OnToolsSubgamesExpand(wxCommandEvent &);
  void OnToolsSubgamesView(wxCommandEvent &);

  void OnToolsSupportUndominated(wxCommandEvent &);
  void OnToolsSupportNew(wxCommandEvent &);
  void OnToolsSupportEdit(wxCommandEvent &);
  void OnToolsSupportDelete(wxCommandEvent &);
  void OnToolsSupportSelectFromList(wxCommandEvent &);
  void OnToolsSupportSelectPrevious(wxCommandEvent &);
  void OnToolsSupportSelectNext(wxCommandEvent &);
  void OnToolsSupportReachable(wxCommandEvent &);

  void OnToolsEquilibriumStandard(wxCommandEvent &);
  void OnToolsEquilibriumCustom(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgEnumPure(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgLcp(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgLp(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgQre(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgEnumPure(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgEnumMixed(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgLcp(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgLp(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgQre(wxCommandEvent &);

  void OnToolsNormalReduced(wxCommandEvent &);
  void OnToolsNormalAgent(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

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
  EfgShow(FullEfg &p_efg, wxWindow *p_parent);
  virtual ~EfgShow();

  // PROFILE ACCESS AND MANIPULATION
  void RemoveProfiles(void);
  void ChangeProfile(int soln);
  int CurrentProfile(void) const { return m_currentProfile; }
  const BehavSolution &GetCurrentProfile(void) const;

  gText GetRealizProb(const Node *) const;
  gText GetBeliefProb(const Node *) const;
  gText GetNodeValue(const Node *) const;
  gText GetInfosetProb(const Node *) const;
  gText GetInfosetValue(const Node *) const;
  gText GetActionValue(const Node *, int act) const;
  gText GetActionProb(const Node *, int act) const;
  gNumber ActionProb(const Node *n, int br) const;

  void OnSelectedMoved(const Node *n);

  // Solution interface to the algorithms
  void PickSolutions(const Efg::Game &, gList<BehavSolution> &);

  // Solution interface to normal form
  void AddProfile(const BehavSolution &, bool map);
  FullEfg *Game(void) { return &m_efg; }


  // Interface for infoset hilighting between the tree and solution display
  void HilightInfoset(int pl, int iset, int who);

  gText UniqueSupportName(void) const;

  // Currently used support
  EFSupport *GetSupport(void);
  const gList<EFSupport *> &Supports(void) const { return m_supports; }
  void SetSupportNumber(int p_number);
  void OnSupportsEdited(void);

  void UpdateMenus(void);
  int NumDecimals(void) const;

  void OnOutcomesEdited(void);

  // File name
  void SetFilename(const wxString &s);
  const wxString &Filename(void) const { return m_filename; }

  Node *Cursor(void) const;

  DECLARE_EVENT_TABLE()
};

#endif // EFGSHOW_H

