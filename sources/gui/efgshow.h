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
#include "nash/behavsol.h"

class EfgProfileList;
class EfgNavigateWindow;
class EfgOutcomeWindow;
class EfgSupportWindow;
class TreeWindow;

const int idEFG_SOLUTION_LIST = 900;

class EfgShow : public wxFrame {
private:
  efgGame &m_efg;
  TreeWindow *m_treeWindow;
  Node *m_cursor;

  int m_currentProfile;
  gList<BehavSolution> m_profiles;

  EFSupport *m_currentSupport;
  gList<EFSupport *> m_supports;

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

  void OnEditToggleSubgame(wxCommandEvent &);
  void OnEditMarkSubgameTree(wxCommandEvent &);
  void OnEditUnmarkSubgameTree(wxCommandEvent &);

  void OnEditMove(wxCommandEvent &);
  void OnEditGame(wxCommandEvent &);

  void OnViewProfiles(wxCommandEvent &);
  void OnViewCursor(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewZoomIn(wxCommandEvent &);
  void OnViewZoomOut(wxCommandEvent &);
  void OnViewSupportReachable(wxCommandEvent &);

  void OnFormatDisplayLayout(wxCommandEvent &);
  void OnFormatDisplayLegend(wxCommandEvent &);
  void OnFormatDisplayColors(wxCommandEvent &);
  void OnFormatDisplayDecimals(wxCommandEvent &);
  void OnFormatFontsAboveNode(wxCommandEvent &);
  void OnFormatFontsBelowNode(wxCommandEvent &);
  void OnFormatFontsAfterNode(wxCommandEvent &);
  void OnFormatFontsAboveBranch(wxCommandEvent &);
  void OnFormatFontsBelowBranch(wxCommandEvent &);

  void OnToolsDominance(wxCommandEvent &);
  void OnToolsEquilibrium(wxCommandEvent &);
  void OnToolsQre(wxCommandEvent &);
  void OnToolsNormalReduced(wxCommandEvent &);
  void OnToolsNormalAgent(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

  void OnSupportDuplicate(wxCommandEvent &);
  void OnSupportDelete(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesDuplicate(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);
  void OnProfilesProperties(wxCommandEvent &);
  void OnProfileSelected(wxListEvent &);

  void OnInfoNotebookPage(wxNotebookEvent &);

public:
  // CONSTRUCTOR AND DESTRUCTOR
  EfgShow(efgGame &p_efg, wxWindow *p_parent);
  virtual ~EfgShow();

  // PROFILE ACCESS AND MANIPULATION
  void AddProfile(const BehavSolution &, bool map);
  void RemoveProfiles(void);
  void ChangeProfile(int);
  int CurrentProfile(void) const { return m_currentProfile; }
  const BehavSolution &GetCurrentProfile(void) const;
  const gList<BehavSolution> &Profiles(void) const { return m_profiles; }
  gText UniqueProfileName(void) const;

  // SUPPORT ACCESS AND MANIPULATION
  EFSupport *GetSupport(void);
  const gList<EFSupport *> &Supports(void) const { return m_supports; }
  void SetSupportNumber(int p_number);
  gText UniqueSupportName(void) const;
  void OnSupportsEdited(void);

  gText GetRealizProb(const Node *) const;
  gText GetBeliefProb(const Node *) const;
  gText GetNodeValue(const Node *) const;
  gText GetInfosetProb(const Node *) const;
  gText GetInfosetValue(const Node *) const;
  gText GetActionValue(const Node *, int act) const;
  gText GetActionProb(const Node *, int act) const;
  gNumber ActionProb(const Node *n, int br) const;

  efgGame *Game(void) { return &m_efg; }


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

