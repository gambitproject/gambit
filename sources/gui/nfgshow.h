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

#include "nfg.h"
#include "nfgiter.h"
#include "mixedsol.h"

#include "gambit.h"

const int idNFG_SOLUTION_LIST = 1900;

class NfgProfileList;
class NfgTable;
class dialogNfgSupportInspect;

class NfgShow : public wxFrame {
private:
  GambitFrame *m_parent;
  Nfg &m_nfg;

  NfgTable *m_table;
  NfgProfileList *m_solutionTable;

  wxSashWindow *m_solutionSashWindow;

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
  void OnFileSave(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &);

  void OnEditLabel(wxCommandEvent &);
  void OnEditPlayers(wxCommandEvent &);
  void OnEditStrategies(wxCommandEvent &);
  void OnEditOutcomeNew(wxCommandEvent &);
  void OnEditOutcomeDelete(wxCommandEvent &);
  void OnEditOutcomeAttach(wxCommandEvent &);
  void OnEditOutcomeDetach(wxCommandEvent &);
  void OnEditOutcomePayoffs(wxCommandEvent &);

  void OnSupportUndominated(wxCommandEvent &);
  void OnSupportNew(wxCommandEvent &);
  void OnSupportEdit(wxCommandEvent &);
  void OnSupportDelete(wxCommandEvent &);
  void OnSupportSelectFromList(wxCommandEvent &);
  void OnSupportSelectPrevious(wxCommandEvent &);
  void OnSupportSelectNext(wxCommandEvent &);

  void OnSolveStandard(wxCommandEvent &);
  void OnSolveCustom(wxCommandEvent &);
  void OnSolveCustomYamamoto(wxCommandEvent &);

  void OnViewSolutions(wxCommandEvent &);
  void OnViewDominance(wxCommandEvent &);
  void OnViewProbabilities(wxCommandEvent &);
  void OnViewValues(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewGameInfo(wxCommandEvent &);

  void OnPrefsDisplayColumns(wxCommandEvent &);
  void OnPrefsDisplayDecimals(wxCommandEvent &);
  void OnPrefsFontData(wxCommandEvent &);
  void OnPrefsFontLabels(wxCommandEvent &);
  void OnPrefsColors(wxCommandEvent &);
  void OnPrefsSave(wxCommandEvent &);
  void OnPrefsLoad(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesClone(wxCommandEvent &);
  void OnProfilesRename(wxCommandEvent &);
  void OnProfilesEdit(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnSize(wxSizeEvent &);
  void OnSashDrag(wxSashEvent &);
  void OnSetFocus(wxFocusEvent &);
  void OnSolutionSelected(wxListEvent &);
  void OnActivate(wxActivateEvent &);

public:
  NfgShow(Nfg &N, GambitFrame *p_frame);
  virtual ~NfgShow();

  NFSupport *CurrentSupport(void) const { return m_currentSupport; }
  int NumSupports(void) const { return m_supports.Length(); }
  
  void OutcomePayoffs(int st1, int st2, bool next = false);
  void UpdateProfile(gArray<int> &profile);
  void SetStrategy(int p_player, int p_strategy);
  
  void ChangeSolution(int sol);

  int CurrentSolution(void) const { return m_currentSolution; }
  const gList<MixedSolution> &Solutions(void) const;

  void SolutionToExtensive(const MixedSolution &mp, bool set = false);
  void AddSolution(const MixedSolution &, bool);

  void SetFilename(const wxString &s);
  const wxString &Filename(void) const { return m_filename; }

  const Nfg &Game(void) const { return m_nfg; }  

  bool GameIsDirty(void) const { return m_nfg.IsDirty(); }

  DECLARE_EVENT_TABLE()
};


#endif  // NFGSHOW_H



