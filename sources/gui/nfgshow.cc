//
// FILE: nfgshow.cc -- Implementation of NfgShow class
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/grid.h"
#include "wx/fontdlg.h"
#include "wx/wizard.h"

#include "guishare/wxstatus.h"
#include "nfgshow.h"
#include "nfgtable.h"
#include "nfgsoln.h"
#include "nfgprint.h"
#include "nfgprofile.h"
#include "nfgnavigate.h"
#include "nfgoutcome.h"
#include "nfgsupport.h"
#include "mixededit.h"
#include "nfgconst.h"

#include "efg.h"
#include "efgutils.h"
#include "behavsol.h"
#include "nfplayer.h"
#include "nfdom.h"
#include "nfgciter.h"
#include "yamamoto.h"

#include "nfgsolvd.h"
#include "nfgsolng.h"

#include "gambit.h"
#include "efgshow.h"
#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgeditsupport.h"
#include "dlnfgplayers.h"
#include "dlstrategies.h"

#include "algenumpure.h"
#include "dlqre.h"
#include "nfgqre.h"

#include "dlelim.h"
#include "dlsupportselect.h"

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
  EVT_MENU(NFG_EDIT_LABEL, NfgShow::OnEditLabel)
  EVT_MENU(NFG_EDIT_PLAYERS, NfgShow::OnEditPlayers)
  EVT_MENU(NFG_EDIT_STRATS, NfgShow::OnEditStrategies)
  EVT_MENU(NFG_EDIT_OUTCOMES_NEW, NfgShow::OnEditOutcomeNew)
  EVT_MENU(NFG_EDIT_OUTCOMES_DELETE, NfgShow::OnEditOutcomeDelete)
  EVT_MENU(NFG_EDIT_OUTCOMES_ATTACH, NfgShow::OnEditOutcomeAttach)
  EVT_MENU(NFG_EDIT_OUTCOMES_DETACH, NfgShow::OnEditOutcomeDetach)
  EVT_MENU(NFG_EDIT_OUTCOMES_PAYOFFS, NfgShow::OnEditOutcomePayoffs)
  EVT_MENU(NFG_VIEW_PROFILES, NfgShow::OnViewProfiles)
  EVT_MENU(NFG_VIEW_NAVIGATION, NfgShow::OnViewNavigation)
  EVT_MENU(NFG_VIEW_OUTCOMES, NfgShow::OnViewOutcomes)
  EVT_MENU(NFG_VIEW_SUPPORTS, NfgShow::OnViewSupports)
  EVT_MENU(NFG_VIEW_DOMINANCE, NfgShow::OnViewDominance)
  EVT_MENU(NFG_VIEW_PROBABILITIES, NfgShow::OnViewProbabilities)
  EVT_MENU(NFG_VIEW_VALUES, NfgShow::OnViewValues)
  EVT_MENU(NFG_VIEW_OUTCOME_LABELS, NfgShow::OnViewOutcomeLabels)
  EVT_MENU(NFG_VIEW_GAMEINFO, NfgShow::OnViewGameInfo)
  EVT_MENU(NFG_FORMAT_DISPLAY_COLUMNS, NfgShow::OnFormatDisplayColumns)
  EVT_MENU(NFG_FORMAT_DISPLAY_DECIMALS, NfgShow::OnFormatDisplayDecimals)
  EVT_MENU(NFG_FORMAT_FONT_DATA, NfgShow::OnFormatFontData)
  EVT_MENU(NFG_FORMAT_FONT_LABELS, NfgShow::OnFormatFontLabels)
  EVT_MENU(NFG_FORMAT_COLORS, NfgShow::OnFormatColors)
  EVT_MENU(NFG_FORMAT_SAVE, NfgShow::OnFormatSave)
  EVT_MENU(NFG_FORMAT_LOAD, NfgShow::OnFormatLoad)
  EVT_MENU(NFG_TOOLS_SUPPORT_UNDOMINATED, NfgShow::OnToolsSupportUndominated)
  EVT_MENU(NFG_TOOLS_SUPPORT_NEW, NfgShow::OnToolsSupportNew)
  EVT_MENU(NFG_TOOLS_SUPPORT_EDIT, NfgShow::OnToolsSupportEdit)
  EVT_MENU(NFG_TOOLS_SUPPORT_DELETE, NfgShow::OnToolsSupportDelete)
  EVT_MENU(NFG_TOOLS_SUPPORT_SELECT_FROMLIST, 
	   NfgShow::OnToolsSupportSelectFromList)
  EVT_MENU(NFG_TOOLS_SUPPORT_SELECT_PREVIOUS,
	   NfgShow::OnToolsSupportSelectPrevious)
  EVT_MENU(NFG_TOOLS_SUPPORT_SELECT_NEXT, NfgShow::OnToolsSupportSelectNext)
  EVT_MENU(NFG_TOOLS_EQUILIBRIUM_STANDARD, NfgShow::OnToolsEquilibriumStandard)
  EVT_MENU(NFG_TOOLS_EQUILIBRIUM_CUSTOM_ENUMPURE,
	   NfgShow::OnToolsEquilibriumCustomEnumPure)
  EVT_MENU(NFG_TOOLS_EQUILIBRIUM_CUSTOM_QRE,
	   NfgShow::OnToolsEquilibriumCustomQre)
  EVT_MENU(NFG_TOOLS_EQUILIBRIUM_CUSTOM_YAMAMOTO,
	   NfgShow::OnToolsEquilibriumCustomYamamoto)
  EVT_MENU_RANGE(NFG_TOOLS_EQUILIBRIUM_CUSTOM_ENUMPURE,
		 NFG_TOOLS_EQUILIBRIUM_CUSTOM_QREGRID,
		 NfgShow::OnToolsEquilibriumCustom)
  EVT_MENU(wxID_HELP_CONTENTS, NfgShow::OnHelpContents)
  EVT_MENU(wxID_HELP_INDEX, NfgShow::OnHelpIndex)
  EVT_MENU(wxID_ABOUT, NfgShow::OnHelpAbout)
  EVT_MENU(NFG_PROFILES_NEW, NfgShow::OnProfilesNew)
  EVT_MENU(NFG_PROFILES_CLONE, NfgShow::OnProfilesClone)
  EVT_MENU(NFG_PROFILES_RENAME, NfgShow::OnProfilesRename)
  EVT_MENU(NFG_PROFILES_EDIT, NfgShow::OnProfilesEdit)
  EVT_LIST_ITEM_ACTIVATED(idNFG_SOLUTION_LIST, NfgShow::OnProfilesEdit)
  EVT_MENU(NFG_PROFILES_DELETE, NfgShow::OnProfilesDelete)
  EVT_SIZE(NfgShow::OnSize)
  EVT_CLOSE(NfgShow::OnCloseWindow)
  EVT_SASH_DRAGGED(idSOLUTIONWINDOW, NfgShow::OnSashDrag)
  EVT_SET_FOCUS(NfgShow::OnSetFocus)
  EVT_LIST_ITEM_SELECTED(idNFG_SOLUTION_LIST, NfgShow::OnSolutionSelected)
  EVT_NOTEBOOK_PAGE_CHANGED(idINFONOTEBOOK, NfgShow::OnInfoNotebookPage)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//               NfgShow: Constructor and destructor
//----------------------------------------------------------------------

NfgShow::NfgShow(Nfg &p_nfg, wxWindow *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(500, 500)),
    m_nfg(p_nfg),
    m_table(0), m_solutionTable(0),
    m_solutionSashWindow(0), m_infoSashWindow(0),
    m_navigateWindow(0), m_outcomeWindow(0), m_supportWindow(0)
{
#ifdef __WXMSW__
  SetIcon(wxIcon("nfg_icn"));
#else
#include "nfg.xbm"
  SetIcon(wxIcon(nfg_bits, nfg_width, nfg_height));
#endif  // __WXMSW__

  m_currentSolution = 0;
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

  CreateStatusBar(3);
  MakeToolbar();

  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_solutionTable = new NfgProfileList(this, m_solutionSashWindow);
  m_solutionTable->Show(true);
  m_solutionSashWindow->Show(false);

  m_infoSashWindow = new wxSashWindow(this, idINFOWINDOW,
				      wxPoint(0, 40), wxSize(200, 200),
				      wxNO_BORDER | wxSW_3D);
  m_infoSashWindow->SetSashVisible(wxSASH_RIGHT, true);
  m_infoSashWindow->Show(true);
  m_infoSashWindow->SetSashVisible(wxSASH_LEFT, false);

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

void NfgShow::AddSolution(const MixedSolution &p_profile, bool p_map)
{
  m_solutionTable->Append(p_profile);
  if (m_nfg.AssociatedEfg() && p_map) {
    wxGetApp().GetWindow(m_nfg.AssociatedEfg())->AddProfile(BehavProfile<gNumber>(p_profile), false);
  }
  m_solutionTable->UpdateValues();
  UpdateMenus();
}

void NfgShow::ChangeSolution(int sol)
{
  m_currentSolution = sol;

  if (sol > 0) {
    m_table->SetSolution((*m_solutionTable)[m_currentSolution]);
  }
  if (m_solutionTable) {
    m_solutionTable->UpdateValues();
  }
}

void NfgShow::OnSolutionSelected(wxListEvent &p_event)
{
  m_currentSolution = p_event.m_itemIndex + 1;
  m_table->SetSolution((*m_solutionTable)[m_currentSolution]);
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
  m_table->SetProfile(p_profile);
}

gArray<int> NfgShow::GetProfile(void) const
{
  return m_navigateWindow->GetProfile();
}

void NfgShow::UpdateProfile(gArray<int> &profile)
{
  //  m_table->OnChangeValues();
}

void NfgShow::OnOutcomesEdited(void)
{
  m_table->Refresh();
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
  editMenu->Append(NFG_EDIT_LABEL, "&Label", "Set the label of the game");
  editMenu->Append(NFG_EDIT_PLAYERS, "&Players", "Edit player names");
  editMenu->Append(NFG_EDIT_STRATS, "&Strategies", "Edit strategy names");

  wxMenu *editOutcomesMenu = new wxMenu;
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_NEW, "&New",
			   "Create a new outcome");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_DELETE, "Dele&te",
			   "Delete an outcome");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_ATTACH, "&Attach",
			   "Attach an outcome to the current contingency");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_DETACH, "&Detach",
			   "Set the outcome for the current contingency to null");
  editOutcomesMenu->Append(NFG_EDIT_OUTCOMES_PAYOFFS, "&Payoffs",
			   "Set the payoffs for outcome of the current contingency");
  editMenu->Append(NFG_EDIT_OUTCOMES,  "&Outcomes",  editOutcomesMenu,
		    "Set/Edit outcomes");

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(NFG_VIEW_PROFILES, "&Profiles",
		   "Display/hide profiles window", true);
  viewMenu->AppendSeparator();
  viewMenu->Append(NFG_VIEW_NAVIGATION, "&Navigation",
		   "Display navigation window", true);
  viewMenu->Check(NFG_VIEW_NAVIGATION, true);
  viewMenu->Append(NFG_VIEW_OUTCOMES, "&Outcomes", 
		   "Display and edit outcomes", true);
  viewMenu->Check(NFG_VIEW_OUTCOMES, false);
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
  viewMenu->Append(NFG_VIEW_GAMEINFO, "Game&Info",
		   "Display information about the game");
  
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
  wxMenu *supportsMenu = new wxMenu;
  supportsMenu->Append(NFG_TOOLS_SUPPORT_UNDOMINATED, "&Undominated",
		       "Find undominated strategies");
  supportsMenu->Append(NFG_TOOLS_SUPPORT_NEW, "&New",
		       "Create a new support");
  supportsMenu->Append(NFG_TOOLS_SUPPORT_EDIT, "&Edit",
		       "Edit the currently displayed support");
  supportsMenu->Append(NFG_TOOLS_SUPPORT_DELETE, "&Delete",
		       "Delete a support");
  wxMenu *supportsSelectMenu = new wxMenu;
  supportsSelectMenu->Append(NFG_TOOLS_SUPPORT_SELECT_FROMLIST,
			     "From &List...",
			     "Select a support from the list of defined supports");
  supportsSelectMenu->Append(NFG_TOOLS_SUPPORT_SELECT_PREVIOUS, "&Previous",
			     "Select the previous support from the list");
  supportsSelectMenu->Append(NFG_TOOLS_SUPPORT_SELECT_NEXT, "&Next",
			     "Select the next support from the list");
  supportsMenu->Append(NFG_TOOLS_SUPPORT_SELECT, "&Select", supportsSelectMenu,
		       "Change the current support");
  toolsMenu->Append(NFG_TOOLS_SUPPORT, "&Support", supportsMenu,
		    "Manipulate supports");

  wxMenu *solveMenu = new wxMenu;
  solveMenu->Append(NFG_TOOLS_EQUILIBRIUM_STANDARD,  "S&tandard...",
		    "Standard solutions");
  
  wxMenu *solveCustomMenu = new wxMenu;
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_ENUMPURE, "EnumPure",
			  "Enumerate pure strategy equilibria");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_ENUMMIXED, "EnumMixed",
			  "Enumerate mixed strategy equilibria");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_LCP, "LCP",
			  "Solve via linear complementarity program");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_LP, "LP",
			  "Solve via linear program");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_LIAP, "Liapunov",
			  "Minimization of liapunov function");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_SIMPDIV, "Simpdiv",
			  "Solve via simplicial subdivision");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_POLENUM, "PolEnum",
			  "Enumeration by systems of polynomials");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_QRE, "QRE",
			  "Compute quantal response equilibrium");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_QREGRID, "QREGrid",
			  "Compute quantal response equilibrium");
  solveCustomMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM_YAMAMOTO, "Yamamoto",
			  "Compute a proper equilibrium via Yamamoto's algorithm");
  solveMenu->Append(NFG_TOOLS_EQUILIBRIUM_CUSTOM, "Custom", solveCustomMenu,
		    "Solve with a particular algorithm");
  toolsMenu->Append(NFG_TOOLS_EQUILIBRIUM, "&Equilibrium", solveMenu,
		    "Compute (Nash) equilibria");
  
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
  gArray<int> profile(GetProfile());

  menu->Enable(NFG_EDIT_OUTCOMES_DELETE, m_nfg.NumOutcomes() > 0);
  menu->Enable(NFG_EDIT_OUTCOMES_ATTACH, m_nfg.NumOutcomes() > 0);
  menu->Enable(NFG_EDIT_OUTCOMES_DETACH, m_nfg.GetOutcome(profile) != 0);
  menu->Enable(NFG_EDIT_OUTCOMES_PAYOFFS, m_nfg.GetOutcome(profile) != 0);

  menu->Enable(NFG_TOOLS_SUPPORT_SELECT_FROMLIST, NumSupports() > 1);
  menu->Enable(NFG_TOOLS_SUPPORT_SELECT_PREVIOUS, NumSupports() > 1);
  menu->Enable(NFG_TOOLS_SUPPORT_SELECT_NEXT, NumSupports() > 1);

  menu->Enable(NFG_TOOLS_EQUILIBRIUM_CUSTOM_ENUMMIXED, 
	       m_nfg.NumPlayers() == 2);
  menu->Enable(NFG_TOOLS_EQUILIBRIUM_CUSTOM_LP,
	       m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg));
  menu->Enable(NFG_TOOLS_EQUILIBRIUM_CUSTOM_LCP, m_nfg.NumPlayers() == 2);

  menu->Enable(NFG_VIEW_PROBABILITIES, m_solutionTable->Length() > 0);
  menu->Enable(NFG_VIEW_VALUES, m_solutionTable->Length() > 0);

  SetStatusText((char *)
		("Support: " + CurrentSupport()->GetName()), 1);
  if (CurrentSolution() > 0) {
    SetStatusText((char *) ("Solution: " + 
			    ToText(CurrentSolution())),
		  2);
  }
  else {
    SetStatusText("No solution displayed", 2);
  }
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

void NfgShow::OnEditLabel(wxCommandEvent &)
{
  const char *label = wxGetTextFromUser("Label of game", "Label Game",
					(char *) m_nfg.GetTitle());
  if (label) {
    m_nfg.SetTitle(label);
    SetFilename(Filename());
  }
}

void NfgShow::OnEditPlayers(wxCommandEvent &)
{
  dialogNfgPlayers dialog(m_nfg, this);
  dialog.ShowModal();
}

void NfgShow::OnEditStrategies(wxCommandEvent &)
{
  dialogStrategies dialog(m_nfg, this);
  dialog.ShowModal();

  if (dialog.GameChanged()) {
    m_table->Refresh();
  }
}

void NfgShow::OnEditOutcomeAttach(wxCommandEvent &)
{
  if (m_nfg.NumOutcomes() == 0)
    return;

  dialogNfgOutcomeSelect dialog(m_nfg, this);
    
  if (dialog.ShowModal() == wxID_OK) {
    m_nfg.SetOutcome(GetProfile(), dialog.GetOutcome());
    m_table->Refresh();
  }
}

void NfgShow::OnEditOutcomeDetach(wxCommandEvent &)
{
  m_nfg.SetOutcome(GetProfile(), 0);
  m_table->Refresh();
}

void NfgShow::OnEditOutcomeNew(wxCommandEvent &)
{
  dialogNfgPayoffs dialog(m_nfg, 0, this);

  if (dialog.ShowModal() == wxID_OK) {
    NFOutcome *outc = m_nfg.NewOutcome();
    gArray<gNumber> payoffs(dialog.Payoffs());

    for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++)
      m_nfg.SetPayoff(outc, pl, payoffs[pl]);
    outc->SetName(dialog.Name());
  }
}

void NfgShow::OnEditOutcomeDelete(wxCommandEvent &)
{
  if (m_nfg.NumOutcomes() == 0)
    return;

  dialogNfgOutcomeSelect dialog(m_nfg, this);
    
  if (dialog.ShowModal() == wxID_OK) {
    m_nfg.DeleteOutcome(dialog.GetOutcome());
    m_table->Refresh();
  }
}

void NfgShow::OnEditOutcomePayoffs(wxCommandEvent &)
{
  if (m_table->GetRowStrategy() <= m_currentSupport->NumStrats(m_table->GetRowPlayer()) &&
      m_table->GetColStrategy() <= m_currentSupport->NumStrats(m_table->GetColPlayer())) {
    OutcomePayoffs(m_table->GetRowStrategy(),
		   m_table->GetColStrategy(), false);
  }
}

//----------------------------------------------------------------------
//                NfgShow: Menu handlers - View menu
//----------------------------------------------------------------------

void NfgShow::OnViewProfiles(wxCommandEvent &)
{
  if (m_solutionSashWindow->IsShown()) {
    m_solutionTable->Show(false);
    m_solutionSashWindow->Show(false);
    GetMenuBar()->Check(NFG_VIEW_PROFILES, false);
  }
  else {
    m_solutionTable->Show(true);
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
  m_table->Refresh();
}

void NfgShow::OnViewGameInfo(wxCommandEvent &)
{
  gText message = "Number of Players: " + ToText(m_nfg.NumPlayers()) + "\n";
  message += "Is ";
  message += ((IsConstSum(m_nfg)) ? " " : "NOT ");
  message += "constant sum\n";
  message += "Game ";
  message += ((m_nfg.IsDirty()) ? "HAS " : "has not ");
  message += "been modified\n";

  wxMessageBox((char *) message, "Game Information", wxOK, this);
}

//----------------------------------------------------------------------
//               NfgShow: Menu handlers - Format menu
//----------------------------------------------------------------------

void NfgShow::OnFormatDisplayColumns(wxCommandEvent &)
{
  guiSliderDialog dialog(this, "Column width", 0, 100, 20);

  if (dialog.ShowModal() == wxID_OK) {
    //    for (int i = 1; i <= m_currentSupport->NumStrats(m_); i++) {
    //      m_table->SetColumnWidth(i - 1, dialog.GetValue());
    //    }
  }
}

void NfgShow::OnFormatDisplayDecimals(wxCommandEvent &)
{
  guiSliderDialog dialog(this, "Decimal places", 0, 25,
			 m_table->GetDecimals());

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
//            NfgShow: Menu handlers - Tools->Support menu
//----------------------------------------------------------------------

void NfgShow::OnToolsSupportUndominated(wxCommandEvent &)
{
  gArray<gText> playerNames(m_nfg.NumPlayers());
  for (int pl = 1; pl <= playerNames.Length(); pl++)
    playerNames[pl] = m_nfg.Players()[pl]->GetName();
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

void NfgShow::OnToolsSupportNew(wxCommandEvent &)
{
  NFSupport newSupport(m_nfg);
  newSupport.SetName(UniqueSupportName());
  dialogNfgEditSupport dialog(newSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      NFSupport *support = new NFSupport(dialog.Support());
      support->SetName(dialog.Name());
      m_supports.Append(support);

      m_currentSupport = support;
      OnSupportsEdited();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnToolsSupportEdit(wxCommandEvent &)
{
  dialogNfgEditSupport dialog(*m_currentSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      *m_currentSupport = dialog.Support();
      m_currentSupport->SetName(dialog.Name());
      OnSupportsEdited();
      m_table->Refresh();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnToolsSupportDelete(wxCommandEvent &)
{
  if (m_supports.Length() == 1)  return;

  dialogSupportSelect dialog(this, m_supports,
			     m_currentSupport, "Delete Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      delete m_supports.Remove(dialog.Selected());
      if (!m_supports.Find(m_currentSupport)) {
	m_currentSupport = m_supports[1];
      }
      OnSupportsEdited();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnToolsSupportSelectFromList(wxCommandEvent &)
{
  dialogSupportSelect dialog(this, m_supports,
			     m_currentSupport, "Select Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_currentSupport = m_supports[dialog.Selected()];
      OnSupportsEdited();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnToolsSupportSelectPrevious(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == 1) {
    m_currentSupport = m_supports[m_supports.Length()];
  }
  else {
    m_currentSupport = m_supports[index - 1];
  }
  OnSupportsEdited();
}

void NfgShow::OnToolsSupportSelectNext(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == m_supports.Length()) {
    m_currentSupport = m_supports[1];
  }
  else {
    m_currentSupport = m_supports[index + 1];
  }
  OnSupportsEdited();
}

//----------------------------------------------------------------------
//          NfgShow: Menu handlers - Tools->Equilibrium menu
//----------------------------------------------------------------------

void NfgShow::OnToolsEquilibriumStandard(wxCommandEvent &)
{ 
  dialogNfgSolveStandard dialog(this, m_nfg);
  
  if (dialog.ShowModal() != wxID_OK)
    return;

  int old_max_soln = m_solutionTable->Length();  // used for extensive update

  guiNfgSolution *solver = 0;

  switch (dialog.Number()) {
  case nfgSTANDARD_ONE:
    if (dialog.Type() == nfgSTANDARD_NASH) {
      if (m_nfg.NumPlayers() == 2) {
	if (IsConstSum(m_nfg))
	  solver = new guinfgLp(this, 1, dialog.Precision(), true);
	else
	  solver = new guinfgLcp(this, 1, dialog.Precision(), true);
      }
      else
	solver = new guinfgSimpdiv(this, 1, dialog.Precision(), true);
    }
    else {  // nfgSTANDARD_PERFECT
      if (m_nfg.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(this, 1, dialog.Precision(), true);
      }
      else {
	wxMessageBox("One-Perfect only implemented for 2-player games",
		     "Standard Solution");
	return;
      }
    }
    break;

  case nfgSTANDARD_TWO:
    if (dialog.Type() == nfgSTANDARD_NASH) {
      if (m_nfg.NumPlayers() == 2)
	solver = new guinfgEnumMixed(this, 2, dialog.Precision(), false);
      else
	solver = new guinfgLiap(this, 2, 10, false);
    }
    else {  // nfgSTANDARD_PERFECT
      if (m_nfg.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(this, 2, dialog.Precision(), true);
	wxMessageBox("Not guaranteed to find 2 solutions", "Warning");
      }
      else {
	wxMessageBox("Two-Perfect not implemented", "Standard Solution");
	return;
      }
    }
    break;

  case nfgSTANDARD_ALL:
    if (dialog.Type() == nfgSTANDARD_NASH) {
      if (m_nfg.NumPlayers() == 2)
	solver = new guinfgEnumMixed(this, 0, dialog.Precision(), false);
      else {
	solver = new guinfgPolEnum(this, 0, false);
      }
    }
    else {  // nfgSTANDARD_PERFECT
      if (m_nfg.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(this, 0, dialog.Precision(), true);
	wxMessageBox("Not guaranteed to find all solutions", "Warning");
      }
      else {
	wxMessageBox("All-Perfect only implemented for 2-player games",
		     "Standard Solution");
	return;
      }
    }
    break;
  }

  wxBeginBusyCursor();

  try {
    NFSupport support = solver->Eliminate(*m_currentSupport);
    gList<MixedSolution> solutions = solver->Solve(support);
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddSolution(solutions[soln], true);
    }
    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), this);
    wxEndBusyCursor();
  }
    
  delete solver;

  if (old_max_soln != m_solutionTable->Length()) {
    if (!m_table->ShowProbs()) {
      m_table->ToggleProbs();
      GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, true);
    }

    ChangeSolution(m_solutionTable->Length());
  }  

  UpdateMenus();
}

void NfgShow::OnToolsEquilibriumCustom(wxCommandEvent &p_event)
{
  int id = p_event.GetId();

  guiNfgSolution *solver;

  switch (id) {
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_ENUMMIXED:
    solver = new guinfgEnumMixed(this);
    break;
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_LCP:      
    solver = new guinfgLcp(this);
    break;
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_LP:       
    solver = new guinfgLp(this);
    break;
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_LIAP:
    solver = new guinfgLiap(this);
    break;
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_SIMPDIV:
    solver = new guinfgSimpdiv(this);
    break;
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_POLENUM:
    solver = new guinfgPolEnum(this);
    break;
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_QRE:
    solver = new guinfgQre(this);
    break;
  case NFG_TOOLS_EQUILIBRIUM_CUSTOM_QREGRID:
    solver = new guinfgQreAll(this);
    break;
  default:
    // shouldn't happen.  we'll ignore silently
    return;
  }

  if (!solver->SolveSetup()) {
    delete solver;
    return;
  }

  wxBeginBusyCursor();

  try {
    NFSupport support = solver->Eliminate(*m_currentSupport);
    gList<MixedSolution> solutions = solver->Solve(support);
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddSolution(solutions[soln], true);
    }
    ChangeSolution(m_solutionTable->Length());

    if (solutions.Length() > 0 && !m_table->ShowProbs()) {
      m_table->ToggleProbs();
      GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, true);
    }

    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), this);
    wxEndBusyCursor();
  }
    
  delete solver;
  UpdateMenus();
}

void NfgShow::OnToolsEquilibriumCustomEnumPure(wxCommandEvent &)
{
  gList<MixedSolution> solutions;
  if (EnumPureNfg(this, *m_currentSupport, solutions)) {
    for (int soln = 1; soln <= solutions.Length(); soln++) {
      AddSolution(solutions[soln], true);
    }
    ChangeSolution(m_solutionTable->Length());

    if (solutions.Length() > 0 && !m_table->ShowProbs()) {
      m_table->ToggleProbs();
      GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, true);
    }

    UpdateMenus();
  }
}

void NfgShow::OnToolsEquilibriumCustomQre(wxCommandEvent &)
{
  dialogQre dialog(this);

  if (dialog.ShowModal() == wxID_OK) {
    gOutput *pxifile = &gnull;
    if (dialog.GeneratePXIFile()) {
      wxFileDialog fileDialog(this, "Choose file for PXI output",
			      wxPathOnly(m_filename),
			      "", "*.pxi",
			      wxSAVE | wxOVERWRITE_PROMPT);
      if (fileDialog.ShowModal() != wxID_OK) {
	return;
      }

      try {
	pxifile = new gFileOutput(fileDialog.GetPath().c_str());
      }
      catch (gFileOutput::OpenFailed &ex) {
	wxMessageBox(wxString::Format("Could not open file '%s' for writing.",
				      fileDialog.GetPath().c_str()),
		     "Error", wxOK, this);
	return;
      }
    }

    Correspondence<double, MixedSolution> solutions;

    try {
      QreNfg qre;
      qre.SetMaxLambda(dialog.MaxLambda());
      qre.SetStepSize(dialog.StepSize());

      wxStatus status(this, "QreSolve Progress");
      wxBusyCursor cursor;
      qre.Solve(m_nfg, *pxifile, status, solutions);
    }
    catch (gSignalBreak &) {
      guiExceptionDialog("Algorithm canceled by user", this);
    }
    catch (gException &ex) {
      guiExceptionDialog(ex.Description(), this);
      if (pxifile != &gnull) {
	delete pxifile;
      }
      return;
    }

    if (pxifile != &gnull) {
      delete pxifile;
    }

    if (solutions.NumPoints(1) > 0) {
      for (int soln = 1; soln <= solutions.NumPoints(1); soln++) {
	AddSolution(solutions.GetPoint(1, soln), true);
      }
      ChangeSolution(m_solutionTable->Length());
      UpdateMenus();
      if (!m_solutionSashWindow->IsShown())  {
	m_solutionTable->Show(true);
	m_solutionSashWindow->Show(true);
	GetMenuBar()->Check(efgmenuVIEW_PROFILES, true);
	AdjustSizes();
      }
    }
    else {
      wxMessageBox("The algorithm returned no solutions.",
		   "Notification", wxOK, this);
    }
  }
}

namespace YamamotoWizard {

class SupportsPage : public wxWizardPageSimple {
public:
  SupportsPage(wxWizard *p_parent);
};

SupportsPage::SupportsPage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{ 
  wxStaticBox *box = new wxStaticBox(this, -1, "Choose Support");
  wxStaticBoxSizer *sizer = new wxStaticBoxSizer(box, wxHORIZONTAL);
					      
  sizer->Add(new wxStaticText(this, -1, 
			      "Yamamoto's algorithm computes a proper\n"
			      "equilibrium.  Since proper equilibria\n"
			      "never involve playing a dominated strategy\n"
			      "with positive probability, the algorithm\n"
			      "automatically will eliminate strictly\n"
			      "dominated strategies."),
	     0, wxEXPAND, 5);

  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(sizer, 0, wxCENTRE | wxALL, 5);
  
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
}

class ParametersPage : public wxWizardPageSimple {
public:
  ParametersPage(wxWizard *p_parent);
};

ParametersPage::ParametersPage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{ 
  wxStaticBox *box = new wxStaticBox(this, -1, "Algorithm Parameters");
  wxStaticBoxSizer *sizer = new wxStaticBoxSizer(box, wxHORIZONTAL);
					      
  sizer->Add(new wxStaticText(this, -1, 
			      "Yamamoto's algorithm currently accepts\n"
			      "no parameters.\n"),
	     0, wxEXPAND, 5);

  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(sizer, 0, wxCENTRE | wxALL, 5);
  
  SetSizer(topSizer); 
  topSizer->Fit(this);
  topSizer->SetSizeHints(this); 
  Layout();
}

}  // YamamotoWizard

void NfgShow::OnToolsEquilibriumCustomYamamoto(wxCommandEvent &)
{
  wxWizard *wizard = wxWizard::Create(this, -1, "Yamamoto's Algorithm Parameters");
  YamamotoWizard::SupportsPage *page1 =
    new YamamotoWizard::SupportsPage(wizard);
  YamamotoWizard::ParametersPage *page2 =
    new YamamotoWizard::ParametersPage(wizard);
  page1->SetNext(page2);
  page2->SetPrev(page1);

  wxBeginBusyCursor();

  if (wizard->RunWizard(page1)) {
    gList<MixedSolution> solutions;
    wxStatus status(this, "YamamotoSolve Progress");
    
    try {
      Yamamoto(NFSupport(m_nfg), status, solutions);
      if (solutions.Length() > 0) {
	AddSolution(solutions[1], true);
      }

      if (solutions.Length() > 0 && !m_table->ShowProbs()) {
	m_table->ToggleProbs();
	GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, true);
	ChangeSolution(m_solutionTable->Length());
	if (!m_solutionSashWindow->IsShown()) {
	  m_solutionTable->Show(true);
	  m_solutionSashWindow->Show(true);
	  GetMenuBar()->Check(NFG_VIEW_PROFILES, true);
	  AdjustSizes();
	}
      }
    }
    catch (...) {

    }
  }

  wxEndBusyCursor();

  UpdateMenus();
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
//              EfgShow: Menu handlers - Profiles menu
//----------------------------------------------------------------------

void NfgShow::OnProfilesNew(wxCommandEvent &)
{
  MixedSolution profile = MixedProfile<gNumber>(NFSupport(m_nfg));

  dialogMixedEditor dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddSolution(dialog.GetProfile(), true);
    ChangeSolution(m_solutionTable->Length());
    UpdateMenus();
  }
}

void NfgShow::OnProfilesClone(wxCommandEvent &)
{
  MixedSolution profile((*m_solutionTable)[m_currentSolution]);
  
  dialogMixedEditor dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    AddSolution(dialog.GetProfile(), true);
    ChangeSolution(m_solutionTable->Length());
    UpdateMenus();
  }
}

void NfgShow::OnProfilesRename(wxCommandEvent &)
{
  if (m_currentSolution > 0) {
    wxTextEntryDialog dialog(this, "Enter new name for profile",
			     "Rename profile",
			     (char *) (*m_solutionTable)[m_currentSolution].GetName());

    if (dialog.ShowModal() == wxID_OK) {
      (*m_solutionTable)[m_currentSolution].SetName(dialog.GetValue().c_str());
      m_solutionTable->UpdateValues();
    }
  }
}

void NfgShow::OnProfilesEdit(wxCommandEvent &)
{
  if (m_currentSolution > 0) {
    dialogMixedEditor dialog(this, (*m_solutionTable)[m_currentSolution]);

    if (dialog.ShowModal() == wxID_OK) {
      (*m_solutionTable)[m_currentSolution] = dialog.GetProfile();
      ChangeSolution(m_currentSolution);
    }
  }
}

void NfgShow::OnProfilesDelete(wxCommandEvent &)
{
  m_solutionTable->Remove(m_currentSolution);
  m_currentSolution = (m_solutionTable->Length() > 0) ? 1 : 0;
  ChangeSolution(m_currentSolution);
  UpdateMenus();
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
      m_table->SetSize(250, 0, width - 250, height);
    }
  }
  else if (m_table) {
    m_table->SetSize(0, 0, width, height);
  }

  if (m_navigateWindow && m_infoSashWindow->IsShown()) {
    m_infoSashWindow->SetSize(0, 0, 250, height);
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
		     clientHeight - p_event.GetDragRect().height - 40);
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

void NfgShow::OutcomePayoffs(int st1, int st2, bool next)
{
  gArray<int> profile(GetProfile());
  profile[m_table->GetRowPlayer()] = st1;
  profile[m_table->GetColPlayer()] = st2;

  dialogNfgPayoffs dialog(m_nfg, m_nfg.GetOutcome(profile), this);

  if (dialog.ShowModal() == wxID_OK) {
    NFOutcome *outc = m_nfg.GetOutcome(profile);
    gArray<gNumber> payoffs(dialog.Payoffs());

    if (!outc) {
      outc = m_nfg.NewOutcome();
      m_nfg.SetOutcome(profile, outc);
    }

    for (int i = 1; i <= m_nfg.NumPlayers(); i++)
      m_nfg.SetPayoff(outc, i, payoffs[i]);
    outc->SetName(dialog.Name());

    m_table->Refresh();
  }
}

const gList<MixedSolution> &NfgShow::Solutions(void) const
{
  return *m_solutionTable;
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

