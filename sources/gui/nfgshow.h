//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to class representing normal form viewing frame
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef NFGSHOW_H
#define NFGSHOW_H

#include "wx/wx.h"
#include "wx/listctrl.h"
#include "wx/sashwin.h"
#include "wx/printdlg.h"
#include "wx/notebook.h"

#include "base/gmisc.h"

#include "game/nfg.h"
#include "game/nfgiter.h"
#include "nash/mixedsol.h"

#include "gambit.h"
#include "nfgconst.h"

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
  NfgProfileList *m_profileTable;
  wxNotebook *m_infoNotebook;

  wxSashWindow *m_solutionSashWindow, *m_infoSashWindow;
  NfgNavigateWindow *m_navigateWindow;
  NfgOutcomeWindow *m_outcomeWindow;
  NfgSupportWindow *m_supportWindow;

  int m_currentProfile;
  gList<MixedSolution> m_profiles;

  gbtNfgSupport *m_currentSupport;
  gList<gbtNfgSupport *> m_supports;

  wxString m_filename;

  wxPageSetupData m_pageSetupData;
  wxPrintData m_printData;

  void MakeMenus(void);
  void MakeToolbar(void);
  void UpdateMenus(void);
  void AdjustSizes(void);

  // Menu event handlers
  void OnFileNew(wxCommandEvent &);
  void OnFileOpen(wxCommandEvent &);
  void OnFileSave(wxCommandEvent &);
  void OnFileImportComLab(wxCommandEvent &);
  void OnFileExportComLab(wxCommandEvent &);
  void OnFileExportHTML(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &);
  void OnFileExit(wxCommandEvent &);
  void OnFileMRUFile(wxCommandEvent &);

  void OnEditStrategies(wxCommandEvent &);
  void OnEditContingency(wxCommandEvent &);
  void OnEditGame(wxCommandEvent &);

  void OnViewProfiles(wxCommandEvent &);
  void OnViewNavigation(wxCommandEvent &);
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewDominance(wxCommandEvent &);
  void OnViewProbabilities(wxCommandEvent &);
  void OnViewValues(wxCommandEvent &);
  void OnViewOutcomeLabels(wxCommandEvent &);

  void OnFormatDisplayDecimals(wxCommandEvent &);
  void OnFormatFontData(wxCommandEvent &);
  void OnFormatFontLabels(wxCommandEvent &);

  void OnToolsDominance(wxCommandEvent &);
  void OnToolsEquilibrium(wxCommandEvent &);
  void OnToolsQre(wxCommandEvent &);
  void OnToolsCH(wxCommandEvent &);

  void OnHelpAbout(wxCommandEvent &);

  void OnSupportDuplicate(wxCommandEvent &);
  void OnSupportDelete(wxCommandEvent &);

  void OnProfilesNew(wxCommandEvent &);
  void OnProfilesDuplicate(wxCommandEvent &);
  void OnProfilesDelete(wxCommandEvent &);
  void OnProfilesProperties(wxCommandEvent &);
  void OnProfilesReport(wxCommandEvent &);

  void OnInfoNotebookPage(wxNotebookEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnSize(wxSizeEvent &);
  void OnSashDrag(wxSashEvent &);
  void OnSetFocus(wxFocusEvent &);
  void OnProfileSelected(wxListEvent &);

public:
  // CONSTRUCTOR AND DESTRUCTOR
  NfgShow(Nfg &N, wxWindow *p_window);
  virtual ~NfgShow();

  // PROFILE ACCESS AND MANIPULATION
  void AddProfile(const MixedSolution &, bool);
  void RemoveProfile(int);
  void ChangeProfile(int);
  int CurrentProfile(void) const { return m_currentProfile; }
  const gList<MixedSolution> &Profiles(void) const { return m_profiles; }
  gText UniqueProfileName(void) const;

  // SUPPORT ACCESS AND MANIPULATION
  gbtNfgSupport *GetSupport(void) { return m_currentSupport; }
  const gList<gbtNfgSupport *> &Supports(void) const { return m_supports; }
  void SetSupportNumber(int p_number);
  gText UniqueSupportName(void) const;
  void OnSupportsEdited(void);
  
  void UpdateProfile(gArray<int> &profile);
  void SetStrategy(int p_player, int p_strategy);
  
  void SetFilename(const wxString &s);
  const wxString &Filename(void) const { return m_filename; }

  const Nfg &Game(void) const { return m_nfg; }  
  Nfg &Game(void) { return m_nfg; }

  bool GameIsDirty(void) const { return m_nfg.IsDirty(); }

  void SetPlayers(int, int);
  void SetProfile(const gArray<int> &);
  gArray<int> GetContingency(void) const;

  void OnOutcomesEdited(void);
  gText UniqueOutcomeName(void) const;

  DECLARE_EVENT_TABLE()
};


#endif  // NFGSHOW_H



