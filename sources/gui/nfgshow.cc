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

#include "game/efg.h"
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

//=====================================================================
//                 Implementation of class NfgShow
//=====================================================================

BEGIN_EVENT_TABLE(NfgShow, wxFrame)
  EVT_MENU(wxID_NEW, NfgShow::OnFileNew)
  EVT_MENU(wxID_OPEN, NfgShow::OnFileOpen)
  EVT_MENU(wxID_CLOSE, NfgShow::Close)
  EVT_MENU(wxID_SAVE, NfgShow::OnFileSave)
  EVT_MENU(wxID_SAVEAS, NfgShow::OnFileSave)
  EVT_MENU(GBT_MENU_FILE_EXPORT_HTML, NfgShow::OnFileExportHTML)
  EVT_MENU(wxID_PRINT_SETUP, NfgShow::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, NfgShow::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, NfgShow::OnFilePrint)
  EVT_MENU(wxID_EXIT, NfgShow::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, NfgShow::OnFileMRUFile)
  EVT_MENU(GBT_MENU_EDIT_STRATS, NfgShow::OnEditStrategies)
  EVT_MENU(GBT_MENU_EDIT_CONTINGENCY, NfgShow::OnEditContingency)
  EVT_MENU(GBT_MENU_EDIT_GAME, NfgShow::OnEditGame)
  EVT_MENU(GBT_MENU_VIEW_PROFILES, NfgShow::OnViewProfiles)
  EVT_MENU(GBT_MENU_VIEW_OUTCOMES, NfgShow::OnViewOutcomes)
  EVT_MENU(GBT_MENU_VIEW_SUPPORTS, NfgShow::OnViewSupports)
  EVT_MENU(GBT_MENU_VIEW_DOMINANCE, NfgShow::OnViewDominance)
  EVT_MENU(GBT_MENU_VIEW_PROBABILITIES, NfgShow::OnViewProbabilities)
  EVT_MENU(GBT_MENU_VIEW_VALUES, NfgShow::OnViewValues)
  EVT_MENU(GBT_MENU_VIEW_OUTCOME_LABELS, NfgShow::OnViewOutcomeLabels)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_COLORS, NfgShow::OnFormatDisplayColors)
  EVT_MENU(GBT_MENU_FORMAT_DISPLAY_DECIMALS, 
	   NfgShow::OnFormatDisplayDecimals)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_DATA, NfgShow::OnFormatFontData)
  EVT_MENU(GBT_MENU_FORMAT_FONTS_LABELS, NfgShow::OnFormatFontLabels)
  EVT_MENU(GBT_MENU_FORMAT_AUTOSIZE, NfgShow::OnFormatAutosize)
  EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, NfgShow::OnToolsDominance)
  EVT_MENU(GBT_MENU_TOOLS_EQUILIBRIUM, NfgShow::OnToolsEquilibrium)
  EVT_MENU(GBT_MENU_TOOLS_QRE, NfgShow::OnToolsQre)
  // EVT_MENU(GBT_MENU_TOOLS_CH, NfgShow::OnToolsCH)
  EVT_MENU(wxID_ABOUT, NfgShow::OnHelpAbout)
  EVT_MENU(GBT_MENU_SUPPORTS_DUPLICATE, NfgShow::OnSupportDuplicate)
  EVT_MENU(GBT_MENU_SUPPORTS_DELETE, NfgShow::OnSupportDelete)
  EVT_MENU(GBT_MENU_PROFILES_NEW, NfgShow::OnProfilesNew)
  EVT_MENU(GBT_MENU_PROFILES_DUPLICATE, NfgShow::OnProfilesDuplicate)
  EVT_MENU(GBT_MENU_PROFILES_DELETE, NfgShow::OnProfilesDelete)
  EVT_MENU(GBT_MENU_PROFILES_PROPERTIES, NfgShow::OnProfilesProperties)
  EVT_MENU(GBT_MENU_PROFILES_REPORT, NfgShow::OnProfilesReport)
  EVT_SIZE(NfgShow::OnSize)
  EVT_CLOSE(NfgShow::OnCloseWindow)
  EVT_SET_FOCUS(NfgShow::OnSetFocus)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//               NfgShow: Constructor and destructor
//----------------------------------------------------------------------

NfgShow::NfgShow(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(800, 500)),
    gbtGameView(p_doc),
    m_table(0)
{
#ifdef __WXMSW__
  SetIcon(wxIcon("nfg_icn"));
#else
#include "bitmaps/nfg.xbm"
  SetIcon(wxIcon(nfg_bits, nfg_width, nfg_height));
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

NfgShow::~NfgShow()
{
  wxGetApp().GetFileHistory().RemoveMenu(GetMenuBar()->GetMenu(0));
}

void NfgShow::OnUpdate(gbtGameView *)
{
  if (!m_doc->ShowNfg()) {
    Show(false);
    return;
  }

  if (m_doc->GetFilename() != "") {
    SetTitle(wxString::Format("Gambit - [%s] %s", 
			      m_doc->GetFilename().c_str(), 
			      (char *) m_doc->GetNfg().GetLabel()));
  }
  else {
    SetTitle(wxString::Format("Gambit - %s",
			      (char *) m_doc->GetNfg().GetLabel()));
  }

  wxMenuBar *menu = GetMenuBar();
  gArray<int> profile(m_doc->GetContingency());
  menu->Enable(GBT_MENU_FILE_EXPORT_COMLAB, 
	       m_doc->GetNfg().NumPlayers() == 2);
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
//           NfgShow: Creating and updating menus and toolbar
//----------------------------------------------------------------------

void NfgShow::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  if (!m_doc->HasEfg()) {
    fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new game");
    fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a saved game");
    fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save this game");
    fileMenu->Append(wxID_SAVEAS, "Save &as", "Save game to a different file");
    fileMenu->AppendSeparator();
    wxMenu *fileExportMenu = new wxMenu;
    fileExportMenu->Append(GBT_MENU_FILE_EXPORT_HTML, "&HTML",
			   "Save this game in HTML format");
    fileMenu->Append(GBT_MENU_FILE_EXPORT, "&Export", fileExportMenu,
		     "Export the game in various formats");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_PRINT_SETUP, "Page Se&tup",
		     "Set up preferences for printing");
    fileMenu->Append(wxID_PREVIEW, "Print Pre&view",
		     "View a preview of the game printout");
    fileMenu->Append(wxID_PRINT, "&Print\tCtrl-P", "Print this game");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tCtrl-X", "Exit Gambit");
  }
  else {
    fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_PRINT_SETUP, "Page Se&tup",
		     "Set up preferences for printing");
    fileMenu->Append(wxID_PREVIEW, "Print Pre&view",
		     "View a preview of the game printout");
    fileMenu->Append(wxID_PRINT, "&Print\tCtrl-P", "Print this game");
  }
  
  wxMenu *editMenu = new wxMenu;
  if (!m_doc->HasEfg()) {
    editMenu->Append(wxID_CUT, "Cu&t", "Cut the current selection");
    editMenu->Append(wxID_COPY, "&Copy", "Copy the current selection");
    editMenu->Append(wxID_PASTE, "&Paste", "Paste from clipboard");
    // For the moment, these are not implemented -- leave disabled
    editMenu->Enable(wxID_CUT, false);
    editMenu->Enable(wxID_COPY, false);
    editMenu->Enable(wxID_PASTE, false);
    editMenu->AppendSeparator();
    editMenu->Append(GBT_MENU_EDIT_STRATS, "&Strategies",
		     "Edit strategy names");
    editMenu->Append(GBT_MENU_EDIT_CONTINGENCY, "&Contingency",
		     "Edit the selected contingency");
  }
  editMenu->Append(GBT_MENU_EDIT_GAME, "&Game", "Edit game properties");

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_MENU_VIEW_PROFILES, "&Profiles",
		   "Display/hide profiles window", true);
  viewMenu->Check(GBT_MENU_VIEW_PROFILES, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_OUTCOMES, "&Outcomes", 
		   "Display and edit outcomes", true);
  viewMenu->Check(GBT_MENU_VIEW_OUTCOMES, false);
  viewMenu->Append(GBT_MENU_VIEW_SUPPORTS, "&Supports",
		   "Display and edit supports", true);
  viewMenu->Check(GBT_MENU_VIEW_SUPPORTS, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_MENU_VIEW_DOMINANCE, "&Dominance",
		   "Display dominance information", TRUE);
  viewMenu->Append(GBT_MENU_VIEW_PROBABILITIES, "&Probabilities",
		   "Display solution probabilities", TRUE);
  viewMenu->Append(GBT_MENU_VIEW_VALUES, "&Values",
		   "Display strategy values", TRUE);
  if (!m_doc->HasEfg()) {
    // "Outcomes" aren't implemented in reduced normal forms
    // This probably belongs in formatting instead
    viewMenu->AppendSeparator();
    viewMenu->Append(GBT_MENU_VIEW_OUTCOME_LABELS, "Outcome &Labels",
		     "Display outcome labels", TRUE);
  }

  wxMenu *formatMenu = new wxMenu;
  wxMenu *formatDisplayMenu = new wxMenu;
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_COLORS,
			    "&Colors", "Set colors");
  formatDisplayMenu->Append(GBT_MENU_FORMAT_DISPLAY_DECIMALS,
			    "&Decimal Places",
			   "Set number of decimal places to display");

  formatMenu->Append(GBT_MENU_FORMAT_DISPLAY, "&Display", formatDisplayMenu,
		    "Configure display options");
  wxMenu *formatFontsMenu = new wxMenu;
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_DATA, "&Data",
			  "Set data font");
  formatFontsMenu->Append(GBT_MENU_FORMAT_FONTS_LABELS, "&Labels", 
			  "Set label font");
  formatMenu->Append(GBT_MENU_FORMAT_FONTS, "&Fonts",
		     formatFontsMenu, "Set fonts");
  formatMenu->Append(GBT_MENU_FORMAT_AUTOSIZE, "&Autosize",
		     "Automatically size grid rows and columns");

  wxMenu *toolsMenu = new wxMenu;
  toolsMenu->Append(GBT_MENU_TOOLS_DOMINANCE, "&Dominance",
		       "Find undominated strategies");
  toolsMenu->Append(GBT_MENU_TOOLS_EQUILIBRIUM, "&Equilibrium",
		    "Compute Nash equilibria (and refinements)");
  toolsMenu->Append(GBT_MENU_TOOLS_QRE, "&Qre",
		    "Compute quantal response equilibria");
  toolsMenu->Append(GBT_MENU_TOOLS_CH, "&CH",
		    "Compute cognitive hierarchy correspondence");

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, "&About", "About Gambit");

  wxMenuBar *menuBar = new wxMenuBar(wxMB_DOCKABLE);
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(formatMenu, "&Format");
  menuBar->Append(toolsMenu, "&Tools");
  menuBar->Append(helpMenu, "&Help");

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

void NfgShow::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE |
				     wxTB_HORIZONTAL);
  toolBar->SetMargins(4, 4);

  toolBar->AddTool(wxID_NEW, wxBITMAP(new), wxNullBitmap, false,
		   -1, -1, 0, "New game", "Create a new game");
  toolBar->AddTool(wxID_OPEN, wxBITMAP(open), wxNullBitmap, false,
		   -1, -1, 0, "Open file", "Open a saved game");
  toolBar->AddTool(wxID_SAVE, wxBITMAP(save), wxNullBitmap, false,
		   -1, -1, 0, "Save game", "Save this game");
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_PREVIEW, wxBITMAP(preview), wxNullBitmap,
		   false, -1, -1, 0, "Print Preview",
		   "View a preview of the game printout");
  toolBar->AddTool(wxID_PRINT, wxBITMAP(print), wxNullBitmap, false,
		   -1, -1, 0, "Print", "Print this game");
  toolBar->AddSeparator();

  toolBar->AddTool(wxID_ABOUT, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, "Help", "Table of contents");

  toolBar->Realize();
  toolBar->SetRows(1);
}

//----------------------------------------------------------------------
//               NfgShow: Menu handlers - File menu
//----------------------------------------------------------------------

void NfgShow::OnFileNew(wxCommandEvent &)
{
  wxGetApp().OnFileNew(this);
}

void NfgShow::OnFileOpen(wxCommandEvent &)
{
  wxGetApp().OnFileOpen(this);
}

void NfgShow::OnFileSave(wxCommandEvent &p_event)
{
  if (p_event.GetId() == wxID_SAVEAS || m_doc->GetFilename() == "") {
    wxFileDialog dialog(this, "Choose file",
			wxPathOnly(m_doc->GetFilename()),
			wxFileNameFromPath(m_doc->GetFilename()), "*.nfg",
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
    gFileOutput file(m_doc->GetFilename().c_str());
    gbtNfgGame nfg = CompressNfg(m_doc->GetNfg(), m_doc->GetNfgSupport());
    nfg.WriteNfg(file);
    m_doc->SetIsModified(false);
  }
  catch (gFileOutput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open %s for writing.",
				  m_doc->GetFilename().c_str()),
		 "Error", wxOK, this);
  }
  catch (gFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				  m_doc->GetFilename().c_str()),
		 "Error", wxOK, this);
  }
  catch (gException &) {
    wxMessageBox("Internal exception in Gambit", "Error", wxOK, this);
  }
}

void NfgShow::OnFileExportHTML(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose output file", wxGetApp().CurrentDir(), "",
		      "HTML files (*.html)|*.html|"
		      "HTML files (*.htm)|*.htm", wxSAVE);

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }
  
  try {
    gFileOutput file(dialog.GetPath().c_str());
    file << gbtBuildHtml(m_doc->GetNfg(),
			 m_doc->GetRowPlayer(),
			 m_doc->GetColPlayer()).c_str() << '\n';
  }
  catch (gFileOutput::OpenFailed &) { 
    wxMessageBox(wxString::Format("Could not open %s for writing.",
				  m_doc->GetFilename().c_str()),
		 "Error", wxOK, this);
  }
  catch (gFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				  m_doc->GetFilename().c_str()),
		 "Error", wxOK, this);
  }
  catch (...) {
    wxMessageBox("An internal exeception occurred in Gambit", "Error",
		 wxOK, this);
  }
}

void NfgShow::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupData();
  }
}

void NfgShow::OnFilePrintPreview(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview = 
    new wxPrintPreview(new NfgPrintout(m_doc->GetNfg(),
				       m_doc->GetRowPlayer(), 
				       m_doc->GetColPlayer(),
				       (char *) m_doc->GetNfg().GetLabel()),
		       new NfgPrintout(m_doc->GetNfg(),
				       m_doc->GetRowPlayer(),
				       m_doc->GetColPlayer(),
				       (char *) m_doc->GetNfg().GetLabel()),
		       &data);

  if (!preview->Ok()) {
    delete preview;
    return;
  }

  wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
					     "Print Preview",
					     wxPoint(100, 100),
					     wxSize(600, 650));
  frame->Initialize();
  frame->Show(true);
}

void NfgShow::OnFilePrint(wxCommandEvent &)
{
  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  NfgPrintout printout(m_doc->GetNfg(),
		       m_doc->GetRowPlayer(), m_doc->GetColPlayer(),
		       (char *) m_doc->GetNfg().GetLabel());

  if (!printer.Print(this, &printout, true)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      wxMessageBox("There was an error in printing", "Error", wxOK);
    }
    // Otherwise, user hit "cancel"; just be quiet and return.
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}

void NfgShow::OnFileExit(wxCommandEvent &)
{
  while (wxGetApp().GetTopWindow()) {
    delete wxGetApp().GetTopWindow();
  }
}

void NfgShow::OnFileMRUFile(wxCommandEvent &p_event)
{
  wxGetApp().OnFileMRUFile(p_event);
}

//----------------------------------------------------------------------
//                NfgShow: Menu handlers - Edit menu
//----------------------------------------------------------------------

void NfgShow::OnEditStrategies(wxCommandEvent &)
{
  dialogStrategies dialog(this, m_doc->GetNfg());

  if (dialog.ShowModal() == wxID_OK) {
    for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
      gbtNfgPlayer player = m_doc->GetNfg().GetPlayer(pl);
      for (int st = 1; st <= player.NumStrategies(); st++) {
	player.GetStrategy(st).SetLabel(dialog.GetStrategyName(pl, st));
      }
    }
    m_doc->UpdateViews();
  }
}

void NfgShow::OnEditContingency(wxCommandEvent &)
{
  dialogEditContingency dialog(this, m_doc->GetNfg(), m_doc->GetContingency());

  if (dialog.ShowModal() == wxID_OK) {
    StrategyProfile profile(m_doc->GetNfg());
    for (int pl = 1; pl <= m_doc->GetNfg().NumPlayers(); pl++) {
      profile.Set(pl, m_doc->GetNfg().GetPlayer(pl).GetStrategy(m_doc->GetContingency()[pl]));
    }

    if (dialog.GetOutcome() == 0) { 
      profile.SetOutcome(0);
    }
    else {
      profile.SetOutcome(m_doc->GetNfg().GetOutcome(dialog.GetOutcome()));
    }
    m_doc->UpdateViews();
  }
}

void NfgShow::OnEditGame(wxCommandEvent &)
{
  gbtDialogEditGame dialog(this, m_doc);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->Submit(dialog.GetCommand());
  }
}


//----------------------------------------------------------------------
//                NfgShow: Menu handlers - View menu
//----------------------------------------------------------------------

void NfgShow::OnViewProfiles(wxCommandEvent &)
{
  m_doc->SetShowProfiles(!m_doc->ShowProfiles());
}

void NfgShow::OnViewOutcomes(wxCommandEvent &)
{
  m_doc->SetShowOutcomes(!m_doc->ShowOutcomes());
}

void NfgShow::OnViewSupports(wxCommandEvent &)
{
  m_doc->SetShowNfgSupports(!m_doc->ShowNfgSupports());
}


void NfgShow::OnViewDominance(wxCommandEvent &)
{
  m_table->ToggleDominance();
}

void NfgShow::OnViewProbabilities(wxCommandEvent &)
{
  m_table->ToggleProbs();
}

void NfgShow::OnViewValues(wxCommandEvent &)
{
  m_table->ToggleValues();
}

void NfgShow::OnViewOutcomeLabels(wxCommandEvent &)
{
  m_doc->GetPreferences().SetOutcomeLabel(1 - m_doc->GetPreferences().OutcomeLabel());
  m_doc->UpdateViews();
}

//----------------------------------------------------------------------
//               NfgShow: Menu handlers - Format menu
//----------------------------------------------------------------------

void NfgShow::OnFormatDisplayColors(wxCommandEvent &)
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

void NfgShow::OnFormatDisplayDecimals(wxCommandEvent &)
{
  dialogSpinCtrl dialog(this, "Decimal places", 0, 25,
			m_doc->GetPreferences().NumDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_doc->GetPreferences().SetNumDecimals(dialog.GetValue());
    m_doc->GetPreferences().SaveOptions();
    m_doc->UpdateViews();
  }
}

void NfgShow::OnFormatFontData(wxCommandEvent &)
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

void NfgShow::OnFormatFontLabels(wxCommandEvent &)
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

void NfgShow::OnFormatAutosize(wxCommandEvent &)
{
  m_table->AutoSizeRows();
  m_table->AutoSizeColumns();
  // Set all strategy columns to be the same width, which is
  // the narrowest width which fits all the entries
  int max = 0, colPlayer = m_doc->GetColPlayer();
  for (int col = 0; col < m_doc->GetNfgSupport().NumStrats(colPlayer); col++) {
    if (m_table->GetColSize(col) > max) {
      max = m_table->GetColSize(col);
    }
  }
  for (int col = 0; col < m_doc->GetNfgSupport().NumStrats(colPlayer); col++) {
    m_table->SetColSize(col, max);
  }
}

//----------------------------------------------------------------------
//                 NfgShow: Menu handlers - Tools
//----------------------------------------------------------------------

void NfgShow::OnToolsDominance(wxCommandEvent &)
{
  gArray<gText> playerNames(m_doc->GetNfg().NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++) {
    playerNames[pl] = m_doc->GetNfg().GetPlayer(pl).GetLabel();
  }
  dialogElimMixed dialog(this, playerNames);

  if (dialog.ShowModal() == wxID_OK) {
    gbtNfgSupport support(m_doc->GetNfgSupport());
    wxStatus status(this, "Dominance Elimination");

    try {
      gbtNfgSupport newSupport(support);

      while (true) {
	gNullOutput gnull;
	if (dialog.DomMixed()) {
	  newSupport = support.MixedUndominated(dialog.DomStrong(),
						precRATIONAL,
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
	  newSupport.SetLabel(m_doc->UniqueNfgSupportName());
	  m_doc->AddNfgSupport(new gbtNfgSupport(newSupport));
	  support = newSupport;
	}

	if (!dialog.Iterative()) {
	  // Bit of a kludge: short-circuit loop if iterative not requested
	  break;
	}
      }
    }
    catch (gSignalBreak &) { }

    if (m_doc->GetNfgSupport() != support) {
      m_doc->SetNfgSupport(m_doc->AllNfgSupports().Length());
      if (!m_table->ShowDominance()) {
	m_table->ToggleDominance();
	GetMenuBar()->Check(GBT_MENU_VIEW_DOMINANCE, true);
      }
    }
  }
}

void NfgShow::OnToolsEquilibrium(wxCommandEvent &)
{ 
  dialogNfgNash dialog(this, m_doc->GetNfgSupport());

  if (dialog.ShowModal() == wxID_OK) {
    gbtNfgNashAlgorithm *algorithm = dialog.GetAlgorithm();

    if (!algorithm) {
      return;
    }

    try {
      wxStatus status(this, algorithm->GetAlgorithm() + "Solve Progress");
      gList<MixedSolution> solutions;
      solutions = algorithm->Solve(m_doc->GetNfgSupport(), status);

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

void NfgShow::OnToolsQre(wxCommandEvent &)
{
  dialogNfgQre dialog(this, m_doc->GetNfgSupport());

  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;

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

	wxStatus status(this, "QreGridSolve Progress");
	gNullOutput gnull;
	algorithm.Solve(m_doc->GetNfgSupport(), gnull, status, solutions);
      }
      else {
	gbtNfgNashLogit algorithm;
	algorithm.SetFullGraph(true);
	algorithm.SetMaxLambda(10000000);

	wxStatus status(this, "QreSolve Progress");
	solutions = algorithm.Solve(m_doc->GetNfgSupport(), status);
      }
    }
    catch (gSignalBreak &) { }
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

#ifdef UNUSED
// Commented out for now, until this gets implemented

void NfgShow::OnToolsCH(wxCommandEvent &)
{
  dialogNfgCH dialog(this, m_doc->GetNfgSupport());

  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;

    try {
      gbtNfgBehavCH algorithm;
      algorithm.SetMinTau(dialog.MinTau());
      algorithm.SetMaxTau(dialog.MaxTau());
      algorithm.SetStepTau(dialog.StepTau());

      wxStatus status(this, "CHSolve Progress");
      gNullOutput gnull;
      solutions = algorithm.Solve(m_doc->GetNfgSupport(), status);
    }
    catch (gSignalBreak &) { }
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
//                NfgShow: Menu handlers - Help menu
//----------------------------------------------------------------------

void NfgShow::OnHelpAbout(wxCommandEvent &)
{
  wxGetApp().OnHelpAbout(this);
}

//----------------------------------------------------------------------
//               NfgShow: Menu handlers - Support menu
//----------------------------------------------------------------------

void NfgShow::OnSupportDuplicate(wxCommandEvent &)
{
  gbtNfgSupport *newSupport = new gbtNfgSupport(m_doc->GetNfgSupport());
  newSupport->SetLabel(m_doc->UniqueNfgSupportName());
  m_doc->AddNfgSupport(newSupport);
  m_doc->SetNfgSupport(m_doc->AllNfgSupports().Length());
}

void NfgShow::OnSupportDelete(wxCommandEvent &)
{
  m_doc->DeleteNfgSupport();
}

//----------------------------------------------------------------------
//              NfgShow: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void NfgShow::OnProfilesNew(wxCommandEvent &)
{
  MixedSolution profile = MixedProfile<gNumber>(gbtNfgSupport(m_doc->GetNfg()));

  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
  }
}

void NfgShow::OnProfilesDuplicate(wxCommandEvent &)
{
  MixedSolution profile(m_doc->GetMixedProfile());
  
  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
  }
}

void NfgShow::OnProfilesDelete(wxCommandEvent &)
{
  m_doc->RemoveProfile(m_doc->AllMixedProfiles().Find(m_doc->GetMixedProfile()));
}

void NfgShow::OnProfilesProperties(wxCommandEvent &)
{
  if (m_doc->IsProfileSelected()) {
    dialogEditMixed dialog(this, m_doc->GetMixedProfile());

    if (dialog.ShowModal() == wxID_OK) {
      m_doc->SetCurrentProfile(dialog.GetProfile());
    }
  }
}

void NfgShow::OnProfilesReport(wxCommandEvent &)
{
  // FIXME: Report should be generated by document class
  // dialogReport dialog(this, m_profileGrid->GetReport());
  // dialog.ShowModal();
}

//----------------------------------------------------------------------
//                  NfgShow: Non-menu event handlers
//----------------------------------------------------------------------

void NfgShow::OnCloseWindow(wxCloseEvent &p_event)
{
  if (m_doc->HasEfg()) {
    // Simply hide the window, don't actually destroy it
    m_doc->SetShowNfg(false);
    p_event.Veto();
    return;
  }

  if (p_event.CanVeto() && m_doc->IsModified()) {
    if (wxMessageBox("Game has been modified.  Close anyway?", "Warning",
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }

  Show(false);
  Destroy();
}

void NfgShow::OnSetFocus(wxFocusEvent &)
{
  m_table->SetFocus();
}

void NfgShow::OnSize(wxSizeEvent &)
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
