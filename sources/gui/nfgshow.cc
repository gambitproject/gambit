//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of normal form display class
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/grid.h"
#include "wx/fontdlg.h"
#include "dlspinctrl.h"
#include "wxstatus.h"

#include "nfgshow.h"
#include "nfgtable.h"
#include "nfgprint.h"
#include "profile.h"
#include "nfgnavigate.h"
#include "outcomes.h"
#include "nfgsupport.h"
#include "id.h"

#include "game/game.h"
#include "game/efgutils.h"
#include "nash/behavsol.h"
#include "game/nfgciter.h"

#include "gambit.h"
#include "efgshow.h"
#include "dlefgcolor.h"
#include "dlnfgstrategies.h"
#include "dleditcont.h"
#include "dleditgame.h"
#include "dleditmixed.h"
#include "dlelimmixed.h"
#include "dlnfgnash.h"
#include "dlnfgqre.h"
#include "dlqrefile.h"
#include "dlch.h"
#include "dlreport.h"
#include "nash/nfglogit.h"
#include "nash/nfgqregrid.h"
#include "nash/nfgch.h"

//---------------------------------------------------------------------
//                   class gbtCmdAddNfgSupport
//---------------------------------------------------------------------

//
// Adds a support to the current support list
//
class gbtCmdAddNfgSupport : public gbtGameCommand {
private:
  gbtNfgSupport m_support;

public:
  gbtCmdAddNfgSupport(const gbtNfgSupport &p_support) 
    : m_support(p_support) { }
  virtual ~gbtCmdAddNfgSupport() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdAddNfgSupport::Do(gbtGameDocument *p_doc)
{
  p_doc->GetNfgSupportList().Append(m_support);
}

//---------------------------------------------------------------------
//                   class gbtCmdRemoveNfgSupport
//---------------------------------------------------------------------

//
// Removes the current support from the support list
//
class gbtCmdRemoveNfgSupport : public gbtGameCommand {
public:
  gbtCmdRemoveNfgSupport(void) { }
  virtual ~gbtCmdRemoveNfgSupport() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdRemoveNfgSupport::Do(gbtGameDocument *p_doc)
{
  p_doc->GetNfgSupportList().Remove();
}

//=====================================================================
//                 Implementation of class gbtNfgFrame
//=====================================================================

BEGIN_EVENT_TABLE(gbtNfgFrame, wxFrame)
  EVT_MENU(wxID_NEW, gbtNfgFrame::OnFileNew)
  EVT_MENU(wxID_OPEN, gbtNfgFrame::OnFileOpen)
  EVT_MENU(wxID_CLOSE, gbtNfgFrame::Close)
  EVT_MENU(wxID_SAVE, gbtNfgFrame::OnFileSave)
  EVT_MENU(wxID_SAVEAS, gbtNfgFrame::OnFileSave)
  EVT_MENU(GBT_MENU_FILE_EXPORT_HTML, gbtNfgFrame::OnFileExportHTML)
  EVT_MENU(wxID_PRINT_SETUP, gbtNfgFrame::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, gbtNfgFrame::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, gbtNfgFrame::OnFilePrint)
  EVT_MENU(wxID_EXIT, gbtNfgFrame::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, gbtNfgFrame::OnFileMRUFile)
  EVT_MENU(GBT_MENU_EDIT_STRATS, gbtNfgFrame::OnEditStrategies)
  EVT_MENU(GBT_MENU_EDIT_CONTINGENCY, gbtNfgFrame::OnEditContingency)
  EVT_MENU(GBT_MENU_EDIT_GAME, gbtNfgFrame::OnEditGame)
  EVT_MENU(GBT_MENU_VIEW_PROFILES, gbtNfgFrame::OnViewProfiles)
  EVT_MENU(GBT_MENU_VIEW_OUTCOMES, gbtNfgFrame::OnViewOutcomes)
  EVT_MENU(GBT_MENU_VIEW_SUPPORTS, gbtNfgFrame::OnViewSupports)
  EVT_MENU(GBT_MENU_VIEW_DOMINANCE, gbtNfgFrame::OnViewDominance)
  EVT_MENU(GBT_MENU_VIEW_PROBABILITIES, gbtNfgFrame::OnViewProbabilities)
  EVT_MENU(GBT_MENU_VIEW_VALUES, gbtNfgFrame::OnViewValues)
  EVT_MENU(GBT_MENU_VIEW_OUTCOME_LABELS, gbtNfgFrame::OnViewOutcomeLabels)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_COLORS, gbtNfgFrame::OnFormatDisplayColors)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_DECIMALS, 
	   gbtNfgFrame::OnFormatDisplayDecimals)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_DATA, gbtNfgFrame::OnFormatFontData)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_LABELS, gbtNfgFrame::OnFormatFontLabels)
  EVT_MENU(GBT_MENU_FORMAT_AUTOSIZE, gbtNfgFrame::OnFormatAutosize)
  EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, gbtNfgFrame::OnToolsDominance)
  EVT_MENU(GBT_MENU_TOOLS_EQUILIBRIUM, gbtNfgFrame::OnToolsEquilibrium)
  EVT_MENU(GBT_MENU_TOOLS_QRE, gbtNfgFrame::OnToolsQre)
  // EVT_MENU(GBT_MENU_TOOLS_CH, gbtNfgFrame::OnToolsCH)
  EVT_MENU(wxID_ABOUT, gbtNfgFrame::OnHelpAbout)
  EVT_MENU(GBT_MENU_SUPPORTS_DUPLICATE, gbtNfgFrame::OnSupportDuplicate)
  EVT_MENU(GBT_MENU_SUPPORTS_DELETE, gbtNfgFrame::OnSupportDelete)
  EVT_MENU(GBT_MENU_PROFILES_NEW, gbtNfgFrame::OnProfilesNew)
  EVT_MENU(GBT_MENU_PROFILES_DUPLICATE, gbtNfgFrame::OnProfilesDuplicate)
  EVT_MENU(GBT_MENU_PROFILES_DELETE, gbtNfgFrame::OnProfilesDelete)
  EVT_MENU(GBT_MENU_PROFILES_PROPERTIES, gbtNfgFrame::OnProfilesProperties)
  EVT_MENU(GBT_MENU_PROFILES_REPORT, gbtNfgFrame::OnProfilesReport)
  EVT_SIZE(gbtNfgFrame::OnSize)
  EVT_CLOSE(gbtNfgFrame::OnCloseWindow)
  EVT_SET_FOCUS(gbtNfgFrame::OnSetFocus)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//               gbtNfgFrame: Constructor and destructor
//----------------------------------------------------------------------

gbtNfgFrame::gbtNfgFrame(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxFrame(p_parent, -1, wxT(""), wxDefaultPosition, wxSize(800, 500)),
    gbtGameView(p_doc),
    m_table(0)
{
#ifdef __WXMSW__
  SetIcon(wxIcon("nfg_icn"));
#else
#include "bitmaps/nfg.xbm"
  SetIcon(wxIcon(wxString::Format(wxT("%s"), nfg_bits),
		 nfg_width, nfg_height));
#endif  // __WXMSW__

  CreateStatusBar();

  wxAcceleratorEntry entries[5];
  entries[0].Set(wxACCEL_CTRL, (int) 'N', wxID_NEW);
  entries[1].Set(wxACCEL_CTRL, (int) 'O', wxID_OPEN);
  entries[2].Set(wxACCEL_CTRL, (int) 'S', wxID_SAVE);
  entries[3].Set(wxACCEL_CTRL, (int) 'P', wxID_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
  wxAcceleratorTable accel(5, entries);
  SetAcceleratorTable(accel);

  MakeMenus();
  if (!m_doc->HasEfg()) {
    MakeToolbar();
    (void) new gbtProfileFrame(m_doc, this);
    (void) new gbtOutcomeFrame(m_doc, this);
  }

  (void) new gbtNfgSupportFrame(m_doc, this);

  m_table = new gbtNfgTable(m_doc, this);
  m_table->SetFocus();

  m_nav = new gbtNfgNavigate(m_doc, this);
  // Dummy in a size event to get layout correct
  wxSizeEvent foo;
  OnSize(foo);
  Show(true);

  m_doc->UpdateViews();
}

gbtNfgFrame::~gbtNfgFrame()
{
  wxGetApp().GetFileHistory().RemoveMenu(GetMenuBar()->GetMenu(0));
}

void gbtNfgFrame::OnUpdate(gbtGameView *)
{
  if (!m_doc->ShowNfg()) {
    Show(false);
    return;
  }

  if (m_doc->GetFilename() != wxT("")) {
    SetTitle(wxString::Format(wxT("Gambit - [%s] %s"), 
			      (const char *) m_doc->GetFilename().mb_str(), 
			      (char *) m_doc->GetGame()->GetLabel()));
  }
  else {
    SetTitle(wxString::Format(wxT("Gambit - %s"),
			      (char *) m_doc->GetGame()->GetLabel()));
  }

  wxMenuBar *menu = GetMenuBar();
  gbtArray<int> profile(m_doc->GetContingency());
  menu->Enable(GBT_MENU_FILE_EXPORT_COMLAB, 
	       m_doc->GetGame()->NumPlayers() == 2);
  menu->Check(GBT_MENU_VIEW_PROFILES, m_doc->ShowProfiles());
  menu->Check(GBT_MENU_VIEW_OUTCOMES, m_doc->ShowOutcomes());
  menu->Check(GBT_MENU_VIEW_SUPPORTS, m_doc->ShowNfgSupports());
  menu->Enable(GBT_MENU_VIEW_PROBABILITIES, m_doc->IsProfileSelected());
  menu->Enable(GBT_MENU_VIEW_VALUES, m_doc->IsProfileSelected());
  menu->Check(GBT_MENU_VIEW_OUTCOME_LABELS,
	      m_doc->GetPreferences().OutcomeLabel() == GBT_OUTCOME_LABEL_LABEL);
  Show(true);
}

//----------------------------------------------------------------------
//           gbtNfgFrame: Creating and updating menus and toolbar
//----------------------------------------------------------------------

void gbtNfgFrame::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  if (!m_doc->HasEfg()) {
    fileMenu->Append(wxID_NEW, _("&New\tCtrl-N"), _("Create a new game"));
    fileMenu->Append(wxID_OPEN, _("&Open\tCtrl-O"), _("Open a saved game"));
    fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_SAVE, _("&Save\tCtrl-S"), _("Save this game"));
    fileMenu->Append(wxID_SAVEAS, _("Save &as"), 
		     _("Save game to a different file"));
    fileMenu->AppendSeparator();
    wxMenu *fileExportMenu = new wxMenu;
    fileExportMenu->Append(GBT_MENU_FILE_EXPORT_HTML, _("&HTML"),
			   _("Save this game in HTML format"));
    fileMenu->Append(GBT_MENU_FILE_EXPORT, _("&Export"), fileExportMenu,
		     _("Export the game in various formats"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_PRINT_SETUP, _("Page Se&tup"),
		     _("Set up preferences for printing"));
    fileMenu->Append(wxID_PREVIEW, _("Print Pre&view"),
		     _("View a preview of the game printout"));
    fileMenu->Append(wxID_PRINT, _("&Print\tCtrl-P"), _("Print this game"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, _("E&xit\tCtrl-X"), _("Exit Gambit"));
  }
  else {
    fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_PRINT_SETUP, _("Page Se&tup"),
		     _("Set up preferences for printing"));
    fileMenu->Append(wxID_PREVIEW, _("Print Pre&view"),
		     _("View a preview of the game printout"));
    fileMenu->Append(wxID_PRINT, _("&Print\tCtrl-P"), _("Print this game"));
  }
  
  wxMenu *editMenu = new wxMenu;
  if (!m_doc->HasEfg()) {
    editMenu->Append(wxID_CUT, _("Cu&t"), _("Cut the current selection"));
    editMenu->Append(wxID_COPY, _("&Copy"), _("Copy the current selection"));
    editMenu->Append(wxID_PASTE, _("&Paste"), _("Paste from clipboard"));
    // For the moment, these are not implemented -- leave disabled
    editMenu->Enable(wxID_CUT, false);
    editMenu->Enable(wxID_COPY, false);
    editMenu->Enable(wxID_PASTE, false);
    editMenu->AppendSeparator();
    editMenu->Append(GBT_MENU_EDIT_STRATS, _("&Strategies"),
		     _("Edit strategy names"));
    editMenu->Append(GBT_MENU_EDIT_CONTINGENCY, _("&Contingency"),
		     _("Edit the selected contingency"));
  }
  editMenu->Append(GBT_MENU_EDIT_GAME, _("&Game"), _("Edit game properties"));

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_VIEW_PROFILES, _("&Profiles"),
		   _("Display/hide profiles window"), true);
  viewMenu->Check(GBT_MENU_VIEW_PROFILES, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_OUTCOMES, _("&Outcomes"), 
		   _("Display and edit outcomes"), true);
  viewMenu->Check(GBT_MENU_VIEW_OUTCOMES, false);
  viewMenu->Append(GBT_MENU_VIEW_SUPPORTS, _("&Supports"),
		   _("Display and edit supports"), true);
  viewMenu->Check(GBT_MENU_VIEW_SUPPORTS, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_DOMINANCE, _("&Dominance"),
		   _("Display dominance information"), true);
  viewMenu->Append(GBT_MENU_VIEW_PROBABILITIES, _("&Probabilities"),
		   _("Display solution probabilities"), true);
  viewMenu->Append(GBT_MENU_VIEW_VALUES, _("&Values"),
		   _("Display strategy values"), true);
  if (!m_doc->HasEfg()) {
    // "Outcomes" aren't implemented in reduced normal forms
    // This probably belongs in formatting instead
    viewMenu->AppendSeparator();
    viewMenu->Append(GBT_MENU_VIEW_OUTCOME_LABELS, _("Outcome &Labels"),
		     _("Display outcome labels"), true);
  }

  wxMenu *formatMenu = new wxMenu;
  wxMenu *formatDisplayMenu = new wxMenu;
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_COLORS,
			    _("&Colors"), _("Set colors"));
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_DECIMALS,
			    _("&Decimal Places"),
			    _("Set number of decimal places to display"));

  formatMenu->Append(GBT_MENU_FORMAT_DISPLAY, _("&Display"), formatDisplayMenu,
		     _("Configure display options"));
  wxMenu *formatFontsMenu = new wxMenu;
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_DATA, _("&Data"),
			  _("Set data font"));
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_LABELS, _("&Labels"), 
			  _("Set label font"));
  formatMenu->Append(GBT_MENU_FORMAT_FONTS, _("&Fonts"),
		     formatFontsMenu, _("Set fonts"));
  formatMenu->Append(GBT_MENU_FORMAT_AUTOSIZE, _("&Autosize"),
		     _("Automatically size grid rows and columns"));

  wxMenu *toolsMenu = new wxMenu;
  toolsMenu->Append(GBT_MENU_TOOLS_DOMINANCE, _("&Dominance"),
		    _("Find undominated strategies"));
  toolsMenu->Append(GBT_MENU_TOOLS_EQUILIBRIUM, _("&Equilibrium"),
		    _("Compute Nash equilibria (and refinements)"));
  toolsMenu->Append(GBT_MENU_TOOLS_QRE, _("&Qre"),
		    _("Compute quantal response equilibria"));
  toolsMenu->Append(GBT_MENU_TOOLS_CH, _("&CH"),
		    _("Compute cognitive hierarchy correspondence"));

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, _("&About"), _("About Gambit"));

  wxMenuBar *menuBar = new wxMenuBar(wxMB_DOCKABLE);
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(editMenu, _("&Edit"));
  menuBar->Append(viewMenu, _("&View"));
  menuBar->Append(formatMenu, _("&Format"));
  menuBar->Append(toolsMenu, _("&Tools"));
  menuBar->Append(helpMenu, _("&Help"));

  SetMenuBar(menuBar);

  wxGetApp().GetFileHistory().UseMenu(menuBar->GetMenu(0));
  wxGetApp().GetFileHistory().AddFilesToMenu(menuBar->GetMenu(0));
}

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/preview.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/help.xpm"

void gbtNfgFrame::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE |
				     wxTB_HORIZONTAL);
  toolBar->SetMargins(4, 4);

  toolBar->AddTool(wxID_NEW, wxBITMAP(new), wxNullBitmap, false,
		   -1, -1, 0, _("New game"), _("Create a new game"));
  toolBar->AddTool(wxID_OPEN, wxBITMAP(open), wxNullBitmap, false,
		   -1, -1, 0, _("Open file"), _("Open a saved game"));
  toolBar->AddTool(wxID_SAVE, wxBITMAP(save), wxNullBitmap, false,
		   -1, -1, 0, _("Save game"), _("Save this game"));
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_PREVIEW, wxBITMAP(preview), wxNullBitmap,
		   false, -1, -1, 0, _("Print Preview"),
		   _("View a preview of the game printout"));
  toolBar->AddTool(wxID_PRINT, wxBITMAP(print), wxNullBitmap, false,
		   -1, -1, 0, _("Print"), _("Print this game"));
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_ABOUT, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, _("Help"), _("Table of contents"));

  toolBar->Realize();
  toolBar->SetRows(1);
}

//----------------------------------------------------------------------
//               gbtNfgFrame: Menu handlers - File menu
//----------------------------------------------------------------------

void gbtNfgFrame::OnFileNew(wxCommandEvent &)
{
  wxGetApp().OnFileNew(this);
}

void gbtNfgFrame::OnFileOpen(wxCommandEvent &)
{
  wxGetApp().OnFileOpen(this);
}

void gbtNfgFrame::OnFileSave(wxCommandEvent &p_event)
{
  if (p_event.GetId() == wxID_SAVEAS || m_doc->GetFilename() == wxT("")) {
    wxFileDialog dialog(this, _("Choose file"),
			wxPathOnly(m_doc->GetFilename()),
			wxFileNameFromPath(m_doc->GetFilename()), wxT("*.nfg"),
			wxSAVE | wxOVERWRITE_PROMPT);

    switch (dialog.ShowModal()) {
    case wxID_OK:
      m_doc->SetFilename(dialog.GetPath());
      break;
    case wxID_CANCEL:
    default:
      return;
    }
  }

  try {
    gbtFileOutput file(m_doc->GetFilename().mb_str());
    gbtGame nfg = CompressNfg(m_doc->GetGame(),
			      m_doc->GetNfgSupportList().GetCurrent());
    nfg->WriteNfg(file);
    m_doc->SetIsModified(false);
  }
  catch (gbtFileOutput::OpenFailed &) {
    wxMessageBox(wxString::Format(_("Could not open %s for writing."),
				  (const char *) m_doc->GetFilename().mb_str()),
		 _("Error"), wxOK, this);
  }
  catch (gbtFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format(_("Write error occurred in saving %s."),
				  (const char *) m_doc->GetFilename().mb_str()),
		 _("Error"), wxOK, this);
  }
  catch (gbtException &) {
    wxMessageBox(_("Internal exception in Gambit"), _("Error"), wxOK, this);
  }
}

void gbtNfgFrame::OnFileExportHTML(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose output file"),
		      wxGetApp().CurrentDir(), wxT(""),
		      _("HTML files (*.html)|*.html|"
			"HTML files (*.htm)|*.htm"), wxSAVE);

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }
  
  try {
    gbtFileOutput file(dialog.GetPath().mb_str());
    file << gbtBuildHtml(m_doc->GetGame(),
			 m_doc->GetRowPlayer(),
			 m_doc->GetColPlayer()).mb_str() << '\n';
  }
  catch (gbtFileOutput::OpenFailed &) { 
    wxMessageBox(wxString::Format(_("Could not open %s for writing."),
				  (const char *) m_doc->GetFilename().mb_str()),
		 _("Error"), wxOK, this);
  }
  catch (gbtFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format(_("Write error occurred in saving %s."),
				  (const char *) m_doc->GetFilename().mb_str()),
		 _("Error"), wxOK, this);
  }
  catch (...) {
    wxMessageBox(_("An internal exeception occurred in Gambit"), _("Error"),
		 wxOK, this);
  }
}

void gbtNfgFrame::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupData();
  }
}

void gbtNfgFrame::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview = 
    new wxPrintPreview(new gbtNfgPrintout(m_doc->GetGame(),
				       m_doc->GetRowPlayer(), 
				       m_doc->GetColPlayer(),
				       wxString::Format(wxT("%s"), (char *) m_doc->GetGame()->GetLabel())),
		       new gbtNfgPrintout(m_doc->GetGame(),
				       m_doc->GetRowPlayer(),
				       m_doc->GetColPlayer(),
				       wxString::Format(wxT("%s"), (char *) m_doc->GetGame()->GetLabel())),
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

void gbtNfgFrame::OnFilePrint(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  gbtNfgPrintout printout(m_doc->GetGame(),
		       m_doc->GetRowPlayer(), m_doc->GetColPlayer(),
		       wxString::Format(wxT("%s"),
					(char *) m_doc->GetGame()->GetLabel()));

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

void gbtNfgFrame::OnFileExit(wxCommandEvent &)
{
  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void gbtNfgFrame::OnFileMRUFile(wxCommandEvent &p_event)
{
  wxGetApp().OnFileMRUFile(p_event);
}

//----------------------------------------------------------------------
//                gbtNfgFrame: Menu handlers - Edit menu
//----------------------------------------------------------------------

void gbtNfgFrame::OnEditStrategies(wxCommandEvent &)
{
  dialogStrategies dialog(this, m_doc->GetGame());

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->Submit(dialog.GetCommand());
  }
}

void gbtNfgFrame::OnEditContingency(wxCommandEvent &)
{
  dialogEditContingency dialog(this, m_doc->GetGame(), m_doc->GetContingency());

  if (dialog.ShowModal() == wxID_OK) {
    gbtNfgContingency profile(m_doc->GetGame());
    for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
      profile.SetStrategy(m_doc->GetGame()->GetPlayer(pl)->GetStrategy(m_doc->GetContingency()[pl]));
    }

    if (dialog.GetOutcome() == 0) { 
      profile.SetOutcome(0);
    }
    else {
      profile.SetOutcome(m_doc->GetGame()->GetOutcome(dialog.GetOutcome()));
    }
    m_doc->UpdateViews();
  }
}

void gbtNfgFrame::OnEditGame(wxCommandEvent &)
{
  gbtDialogEditGame dialog(this, m_doc);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->Submit(dialog.GetCommand());
  }
}


//----------------------------------------------------------------------
//                gbtNfgFrame: Menu handlers - View menu
//----------------------------------------------------------------------

void gbtNfgFrame::OnViewProfiles(wxCommandEvent &)
{
  m_doc->SetShowProfiles(!m_doc->ShowProfiles());
}

void gbtNfgFrame::OnViewOutcomes(wxCommandEvent &)
{
  m_doc->SetShowOutcomes(!m_doc->ShowOutcomes());
}

void gbtNfgFrame::OnViewSupports(wxCommandEvent &)
{
  m_doc->SetShowNfgSupports(!m_doc->ShowNfgSupports());
}


void gbtNfgFrame::OnViewDominance(wxCommandEvent &)
{
  m_table->ToggleDominance();
}

void gbtNfgFrame::OnViewProbabilities(wxCommandEvent &)
{
  m_table->ToggleProbs();
}

void gbtNfgFrame::OnViewValues(wxCommandEvent &)
{
  m_table->ToggleValues();
}

void gbtNfgFrame::OnViewOutcomeLabels(wxCommandEvent &)
{
  m_doc->GetPreferences().SetOutcomeLabel(1 - m_doc->GetPreferences().OutcomeLabel());
  m_doc->UpdateViews();
}

//----------------------------------------------------------------------
//               gbtNfgFrame: Menu handlers - Format menu
//----------------------------------------------------------------------

void gbtNfgFrame::OnFormatDisplayColors(wxCommandEvent &)
{
  dialogEfgColor dialog(this, m_doc->GetPreferences());

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetChanceColor(dialog.GetChanceColor());
    m_doc->GetPreferences().SetTerminalColor(dialog.GetTerminalColor());
    for (int pl = 1; pl <= 8; pl++) {
      m_doc->GetPreferences().SetPlayerColor(pl, dialog.GetPlayerColor(pl));
    }
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews(this);
  }
}

void gbtNfgFrame::OnFormatDisplayDecimals(wxCommandEvent &)
{
  dialogSpinCtrl dialog(this, _("Decimal places"), 0, 25,
			m_doc->GetPreferences().NumDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetNumDecimals(dialog.GetValue());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtNfgFrame::OnFormatFontData(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_doc->GetPreferences().GetDataFont());
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetDataFont(dialog.GetFontData().GetChosenFont());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtNfgFrame::OnFormatFontLabels(wxCommandEvent &)
{
  wxFontData data;
  data.SetInitialFont(m_doc->GetPreferences().GetLabelFont());
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetLabelFont(dialog.GetFontData().GetChosenFont());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void gbtNfgFrame::OnFormatAutosize(wxCommandEvent &)
{
  m_table->AutoSizeRows();
  m_table->AutoSizeColumns();
  // Set all strategy columns to be the same width, which is
  // the narrowest width which fits all the entries
  int max = 0, colPlayer = m_doc->GetColPlayer();
  const gbtNfgSupport &support = m_doc->GetNfgSupportList().GetCurrent();
  for (int col = 0; col < support.NumStrats(colPlayer); col++) {
    if (m_table->GetColSize(col) > max) {
      max = m_table->GetColSize(col);
    }
  }
  for (int col = 0; col < support.NumStrats(colPlayer); col++) {
    m_table->SetColSize(col, max);
  }
}

//----------------------------------------------------------------------
//                 gbtNfgFrame: Menu handlers - Tools
//----------------------------------------------------------------------

void gbtNfgFrame::OnToolsDominance(wxCommandEvent &)
{
  gbtArray<gbtText> playerNames(m_doc->GetGame()->NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++) {
    playerNames[pl] = m_doc->GetGame()->GetPlayer(pl)->GetLabel();
  }
  dialogElimMixed dialog(this, playerNames);

  if (dialog.ShowModal() == wxID_OK) {
    gbtNfgSupport support(m_doc->GetNfgSupportList().GetCurrent());
    gbtProgressDialog status(this, "Dominance Elimination");

    try {
      gbtNfgSupport newSupport(support);

      while (true) {
	gbtNullOutput gnull;
	if (dialog.DomMixed()) {
	  newSupport = support.MixedUndominated(dialog.DomStrong(),
						GBT_PREC_RATIONAL,
						dialog.Players(),
						gnull, status);
	}
	else {
	  newSupport = support.Undominated(dialog.DomStrong(), 
					   dialog.Players(),
					   gnull, status);
	}

	if (newSupport == support) {
	  break;
	}
	else {
	  newSupport.SetLabel(m_doc->GetNfgSupportList().GenerateUniqueLabel());
	  m_doc->Submit(new gbtCmdAddNfgSupport(newSupport));
	  support = newSupport;
	}

	if (!dialog.Iterative()) {
	  // Bit of a kludge: short-circuit loop if iterative not requested
	  break;
	}
      }
    }
    catch (gbtSignalBreak &) { }

    if (!m_table->ShowDominance()) {
      m_table->ToggleDominance();
      GetMenuBar()->Check(GBT_MENU_VIEW_DOMINANCE, true);
    }
  }
}

void gbtNfgFrame::OnToolsEquilibrium(wxCommandEvent &)
{ 
  dialogNfgNash dialog(this, m_doc->GetNfgSupportList().GetCurrent());

  if (dialog.ShowModal() == wxID_OK) {
    gbtNfgNashAlgorithm *algorithm = dialog.GetAlgorithm();

    if (!algorithm) {
      return;
    }

    try {
      gbtProgressDialog status(this,
			       algorithm->GetAlgorithm() + "Solve Progress");
      gbtList<MixedSolution> solutions;
      solutions = algorithm->Solve(m_doc->GetNfgSupportList().GetCurrent(),
				   status);

      for (int soln = 1; soln <= solutions.Length(); soln++) {
	m_doc->AddProfile(solutions[soln]);
      }
      m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
   
      if (solutions.Length() > 0 && !m_table->ShowProbs()) {
	m_table->ToggleProbs();
	GetMenuBar()->Check(GBT_MENU_VIEW_PROBABILITIES, true);
      }
      m_doc->SetShowProfiles(true);
    }
    catch (...) { }

    delete algorithm;
  }
}

void gbtNfgFrame::OnToolsQre(wxCommandEvent &)
{
  dialogNfgQre dialog(this, m_doc->GetNfgSupportList().GetCurrent());

  if (dialog.ShowModal() == wxID_OK) {
    gbtList<MixedSolution> solutions;

    try {
      if (dialog.UseGridSearch()) {
	QreNfgGrid algorithm;
	algorithm.SetFullGraph(true);
	algorithm.SetMinLambda(dialog.StartLambda());
	algorithm.SetMaxLambda(dialog.StopLambda());
	algorithm.SetDelLambda(dialog.StepLambda());
	algorithm.SetPowLambda(1);
	algorithm.SetDelP1(dialog.Del1());
	algorithm.SetDelP2(dialog.Del2());
	algorithm.SetTol1(dialog.Tol1());
	algorithm.SetTol2(dialog.Tol2());

	gbtProgressDialog status(this, "QreGridSolve Progress");
	gbtNullOutput gnull;
	algorithm.Solve(m_doc->GetNfgSupportList().GetCurrent(),
			gnull, status, solutions);
      }
      else {
	gbtNfgNashLogit algorithm;
	algorithm.SetFullGraph(true);
	algorithm.SetMaxLambda(10000000);

	gbtProgressDialog status(this, "QreSolve Progress");
	solutions = algorithm.Solve(m_doc->GetNfgSupportList().GetCurrent(),
				    status);
      }
    }
    catch (gbtSignalBreak &) { }
    catch (...) {
      wxMessageDialog message(this,
			      _("An exception occurred in computing equilibria"),
			      _("Error"), wxID_OK);
      message.ShowModal();
    }

    if (solutions.Length() > 0) {
      (void) new dialogQreFile(this, m_doc, solutions);
    }
  }
}

#ifdef UNUSED
// Commented out for now, until this gets implemented

void gbtNfgFrame::OnToolsCH(wxCommandEvent &)
{
  dialogNfgCH dialog(this, m_doc->GetNfgSupport());

  if (dialog.ShowModal() == wxID_OK) {
    gbtList<MixedSolution> solutions;

    try {
      gbtNfgBehavCH algorithm;
      algorithm.SetMinTau(dialog.MinTau());
      algorithm.SetMaxTau(dialog.MaxTau());
      algorithm.SetStepTau(dialog.StepTau());

      wxStatus status(this, "CHSolve Progress");
      gbtNullOutput gnull;
      solutions = algorithm.Solve(m_doc->GetNfgSupport(), status);
    }
    catch (gbtSignalBreak &) { }
    catch (...) {
      wxMessageDialog message(this,
			      "An exception occurred in computing equilibria",
			      "Error", wxID_OK);
      message.ShowModal();
    }

    if (solutions.Length() > 0) {
      dialogQreFile fileDialog(this, solutions);
      if (fileDialog.ShowModal() == wxID_OK) {
	
      }
    }
  }

}

#endif  // UNUSED


//----------------------------------------------------------------------
//                gbtNfgFrame: Menu handlers - Help menu
//----------------------------------------------------------------------

void gbtNfgFrame::OnHelpAbout(wxCommandEvent &)
{
  wxGetApp().OnHelpAbout(this);
}

//----------------------------------------------------------------------
//               gbtNfgFrame: Menu handlers - Support menu
//----------------------------------------------------------------------

void gbtNfgFrame::OnSupportDuplicate(wxCommandEvent &)
{
  gbtNfgSupport newSupport(m_doc->GetNfgSupportList().GetCurrent());
  newSupport.SetLabel(m_doc->GetNfgSupportList().GenerateUniqueLabel());
  m_doc->Submit(new gbtCmdAddNfgSupport(newSupport));
}

void gbtNfgFrame::OnSupportDelete(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdRemoveNfgSupport());
}

//----------------------------------------------------------------------
//              gbtNfgFrame: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void gbtNfgFrame::OnProfilesNew(wxCommandEvent &)
{
  MixedSolution profile = gbtMixedProfile<gbtNumber>(gbtNfgSupport(m_doc->GetGame()));

  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
  }
}

void gbtNfgFrame::OnProfilesDuplicate(wxCommandEvent &)
{
  MixedSolution profile(m_doc->GetMixedProfile());
  
  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
  }
}

void gbtNfgFrame::OnProfilesDelete(wxCommandEvent &)
{
  m_doc->RemoveProfile(m_doc->AllMixedProfiles().Find(m_doc->GetMixedProfile()));
}

void gbtNfgFrame::OnProfilesProperties(wxCommandEvent &)
{
  if (m_doc->IsProfileSelected()) {
    dialogEditMixed dialog(this, m_doc->GetMixedProfile());

    if (dialog.ShowModal() == wxID_OK) {
      m_doc->SetCurrentProfile(dialog.GetProfile());
    }
  }
}

void gbtNfgFrame::OnProfilesReport(wxCommandEvent &)
{
  // FIXME: Report should be generated by document class
  // dialogReport dialog(this, m_profileGrid->GetReport());
  // dialog.ShowModal();
}

//----------------------------------------------------------------------
//                  gbtNfgFrame: Non-menu event handlers
//----------------------------------------------------------------------

void gbtNfgFrame::OnCloseWindow(wxCloseEvent &p_event)
{
  if (m_doc->HasEfg()) {
    // Simply hide the window, don't actually destroy it
    m_doc->SetShowNfg(false);
    p_event.Veto();
    return;
  }

  if (p_event.CanVeto() && m_doc->IsModified()) {
    if (wxMessageBox(_("Game has been modified.  Close anyway?"), _("Warning"),
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }

  Show(false);
  Destroy();
}

void gbtNfgFrame::OnSetFocus(wxFocusEvent &)
{
  m_table->SetFocus();
}

void gbtNfgFrame::OnSize(wxSizeEvent &)
{
  wxSize size = GetClientSize();

  if (m_nav && m_table) {
    m_nav->SetSize(0, 0, 
		   m_nav->GetBestSize().GetWidth() + 2, size.GetHeight());
    m_table->SetSize(m_nav->GetSize().GetWidth() + 2,
		     0,
		     size.GetWidth() - m_nav->GetSize().GetWidth(),
		     size.GetHeight());
  }
}
