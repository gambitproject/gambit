//
// FILE: nfgshow.h -- Declaration of normal form display class
//
// $Id$
//

#ifndef NFGSHOW_H
#define NFGSHOW_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/sashwin.h"
#include "wx/printdlg.h"
#include "guishare/wxmisc.h"

#include "base/gmisc.h"

#include "game/nfg.h"
#include "game/nfgiter.h"
#include "game/mixedsol.h"

#include "gambit.h"

const int idNFG_SOLUTION_LIST = 1900;

class NfgNavigateWindow;
class NfgOutcomeWindow;
class NfgSupportWindow;
class NfgProfileList;
class NfgTable;
class dialogNfgSupportInspect;

class NfgShow : public wxFrame {
private:
  Nfg &m_nfg;

  NfgTable *m_table;
  NfgProfileList *m_solutionTable;
  wxNotebook *m_infoNotebook;

  wxSashWindow *m_solutionSashWindow, *m_infoSashWindow;
  NfgNavigateWindow *m_navigateWindow;
  NfgOutcomeWindow *m_outcomeWindow;
  NfgSupportWindow *m_supportWindow;

  gList<NFSupport *> m_supports;
  NFSupport *m_currentSupport;
  int m_currentSolution;

  wxString m_filename;

  wxPageSetupData m_pageSetupData;
  wxPrintData m_printData;

  void MakeMenus(void);
  void MakeToolbar(void);
  void UpdateMenus(void);
  gText UniqueSupportName(void) const;
  void AdjustSizes(void);

  // Menu event handlers
  void OnFileNew(wxCommandEvent &);
  void OnFileOpen(wxCommandEvent &);
  void OnFileSave(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &);
  void OnFileExit(wxCommandEvent &);
  void OnFileMRUFile(wxCommandEvent &);

  void OnEditStrategies(wxCommandEvent &);
  void OnEditOutcomeNew(wxCommandEvent &);
  void OnEditOutcomeDelete(wxCommandEvent &);
  void OnEditOutcomeAttach(wxCommandEvent &);
  void OnEditOutcomeDetach(wxCommandEvent &);
  void OnEditOutcomePayoffs(wxCommandEvent &);
  void OnEditGame(wxCommandEvent &);

  void OnViewProfiles(wxCommandEvent &);
  void OnViewNavigation(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewDominance(wxCommandEvent &);
  void OnViewProbabilities(wxCommandEvent &);
  void OnViewValues(wxCommandEvent &);
  void OnViewOutcomeLabels(wxCommandEvent &);

  void OnFormatDisplayColumns(wxCommandEvent &);
  void OnFormatDisplayDecimals(wxCommandEvent &);
  void OnFormatFontData(wxCommandEvent &);
  void OnFormatFontLabels(wxCommandEvent &);
  void OnFormatColors(wxCommandEvent &);
  void OnFormatSave(wxCommandEvent &);
  void OnFormatLoad(wxCommandEvent &);

  void OnToolsSupportUndominated(wxCommandEvent &);
  void OnToolsSupportNew(wxCommandEvent &);
  void OnToolsSupportEdit(wxCommandEvent &);
  void OnToolsSupportDelete(wxCommandEvent &);
  void OnToolsSupportSelectFromList(wxCommandEvent &);
  void OnToolsSupportSelectPrevious(wxCommandEvent &);
  void OnToolsSupportSelectNext(wxCommandEvent &);

  void OnToolsEquilibriumStandard(wxCommandEvent &);
  void OnToolsEquilibriumCustomEnumPure(wxCommandEvent &);
  void OnToolsEquilibriumCustomEnumMixed(wxCommandEvent &);
  void OnToolsEquilibriumCustomLcp(wxCommandEvent &);
  void OnToolsEquilibriumCustomLiap(wxCommandEvent &);
  void OnToolsEquilibriumCustomLp(wxCommandEvent &);
  void OnToolsEquilibriumCustomPolEnum(wxCommandEvent &);
  void OnToolsEquilibriumCustomQre(wxCommandEvent &);
  void OnToolsEquilibriumCustomQreGrid(wxCommandEvent &);
  void OnToolsEquilibriumCustomSimpdiv(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesClone(wxCommandEvent &);
  void OnProfilesRename(wxCommandEvent &);
  void OnProfilesEdit(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);

  void OnInfoNotebookPage(wxNotebookEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnSize(wxSizeEvent &);
  void OnSashDrag(wxSashEvent &);
  void OnSetFocus(wxFocusEvent &);
  void OnSolutionSelected(wxListEvent &);

public:
  NfgShow(Nfg &N, wxWindow *p_window);
  virtual ~NfgShow();

  NFSupport *CurrentSupport(void) const { return m_currentSupport; }
  int NumSupports(void) const { return m_supports.Length(); }
  
  void OutcomePayoffs(int st1, int st2, bool next = false);
  void UpdateProfile(gArray<int> &profile);
  void SetStrategy(int p_player, int p_strategy);
  
  void ChangeSolution(int sol);

  int CurrentSolution(void) const { return m_currentSolution; }
  const gList<MixedSolution> &Solutions(void) const;

  void AddSolution(const MixedSolution &, bool);

  void SetFilename(const wxString &s);
  const wxString &Filename(void) const { return m_filename; }

  const Nfg &Game(void) const { return m_nfg; }  
  Nfg &Game(void) { return m_nfg; }

  bool GameIsDirty(void) const { return m_nfg.IsDirty(); }

  void SetPlayers(int, int);
  void SetProfile(const gArray<int> &);
  gArray<int> GetProfile(void) const;

  void OnOutcomesEdited(void);

  // Currently used support
  NFSupport *GetSupport(void) { return m_currentSupport; }
  const gList<NFSupport *> &Supports(void) const { return m_supports; }
  void SetSupportNumber(int p_number);
  void OnSupportsEdited(void);

  DECLARE_EVENT_TABLE()
};


#endif  // NFGSHOW_H



