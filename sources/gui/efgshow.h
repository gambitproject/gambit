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
#include "game/behavsol.h"

class EfgProfileList;
class EfgNavigateWindow;
class EfgOutcomeWindow;
class EfgSupportWindow;
class TreeWindow;

const int idEFG_SOLUTION_LIST = 900;

class EfgShow : public wxFrame {
private:
  FullEfg &m_efg;
  TreeWindow *m_treeWindow;
  Node *m_cursor;

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

  void OnEditInsert(wxCommandEvent &);
  void OnEditDelete(wxCommandEvent &);
  void OnEditReveal(wxCommandEvent &);

  void OnEditMove(wxCommandEvent &);
  void OnEditGame(wxCommandEvent &);

  void OnViewProfiles(wxCommandEvent &);
  void OnViewCursor(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewZoomIn(wxCommandEvent &);
  void OnViewZoomOut(wxCommandEvent &);
  void OnViewSupportReachable(wxCommandEvent &);

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

  void OnToolsDominance(wxCommandEvent &);

  void OnToolsEquilibriumStandard(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgEnumPure(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgLcp(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgLiap(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgLp(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgPolEnum(wxCommandEvent &);
  void OnToolsEquilibriumCustomEfgQre(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgEnumPure(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgEnumMixed(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgLcp(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgLiap(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgLp(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgQre(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgQreGrid(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgPolEnum(wxCommandEvent &);
  void OnToolsEquilibriumCustomNfgSimpdiv(wxCommandEvent &);

  void OnToolsNormalReduced(wxCommandEvent &);
  void OnToolsNormalAgent(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

  void OnSupportDuplicate(wxCommandEvent &);
  void OnSupportDelete(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesClone(wxCommandEvent &);
  void OnProfilesRename(wxCommandEvent &);
  void OnProfilesEdit(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);

  void OnInfoNotebookPage(wxNotebookEvent &);

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

  // Solution interface to normal form
  void AddProfile(const BehavSolution &, bool map);
  FullEfg *Game(void) { return &m_efg; }

  gText UniqueSupportName(void) const;

  // Currently used support
  EFSupport *GetSupport(void);
  const gList<EFSupport *> &Supports(void) const { return m_supports; }
  void SetSupportNumber(int p_number);
  void OnSupportsEdited(void);

  void UpdateMenus(void);
  int NumDecimals(void) const;

  void OnOutcomesEdited(void);

  void SetFilename(const wxString &s);
  const wxString &Filename(void) const { return m_filename; }

  void SetCursor(Node *m_node);
  Node *Cursor(void) const { return m_cursor; }

  void OnEditNode(wxCommandEvent &);
  void OnTreeChanged(bool, bool);

  DECLARE_EVENT_TABLE()
};

#endif // EFGSHOW_H

