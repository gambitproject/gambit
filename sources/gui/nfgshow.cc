//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of normal form display class
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/grid.h"
#include "wx/fontdlg.h"
#include "guishare/dlspinctrl.h"
#include "guishare/wxstatus.h"

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
#include "game/nfplayer.h"
#include "game/nfgciter.h"

#include "gambit.h"
#include "efgshow.h"
#include "dlnfgstrategies.h"
#include "dleditcont.h"
#include "dlnfgproperties.h"
#include "dleditmixed.h"
#include "dlelimmixed.h"
#include "dlnfgnash.h"

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
  EVT_MENU(wxID_PRINT_SETUP, NfgShow::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, NfgShow::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, NfgShow::OnFilePrint)
  EVT_MENU(wxID_EXIT, NfgShow::OnFileExit)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, NfgShow::OnFileMRUFile)
  EVT_MENU(NFG_EDIT_STRATS, NfgShow::OnEditStrategies)
  EVT_MENU(NFG_EDIT_CONTINGENCY, NfgShow::OnEditContingency)
  EVT_MENU(NFG_EDIT_GAME, NfgShow::OnEditGame)
  EVT_MENU(NFG_VIEW_PROFILES, NfgShow::OnViewProfiles)
  EVT_MENU(NFG_VIEW_NAVIGATION, NfgShow::OnViewNavigation)
  EVT_MENU(NFG_VIEW_OUTCOMES, NfgShow::OnViewOutcomes)
  EVT_MENU(NFG_VIEW_SUPPORTS, NfgShow::OnViewSupports)
  EVT_MENU(NFG_VIEW_DOMINANCE, NfgShow::OnViewDominance)
  EVT_MENU(NFG_VIEW_PROBABILITIES, NfgShow::OnViewProbabilities)
  EVT_MENU(NFG_VIEW_VALUES, NfgShow::OnViewValues)
  EVT_MENU(NFG_VIEW_OUTCOME_LABELS, NfgShow::OnViewOutcomeLabels)
  EVT_MENU(NFG_FORMAT_DISPLAY_COLUMNS, NfgShow::OnFormatDisplayColumns)
  EVT_MENU(NFG_FORMAT_DISPLAY_DECIMALS, NfgShow::OnFormatDisplayDecimals)
  EVT_MENU(NFG_FORMAT_FONT_DATA, NfgShow::OnFormatFontData)
  EVT_MENU(NFG_FORMAT_FONT_LABELS, NfgShow::OnFormatFontLabels)
  EVT_MENU(NFG_FORMAT_COLORS, NfgShow::OnFormatColors)
  EVT_MENU(NFG_FORMAT_SAVE, NfgShow::OnFormatSave)
  EVT_MENU(NFG_FORMAT_LOAD, NfgShow::OnFormatLoad)
  EVT_MENU(NFG_TOOLS_DOMINANCE, NfgShow::OnToolsDominance)
  EVT_MENU(NFG_TOOLS_EQUILIBRIUM, NfgShow::OnToolsEquilibrium)
  EVT_MENU(wxID_HELP_CONTENTS, NfgShow::OnHelpContents)
  EVT_MENU(wxID_HELP_INDEX, NfgShow::OnHelpIndex)
  EVT_MENU(wxID_ABOUT, NfgShow::OnHelpAbout)
  EVT_MENU(NFG_SUPPORT_DUPLICATE, NfgShow::OnSupportDuplicate)
  EVT_MENU(NFG_SUPPORT_DELETE, NfgShow::OnSupportDelete)
  EVT_MENU(NFG_PROFILES_NEW, NfgShow::OnProfilesNew)
  EVT_MENU(NFG_PROFILES_DUPLICATE, NfgShow::OnProfilesDuplicate)
  EVT_MENU(NFG_PROFILES_DELETE, NfgShow::OnProfilesDelete)
  EVT_MENU(NFG_PROFILES_PROPERTIES, NfgShow::OnProfilesProperties)
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

NfgShow::NfgShow(Nfg &p_nfg, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(500, 500)),
    m_nfg(p_nfg),
    m_table(0), m_profileTable(0),
    m_solutionSashWindow(0), m_infoSashWindow(0),
    m_navigateWindow(0), m_outcomeWindow(0), m_supportWindow(0)
{
#ifdef __WXMSW__
  SetIcon(wxIcon("nfg_icn"));
#else
#include "bitmaps/nfg.xbm"
  SetIcon(wxIcon(nfg_bits, nfg_width, nfg_height));
#endif  // __WXMSW__

  m_currentProfile = 0;
  m_currentSupport = new NFSupport(m_nfg);    // base support
  m_currentSupport->SetName("Full Support");
  m_supports.Append(m_currentSupport);

  MakeMenus();

  wxAcceleratorEntry entries[6];
  entries[0].Set(wxACCEL_CTRL, (int) 'N', wxID_NEW);
  entries[1].Set(wxACCEL_CTRL, (int) 'O', wxID_OPEN);
  entries[2].Set(wxACCEL_CTRL, (int) 'S', wxID_SAVE);
  entries[3].Set(wxACCEL_CTRL, (int) 'P', wxID_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
  entries[5].Set(wxACCEL_NORMAL, WXK_F1, wxID_HELP_CONTENTS);
  wxAcceleratorTable accel(6, entries);
  SetAcceleratorTable(accel);

  CreateStatusBar();
  MakeToolbar();

  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_profileTable = new NfgProfileList(this, m_solutionSashWindow);
  m_profileTable->Show(false);
  m_solutionSashWindow->Show(false);

  m_infoSashWindow = new wxSashWindow(this, idINFOWINDOW,
				      wxPoint(0, 40), wxSize(200, 200),
				      wxNO_BORDER | wxSW_3D);
  m_infoSashWindow->SetSashVisible(wxSASH_RIGHT, true);
  m_infoSashWindow->Show(true);

  m_infoNotebook = new wxNotebook(m_infoSashWindow, idINFONOTEBOOK);
  m_infoNotebook->Show(true);

  m_navigateWindow = new NfgNavigateWindow(this, m_infoNotebook);
  m_navigateWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_navigateWindow, "Navigation");

  m_outcomeWindow = new NfgOutcomeWindow(this, m_infoNotebook);
  m_outcomeWindow->UpdateValues();
  m_outcomeWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_outcomeWindow, "Outcomes");

  m_supportWindow = new NfgSupportWindow(this, m_infoNotebook);
  m_supportWindow->SetSize(200, 200);
  m_infoNotebook->AddPage(m_supportWindow, "Supports");
  m_infoNotebook->SetSelection(0);

  m_table = new NfgTable(m_nfg, this);
  m_table->SetSize(0, 0, 200, 200);

  m_nfg.SetIsDirty(false);
  GetMenuBar()->Check(NFG_VIEW_OUTCOMES, !m_table->OutcomeValues());
  UpdateMenus();
  m_table->SetFocus();

  AdjustSizes();
  Show(true);
}

NfgShow::~NfgShow()
{
  wxGetApp().RemoveGame(&m_nfg);
}

//----------------------------------------------------------------------
//                NfgShow: Manipulation of profile list
//----------------------------------------------------------------------

void NfgShow::AddProfile(const MixedSolution &p_profile, bool p_map)
{
  if (p_profile.GetName() == "") {
    MixedSolution tmp(p_profile);
    tmp.SetName(UniqueProfileName());
    m_profiles.Append(tmp);
  }
  else {
    m_profiles.Append(p_profile);
  }

  if (m_nfg.AssociatedEfg() && p_map) {
    wxGetApp().GetWindow(m_nfg.AssociatedEfg())->AddProfile(BehavProfile<gNumber>(*p_profile.Profile()), false);
  }
  m_profileTable->UpdateValues();
  UpdateMenus();
}

void NfgShow::ChangeProfile(int sol)
{
  m_currentProfile = sol;

  if (sol > 0) {
    m_table->SetProfile(m_profiles[m_currentProfile]);
    if (m_profileTable) {
      m_profileTable->UpdateValues();
    }
  }
}

void NfgShow::OnProfileSelected(wxListEvent &p_event)
{
  m_currentProfile = p_event.GetIndex() + 1;
  m_table->SetProfile(m_profiles[m_currentProfile]);
}
 
gText NfgShow::UniqueProfileName(void) const
{
  int number = m_profiles.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_profiles.Length(); i++) {
      if (m_profiles[i].GetName() == "Profile" + ToText(number)) {
	break;
      }
    }

    if (i > m_profiles.Length()) {
      return "Profile" + ToText(number);
    }
    
    number++;
  }
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
  for (int i = 1; i <= m_profiles.Length(); i++) {
    m_profiles[i].Invalidate();
  }
  m_table->RefreshTable();
  m_profileTable->UpdateValues();
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
  editMenu->Append(NFG_EDIT_STRATS, "&Strategies", "Edit strategy names");
  editMenu->Append(NFG_EDIT_CONTINGENCY, "&Contingency",
		   "Edit the selected contingency");
  editMenu->Append(NFG_EDIT_GAME, "&Game", "Edit game properties");

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(NFG_VIEW_PROFILES, "&Profiles",
		   "Display/hide profiles window", true);
  viewMenu->Check(NFG_VIEW_PROFILES, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(NFG_VIEW_NAVIGATION, "&Navigation",
		   "Display navigation window", true);
  viewMenu->Check(NFG_VIEW_NAVIGATION, true);
  viewMenu->Append(NFG_VIEW_OUTCOMES, "&Outcomes", 
		   "Display and edit outcomes", true);
  viewMenu->Check(NFG_VIEW_OUTCOMES, false);
  viewMenu->Append(NFG_VIEW_SUPPORTS, "&Supports",
		   "Display and edit supports", true);
  viewMenu->Check(NFG_VIEW_SUPPORTS, false);
  viewMenu->AppendSeparator();
  viewMenu->Append(NFG_VIEW_DOMINANCE, "&Dominance",
		   "Display dominance information", TRUE);
  viewMenu->Append(NFG_VIEW_PROBABILITIES, "&Probabilities",
		   "Display solution probabilities", TRUE);
  viewMenu->Append(NFG_VIEW_VALUES, "&Values",
		   "Display strategy values", TRUE);
  viewMenu->AppendSeparator();
  // This probably belongs in formatting instead
  viewMenu->Append(NFG_VIEW_OUTCOME_LABELS, "Outcome &Labels",
		   "Display outcome labels", TRUE);
  
  wxMenu *formatMenu = new wxMenu;
  wxMenu *formatDisplayMenu = new wxMenu;
  formatDisplayMenu->Append(NFG_FORMAT_DISPLAY_COLUMNS, "&Column Width",
			   "Set column width");
  formatDisplayMenu->Append(NFG_FORMAT_DISPLAY_DECIMALS, "&Decimal Places",
			   "Set number of decimal places to display");

  formatMenu->Append(NFG_FORMAT_DISPLAY, "&Display", formatDisplayMenu,
		    "Configure display options");
  wxMenu *formatFontMenu = new wxMenu;
  formatFontMenu->Append(NFG_FORMAT_FONT_DATA, "&Data", "Set data font");
  formatFontMenu->Append(NFG_FORMAT_FONT_LABELS, "&Labels", "Set label font");
  formatMenu->Append(NFG_FORMAT_FONT, "&Font", formatFontMenu, "Set fonts");
  formatMenu->Append(NFG_FORMAT_COLORS, "&Colors", "Set player colors");
  formatMenu->AppendSeparator();
  formatMenu->Append(NFG_FORMAT_SAVE, "&Save", "Save current configuration");
  formatMenu->Append(NFG_FORMAT_LOAD, "&Load", "Load configuration");

  wxMenu *toolsMenu = new wxMenu;
  toolsMenu->Append(NFG_TOOLS_DOMINANCE, "&Dominance",
		       "Find undominated strategies");
  toolsMenu->Append(NFG_TOOLS_EQUILIBRIUM, "&Equilibrium",
		    "Compute Nash equilibria (and refinements)");
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");
  helpMenu->Append(wxID_HELP_INDEX, "&Index", "Index of help file");
  helpMenu->AppendSeparator();
  helpMenu->Append(wxID_ABOUT, "&About", "About Gambit");

  wxMenuBar *menuBar = new wxMenuBar(wxMB_DOCKABLE);
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(formatMenu, "&Format");
  menuBar->Append(toolsMenu, "&Tools");
  menuBar->Append(helpMenu, "&Help");

  SetMenuBar(menuBar);
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

  toolBar->AddTool(wxID_HELP_CONTENTS, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, "Help", "Table of contents");

  toolBar->Realize();
  toolBar->SetRows(1);
}

void NfgShow::UpdateMenus(void)
{
  wxMenuBar *menu = GetMenuBar();
  gArray<int> profile(GetContingency());
  menu->Enable(NFG_VIEW_PROBABILITIES, m_profiles.Length() > 0);
  menu->Enable(NFG_VIEW_VALUES, m_profiles.Length() > 0);
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

  Nfg *nfg = 0;
  try {
    gFileOutput file(m_filename.c_str());
    nfg = CompressNfg(m_nfg, *m_currentSupport);
    nfg->WriteNfgFile(file, 6);
    delete nfg;
  }
  catch (gFileOutput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open %s for writing.",
				  m_filename.c_str()),
		 "Error", wxOK, this);
    if (nfg)  delete nfg;
  }
  catch (gFileOutput::WriteFailed &) {
    wxMessageBox(wxString::Format("Write error occurred in saving %s.\n",
				  m_filename.c_str()),
		 "Error", wxOK, this);
    if (nfg)  delete nfg;
  }
  catch (gException &) {
    wxMessageBox("Internal exception in Gambit", "Error", wxOK, this);
    if (nfg)  delete nfg;
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
  wxPrintPreview *preview = new wxPrintPreview(new NfgPrintout(m_table),
					       new NfgPrintout(m_table),
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
  NfgPrintout printout(m_table);

  if (!printer.Print(this, &printout, true)) {
    wxMessageBox("There was an error in printing", "Error", wxOK);
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
  dialogStrategies dialog(this, m_nfg);

  if (dialog.ShowModal() == wxID_OK) {
    for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
      NFPlayer *player = m_nfg.Players()[pl];
      for (int st = 1; st <= player->NumStrats(); st++) {
	player->Strategies()[st]->SetName(dialog.GetStrategyName(pl, st));
      }
    }
    m_table->RefreshTable();
  }
}

void NfgShow::OnEditContingency(wxCommandEvent &)
{
  dialogEditContingency dialog(this, m_nfg, GetContingency());

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.GetOutcome() == 0) { 
      m_nfg.SetOutcome(GetContingency(), 0);
    }
    else {
      m_nfg.SetOutcome(GetContingency(),
		       m_nfg.Outcomes()[dialog.GetOutcome()]);
    }
    m_table->RefreshTable();
  }
}

void NfgShow::OnEditGame(wxCommandEvent &)
{
  dialogNfgProperties dialog(this, m_nfg, m_filename);
  if (dialog.ShowModal() == wxID_OK) {
    m_nfg.SetTitle(dialog.GetGameTitle().c_str());
    SetFilename(Filename());
    m_nfg.SetComment(dialog.GetComment().c_str());
    for (int pl = 1; pl <= dialog.NumPlayers(); pl++) {
      m_nfg.Players()[pl]->SetName(dialog.GetPlayerName(pl).c_str());
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
    GetMenuBar()->Check(NFG_VIEW_PROFILES, false);
  }
  else {
    m_profileTable->Show(true);
    m_solutionSashWindow->Show(true);
    GetMenuBar()->Check(NFG_VIEW_PROFILES, true);
  }

  AdjustSizes();
}

void NfgShow::OnViewNavigation(wxCommandEvent &)
{
  if (m_infoSashWindow->IsShown() && m_infoNotebook->GetSelection() != 0) {
    m_infoNotebook->SetSelection(0);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(NFG_VIEW_NAVIGATION, true);
    GetMenuBar()->Check(NFG_VIEW_OUTCOMES, false);
    GetMenuBar()->Check(NFG_VIEW_SUPPORTS, false);
  }
  else if (m_infoSashWindow->IsShown()) {
    m_infoSashWindow->Show(false);
    GetMenuBar()->Check(NFG_VIEW_NAVIGATION, false);
  }
  else {
    m_infoSashWindow->Show(true);
    m_infoNotebook->SetSelection(0);
    GetMenuBar()->Check(NFG_VIEW_NAVIGATION, true);
  }

  AdjustSizes();
}

void NfgShow::OnViewOutcomes(wxCommandEvent &)
{
  if (m_infoSashWindow->IsShown() && m_infoNotebook->GetSelection() != 1) {
    m_infoNotebook->SetSelection(1);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(NFG_VIEW_OUTCOMES, true);
    GetMenuBar()->Check(NFG_VIEW_NAVIGATION, false);
    GetMenuBar()->Check(NFG_VIEW_SUPPORTS, false);
  }
  else if (m_infoSashWindow->IsShown()) {
    m_infoSashWindow->Show(false);
    GetMenuBar()->Check(NFG_VIEW_OUTCOMES, false);
  }
  else {
    m_infoSashWindow->Show(true);
    m_infoNotebook->SetSelection(1);
    GetMenuBar()->Check(NFG_VIEW_OUTCOMES, true);
  }

  AdjustSizes();
}

void NfgShow::OnViewSupports(wxCommandEvent &)
{
  if (m_infoSashWindow->IsShown() && m_infoNotebook->GetSelection() != 2) {
    m_infoNotebook->SetSelection(2);
    m_navigateWindow->Show(true);
    GetMenuBar()->Check(NFG_VIEW_OUTCOMES, false);
    GetMenuBar()->Check(NFG_VIEW_NAVIGATION, false);
    GetMenuBar()->Check(NFG_VIEW_SUPPORTS, true);
  }
  else if (m_infoSashWindow->IsShown()) {
    m_infoSashWindow->Show(false);
    GetMenuBar()->Check(NFG_VIEW_SUPPORTS, false);
  }
  else {
    m_infoSashWindow->Show(true);
    m_infoNotebook->SetSelection(2);
    GetMenuBar()->Check(NFG_VIEW_SUPPORTS, true);
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

void NfgShow::OnFormatDisplayColumns(wxCommandEvent &)
{
  dialogSpinCtrl dialog(this, "Column width", 0, 100, 20);

  if (dialog.ShowModal() == wxID_OK) {
    //    for (int i = 1; i <= m_currentSupport->NumStrats(m_); i++) {
    //      m_table->SetColumnWidth(i - 1, dialog.GetValue());
    //    }
  }
}

void NfgShow::OnFormatDisplayDecimals(wxCommandEvent &)
{
  dialogSpinCtrl dialog(this, "Decimal places", 0, 25, m_table->GetDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_table->SetDecimals(dialog.GetValue());
    m_table->Refresh();
  }
}

void NfgShow::OnFormatFontData(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    //    m_drawSettings.SetDataFont(dialog.GetFontData().GetChosenFont());
    m_table->SetCellFont(dialog.GetFontData().GetChosenFont());
    m_table->Refresh();
  }
}

void NfgShow::OnFormatFontLabels(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    // m_drawSettings.SetLabelFont(dialog.GetFontData().GetChosenFont());
    m_table->SetLabelFont(dialog.GetFontData().GetChosenFont());
    m_table->Refresh();
  }
}

void NfgShow::OnFormatColors(wxCommandEvent &)
{
}

void NfgShow::OnFormatSave(wxCommandEvent &)
{
  m_table->SaveSettings();
}

void NfgShow::OnFormatLoad(wxCommandEvent &)
{
  m_table->LoadSettings();
}

//----------------------------------------------------------------------
//            NfgShow: Menu handlers - Tools->Dominance
//----------------------------------------------------------------------

void NfgShow::OnToolsDominance(wxCommandEvent &)
{
  gArray<gText> playerNames(m_nfg.NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++) {
    playerNames[pl] = m_nfg.Players()[pl]->GetName();
  }
  dialogElimMixed dialog(this, playerNames);

  if (dialog.ShowModal() == wxID_OK) {
    NFSupport *sup = m_currentSupport;
    wxStatus status(this, "Dominance Elimination");

    try {
      if (!dialog.DomMixed()) {
	if (dialog.Iterative()) {
	  while ((sup = sup->Undominated(dialog.DomStrong(), 
					 dialog.Players(), gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    m_supports.Append(sup);
	  }
	}
	else {
	  if ((sup = sup->Undominated(dialog.DomStrong(), 
				      dialog.Players(), gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    m_supports.Append(sup);
	  }
	}
      }
      else {
	if (dialog.Iterative()) {
	  while ((sup = sup->MixedUndominated(dialog.DomStrong(), precRATIONAL,
					      dialog.Players(),
					      gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    m_supports.Append(sup);
	  }
	}
	else {
	  if ((sup = sup->MixedUndominated(dialog.DomStrong(), precRATIONAL,
					   dialog.Players(),
					   gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    m_supports.Append(sup);
	  }
	}
      }
    }
    catch (gSignalBreak &) { }

    if (m_currentSupport != sup) {
      m_currentSupport = m_supports[m_supports.Length()];
      if (!m_table->ShowDominance()) {
	m_table->ToggleDominance();
	GetMenuBar()->Check(NFG_VIEW_DOMINANCE, true);
      }
      OnSupportsEdited();
      UpdateMenus();
    }
  }
}

//----------------------------------------------------------------------
//          NfgShow: Menu handlers - Tools->Equilibrium menu
//----------------------------------------------------------------------

void NfgShow::OnToolsEquilibrium(wxCommandEvent &)
{ 
  dialogNfgNash dialog(this, *m_currentSupport);

  if (dialog.ShowModal() == wxID_OK) {
    nfgNashAlgorithm *algorithm = dialog.GetAlgorithm();

    if (!algorithm) {
      return;
    }

    try {
      wxStatus status(this, algorithm->GetAlgorithm() + "Solve Progress");
      gList<MixedSolution> solutions;
      solutions = algorithm->Solve(*m_currentSupport, status);

      for (int soln = 1; soln <= solutions.Length(); soln++) {
	AddProfile(solutions[soln], true);
      }
      ChangeProfile(m_profiles.Length());
   
      if (solutions.Length() > 0 && !m_table->ShowProbs()) {
	m_table->ToggleProbs();
	GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, true);
      }
      if (!m_solutionSashWindow->IsShown()) {
	m_profileTable->Show(true);
	m_solutionSashWindow->Show(true);
	GetMenuBar()->Check(NFG_VIEW_PROFILES, true);
	AdjustSizes();
      }
      
      UpdateMenus();
    }
    catch (...) { }

    delete algorithm;
  }
}

//----------------------------------------------------------------------
//                EfgShow: Menu handlers - Help menu
//----------------------------------------------------------------------

void NfgShow::OnHelpContents(wxCommandEvent &)
{
  wxGetApp().OnHelpContents();
}

void NfgShow::OnHelpIndex(wxCommandEvent &)
{
  wxGetApp().OnHelpIndex();
}

void NfgShow::OnHelpAbout(wxCommandEvent &)
{
  wxGetApp().OnHelpAbout(this);
}

//----------------------------------------------------------------------
//               NfgShow: Menu handlers - Support menu
//----------------------------------------------------------------------

void NfgShow::OnSupportDuplicate(wxCommandEvent &)
{
  NFSupport *newSupport = new NFSupport(*m_currentSupport);
  newSupport->SetName(UniqueSupportName());
  m_supports.Append(newSupport);
  m_currentSupport = newSupport;
  OnSupportsEdited();
}

void NfgShow::OnSupportDelete(wxCommandEvent &)
{
  delete m_supports.Remove(m_supports.Find(m_currentSupport));
  m_currentSupport = m_supports[1];
  OnSupportsEdited();
}

//----------------------------------------------------------------------
//              NfgShow: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void NfgShow::OnProfilesNew(wxCommandEvent &)
{
  MixedSolution profile = MixedProfile<gNumber>(NFSupport(m_nfg));

  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddProfile(dialog.GetProfile(), true);
    ChangeProfile(m_profiles.Length());
    UpdateMenus();
  }
}

void NfgShow::OnProfilesDuplicate(wxCommandEvent &)
{
  MixedSolution profile(m_profiles[m_currentProfile]);
  
  dialogEditMixed dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddProfile(dialog.GetProfile(), true);
    ChangeProfile(m_profiles.Length());
    UpdateMenus();
  }
}

void NfgShow::OnProfilesDelete(wxCommandEvent &)
{
  m_profiles.Remove(m_currentProfile);
  m_currentProfile = (m_profiles.Length() > 0) ? 1 : 0;
  ChangeProfile(m_currentProfile);
  UpdateMenus();
}

void NfgShow::OnProfilesProperties(wxCommandEvent &)
{
  if (m_currentProfile > 0) {
    dialogEditMixed dialog(this, m_profiles[m_currentProfile]);

    if (dialog.ShowModal() == wxID_OK) {
      m_profiles[m_currentProfile] = dialog.GetProfile();
      ChangeProfile(m_currentProfile);
    }
  }
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
  GetMenuBar()->Check(NFG_VIEW_NAVIGATION, p_event.GetSelection() == 0);
  GetMenuBar()->Check(NFG_VIEW_OUTCOMES, p_event.GetSelection() == 1);
  GetMenuBar()->Check(NFG_VIEW_SUPPORTS, p_event.GetSelection() == 2);
}

//----------------------------------------------------------------------
//                   NfgShow: Miscellaneous members
//----------------------------------------------------------------------

gText NfgShow::UniqueSupportName(void) const
{
  int number = m_supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_supports.Length(); i++) {
      if (m_supports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

void NfgShow::SetFilename(const wxString &p_name)
{
  m_filename = p_name;
  if (m_filename != "") {
    SetTitle(wxString::Format("Gambit - [%s] %s", m_filename.c_str(), 
			      (char *) m_nfg.GetTitle()));
  }
  else {
    SetTitle(wxString::Format("Gambit - %s", (char *) m_nfg.GetTitle()));
  }
  wxGetApp().SetFilename(this, p_name.c_str());
}

void NfgShow::SetSupportNumber(int p_number)
{
  if (p_number >= 1 && p_number <= m_supports.Length()) {
    m_currentSupport = m_supports[p_number];
    OnSupportsEdited();
  }
}

void NfgShow::OnSupportsEdited(void)
{
  m_table->SetSupport(*m_currentSupport);
  m_supportWindow->UpdateValues();
}

#include "base/glist.imp"
template class gList<NFSupport *>;

