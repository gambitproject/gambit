//
// FILE: nfgshow.cc -- Implementation of NfgShow class
//
// $Id$
//

#include "wx/wx.h"
#include "wx/grid.h"
#include "wx/fontdlg.h"

#include "wxstatus.h"
#include "nfgshow.h"
#include "nfgtable.h"
#include "nfgsoln.h"
#include "nfgprint.h"
#include "nfgprofile.h"
#include "mixededit.h"
#include "nfgconst.h"

#include "efg.h"
#include "efgutils.h"
#include "behavsol.h"
#include "nfplayer.h"
#include "nfdom.h"
#include "nfgciter.h"

#include "nfgsolvd.h"
#include "nfgsolng.h"

#include "gambit.h"
#include "efgshow.h"
#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgeditsupport.h"
#include "dlnfgplayers.h"
#include "dlstrategies.h"
#include "dlnfgsave.h"

#include "dlelim.h"
#include "dlsupportselect.h"

const int idSOLUTIONWINDOW = 3002;

//----------------------------------------------------------------------
//                   class NfgShow: Member functions
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(NfgShow, wxFrame)
  EVT_MENU(NFG_FILE_SAVE, NfgShow::OnFileSave)
  EVT_MENU(NFG_FILE_PAGE_SETUP, NfgShow::OnFilePageSetup)
  EVT_MENU(NFG_FILE_PRINT_PREVIEW, NfgShow::OnFilePrintPreview)
  EVT_MENU(NFG_FILE_PRINT, NfgShow::OnFilePrint)
  EVT_MENU(NFG_FILE_CLOSE, NfgShow::Close)
  EVT_MENU(NFG_EDIT_LABEL, NfgShow::OnEditLabel)
  EVT_MENU(NFG_EDIT_PLAYERS, NfgShow::OnEditPlayers)
  EVT_MENU(NFG_EDIT_STRATS, NfgShow::OnEditStrategies)
  EVT_MENU(NFG_EDIT_OUTCOMES_NEW, NfgShow::OnEditOutcomeNew)
  EVT_MENU(NFG_EDIT_OUTCOMES_DELETE, NfgShow::OnEditOutcomeDelete)
  EVT_MENU(NFG_EDIT_OUTCOMES_ATTACH, NfgShow::OnEditOutcomeAttach)
  EVT_MENU(NFG_EDIT_OUTCOMES_DETACH, NfgShow::OnEditOutcomeDetach)
  EVT_MENU(NFG_EDIT_OUTCOMES_PAYOFFS, NfgShow::OnEditOutcomePayoffs)
  EVT_MENU(NFG_SUPPORT_UNDOMINATED, NfgShow::OnSupportUndominated)
  EVT_MENU(NFG_SUPPORT_NEW, NfgShow::OnSupportNew)
  EVT_MENU(NFG_SUPPORT_EDIT, NfgShow::OnSupportEdit)
  EVT_MENU(NFG_SUPPORT_DELETE, NfgShow::OnSupportDelete)
  EVT_MENU(NFG_SUPPORT_SELECT_FROMLIST, NfgShow::OnSupportSelectFromList)
  EVT_MENU(NFG_SUPPORT_SELECT_PREVIOUS, NfgShow::OnSupportSelectPrevious)
  EVT_MENU(NFG_SUPPORT_SELECT_NEXT, NfgShow::OnSupportSelectNext)
  EVT_MENU(NFG_SOLVE_STANDARD, NfgShow::OnSolveStandard)
  EVT_MENU_RANGE(NFG_SOLVE_CUSTOM_ENUMPURE, NFG_SOLVE_CUSTOM_QREGRID, NfgShow::OnSolveCustom)
  EVT_MENU(NFG_VIEW_SOLUTIONS, NfgShow::OnViewSolutions)
  EVT_MENU(NFG_VIEW_DOMINANCE, NfgShow::OnViewDominance)
  EVT_MENU(NFG_VIEW_PROBABILITIES, NfgShow::OnViewProbabilities)
  EVT_MENU(NFG_VIEW_VALUES, NfgShow::OnViewValues)
  EVT_MENU(NFG_VIEW_OUTCOMES, NfgShow::OnViewOutcomes)
  EVT_MENU(NFG_VIEW_GAMEINFO, NfgShow::OnViewGameInfo)
  EVT_MENU(NFG_PREFS_DISPLAY_COLUMNS, NfgShow::OnPrefsDisplayColumns)
  EVT_MENU(NFG_PREFS_DISPLAY_DECIMALS, NfgShow::OnPrefsDisplayDecimals)
  EVT_MENU(NFG_PREFS_FONT_DATA, NfgShow::OnPrefsFontData)
  EVT_MENU(NFG_PREFS_FONT_LABELS, NfgShow::OnPrefsFontLabels)
  EVT_MENU(NFG_PREFS_COLORS, NfgShow::OnPrefsColors)
  EVT_MENU(NFG_PREFS_SAVE, NfgShow::OnPrefsSave)
  EVT_MENU(NFG_PREFS_LOAD, NfgShow::OnPrefsLoad)
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
  EVT_ACTIVATE(NfgShow::OnActivate)
  EVT_LIST_ITEM_SELECTED(idNFG_SOLUTION_LIST, NfgShow::OnSolutionSelected)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//                      class NfgShow: Lifecycle
//----------------------------------------------------------------------

NfgShow::NfgShow(Nfg &p_nfg, GambitFrame *p_parent)
  : wxFrame(p_parent, -1, "", wxDefaultPosition, wxSize(500, 500)),
    m_parent(p_parent), m_nfg(p_nfg),
    m_table(0), m_solutionTable(0), m_solutionSashWindow(0)
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
  entries[2].Set(wxACCEL_CTRL, (int) 'S', NFG_FILE_SAVE);
  entries[3].Set(wxACCEL_CTRL, (int) 'P', NFG_FILE_PRINT);
  entries[4].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
  entries[5].Set(wxACCEL_NORMAL, WXK_F1, wxID_HELP_CONTENTS);
  wxAcceleratorTable accel(6, entries);
  SetAcceleratorTable(accel);

  CreateStatusBar(3);
  MakeToolbar();

  m_table = new NfgTable(m_nfg, this);
  m_table->SetSize(0, 0, 200, 200);

  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_solutionTable = new NfgProfileList(this, m_solutionSashWindow);
  m_solutionTable->Show(true);
  m_solutionSashWindow->Show(false);

  m_nfg.SetIsDirty(false);
  GetMenuBar()->Check(NFG_VIEW_OUTCOMES, !m_table->OutcomeValues());
  UpdateMenus();
  m_table->SetFocus();
  Show(true);
}

NfgShow::~NfgShow()
{
  m_parent->RemoveGame(&m_nfg);
}

//----------------------------------------------------------------------
//               class NfgShow: Private member functions
//----------------------------------------------------------------------

void NfgShow::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_NEW, "&New\tCtrl-N", "Create a new game");
  fileMenu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a saved game");
  fileMenu->Append(NFG_FILE_CLOSE, "&Close", "Close this window");
  fileMenu->AppendSeparator();
  fileMenu->Append(NFG_FILE_SAVE, "&Save\tCtrl-S", "Save this game");
  fileMenu->AppendSeparator();
  fileMenu->Append(NFG_FILE_PAGE_SETUP, "Page Se&tup",
		   "Set up preferences for printing");
  fileMenu->Append(NFG_FILE_PRINT_PREVIEW, "Print Pre&view",
		   "View a preview of the game printout");
  fileMenu->Append(NFG_FILE_PRINT, "&Print\tCtrl-P", "Print this game");
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

  wxMenu *supportsMenu = new wxMenu;
  supportsMenu->Append(NFG_SUPPORT_UNDOMINATED, "&Undominated",
		       "Find undominated strategies");
  supportsMenu->Append(NFG_SUPPORT_NEW, "&New",
		       "Create a new support");
  supportsMenu->Append(NFG_SUPPORT_EDIT, "&Edit",
		       "Edit the currently displayed support");
  supportsMenu->Append(NFG_SUPPORT_DELETE, "&Delete",
		       "Delete a support");
  wxMenu *supportsSelectMenu = new wxMenu;
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_FROMLIST, "From &List...",
			     "Select a support from the list of defined supports");
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_PREVIOUS, "&Previous",
			     "Select the previous support from the list");
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_NEXT, "&Next",
			     "Select the next support from the list");
  supportsMenu->Append(NFG_SUPPORT_SELECT, "&Select", supportsSelectMenu,
		       "Change the current support");

  wxMenu *solveMenu = new wxMenu;
  solveMenu->Append(NFG_SOLVE_STANDARD,  "S&tandard...",
		    "Standard solutions");
  
  wxMenu *solveCustomMenu = new wxMenu;
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_ENUMPURE, "EnumPure",
			  "Enumerate pure strategy equilibria");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_ENUMMIXED, "EnumMixed",
			  "Enumerate mixed strategy equilibria");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LCP, "LCP",
			  "Solve via linear complementarity program");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LP, "LP",
			  "Solve via linear program");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_LIAP, "Liapunov",
			  "Minimization of liapunov function");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_SIMPDIV, "Simpdiv",
			  "Solve via simplicial subdivision");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_POLENUM, "PolEnum",
			  "Enumeration by systems of polynomials");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_QRE, "QRE",
			  "Compute quantal response equilibrium");
  solveCustomMenu->Append(NFG_SOLVE_CUSTOM_QREGRID, "QREGrid",
			  "Compute quantal response equilibrium");
  solveMenu->Append(NFG_SOLVE_CUSTOM, "Custom", solveCustomMenu,
		    "Solve with a particular algorithm");

  
  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(NFG_VIEW_SOLUTIONS, "&Solutions",
		   "Display solutions", true);
  viewMenu->Append(NFG_VIEW_DOMINANCE, "&Dominance",
		   "Display dominance information", TRUE);
  viewMenu->Append(NFG_VIEW_PROBABILITIES, "&Probabilities",
		   "Display solution probabilities", TRUE);
  viewMenu->Append(NFG_VIEW_VALUES, "&Values",
		   "Display strategy values", TRUE);
  viewMenu->Append(NFG_VIEW_OUTCOMES, "&Outcomes",
		   "Display outcome names", TRUE);
  viewMenu->Append(NFG_VIEW_GAMEINFO, "Game&Info",
		   "Display information about the game");
  
  wxMenu *prefsMenu = new wxMenu;
  wxMenu *prefsDisplayMenu = new wxMenu;
  prefsDisplayMenu->Append(NFG_PREFS_DISPLAY_COLUMNS, "&Column Width",
			   "Set column width");
  prefsDisplayMenu->Append(NFG_PREFS_DISPLAY_DECIMALS, "&Decimal Places",
			   "Set number of decimal places to display");

  prefsMenu->Append(NFG_PREFS_DISPLAY, "&Display", prefsDisplayMenu,
		    "Configure display options");
  wxMenu *prefsFontMenu = new wxMenu;
  prefsFontMenu->Append(NFG_PREFS_FONT_DATA, "&Data", "Set data font");
  prefsFontMenu->Append(NFG_PREFS_FONT_LABELS, "&Labels", "Set label font");
  prefsMenu->Append(NFG_PREFS_FONT, "&Font", prefsFontMenu, "Set fonts");
  prefsMenu->Append(NFG_PREFS_COLORS, "&Colors", "Set player colors");
  prefsMenu->AppendSeparator();
  prefsMenu->Append(NFG_PREFS_SAVE, "&Save", "Save current configuration");
  prefsMenu->Append(NFG_PREFS_LOAD, "&Load", "Load configuration");

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");
  helpMenu->Append(wxID_ABOUT, "&About", "About Gambit");

  wxMenuBar *menuBar = new wxMenuBar(wxMB_DOCKABLE);
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(supportsMenu, "S&upports");
  menuBar->Append(solveMenu, "&Solve");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(prefsMenu, "&Prefs");
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
  toolBar->AddTool(NFG_FILE_SAVE, wxBITMAP(save), wxNullBitmap, false,
		   -1, -1, 0, "Save game", "Save this game");
  toolBar->AddSeparator();

  toolBar->AddTool(NFG_FILE_PRINT_PREVIEW, wxBITMAP(preview), wxNullBitmap,
		   false, -1, -1, 0, "Print Preview",
		   "View a preview of the game printout");
  toolBar->AddTool(NFG_FILE_PRINT, wxBITMAP(print), wxNullBitmap, false,
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
  gArray<int> profile(m_table->GetProfile());

  menu->Enable(NFG_EDIT_OUTCOMES_DELETE, m_nfg.NumOutcomes() > 0);
  menu->Enable(NFG_EDIT_OUTCOMES_ATTACH, m_nfg.NumOutcomes() > 0);
  menu->Enable(NFG_EDIT_OUTCOMES_DETACH, m_nfg.GetOutcome(profile) != 0);
  menu->Enable(NFG_EDIT_OUTCOMES_PAYOFFS, m_nfg.GetOutcome(profile) != 0);

  menu->Enable(NFG_SUPPORT_SELECT_FROMLIST, NumSupports() > 1);
  menu->Enable(NFG_SUPPORT_SELECT_PREVIOUS, NumSupports() > 1);
  menu->Enable(NFG_SUPPORT_SELECT_NEXT, NumSupports() > 1);

  menu->Enable(NFG_SOLVE_CUSTOM_ENUMMIXED, m_nfg.NumPlayers() == 2);
  menu->Enable(NFG_SOLVE_CUSTOM_LP,
	       m_nfg.NumPlayers() == 2 && IsConstSum(m_nfg));
  menu->Enable(NFG_SOLVE_CUSTOM_LCP, m_nfg.NumPlayers() == 2);

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

//----------------------------------------------------------------------
//                    class NfgShow: Event handlers
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
  if (m_table) {
    m_table->SetSize(m_table->GetRect().x, m_table->GetRect().y,
		     width - m_table->GetRect().x, height);
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
  case idSOLUTIONWINDOW:
    m_table->SetSize(m_table->GetRect().x, m_table->GetRect().y,
		     m_table->GetRect().width,
		     clientHeight - p_event.GetDragRect().height - 40);
    m_solutionSashWindow->SetSize(0, clientHeight - p_event.GetDragRect().height,
				  clientWidth, p_event.GetDragRect().height);
    break;
  }
}

void NfgShow::OnSetFocus(wxFocusEvent &)
{
  m_table->SetFocus();
}

void NfgShow::OnActivate(wxActivateEvent &p_event)
{
  if (p_event.GetActive()) {
    m_parent->SetActiveWindow(this);
  }
}

//----------------------------------------------------------------------
//                 class NfgShow: Menu event handlers
//----------------------------------------------------------------------

void NfgShow::OnFileSave(wxCommandEvent &)
{
  static int s_nDecimals = 6;
  dialogNfgSave dialog(Filename(), m_nfg.GetTitle(), s_nDecimals, this);

  if (dialog.ShowModal() == wxID_OK) {
    if (wxFileExists((char *) dialog.Filename())) {
      if (wxMessageBox((char *) ("File " + dialog.Filename() + " exists.  Overwrite?"),
		       "Confirm", wxOK | wxCANCEL) != wxOK) {
	return;
      }
    }

    m_nfg.SetTitle(dialog.Label());

    Nfg *nfg = 0;
    try {
      gFileOutput file(dialog.Filename());
      nfg = CompressNfg(m_nfg, *m_currentSupport);
      nfg->WriteNfgFile(file, s_nDecimals);
      delete nfg;
      SetFileName(dialog.Filename());
    }
    catch (gFileOutput::OpenFailed &) {
      wxMessageBox((char *) ("Could not open " + dialog.Filename() + " for writing."),
		   "Error", wxOK);
      if (nfg)  delete nfg;
    }
    catch (gFileOutput::WriteFailed &) {
      wxMessageBox((char *) ("Write error occurred in saving " + dialog.Filename()),
		   "Error", wxOK);
      if (nfg)  delete nfg;
    }
    catch (gException &) {
      wxMessageBox("Internal exception in Gambit", "Error", wxOK);
      if (nfg)  delete nfg;
    }
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

void NfgShow::OnEditLabel(wxCommandEvent &)
{
  const char *label = wxGetTextFromUser("Label of game", "Label Game",
					(char *) m_nfg.GetTitle());
  if (label) {
    m_nfg.SetTitle(label);
    SetFileName(Filename());
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
    m_table->OnChangeLabels();
    m_table->OnChangeValues();
  }
}

void NfgShow::OnEditOutcomeAttach(wxCommandEvent &)
{
  if (m_nfg.NumOutcomes() == 0)
    return;

  dialogNfgOutcomeSelect dialog(m_nfg, this);
    
  if (dialog.ShowModal() == wxID_OK) {
    m_nfg.SetOutcome(m_table->GetProfile(), dialog.GetOutcome());
    m_table->OnChangeValues();
  }
}

void NfgShow::OnEditOutcomeDetach(wxCommandEvent &)
{
  m_nfg.SetOutcome(m_table->GetProfile(), 0);
  m_table->OnChangeValues();
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
    m_table->OnChangeValues();
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

void NfgShow::OnSupportUndominated(wxCommandEvent &)
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
    }
    else if (!m_table->ShowDominance()) {
      m_table->ToggleDominance();
      GetMenuBar()->Check(NFG_VIEW_DOMINANCE, true);
    }
  }
}

void NfgShow::OnSupportNew(wxCommandEvent &)
{
  NFSupport newSupport(m_nfg);
  newSupport.SetName(UniqueSupportName());
  dialogNfgEditSupport dialog(newSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      NFSupport *support = new NFSupport(dialog.Support());
      support->SetName(dialog.Name());
      m_supports.Append(support);

      ChangeSolution(0);
      m_currentSupport = support;
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnSupportEdit(wxCommandEvent &)
{
  dialogNfgEditSupport dialog(*m_currentSupport, this);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      *m_currentSupport = dialog.Support();
      m_currentSupport->SetName(dialog.Name());
      ChangeSolution(0);
      m_table->OnChangeValues();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnSupportDelete(wxCommandEvent &)
{
  if (m_supports.Length() == 1)  return;

  dialogSupportSelect dialog(this, m_supports,
			     m_currentSupport, "Delete Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      delete m_supports.Remove(dialog.Selected());
      if (!m_supports.Find(m_currentSupport)) {
	m_currentSupport = m_supports[1];
	ChangeSolution(0);
      }
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnSupportSelectFromList(wxCommandEvent &)
{
  dialogSupportSelect dialog(this, m_supports,
			     m_currentSupport, "Select Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_currentSupport = m_supports[dialog.Selected()];
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnSupportSelectPrevious(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == 1) {
    m_currentSupport = m_supports[m_supports.Length()];
  }
  else {
    m_currentSupport = m_supports[index - 1];
  }
}

void NfgShow::OnSupportSelectNext(wxCommandEvent &)
{
  int index = m_supports.Find(m_currentSupport);
  if (index == m_supports.Length()) {
    m_currentSupport = m_supports[1];
  }
  else {
    m_currentSupport = m_supports[index + 1];
  }
}

void NfgShow::OnSolveStandard(wxCommandEvent &)
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

    ChangeSolution(m_solutionTable->VisibleLength());
  }  

  UpdateMenus();
}

void NfgShow::OnSolveCustom(wxCommandEvent &p_event)
{
  int id = p_event.GetInt();

  int old_max_soln = m_solutionTable->Length();  // used for extensive update

  guiNfgSolution *solver;

  switch (id) {
  case NFG_SOLVE_CUSTOM_ENUMPURE:
    solver = new guinfgEnumPure(this);
    break;
  case NFG_SOLVE_CUSTOM_ENUMMIXED:
    solver = new guinfgEnumMixed(this);
    break;
  case NFG_SOLVE_CUSTOM_LCP:      
    solver = new guinfgLcp(this);
    break;
  case NFG_SOLVE_CUSTOM_LP:       
    solver = new guinfgLp(this);
    break;
  case NFG_SOLVE_CUSTOM_LIAP:
    solver = new guinfgLiap(this);
    break;
  case NFG_SOLVE_CUSTOM_SIMPDIV:
    solver = new guinfgSimpdiv(this);
    break;
  case NFG_SOLVE_CUSTOM_POLENUM:
    solver = new guinfgPolEnum(this);
    break;
  case NFG_SOLVE_CUSTOM_QRE:
    solver = new guinfgQre(this);
    break;
  case NFG_SOLVE_CUSTOM_QREGRID:
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

    ChangeSolution(m_solutionTable->VisibleLength());
  }

  UpdateMenus();
}

void NfgShow::OnViewSolutions(wxCommandEvent &)
{
  if (m_solutionSashWindow->IsShown()) {
    m_solutionTable->Show(false);
    m_solutionSashWindow->Show(false);
    GetMenuBar()->Check(NFG_VIEW_SOLUTIONS, false);
  }
  else {
    m_solutionTable->Show(true);
    m_solutionSashWindow->Show(true);
    GetMenuBar()->Check(NFG_VIEW_SOLUTIONS, true);
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

void NfgShow::OnViewOutcomes(wxCommandEvent &)
{
  m_table->SetOutcomeValues(1 - m_table->OutcomeValues());
  m_table->OnChangeValues();
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

void NfgShow::OnPrefsDisplayColumns(wxCommandEvent &)
{
  guiSliderDialog dialog(this, "Column width", 0, 100, 20);

  if (dialog.ShowModal() == wxID_OK) {
    //    for (int i = 1; i <= m_currentSupport->NumStrats(m_); i++) {
    //      m_table->SetColumnWidth(i - 1, dialog.GetValue());
    //    }
  }
}

void NfgShow::OnPrefsDisplayDecimals(wxCommandEvent &)
{
  guiSliderDialog dialog(this, "Decimal places", 0, 25,
			 m_table->GetDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    m_table->SetDecimals(dialog.GetValue());
    m_table->OnChangeValues();
  }
}

void NfgShow::OnPrefsFontData(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    //    m_drawSettings.SetDataFont(dialog.GetFontData().GetChosenFont());
    m_table->SetCellFont(dialog.GetFontData().GetChosenFont());
    m_table->OnChangeValues();
  }
}

void NfgShow::OnPrefsFontLabels(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    // m_drawSettings.SetLabelFont(dialog.GetFontData().GetChosenFont());
    m_table->SetLabelFont(dialog.GetFontData().GetChosenFont());
    m_table->OnChangeLabels();
  }
}

void NfgShow::OnPrefsColors(wxCommandEvent &)
{
}

void NfgShow::OnPrefsSave(wxCommandEvent &)
{
  m_table->SaveSettings();
}

void NfgShow::OnPrefsLoad(wxCommandEvent &)
{
  m_table->LoadSettings();
}

//----------------------------------------------------------------------
//                class NfgShow: Public member functions
//----------------------------------------------------------------------

void NfgShow::SetStrategy(int p_player, int p_strategy)
{
  m_table->SetStrategy(p_player, p_strategy);
}

void NfgShow::UpdateProfile(gArray<int> &profile)
{
  //  m_table->OnChangeValues();
}

void NfgShow::AddSolution(const MixedSolution &p_profile, bool p_map)
{
  m_solutionTable->Append(p_profile);
  if (m_nfg.AssociatedEfg() && p_map) {
    m_parent->GetWindow(m_nfg.AssociatedEfg())->AddSolution(BehavProfile<gNumber>(p_profile), false);
  }
  m_solutionTable->UpdateValues();
  UpdateMenus();
}

void NfgShow::ChangeSolution(int sol)
{
  m_currentSolution = sol;
    
  m_table->SetSolution((*m_solutionTable)[m_currentSolution]);
  if (m_solutionTable) {
    m_solutionTable->UpdateValues();
  }
}

void NfgShow::OnSolutionSelected(wxListEvent &p_event)
{
  m_currentSolution = p_event.m_itemIndex + 1;
  m_table->SetSolution((*m_solutionTable)[m_currentSolution]);
}
 
void NfgShow::SetFileName(const gText &p_fileName)
{
  if (p_fileName != "") {
    m_fileName = p_fileName;
  }
  else {  
    m_fileName = "untitled.nfg";
  }

  SetTitle((char *) ("[" + m_fileName + "] " + m_nfg.GetTitle()));
}

void NfgShow::SolutionToExtensive(const MixedSolution &mp, bool set)
{
#ifdef NOT_PORTED_YET
  const Efg *efg = InterfaceObjectEfg();

  if (efg->AssociatedNfg() != &m_nfg) 
    return;

  if (!IsPerfectRecall(*efg)) {
    if (wxMessageBox("May not be able to find valid behavior strategy\n"
		     "for game of imperfect recall\n"
		     "Continue anyway?",
		     "Convert to behavior strategy",
		     wxOK | wxCANCEL | wxCENTRE) != wxOK)   
      return;
  }

  EFSupport S(*InterfaceObjectEfg());
  BehavProfile<gNumber> bp(mp);
  SolutionToEfg(bp, set);
#endif  // NOT_PORTED_YET
}

void NfgShow::OutcomePayoffs(int st1, int st2, bool next)
{
  gArray<int> profile(m_table->GetProfile());
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

    m_table->OnChangeValues();
  }
}

const gList<MixedSolution> &NfgShow::Solutions(void) const
{
  return *m_solutionTable;
}

#include "base/glist.imp"
template class gList<NFSupport *>;

