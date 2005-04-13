//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of main frame for displaying game
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#ifndef GAME_FRAME_H
#define GAME_FRAME_H

#include <wx/printdlg.h>         // for wxPageSetupData, wxPrintData

#include "game-document.h"

class gbtTreeDisplay;

class gbtGameFrame : public wxFrame, gbtGameView {
private:
  wxPanel *m_treePanel, *m_matrixPanel, *m_schellingPanel;
  wxPanel *m_analysisPanel;
  gbtTreeDisplay *m_treeDisplay;

  // Printing state... should be part of document instead?
  wxPageSetupData m_pageSetupData;
  wxPrintData m_printData;


  // Auxiliary functions for setting up frame
  void MakeMenu(void);

  void OnCloseWindow(wxCloseEvent &);

  // Menu command handlers
  void OnFileNewEfg(wxCommandEvent &);
  void OnFileNewNfg(wxCommandEvent &);
  void OnFileOpen(wxCommandEvent &);
  void OnFileClose(wxCommandEvent &);
  void OnFileSave(wxCommandEvent &);
  void OnFileExportBMP(wxCommandEvent &);
  void OnFileExportJPG(wxCommandEvent &);
  void OnFileExportPNG(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &);
  void OnFileExit(wxCommandEvent &);
  void OnFileMRU(wxCommandEvent &);

  void OnEditUndo(wxCommandEvent &);
  void OnEditRedo(wxCommandEvent &);

  void OnViewEfg(wxCommandEvent &);
  void OnViewNfg(wxCommandEvent &);
  void OnViewZoomIn(wxCommandEvent &);
  void OnViewZoomOut(wxCommandEvent &);

  void OnFormatLayout(wxCommandEvent &);
  void OnFormatLabels(wxCommandEvent &);
  void OnFormatFont(wxCommandEvent &);

  void OnToolsAnalysis(wxCommandEvent &);

  void OnHelpAbout(wxCommandEvent &);

  // Implementation of gbtGameView members
  void OnUpdate(void);

public:
  gbtGameFrame(wxWindow *, gbtGameDocument *);
  virtual ~gbtGameFrame();

  DECLARE_EVENT_TABLE()
};

#endif   // GAME_FRAME_H
