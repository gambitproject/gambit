//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of main frame for displaying game
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

#include <fstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/choicebk.h>     // for use on analysis panel

#include "gambit.h"
#include "game-frame.h"

#include "dialog-tree-layout.h"  // for tree layout dialog
#include "dialog-tree-label.h"   // for tree label dialog
#include "dialog-about.h"        // for "About..." dialog

#include "tree-display.h"
#include "tree-print.h"
#include "table-schelling.h"
#include "table-matrix.h"
#include "panel-nash.h"
#include "panel-qre.h"

class gbtAnalysisPanel : public wxPanel {
private:
  wxChoicebook *m_notebook;
  wxPanel *m_algorithmPanel, *m_qrePanel;

public:
  gbtAnalysisPanel(wxWindow *p_parent, gbtGameDocument *p_doc);
};

gbtAnalysisPanel::gbtAnalysisPanel(wxWindow *p_parent,
				   gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1)
{
  m_notebook = new wxChoicebook(this, -1);
  
  m_algorithmPanel = new gbtNashPanel(m_notebook, p_doc);
  m_qrePanel = new gbtQrePanel(m_notebook, p_doc);

  m_notebook->AddPage(m_algorithmPanel, _("Compute Nash equilibria"));
  m_notebook->AddPage(m_qrePanel, _("Compute quantal response"));
  
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(m_notebook, 1, wxALL | wxEXPAND, 0);

  SetSizer(sizer);
  Layout();
}

//
// This panel dynamically displays either the Schelling-style payoff
// table (for two-player games), or the matrix-style payoff table
// (otherwise).  Also, no table is shown until OnNotebookShow() is
// called: this way, the normal form of a game is not computed unless
// the user asks for it.
//
class gbtStrategicPanel : public wxPanel, public gbtGameView {
private:
  wxPanel *m_matrixPanel, *m_schellingPanel;

public:
  gbtStrategicPanel(wxWindow *, gbtGameDocument *);

  void OnUpdate(void);
  void OnNotebookShow(void);
};

gbtStrategicPanel::gbtStrategicPanel(wxWindow *p_parent,
				     gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc),
    m_matrixPanel(0), m_schellingPanel(0)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  SetSizer(sizer);
  Layout();

  if (!m_doc->GetGame()->HasTree()) {
    OnNotebookShow();
  }
}

void gbtStrategicPanel::OnUpdate(void)
{
  if (!m_matrixPanel && !m_schellingPanel)  return;

  if (m_doc->GetGame()->NumPlayers() == 2) {
    if (!m_schellingPanel) {
      m_schellingPanel = new gbtTableSchelling(this, m_doc);
      GetSizer()->Add(m_schellingPanel, 1, wxEXPAND, 0);
    }

    if (m_matrixPanel) {
      GetSizer()->Show(m_matrixPanel, false);
    }

    GetSizer()->Show(m_schellingPanel, true);
    GetSizer()->Layout();
  }
  else {
    if (!m_matrixPanel) {
      m_matrixPanel = new gbtTableMatrix(this, m_doc);
      GetSizer()->Add(m_matrixPanel, 1, wxEXPAND, 0);
    }
    
    if (m_schellingPanel) {
      GetSizer()->Show(m_schellingPanel, false);
    }
    
    GetSizer()->Show(m_matrixPanel, true);
    GetSizer()->Layout();
  }
}

void gbtStrategicPanel::OnNotebookShow(void)
{
  if (m_matrixPanel || m_schellingPanel)  return;

  if (m_doc->GetGame()->NumPlayers() == 2) {
    m_schellingPanel = new gbtTableSchelling(this, m_doc);
    GetSizer()->Add(m_schellingPanel, 1, wxEXPAND, 0);
  }
  else {
    m_matrixPanel = new gbtTableMatrix(this, m_doc);
    GetSizer()->Add(m_matrixPanel, 1, wxEXPAND, 0);
  }

  GetSizer()->Layout();
}

enum {
  GBT_GAME_NOTEBOOK = 900,

  GBT_MENU_FILE_NEW_EFG = 968,
  GBT_MENU_FILE_NEW_NFG = 969,
  GBT_MENU_FILE_EXPORT = 970,
  GBT_MENU_FILE_EXPORT_BMP = 971,
  GBT_MENU_FILE_EXPORT_JPG = 972,
  GBT_MENU_FILE_EXPORT_PNG = 973,

  GBT_MENU_VIEW_ZOOMIN = 997,
  GBT_MENU_VIEW_ZOOMOUT = 996,

  GBT_MENU_FORMAT_LAYOUT = 960,
  GBT_MENU_FORMAT_LABELS = 961,
  GBT_MENU_FORMAT_FONT = 962,

  GBT_MENU_TOOLS_ANALYSIS = 1000
};

BEGIN_EVENT_TABLE(gbtGameFrame, wxFrame)
  EVT_MENU(GBT_MENU_FILE_NEW_EFG, gbtGameFrame::OnFileNewEfg)
  EVT_MENU(GBT_MENU_FILE_NEW_NFG, gbtGameFrame::OnFileNewNfg)
  EVT_MENU(wxID_OPEN, gbtGameFrame::OnFileOpen)
  EVT_MENU(wxID_CLOSE, gbtGameFrame::OnFileClose)
  EVT_MENU(wxID_SAVE, gbtGameFrame::OnFileSave)
  EVT_MENU(wxID_SAVEAS, gbtGameFrame::OnFileSaveAs)
  EVT_MENU(wxID_PRINT_SETUP, gbtGameFrame::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, gbtGameFrame::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, gbtGameFrame::OnFilePrint)
  EVT_MENU(GBT_MENU_FILE_EXPORT_BMP, gbtGameFrame::OnFileExportBMP)
  EVT_MENU(GBT_MENU_FILE_EXPORT_JPG, gbtGameFrame::OnFileExportJPG)
  EVT_MENU(GBT_MENU_FILE_EXPORT_PNG, gbtGameFrame::OnFileExportPNG)
  EVT_MENU(wxID_EXIT, gbtGameFrame::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, gbtGameFrame::OnFileMRU)
  EVT_MENU(wxID_UNDO, gbtGameFrame::OnEditUndo)
  EVT_MENU(wxID_REDO, gbtGameFrame::OnEditRedo)
  EVT_MENU(GBT_MENU_VIEW_ZOOMIN, gbtGameFrame::OnViewZoomIn)
  EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, gbtGameFrame::OnViewZoomOut)
  EVT_MENU(GBT_MENU_FORMAT_LAYOUT, gbtGameFrame::OnFormatLayout)
  EVT_MENU(GBT_MENU_FORMAT_LABELS, gbtGameFrame::OnFormatLabels)
  EVT_MENU(GBT_MENU_FORMAT_FONT, gbtGameFrame::OnFormatFont)
  EVT_MENU(GBT_MENU_TOOLS_ANALYSIS, gbtGameFrame::OnToolsAnalysis)
  EVT_MENU(wxID_ABOUT, gbtGameFrame::OnHelpAbout)
  EVT_NOTEBOOK_PAGE_CHANGING(GBT_GAME_NOTEBOOK,
  			     gbtGameFrame::OnPageChanging)
  EVT_CLOSE(gbtGameFrame::OnCloseWindow)
END_EVENT_TABLE()

//-------------------------------------------------------------------------
//                       gbtGameFrame: Lifecycle
//-------------------------------------------------------------------------

gbtGameFrame::gbtGameFrame(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxFrame(p_parent, -1, wxT(""),
	    wxDefaultPosition, wxSize(800, 600)), 
    gbtGameView(p_doc)
{
  wxGetApp().AddWindow(this);
  MakeMenu();

  m_notebook = new wxNotebook(this, GBT_GAME_NOTEBOOK, 
			      wxDefaultPosition, wxDefaultSize,
			      wxNB_BOTTOM);

  if (p_doc->GetGame()->HasTree()) {
    m_treePanel = new gbtTreePanel(m_notebook, p_doc);
    m_notebook->AddPage(m_treePanel, wxT("Extensive"));

    m_strategicPanel = new gbtStrategicPanel(m_notebook, p_doc);
    m_notebook->AddPage(m_strategicPanel, wxT("Strategic"));
  }
  else {
    m_strategicPanel = new gbtStrategicPanel(m_notebook, p_doc);
    m_notebook->AddPage(m_strategicPanel, wxT("Strategic"));
  }

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(m_notebook, 1, wxEXPAND, 0);

  m_analysisPanel = new gbtAnalysisPanel(this, p_doc);
  sizer->Add(m_analysisPanel, 1, wxEXPAND, 0);
  sizer->Show(m_analysisPanel, false);

  SetSizer(sizer);
  Layout();

  OnUpdate();
  Show(true);
}

gbtGameFrame::~gbtGameFrame()
{
  wxGetApp().GetFileHistory()->RemoveMenu(GetMenuBar()->GetMenu(0));
  wxGetApp().RemoveWindow(this);
}

//-------------------------------------------------------------------------
//          gbtGameFrame: Auxiliary functions for setting up frame
//-------------------------------------------------------------------------

void gbtGameFrame::MakeMenu(void)
{
  wxMenu *fileMenu = new wxMenu;

  wxMenu *fileNewMenu = new wxMenu;
  fileNewMenu->Append(GBT_MENU_FILE_NEW_EFG,
		      _("&Extensive"), 
		      _("Create a new game in extensive form"));
  fileNewMenu->Append(GBT_MENU_FILE_NEW_NFG,
		      _("&Normal"),
		      _("Create a new game in normal form"));
  fileMenu->Append(wxID_NEW, _("&New"), fileNewMenu,
		   _("Create a new game"));
  fileMenu->Append(wxID_OPEN, _("&Open"), _("Open a saved game"));
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_SAVE, _("&Save"), _("Save this game"));
  fileMenu->Append(wxID_SAVEAS, _("&Save as"), _("Save the game to a different file"));
  
  wxMenu *fileExportMenu = new wxMenu;
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_BMP, _("&BMP"),
			 _("Export game as a Windows bitmap graphic"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_JPG, _("&JPG"),
			 _("Export game as a JPEG graphic"));
  fileExportMenu->Append(GBT_MENU_FILE_EXPORT_PNG, _("&PNG"),
			 _("Export game as a PNG graphic"));
  fileMenu->Append(GBT_MENU_FILE_EXPORT, _("&Export"),
		   fileExportMenu, _("Export the game in a different format"));
  fileMenu->AppendSeparator();

  fileMenu->Append(wxID_PRINT_SETUP, _("Page Se&tup"),
		   _("Set up preferences for printing"));
  fileMenu->Append(wxID_PREVIEW, _("Print Pre&view"),
		   _("View a preview of the game printout"));
  fileMenu->Append(wxID_PRINT, _("&Print"), _("Print this game"));
  fileMenu->AppendSeparator();

  fileMenu->Append(wxID_EXIT, _("E&xit"), _("Exit Gambit"));

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(wxID_UNDO, _("&Undo"), _("Undo the last action"));
  editMenu->Append(wxID_REDO, _("&Redo"), _("Redo the last undone action"));

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_VIEW_ZOOMIN, _("Zoom &in"),
		   _("Increase magnification of tree"));
  viewMenu->Append(GBT_MENU_VIEW_ZOOMOUT, _("Zoom &out"),
		   _("Decrease magnification of tree"));

  wxMenu *formatMenu = new wxMenu;
  formatMenu->Append(GBT_MENU_FORMAT_LAYOUT, _("Tree la&yout"),
		     _("Settings for automatic layout of extensive forms"));
  formatMenu->Append(GBT_MENU_FORMAT_LABELS, _("Tree &labels"),
		     _("Displaying of labels on extensive form objects"));
  formatMenu->Append(GBT_MENU_FORMAT_FONT, _("&Font"),
		     _("Configuring display fonts for game objects"));

  wxMenu *toolsMenu = new wxMenu;	
  toolsMenu->Append(GBT_MENU_TOOLS_ANALYSIS, _("&Analysis"),
		    _("Show or hide panel to control equilibrium analysis"),
		    true);
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, _("&About"), _("About Gambit"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(editMenu, _("&Edit"));
  menuBar->Append(viewMenu, _("&View"));
  menuBar->Append(formatMenu, _("&Format"));
  menuBar->Append(toolsMenu, _("&Tools"));
  menuBar->Append(helpMenu, _("&Help"));

  wxGetApp().GetFileHistory()->UseMenu(fileMenu);
  wxGetApp().GetFileHistory()->AddFilesToMenu(fileMenu);

  SetMenuBar(menuBar);
}

void gbtGameFrame::OnCloseWindow(wxCloseEvent &p_event)
{
  if (p_event.CanVeto() && m_doc->IsModified()) {
    if (wxMessageBox(_("Game has been modified.  Close anyway?"),
		     _("Warning"),
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }
  p_event.Skip();
}

//-------------------------------------------------------------------------
//                 gbtGameFrame: Menu command handlers
//-------------------------------------------------------------------------

void gbtGameFrame::OnFileNewEfg(wxCommandEvent &)
{
  gbtGame efg = NewEfg();
  efg->SetLabel("Untitled extensive form game");
  efg->NewPlayer()->SetLabel("Player1");
  efg->NewPlayer()->SetLabel("Player2");
  (void) new gbtGameFrame(0, new gbtGameDocument(efg));
}

void gbtGameFrame::OnFileNewNfg(wxCommandEvent &)
{
  gbtArray<int> dim(2);  dim[1] = dim[2] = 2;
  gbtGame nfg = NewNfg(dim);
  nfg->SetLabel("Untitled normal form game");
  nfg->GetPlayer(1)->SetLabel("Player1");
  nfg->GetPlayer(1)->GetStrategy(1)->SetLabel("Strategy1");
  nfg->GetPlayer(1)->GetStrategy(2)->SetLabel("Strategy2");
  nfg->GetPlayer(2)->SetLabel("Player2");
  nfg->GetPlayer(2)->GetStrategy(1)->SetLabel("Strategy1");
  nfg->GetPlayer(2)->GetStrategy(2)->SetLabel("Strategy2");
  (void) new gbtGameFrame(0, new gbtGameDocument(nfg));
}


void gbtGameFrame::OnFileOpen(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), wxT(""), wxT(""), 
		      _("Gambit files (*.gbt)|*.gbt|"
			"Extensive form games (*.efg)|*.efg|"
			"Normal form games (*.nfg)|*.nfg|"
			"All files|*.*"));

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.GetPath().substr(dialog.GetPath().length() - 3, 3) == wxT("gbt")) {
      gbtGameDocument *doc = new gbtGameDocument(0);
      doc->Load(dialog.GetPath());
      doc->SetFilename(dialog.GetPath());
      wxGetApp().GetFileHistory()->AddFileToHistory(dialog.GetPath());
      (void) new gbtGameFrame(0, doc);
      return; 
    }

    std::ifstream file(dialog.GetPath().mb_str());

    if (!file.is_open()) {
      wxMessageBox(wxT("Could not open '") +
		   dialog.GetPath() + wxT("' for reading"),
		   _("Error"), wxOK, 0);
      return;
    }

    gbtGame game;

    try {
      game = ReadNfg(file);
      if (file.bad()) {
	wxMessageBox(wxT("An error occurred in reading '") +
		     dialog.GetPath() + wxT("'"),
		     _("Error"), wxOK, 0);
	return;
      }
      gbtGameDocument *doc = new gbtGameDocument(game);
      doc->SetFilename(dialog.GetPath());
      wxGetApp().GetFileHistory()->AddFileToHistory(dialog.GetPath());
      (void) new gbtGameFrame(0, doc);
      return;
    }
    catch (gbtNfgParserException &) {
      // Not a normal form file; eat exception and try extensive form
    }

    try {
      file.seekg(0);
      game = ReadEfg(file);
      game->Canonicalize();
      if (file.bad()) {
	wxMessageBox(wxT("An error occurred in reading '") +
		     dialog.GetPath() + wxT("'"),
		     _("Error"), wxOK, 0);
	return;
      }
    }
    catch (gbtEfgParserException &) {
      wxMessageBox(wxT("File '") + dialog.GetPath() + 
		   wxT("' is not in a recognized format"),
		   _("Error"), wxOK, 0);

      return;
    }

    gbtGameDocument *doc = new gbtGameDocument(game);
    doc->SetFilename(dialog.GetPath());
    wxGetApp().GetFileHistory()->AddFileToHistory(dialog.GetPath());
    (void) new gbtGameFrame(0, doc);
  }
}

void gbtGameFrame::OnFileMRU(wxCommandEvent &p_event)
{
  wxString filename(wxGetApp().GetFileHistory()->GetHistoryFile(p_event.GetId() - wxID_FILE1));

  if (filename.substr(filename.length() - 3, 3) == wxT("gbt")) {
    gbtGameDocument *doc = new gbtGameDocument(0);
    doc->Load(filename);
    doc->SetFilename(filename);
    wxGetApp().GetFileHistory()->AddFileToHistory(filename);
    (void) new gbtGameFrame(0, doc);
    return; 
  }

  std::ifstream file(filename.mb_str());

  if (!file.is_open()) {
    wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				  (const char *) filename.mb_str()),
		 _("Error"), wxOK, 0);
    return;
  }

  gbtGame game;

  try {
    game = ReadNfg(file);
    if (file.bad()) {
      wxMessageBox(wxString::Format(_("An error occured in reading '%s'"),
				    (const char *) filename.mb_str()),
		   _("Error"), wxOK, 0);
      return;
    }

    gbtGameDocument *doc = new gbtGameDocument(game);
    doc->SetFilename(filename);
    wxGetApp().GetFileHistory()->AddFileToHistory(filename);
    (void) new gbtGameFrame(0, doc);
    return;
  }
  catch (gbtNfgParserException &) {
    // Not a normal form file; eat exception and try extensive form
  }

  try {
    file.seekg(0);
    game = ReadEfg(file);
    game->Canonicalize();
    if (file.bad()) {
      wxMessageBox(wxString::Format(_("An error occured in reading '%s'"),
				    (const char *) filename.mb_str()),
		   _("Error"), wxOK, 0);
      return;
    }
  }
  catch (gbtEfgParserException &) {
    wxMessageBox(wxString::Format(_("File '%s' not in a recognized format"),
				  (const char *) filename.mb_str()),
		 _("Error"), wxOK, 0);
    
    return;
  }
  gbtGameDocument *doc = new gbtGameDocument(game);
  doc->SetFilename(filename);
  wxGetApp().GetFileHistory()->AddFileToHistory(filename);
  (void) new gbtGameFrame(0, doc);

}

void gbtGameFrame::OnFileClose(wxCommandEvent &)
{
  Close();
}

void gbtGameFrame::OnFileSave(wxCommandEvent &p_event)
{
  if (m_doc->GetFilename() == wxT("") ||
      m_doc->GetFilename().Right(4) == wxT(".efg") ||
      m_doc->GetFilename().Right(4) == wxT(".nfg")) {
    OnFileSaveAs(p_event);
    return;
  }

  m_doc->Save(m_doc->GetFilename());
  m_doc->SetModified(false);
}

void gbtGameFrame::OnFileSaveAs(wxCommandEvent &)
{
  wxString filter;
  if (m_doc->GetGame()->HasTree()) {
    filter = wxT("Gambit files (*.gbt)|*.gbt|"
		 "Extensive form files (*.efg)|*.efg|"
		 "Normal form files (*.nfg)}*.nfg");
  }
  else {
    filter = wxT("Gambit files (*.gbt)|*.gbt|"
		 "Normal form files (*.nfg)}*.nfg");
  }

  wxFileDialog dialog(this, _("Choose file"),
		      wxPathOnly(m_doc->GetFilename()),
		      wxFileNameFromPath(m_doc->GetFilename()), filter, 
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  try {
    if (dialog.GetPath().Right(4) == wxT(".efg")) {
      std::ofstream file((const char *) dialog.GetPath().mb_str());
      m_doc->GetGame()->WriteEfg(file);
    }
    else if (dialog.GetPath().Right(4) == wxT(".nfg")) {
      std::ofstream file((const char *) dialog.GetPath().mb_str());
      m_doc->GetGame()->WriteNfg(file);
    }
    else {
      m_doc->Save(dialog.GetPath());
      m_doc->SetModified(false);
    }
  }
  catch (...) {
    wxMessageBox(_("An error occurred in writing '") + 
		 dialog.GetPath() + wxT("'."),
		 _("Error"), wxOK, 0);

  }
}

void gbtGameFrame::OnFileExportBMP(wxCommandEvent &)
{
  if (m_treePanel->GetLayout().GetMaxX() > 65000 ||
      m_treePanel->GetLayout().GetMaxY() > 65000) {
    wxMessageDialog dialog(this,
			   _("Game image too large to export to graphics file"),
			   _("Error"), wxOK);
    dialog.ShowModal();
    return;
  }

  wxFileDialog dialog(this, _("Choose output file"), 
		      _T(""), _T(""),
		      _T("Windows bitmap files (*.bmp)|*.bmp"), 
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treePanel->GetLayout().GetMaxX(),
		    m_treePanel->GetLayout().GetMaxY());
    dc.SelectObject(bitmap);
    dc.Clear();
    m_treePanel->GetLayout().DrawTree(dc);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_BMP)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtGameFrame::OnFileExportJPG(wxCommandEvent &)
{
  if (m_treePanel->GetLayout().GetMaxX() > 65000 ||
      m_treePanel->GetLayout().GetMaxY() > 65000) {
    wxMessageDialog dialog(this,
			   _("Game image too large to export to graphics file"),
			   _("Error"), wxOK);
    dialog.ShowModal();
    return;
  }

  wxFileDialog dialog(this, _("Choose output file"), 
		      _T(""), _T(""),
		      _T("JPEG image files (*.jpg)|*.jpg|"
			 "JPEG image files (*.jpeg)|*.jpeg|"), 
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treePanel->GetLayout().GetMaxX(),
		    m_treePanel->GetLayout().GetMaxY());
    dc.SelectObject(bitmap);
    dc.Clear();
    m_treePanel->GetLayout().DrawTree(dc);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtGameFrame::OnFileExportPNG(wxCommandEvent &)
{
  if (m_treePanel->GetLayout().GetMaxX() > 65000 ||
      m_treePanel->GetLayout().GetMaxY() > 65000) {
    wxMessageDialog dialog(this,
			   _("Game image too large to export to graphics file"),
			   _("Error"), wxOK);
    dialog.ShowModal();
    return;
  }

  wxFileDialog dialog(this, _("Choose output file"), 
		      _T(""), _T(""),
		      _T("PNG image files (*.png)|*.png"), 
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    wxMemoryDC dc;
    wxBitmap bitmap(m_treePanel->GetLayout().GetMaxX(),
		    m_treePanel->GetLayout().GetMaxY());
    dc.SelectObject(bitmap);
    dc.Clear();
    m_treePanel->GetLayout().DrawTree(dc);
    if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_PNG)) {
      wxMessageBox(wxString::Format(_("An error occurred in writing '%s'."),
				    (const char *) dialog.GetPath().mb_str()),
		   _("Error"), wxOK, this);
    }
  }
}

void gbtGameFrame::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupData();
  }
}

void gbtGameFrame::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview = 
    new wxPrintPreview(new gbtTreePrintout(m_treePanel->GetLayout(),
					   wxString(m_doc->GetGame()->GetLabel().c_str(), *wxConvCurrent)),
		       new gbtTreePrintout(m_treePanel->GetLayout(),
					   wxString(m_doc->GetGame()->GetLabel().c_str(), *wxConvCurrent)),
		       &data);

  if (!preview->Ok()) {
    delete preview;
    return;
  }

  wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
					     _("Print Preview"),
					     wxPoint(100, 100),
					     wxSize(600, 650));
  frame->Initialize();
  frame->Show(true);
}

void gbtGameFrame::OnFilePrint(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  gbtTreePrintout printout(m_treePanel->GetLayout(),
			   wxString(m_doc->GetGame()->GetLabel().c_str(),
				    *wxConvCurrent));

  if (!printer.Print(this, &printout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      wxMessageBox(_("There was an error in printing"), _("Error"), wxOK);
    }
    // Otherwise, user hit "cancel"; just be quiet and return.
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}

void gbtGameFrame::OnFileExit(wxCommandEvent &)
{
  for (int i = 1; i <= wxGetApp().NumWindows(); i++) {
    if (wxGetApp().GetWindow(i)->GetDocument()->IsModified()) {
      if (wxMessageBox(_("There are modified games.  Exit anyway?"),
		       _("Warning"),
		       wxOK | wxCANCEL) == wxCANCEL) {
	return;
      }
      else {
	break;
      }
    }
  }

  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void gbtGameFrame::OnEditUndo(wxCommandEvent &)
{
  m_doc->Undo();
}

void gbtGameFrame::OnEditRedo(wxCommandEvent &)
{
  m_doc->Redo();
}

void gbtGameFrame::OnViewZoomIn(wxCommandEvent &)
{
  m_doc->SetTreeZoom(m_doc->GetTreeZoom() * 1.1);
}

void gbtGameFrame::OnViewZoomOut(wxCommandEvent &)
{
  m_doc->SetTreeZoom(m_doc->GetTreeZoom() / 1.1);
}

void gbtGameFrame::OnFormatLayout(wxCommandEvent &)
{
  gbtTreeLayoutDialog dialog(this, m_doc->GetTreeOptions());
  
  if (dialog.ShowModal() == wxID_OK) {
    gbtTreeLayoutOptions options = m_doc->GetTreeOptions();
    options.SetChanceToken(dialog.GetChanceToken());
    options.SetPlayerToken(dialog.GetPlayerToken());
    options.SetTerminalToken(dialog.GetTerminalToken());

    options.SetNodeSize(dialog.GetNodeSize());
    options.SetTerminalSpacing(dialog.GetTerminalSpacing());

    options.SetBranchStyle(dialog.GetBranchStyle());
    options.SetBranchLength(dialog.GetBranchLength());
    options.SetTineLength(dialog.GetTineLength());

    m_doc->SetTreeOptions(options);
  }
}

void gbtGameFrame::OnFormatLabels(wxCommandEvent &)
{
  gbtTreeLabelDialog dialog(this, m_doc->GetTreeOptions());

  if (dialog.ShowModal() == wxID_OK) {
    gbtTreeLayoutOptions options = m_doc->GetTreeOptions();
    options.SetNodeAboveLabel(dialog.GetNodeAboveLabel());
    options.SetNodeBelowLabel(dialog.GetNodeBelowLabel());
    options.SetOutcomeLabel(dialog.GetOutcomeLabel());
    options.SetBranchAboveLabel(dialog.GetBranchAboveLabel());
    options.SetBranchBelowLabel(dialog.GetBranchBelowLabel());
    m_doc->SetTreeOptions(options);
  }
}

void gbtGameFrame::OnFormatFont(wxCommandEvent &)
{

}


void gbtGameFrame::OnToolsAnalysis(wxCommandEvent &)
{
  GetSizer()->Show(m_analysisPanel,
		   GetMenuBar()->IsChecked(GBT_MENU_TOOLS_ANALYSIS));
  Layout();
}

void gbtGameFrame::OnHelpAbout(wxCommandEvent &)
{
  gbtAboutDialog(this).ShowModal();
}


void gbtGameFrame::OnPageChanging(wxNotebookEvent &p_event)
{
  if (p_event.GetSelection() == 1) {
    m_strategicPanel->OnNotebookShow();
    m_strategicPanel->Refresh();
  }
  p_event.Skip();
}

//-------------------------------------------------------------------------
//           gbtGameFrame: Implementation of view interface
//-------------------------------------------------------------------------

void gbtGameFrame::OnUpdate(void)
{
  if (m_doc->GetFilename() == wxT("")) {
    wxString title = wxT("Gambit: [<no file>");
    if (m_doc->IsModified()) {
      title += wxT("*");
    }
    title += wxT("] ");
    title += wxString(m_doc->GetGame()->GetLabel().c_str(), *wxConvCurrent);
    SetTitle(title);
  }
  else {
    wxString title = wxT("Gambit: [");
    title += m_doc->GetFilename();
    if (m_doc->IsModified()) {
      title += wxT("*");
    }
    title += wxT("] ");
    title += wxString(m_doc->GetGame()->GetLabel().c_str(), *wxConvCurrent);
    SetTitle(title);
  }

  GetMenuBar()->Enable(GBT_MENU_FILE_EXPORT,
		       m_treePanel && GetSizer()->IsShown(m_treePanel));
  GetMenuBar()->Enable(wxID_PRINT_SETUP,
		       m_treePanel && GetSizer()->IsShown(m_treePanel));
  GetMenuBar()->Enable(wxID_PREVIEW,
		       m_treePanel && GetSizer()->IsShown(m_treePanel));
  GetMenuBar()->Enable(wxID_PRINT,
		       m_treePanel && GetSizer()->IsShown(m_treePanel));

  GetMenuBar()->Enable(wxID_UNDO, m_doc->CanUndo());
  if (m_doc->CanUndo()) {
    GetMenuBar()->SetLabel(wxID_UNDO, 
			   _("Undo ") + m_doc->GetUndoDescription());
  }
  else {
    GetMenuBar()->SetLabel(wxID_UNDO, _("Undo"));
  }

  GetMenuBar()->Enable(wxID_REDO, m_doc->CanRedo());
  if (m_doc->CanRedo()) {
    GetMenuBar()->SetLabel(wxID_REDO, 
			   _("Redo ") + m_doc->GetRedoDescription());
  }
  else {
    GetMenuBar()->SetLabel(wxID_REDO, _("Redo"));
  }
  
  
  GetMenuBar()->Enable(GBT_MENU_VIEW_ZOOMIN,
		       m_doc->GetGame()->HasTree() && 
		       m_notebook->GetSelection() == 0);
  GetMenuBar()->Enable(GBT_MENU_VIEW_ZOOMOUT,
		       m_doc->GetGame()->HasTree() &&
		       m_notebook->GetSelection() == 0);
}
