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
#include "nfgprofile.h"
#include "nfgnavigate.h"
#include "nfgoutcome.h"
#include "nfgsupport.h"
#include "nfgconst.h"

#include "game/efg.h"
#include "game/efgutils.h"
#include "nash/behavsol.h"
#include "game/nfgciter.h"

#include "gambit.h"
#include "efgshow.h"
#include "dlnfgstrategies.h"
#include "dleditcont.h"
#include "dlnfgproperties.h"
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

const int idSOLUTIONWINDOW = 3002;
const int idINFONOTEBOOK = 3003;
const int idINFOWINDOW = 3004;

BEGIN_EVENT_TABLE(NfgShow, wxFrame)
  EVT_MENU(wxID_NEW, NfgShow::OnFileNew)
  EVT_MENU(wxID_OPEN, NfgShow::OnFileOpen)
  EVT_MENU(wxID_CLOSE, NfgShow::Close)
  EVT_MENU(wxID_SAVE, NfgShow::OnFileSave)
  EVT_MENU(wxID_SAVEAS, NfgShow::OnFileSave)
  EVT_MENU(GBT_NFG_MENU_FILE_IMPORT_COMLAB, NfgShow::OnFileImportComLab)
  EVT_MENU(GBT_NFG_MENU_FILE_EXPORT_COMLAB, NfgShow::OnFileExportComLab)
  EVT_MENU(GBT_NFG_MENU_FILE_EXPORT_HTML, NfgShow::OnFileExportHTML)
  EVT_MENU(wxID_PRINT_SETUP, NfgShow::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, NfgShow::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, NfgShow::OnFilePrint)
  EVT_MENU(wxID_EXIT, NfgShow::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, NfgShow::OnFileMRUFile)
  EVT_MENU(GBT_NFG_MENU_EDIT_STRATS, NfgShow::OnEditStrategies)
  EVT_MENU(GBT_NFG_MENU_EDIT_CONTINGENCY, NfgShow::OnEditContingency)
  EVT_MENU(GBT_NFG_MENU_EDIT_GAME, NfgShow::OnEditGame)
  EVT_MENU(GBT_NFG_MENU_VIEW_PROFILES, NfgShow::OnViewProfiles)
  EVT_MENU(GBT_NFG_MENU_VIEW_NAVIGATION, NfgShow::OnViewNavigation)
  EVT_MENU(GBT_NFG_MENU_VIEW_OUTCOMES, NfgShow::OnViewOutcomes)
  EVT_MENU(GBT_NFG_MENU_VIEW_SUPPORTS, NfgShow::OnViewSupports)
  EVT_MENU(GBT_NFG_MENU_VIEW_DOMINANCE, NfgShow::OnViewDominance)
  EVT_MENU(GBT_NFG_MENU_VIEW_PROBABILITIES, NfgShow::OnViewProbabilities)
  EVT_MENU(GBT_NFG_MENU_VIEW_VALUES, NfgShow::OnViewValues)
  EVT_MENU(GBT_NFG_MENU_VIEW_OUTCOME_LABELS, NfgShow::OnViewOutcomeLabels)
  EVT_MENU(GBT_NFG_MENU_FORMAT_DISPLAY_DECIMALS, NfgShow::OnFormatDisplayDecimals)
  EVT_MENU(GBT_NFG_MENU_FORMAT_FONT_DATA, NfgShow::OnFormatFontData)
  EVT_MENU(GBT_NFG_MENU_FORMAT_FONT_LABELS, NfgShow::OnFormatFontLabels)
  EVT_MENU(GBT_NFG_MENU_TOOLS_DOMINANCE, NfgShow::OnToolsDominance)
  EVT_MENU(GBT_NFG_MENU_TOOLS_EQUILIBRIUM, NfgShow::OnToolsEquilibrium)
  EVT_MENU(GBT_NFG_MENU_TOOLS_QRE, NfgShow::OnToolsQre)
  EVT_MENU(GBT_NFG_MENU_TOOLS_CH, NfgShow::OnToolsCH)
  EVT_MENU(wxID_ABOUT, NfgShow::OnHelpAbout)
  EVT_MENU(GBT_NFG_MENU_SUPPORT_DUPLICATE, NfgShow::OnSupportDuplicate)
  EVT_MENU(GBT_NFG_MENU_SUPPORT_DELETE, NfgShow::OnSupportDelete)
  EVT_MENU(GBT_NFG_MENU_PROFILES_NEW, NfgShow::OnProfilesNew)
  EVT_MENU(GBT_NFG_MENU_PROFILES_DUPLICATE, NfgShow::OnProfilesDuplicate)
  EVT_MENU(GBT_NFG_MENU_PROFILES_DELETE, NfgShow::OnProfilesDelete)
  EVT_MENU(GBT_NFG_MENU_PROFILES_PROPERTIES, NfgShow::OnProfilesProperties)
  EVT_MENU(GBT_NFG_MENU_PROFILES_REPORT, NfgShow::OnProfilesReport)
  EVT_LIST_ITEM_ACTIVATED(idNFG_SOLUTION_LIST, NfgShow::OnProfilesProperties)
  EVT_LIST_ITEM_SELECTED(idNFG_SOLUTION_LIST, NfgShow::OnProfileSelected)
  EVT_SIZE(NfgShow::OnSize)
  EVT_CLOSE(NfgShow::OnCloseWindow)
  EVT_SASH_DRAGGED(idINFOWINDOW, NfgShow::OnSashDrag)
  EVT_SASH_DRAGGED(idSOLUTIONWINDOW, NfgShow::OnSashDrag)
  EVT_SET_FOCUS(NfgShow::OnSetFocus)
  EVT_NOTEBOOK_PAGE_CHANGED(idINFONOTEBOOK, NfgShow::OnInfoNotebookPage)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//               NfgShow: Constructor and destructor
//----------------------------------------------------------------------

NfgShow::NfgShow(gbtGameDocument *p_doc, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(500, 500)),
    gbtGameView(p_doc),
    m_table(0), m_profileTable(0),
    m_solutionSashWindow(0), m_infoSashWindow(0),
    m_navigateWindow(0), m_outcomeWindow(0), m_supportWindow(0)
{
  // Temporary hack to make sure this is initialized before
  // child windows do callbacks -- will be removed once document/view
  // implementation is complete.
  p_doc->m_nfgShow = this;
#ifdef __WXMSW__
  SetIcon(wxIcon("nfg_icn"));
#else
#include "bitmaps/nfg.xbm"
  SetIcon(wxIcon(nfg_bits, nfg_width, nfg_height));
#endif  // __WXMSW__

  m_doc->m_curNfgSupport = new gbtNfgSupport(*m_doc->m_nfg);   // base support
  m_doc->m_curNfgSupport->SetName("Full Support");
  m_doc->m_nfgSupports.Append(m_doc->m_curNfgSupport);

  MakeMenus();

  wxAcceleratorEntry entries[5];
  entries[0].Set(wxACCEL_CTRL, (int) 'N', wxID_NEW);
  entries[1].Set(wxACCEL_CTRL, (int) 'O', wxID_OPEN);
  entries[2].Set(wxACCEL_CTRL, (int) 'S', wxID_SAVE);
  entries[3].Set(wxACCEL_CTRL, (int) 'P', wxID_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
  wxAcceleratorTable accel(5, entries);
  SetAcceleratorTable(accel);

  CreateStatusBar();
  MakeToolbar();

  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_profileTable = new NfgProfileList(m_doc, m_solutionSashWindow);
  m_profileTable->Show(false);
  m_solutionSashWindow->Show(false);

  m_infoSashWindow = new wxSashWindow(this, idINFOWINDOW,
				      wxPoint(0, 40), wxSize(200, 200),
				      wxNO_BORDER | wxSW_3D);
  m_infoSashWindow->SetSashVisible(wxSASH_RIGHT, true);
  m_infoSashWindow->Show(true);

  m_infoNotebook = new wxNotebook(m_infoSashWindow, idINFONOTEBOOK);
  m_infoNotebook->Show(true);

  m_navigateWindow = new NfgNavigateWindow(m_doc, m_infoNotebook);
  m_navigateWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_navigateWindow, "Navigation");

  m_outcomeWindow = new NfgOutcomeWindow(m_doc, m_infoNotebook);
  m_outcomeWindow->UpdateValues();
  m_outcomeWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_outcomeWindow, "Outcomes");

  m_supportWindow = new NfgSupportWindow(m_doc, m_infoNotebook);
  m_supportWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_supportWindow, "Supports");
  m_infoNotebook->SetSelection(0);

  m_table = new NfgTable(m_doc, this);
  m_table->SetSize(0, 0, 200, 200);

  m_doc->m_nfg->SetIsDirty(false);
  GetMenuBar()->Check(GBT_NFG_MENU_VIEW_OUTCOMES,
		      !m_table->GetSettings().OutcomeValues());
  UpdateMenus();
  m_table->SetFocus();

  AdjustSizes();
  Show(true);
}

NfgShow::~NfgShow()
{ }

//----------------------------------------------------------------------
//                NfgShow: Manipulation of profile list
//----------------------------------------------------------------------

void NfgShow::OnProfilesEdited(void)
{
  m_profileTable->UpdateValues();
  UpdateMenus();
}

void NfgShow::OnChangeProfile(void)
{
  if (m_doc->IsProfileSelected()) {
    m_table->SetProfile(m_doc->GetMixedProfile());
  }
  else {
    if (m_table->ShowProbs()) {
      m_table->ToggleProbs();
      GetMenuBar()->Check(GBT_NFG_MENU_VIEW_PROBABILITIES, false);
    }
    if (m_table->ShowValues()) {
      m_table->ToggleValues();
      GetMenuBar()->Check(GBT_NFG_MENU_VIEW_VALUES, false);
    }
  }

  if (m_profileTable) {
    m_profileTable->UpdateValues();
  }
}

void NfgShow::OnProfileSelected(wxListEvent &p_event)
{
  m_doc->SetCurrentProfile(p_event.GetIndex());
}
 
//----------------------------------------------------------------------
//            NfgShow: Coordinating updates of child windows
//----------------------------------------------------------------------

void NfgShow::SetPlayers(int p_rowPlayer, int p_colPlayer)
{
  if (m_navigateWindow) {
    m_navigateWindow->SetPlayers(p_rowPlayer, p_colPlayer);
  }
  m_table->SetPlayers(p_rowPlayer, p_colPlayer);
}

void NfgShow::SetStrategy(int p_player, int p_strategy)
{
  if (m_navigateWindow)  {
    m_navigateWindow->SetStrategy(p_player, p_strategy);
  }
  if (m_table) {
    m_table->SetStrategy(p_player, p_strategy);
  }
}

void NfgShow::SetProfile(const gArray<int> &p_profile)
{
  m_navigateWindow->SetProfile(p_profile);
  m_table->SetContingency(p_profile);
}

gArray<int> NfgShow::GetContingency(void) const
{
  return m_navigateWindow->GetProfile();
}

void NfgShow::UpdateProfile(gArray<int> &profile)
{
  //  m_table->OnChangeValues();
}

void NfgShow::OnOutcomesEdited(void)
{
  m_table->RefreshTable();
  m_profileTable->UpdateValues();
}

gText NfgShow::UniqueOutcomeName(void) const
{
  int number = m_doc->m_nfg->NumOutcomes() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_doc->m_nfg->NumOutcomes(); i++) {
      if (m_doc->m_nfg->GetOutcomeId(i).GetLabel() == "Outcome" + ToText(number)) {
	break;
      }
    }

    if (i > m_doc->m_nfg->NumOutcomes()) {
      return "Outcome" + ToText(number);
    }
    
    number++;
  }
}

//----------------------------------------------------------------------
//           NfgShow: Creating and updating menus and toolbar
//----------------------------------------------------------------------

void NfgShow::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new game");
  fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a saved game");
  fileMenu->Append(wxID_CLOSE, "&Close", "Close this window");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_SAVE, "&Save\tCtrl-S", "Save this game");
  fileMenu->Append(wxID_SAVEAS, "Save &as", "Save game to a different file");
  fileMenu->AppendSeparator();
  wxMenu *fileImportMenu = new wxMenu;
  fileImportMenu->Append(GBT_NFG_MENU_FILE_IMPORT_COMLAB, "&ComLabGames",
			 "Import a game saved in ComLabGames format");
  fileMenu->Append(GBT_NFG_MENU_FILE_IMPORT, "&Import", fileImportMenu,
		   "Import a game from various formats");
  wxMenu *fileExportMenu = new wxMenu;
  fileExportMenu->Append(GBT_NFG_MENU_FILE_EXPORT_COMLAB, "&ComLabGames",
			 "Export game to ComLabGames format");
  fileExportMenu->Append(GBT_NFG_MENU_FILE_EXPORT_HTML, "&HTML",
			 "Save this game in HTML format");
  fileMenu->Append(GBT_NFG_MENU_FILE_EXPORT, "&Export", fileExportMenu,
		   "Export the game in various formats");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_PRINT_SETUP, "Page Se&tup",
		   "Set up preferences for printing");
  fileMenu->Append(wxID_PREVIEW, "Print Pre&view",
		   "View a preview of the game printout");
  fileMenu->Append(wxID_PRINT, "&Print\tCtrl-P", "Print this game");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, "E&xit\tCtrl-X", "Exit Gambit");
  
  wxMenu *editMenu = new wxMenu;
  editMenu->Append(wxID_CUT, "Cu&t", "Cut the current selection");
  editMenu->Append(wxID_COPY, "&Copy", "Copy the current selection");
  editMenu->Append(wxID_PASTE, "&Paste", "Paste from clipboard");
  // For the moment, these are not implemented -- leave disabled
  editMenu->Enable(wxID_CUT, false);
  editMenu->Enable(wxID_COPY, false);
  editMenu->Enable(wxID_PASTE, false);
  editMenu->AppendSeparator();
  editMenu->Append(GBT_NFG_MENU_EDIT_STRATS, "&Strategies", "Edit strategy names");
  editMenu->Append(GBT_NFG_MENU_EDIT_CONTINGENCY, "&Contingency",
		   "Edit the selected contingency");
  editMenu->Append(GBT_NFG_MENU_EDIT_GAME, "&Game", "Edit game properties");

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(GBT_NFG_MENU_VIEW_PROFILES, "&Profiles",
		   "Display/hide profiles window", true);
  viewMenu->Check(GBT_NFG_MENU_VIEW_PROFILES, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_NFG_MENU_VIEW_NAVIGATION, "&Navigation",
		   "Display navigation window", true);
  viewMenu->Check(GBT_NFG_MENU_VIEW_NAVIGATION, true);
  viewMenu->Append(GBT_NFG_MENU_VIEW_OUTCOMES, "&Outcomes", 
		   "Display and edit outcomes", true);
  viewMenu->Check(GBT_NFG_MENU_VIEW_OUTCOMES, false);
  viewMenu->Append(GBT_NFG_MENU_VIEW_SUPPORTS, "&Supports",
		   "Display and edit supports", true);
  viewMenu->Check(GBT_NFG_MENU_VIEW_SUPPORTS, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(GBT_NFG_MENU_VIEW_DOMINANCE, "&Dominance",
		   "Display dominance information", TRUE);
  viewMenu->Append(GBT_NFG_MENU_VIEW_PROBABILITIES, "&Probabilities",
		   "Display solution probabilities", TRUE);
  viewMenu->Append(GBT_NFG_MENU_VIEW_VALUES, "&Values",
		   "Display strategy values", TRUE);
  viewMenu->AppendSeparator();
  // This probably belongs in formatting instead
  viewMenu->Append(GBT_NFG_MENU_VIEW_OUTCOME_LABELS, "Outcome &Labels",
		   "Display outcome labels", TRUE);
  
  wxMenu *formatMenu = new wxMenu;
  wxMenu *formatDisplayMenu = new wxMenu;
  formatDisplayMenu->Append(GBT_NFG_MENU_FORMAT_DISPLAY_DECIMALS, "&Decimal Places",
			   "Set number of decimal places to display");

  formatMenu->Append(GBT_NFG_MENU_FORMAT_DISPLAY, "&Display", formatDisplayMenu,
		    "Configure display options");
  wxMenu *formatFontMenu = new wxMenu;
  formatFontMenu->Append(GBT_NFG_MENU_FORMAT_FONT_DATA, "&Data", "Set data font");
  formatFontMenu->Append(GBT_NFG_MENU_FORMAT_FONT_LABELS, "&Labels", "Set label font");
  formatMenu->Append(GBT_NFG_MENU_FORMAT_FONT, "&Font", formatFontMenu, "Set fonts");

  wxMenu *toolsMenu = new wxMenu;
  toolsMenu->Append(GBT_NFG_MENU_TOOLS_DOMINANCE, "&Dominance",
		       "Find undominated strategies");
  toolsMenu->Append(GBT_NFG_MENU_TOOLS_EQUILIBRIUM, "&Equilibrium",
		    "Compute Nash equilibria (and refinements)");
  toolsMenu->Append(GBT_NFG_MENU_TOOLS_QRE, "&Qre",
		    "Compute quantal response equilibria");
  toolsMenu->Append(GBT_NFG_MENU_TOOLS_CH, "&CH",
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

void NfgShow::UpdateMenus(void)
{
  wxMenuBar *menu = GetMenuBar();
  gArray<int> profile(GetContingency());
  menu->Enable(GBT_NFG_MENU_FILE_EXPORT_COMLAB, m_doc->m_nfg->NumPlayers() == 2);
  menu->Enable(GBT_NFG_MENU_VIEW_PROBABILITIES, m_doc->IsProfileSelected());
  menu->Enable(GBT_NFG_MENU_VIEW_VALUES, m_doc->IsProfileSelected());
  menu->Check(GBT_NFG_MENU_VIEW_OUTCOME_LABELS, 
	      !m_table->GetSettings().OutcomeValues());
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
  if (p_event.GetId() == wxID_SAVEAS || m_filename == "") {
    wxFileDialog dialog(this, "Choose file", wxPathOnly(m_filename),
			wxFileNameFromPath(m_filename), "*.nfg",
			wxSAVE | wxOVERWRITE_PROMPT);

    switch (dialog.ShowModal()) {
    case wxID_OK:
      SetFilename(dialog.GetPath());
      break;
    case wxID_CANCEL:
    default:
      return;
    }
  }

  try {
    gFileOutput file(m_filename.c_str());
    gbtNfgGame nfg = CompressNfg(*m_doc->m_nfg, *m_doc->m_curNfgSupport);
    nfg.WriteNfgFile(file, 6);
    m_doc->m_nfg->SetIsDirty(false);
  }
  catch (gFileOutput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open %s for writing.",
				  m_filename.c_str()),
		 "Error", wxOK, this);
  }
  catch (gFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				  m_filename.c_str()),
		 "Error", wxOK, this);
  }
  catch (gException &) {
    wxMessageBox("Internal exception in Gambit", "Error", wxOK, this);
  }
}

void NfgShow::OnFileImportComLab(wxCommandEvent &)
{
  wxGetApp().OnFileImportComLab(this);
}

void NfgShow::OnFileExportComLab(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose file", wxGetApp().CurrentDir(), "",
		      "ComLabGames strategic form games (*.sfg)|*.sfg",
		      wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      gFileOutput file(dialog.GetPath().c_str());
      WriteComLabSfg(file, *m_doc->m_nfg);
    }
    catch (gFileOutput::OpenFailed &) { 
      wxMessageBox(wxString::Format("Could not open %s for writing.",
				    dialog.GetPath().c_str()),
		   "Error", wxOK, this);
    }
    catch (gFileOutput::WriteFailed &) {
      wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				    dialog.GetPath().c_str()),
		   "Error", wxOK, this);
    }
    catch (...) {
      wxMessageBox("An internal exeception occurred in Gambit", "Error",
		   wxOK, this);
    }
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
    file << gbtBuildHtml(*m_doc->m_nfg, m_table->GetRowPlayer(), m_table->GetColPlayer()).c_str() << '\n';
  }
  catch (gFileOutput::OpenFailed &) { 
    wxMessageBox(wxString::Format("Could not open %s for writing.",
				  m_filename.c_str()),
		 "Error", wxOK, this);
  }
  catch (gFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				  m_filename.c_str()),
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
    new wxPrintPreview(new NfgPrintout(*m_doc->m_nfg, 
				       m_table->GetRowPlayer(), 
				       m_table->GetColPlayer(),
				       (char *) m_doc->m_nfg->GetTitle()),
		       new NfgPrintout(*m_doc->m_nfg,
				       m_table->GetRowPlayer(),
				       m_table->GetColPlayer(),
				       (char *) m_doc->m_nfg->GetTitle()),
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
  NfgPrintout printout(*m_doc->m_nfg, m_table->GetRowPlayer(), m_table->GetColPlayer(),
		       (char *) m_doc->m_nfg->GetTitle());

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
  dialogStrategies dialog(this, *m_doc->m_nfg);

  if (dialog.ShowModal() == wxID_OK) {
    for (int pl = 1; pl <= m_doc->m_nfg->NumPlayers(); pl++) {
      gbtNfgPlayer player = m_doc->m_nfg->GetPlayer(pl);
      for (int st = 1; st <= player.NumStrategies(); st++) {
	player.GetStrategy(st).SetLabel(dialog.GetStrategyName(pl, st));
      }
    }
    m_table->RefreshTable();
  }
}

void NfgShow::OnEditContingency(wxCommandEvent &)
{
  dialogEditContingency dialog(this, *m_doc->m_nfg, GetContingency());

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.GetOutcome() == 0) { 
      m_doc->m_nfg->SetOutcome(GetContingency(), 0);
    }
    else {
      m_doc->m_nfg->SetOutcome(GetContingency(),
		       m_doc->m_nfg->GetOutcomeId(dialog.GetOutcome()));
    }
    m_table->RefreshTable();
  }
}

void NfgShow::OnEditGame(wxCommandEvent &)
{
  dialogNfgProperties dialog(this, *m_doc->m_nfg, m_filename);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->m_nfg->SetTitle(dialog.GetGameTitle().c_str());
    SetFilename(Filename());
    m_doc->m_nfg->SetComment(dialog.GetComment().c_str());
    for (int pl = 1; pl <= dialog.NumPlayers(); pl++) {
      m_doc->m_nfg->GetPlayer(pl).SetLabel(dialog.GetPlayerName(pl).c_str());
    }
    m_navigateWindow->UpdateLabels();
    m_outcomeWindow->UpdateValues();
    m_supportWindow->UpdateValues();
  }
}


//----------------------------------------------------------------------
//                NfgShow: Menu handlers - View menu
//----------------------------------------------------------------------

void NfgShow::OnViewProfiles(wxCommandEvent &)
{
  if (m_solutionSashWindow->IsShown()) {
    m_profileTable->Show(false);
    m_solutionSashWindow->Show(false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_PROFILES, false);
  }
  else {
    m_profileTable->Show(true);
    m_solutionSashWindow->Show(true);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_PROFILES, true);
  }

  AdjustSizes();
}

void NfgShow::OnViewNavigation(wxCommandEvent &)
{
  if (m_infoSashWindow->IsShown() && m_infoNotebook->GetSelection() != 0) {
    m_infoNotebook->SetSelection(0);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_NAVIGATION, true);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_OUTCOMES, false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_SUPPORTS, false);
  }
  else if (m_infoSashWindow->IsShown()) {
    m_infoSashWindow->Show(false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_NAVIGATION, false);
  }
  else {
    m_infoSashWindow->Show(true);
    m_infoNotebook->SetSelection(0);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_NAVIGATION, true);
  }

  AdjustSizes();
}

void NfgShow::OnViewOutcomes(wxCommandEvent &)
{
  if (m_infoSashWindow->IsShown() && m_infoNotebook->GetSelection() != 1) {
    m_infoNotebook->SetSelection(1);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_OUTCOMES, true);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_NAVIGATION, false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_SUPPORTS, false);
  }
  else if (m_infoSashWindow->IsShown()) {
    m_infoSashWindow->Show(false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_OUTCOMES, false);
  }
  else {
    m_infoSashWindow->Show(true);
    m_infoNotebook->SetSelection(1);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_OUTCOMES, true);
  }

  AdjustSizes();
}

void NfgShow::OnViewSupports(wxCommandEvent &)
{
  if (m_infoSashWindow->IsShown() && m_infoNotebook->GetSelection() != 2) {
    m_infoNotebook->SetSelection(2);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_OUTCOMES, false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_NAVIGATION, false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_SUPPORTS, true);
  }
  else if (m_infoSashWindow->IsShown()) {
    m_infoSashWindow->Show(false);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_SUPPORTS, false);
  }
  else {
    m_infoSashWindow->Show(true);
    m_infoNotebook->SetSelection(2);
    GetMenuBar()->Check(GBT_NFG_MENU_VIEW_SUPPORTS, true);
  }

  AdjustSizes();
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
  m_table->SetOutcomeValues(1 - m_table->OutcomeValues());
  m_table->RefreshTable();
}

//----------------------------------------------------------------------
//               NfgShow: Menu handlers - Format menu
//----------------------------------------------------------------------

void NfgShow::OnFormatDisplayDecimals(wxCommandEvent &)
{
  dialogSpinCtrl dialog(this, "Decimal places", 0, 25,
			m_table->GetSettings().GetDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_table->GetSettings().SetDecimals(dialog.GetValue());
    m_table->GetSettings().SaveSettings();
    m_table->RefreshTable();
  }
}

void NfgShow::OnFormatFontData(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_table->SetDataFont(dialog.GetFontData().GetChosenFont());
    m_table->RefreshTable();
  }
}

void NfgShow::OnFormatFontLabels(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_table->SetLabelFont(dialog.GetFontData().GetChosenFont());
    m_table->RefreshTable();
  }
}

//----------------------------------------------------------------------
//                 NfgShow: Menu handlers - Tools
//----------------------------------------------------------------------

void NfgShow::OnToolsDominance(wxCommandEvent &)
{
  gArray<gText> playerNames(m_doc->m_nfg->NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++) {
    playerNames[pl] = m_doc->m_nfg->GetPlayer(pl).GetLabel();
  }
  dialogElimMixed dialog(this, playerNames);

  if (dialog.ShowModal() == wxID_OK) {
    gbtNfgSupport support(*m_doc->m_curNfgSupport);
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
	  newSupport.SetName(UniqueSupportName());
	  m_doc->m_nfgSupports.Append(new gbtNfgSupport(newSupport));
	  support = newSupport;
	}

	if (!dialog.Iterative()) {
	  // Bit of a kludge: short-circuit loop if iterative not requested
	  break;
	}
      }
    }
    catch (gSignalBreak &) { }

    if (*m_doc->m_curNfgSupport != support) {
      m_doc->m_curNfgSupport = m_doc->m_nfgSupports[m_doc->m_nfgSupports.Length()];
      if (!m_table->ShowDominance()) {
	m_table->ToggleDominance();
	GetMenuBar()->Check(GBT_NFG_MENU_VIEW_DOMINANCE, true);
      }
      OnSupportsEdited();
      UpdateMenus();
    }
  }
}

void NfgShow::OnToolsEquilibrium(wxCommandEvent &)
{ 
  dialogNfgNash dialog(this, *m_doc->m_curNfgSupport);

  if (dialog.ShowModal() == wxID_OK) {
    gbtNfgNashAlgorithm *algorithm = dialog.GetAlgorithm();

    if (!algorithm) {
      return;
    }

    try {
      wxStatus status(this, algorithm->GetAlgorithm() + "Solve Progress");
      gList<MixedSolution> solutions;
      solutions = algorithm->Solve(*m_doc->m_curNfgSupport, status);

      for (int soln = 1; soln <= solutions.Length(); soln++) {
	m_doc->AddProfile(solutions[soln]);
      }
      m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
   
      if (solutions.Length() > 0 && !m_table->ShowProbs()) {
	m_table->ToggleProbs();
	GetMenuBar()->Check(GBT_NFG_MENU_VIEW_PROBABILITIES, true);
      }
      if (!m_solutionSashWindow->IsShown()) {
	m_profileTable->Show(true);
	m_solutionSashWindow->Show(true);
	GetMenuBar()->Check(GBT_NFG_MENU_VIEW_PROFILES, true);
	AdjustSizes();
      }
      
      UpdateMenus();
    }
    catch (...) { }

    delete algorithm;
  }
}

void NfgShow::OnToolsQre(wxCommandEvent &)
{
  dialogNfgQre dialog(this, *m_doc->m_curNfgSupport);

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
	algorithm.Solve(*m_doc->m_curNfgSupport, gnull, status, solutions);
      }
      else {
	gbtNfgNashLogit algorithm;
	algorithm.SetFullGraph(true);
	algorithm.SetMaxLambda(10000000);

	wxStatus status(this, "QreSolve Progress");
	solutions = algorithm.Solve(*m_doc->m_curNfgSupport, status);
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

void NfgShow::OnToolsCH(wxCommandEvent &)
{
  dialogNfgCH dialog(this, *m_doc->m_curNfgSupport);

  if (dialog.ShowModal() == wxID_OK) {
    gList<MixedSolution> solutions;

    try {
      gbtNfgBehavCH algorithm;
      algorithm.SetMinTau(dialog.MinTau());
      algorithm.SetMaxTau(dialog.MaxTau());
      algorithm.SetStepTau(dialog.StepTau());

      wxStatus status(this, "CHSolve Progress");
      gNullOutput gnull;
      solutions = algorithm.Solve(*m_doc->m_curNfgSupport, status);
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
  gbtNfgSupport *newSupport = new gbtNfgSupport(*m_doc->m_curNfgSupport);
  newSupport->SetName(UniqueSupportName());
  m_doc->m_nfgSupports.Append(newSupport);
  m_doc->m_curNfgSupport = newSupport;
  OnSupportsEdited();
}

void NfgShow::OnSupportDelete(wxCommandEvent &)
{
  delete m_doc->m_nfgSupports.Remove(m_doc->m_nfgSupports.Find(m_doc->m_curNfgSupport));
  m_doc->m_curNfgSupport = m_doc->m_nfgSupports[1];
  OnSupportsEdited();
}

//----------------------------------------------------------------------
//              NfgShow: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void NfgShow::OnProfilesNew(wxCommandEvent &)
{
  MixedSolution profile = MixedProfile<gNumber>(gbtNfgSupport(*m_doc->m_nfg));

  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
    UpdateMenus();
  }
}

void NfgShow::OnProfilesDuplicate(wxCommandEvent &)
{
  MixedSolution profile(m_doc->GetMixedProfile());
  
  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_doc->AddProfile(dialog.GetProfile());
    m_doc->SetCurrentProfile(m_doc->AllMixedProfiles().Length());
    UpdateMenus();
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
  dialogReport dialog(this, m_profileTable->GetReport());
  dialog.ShowModal();
}

//----------------------------------------------------------------------
//                  NfgShow: Non-menu event handlers
//----------------------------------------------------------------------

void NfgShow::OnCloseWindow(wxCloseEvent &p_event)
{
  if (p_event.CanVeto() && GameIsDirty()) {
    if (wxMessageBox("Game has been modified.  Close anyway?", "Warning",
		     wxOK | wxCANCEL) == wxCANCEL) {
      p_event.Veto();
      return;
    }
  }

  Show(false);
  Destroy();
}

void NfgShow::AdjustSizes(void)
{
  int width, height;
  GetClientSize(&width, &height);

  if (m_solutionSashWindow && m_solutionSashWindow->IsShown()) {
    m_solutionSashWindow->SetSize(0, height - m_solutionSashWindow->GetRect().height,
				  width, m_solutionSashWindow->GetRect().height);
    height -= m_solutionSashWindow->GetRect().height;
  }

  if (m_navigateWindow && m_infoSashWindow->IsShown()) {
    if (m_table) {
      m_table->SetSize(m_infoSashWindow->GetRect().width, 0,
		       width - m_infoSashWindow->GetRect().width, height);
    }
  }
  else if (m_table) {
    m_table->SetSize(0, 0, width, height);
  }

  if (m_navigateWindow && m_infoSashWindow->IsShown()) {
    m_infoSashWindow->SetSize(0, 0, m_infoSashWindow->GetRect().width, height);
  }

  if (m_table) {
    m_table->SetFocus();
  }
}


void NfgShow::OnSize(wxSizeEvent &)
{
  AdjustSizes();
}

void NfgShow::OnSashDrag(wxSashEvent &p_event)
{
  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  switch (p_event.GetId()) {
  case idINFOWINDOW:
    m_table->SetSize(p_event.GetDragRect().width,
		     m_table->GetRect().y,
		     clientWidth - p_event.GetDragRect().width,
		     m_table->GetRect().height);
    m_infoSashWindow->SetSize(m_infoSashWindow->GetRect().x,
			      m_infoSashWindow->GetRect().y,
			      p_event.GetDragRect().width,
			      m_infoSashWindow->GetRect().height);
    break;
  case idSOLUTIONWINDOW:
    m_table->SetSize(m_table->GetRect().x, m_table->GetRect().y,
		     m_table->GetRect().width,
		     clientHeight - p_event.GetDragRect().height);
    m_infoSashWindow->SetSize(m_infoSashWindow->GetRect().x,
			      m_infoSashWindow->GetRect().y,
			      m_infoSashWindow->GetRect().width,
			      clientHeight - p_event.GetDragRect().height);
    m_solutionSashWindow->SetSize(0, clientHeight - p_event.GetDragRect().height,
				  clientWidth, p_event.GetDragRect().height);
    break;
  }
}

void NfgShow::OnSetFocus(wxFocusEvent &)
{
  m_table->SetFocus();
}

void NfgShow::OnInfoNotebookPage(wxNotebookEvent &p_event)
{
  GetMenuBar()->Check(GBT_NFG_MENU_VIEW_NAVIGATION, p_event.GetSelection() == 0);
  GetMenuBar()->Check(GBT_NFG_MENU_VIEW_OUTCOMES, p_event.GetSelection() == 1);
  GetMenuBar()->Check(GBT_NFG_MENU_VIEW_SUPPORTS, p_event.GetSelection() == 2);
}

//----------------------------------------------------------------------
//                   NfgShow: Miscellaneous members
//----------------------------------------------------------------------

gText NfgShow::UniqueSupportName(void) const
{
  int number = m_doc->m_nfgSupports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_doc->m_nfgSupports.Length(); i++) {
      if (m_doc->m_nfgSupports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_doc->m_nfgSupports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

void NfgShow::SetFilename(const wxString &p_name)
{
  m_filename = p_name;
  if (m_filename != "") {
    SetTitle(wxString::Format("Gambit - [%s] %s", m_filename.c_str(), 
			      (char *) m_doc->m_nfg->GetTitle()));
  }
  else {
    SetTitle(wxString::Format("Gambit - %s", (char *) m_doc->m_nfg->GetTitle()));
  }
  wxGetApp().SetFilename(this, p_name.c_str());
}

void NfgShow::SetSupportNumber(int p_number)
{
  if (p_number >= 1 && p_number <= m_doc->m_nfgSupports.Length()) {
    m_doc->m_curNfgSupport = m_doc->m_nfgSupports[p_number];
    OnSupportsEdited();
  }
}

void NfgShow::OnSupportsEdited(void)
{
  m_navigateWindow->SetSupport(*m_doc->m_curNfgSupport);
  m_table->SetSupport(*m_doc->m_curNfgSupport);
  m_supportWindow->UpdateValues();
}

#include "base/glist.imp"
template class gList<gbtNfgSupport *>;

