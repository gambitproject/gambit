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

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/printdlg.h>

#include "base/gmisc.h"

#include "game/nfg.h"
#include "game/nfgiter.h"
#include "nash/mixedsol.h"

#include "gambit.h"

class gbtNfgTable;

class NfgShow : public wxFrame, public gbtGameView {
private:
  wxGrid *m_nav;
  gbtNfgTable *m_table;

  wxPageSetupData m_pageSetupData;
  wxPrintData m_printData;

  void MakeMenus(void);
  void MakeToolbar(void);

  // Menu event handlers
  void OnFileNew(wxCommandEvent &);
  void OnFileOpen(wxCommandEvent &);
  void OnFileSave(wxCommandEvent &);
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
  void OnViewOutcomes(wxCommandEvent &);
  void OnViewSupports(wxCommandEvent &);
  void OnViewDominance(wxCommandEvent &);
  void OnViewProbabilities(wxCommandEvent &);
  void OnViewValues(wxCommandEvent &);
  void OnViewOutcomeLabels(wxCommandEvent &);

  void OnFormatDisplayColors(wxCommandEvent &);
  void OnFormatDisplayDecimals(wxCommandEvent &);
  void OnFormatFontData(wxCommandEvent &);
  void OnFormatFontLabels(wxCommandEvent &);
  void OnFormatAutosize(wxCommandEvent &);

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

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnSetFocus(wxFocusEvent &);
  void OnSize(wxSizeEvent &);

  bool IsEfgView(void) const { return false; }
  bool IsNfgView(void) const { return true; }
  void OnUpdate(gbtGameView *);

public:
  // CONSTRUCTOR AND DESTRUCTOR
  NfgShow(gbtGameDocument *, wxWindow *p_window);
  virtual ~NfgShow();

  DECLARE_EVENT_TABLE()
};


#endif  // NFGSHOW_H



