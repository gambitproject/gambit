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
#include "nfgpanel.h"
#include "nfgtable.h"
#include "nfgsoln.h"
#include "nfgprofile.h"
#include "mixededit.h"
#include "nfgconst.h"

#include "efg.h"
#include "efgutils.h"
#include "nfplayer.h"
#include "nfdom.h"
#include "nfgciter.h"

#include "nfgsolvd.h"
#include "nfgsolng.h"

#include "gambit.h"
#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgeditsupport.h"
#include "dlnfgplayers.h"
#include "dlstrategies.h"
#include "dlnfgsave.h"

#include "dlelim.h"
#include "dlsupportselect.h"

const int idPANELWINDOW = 3001;
const int idSOLUTIONWINDOW = 3002;

//=====================================================================
//                       class NfgToolbar
//=====================================================================

const int NFG_TOOLBAR_ID = 100;

class NfgToolbar : public wxToolBar {
private:
  wxFrame *m_parent;

  // Event handlers
  void OnMouseEnter(wxCommandEvent &);

public:
  NfgToolbar(wxFrame *p_frame);
  virtual ~NfgToolbar() { }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(NfgToolbar, wxToolBar)
  EVT_TOOL_ENTER(NFG_TOOLBAR_ID, NfgToolbar::OnMouseEnter)
END_EVENT_TABLE()

NfgToolbar::NfgToolbar(wxFrame *p_frame)
  : wxToolBar(p_frame, NFG_TOOLBAR_ID), m_parent(p_frame)
{
#ifdef __WXMSW__
  wxBitmap saveBitmap("SAVE_BITMAP");
  wxBitmap printBitmap("PRINT_BITMAP");
  wxBitmap solveBitmap("SOLVE_BITMAP");
  wxBitmap helpBitmap("HELP_BITMAP");
  wxBitmap optionsBitmap("OPTIONS_BITMAP");
  wxBitmap inspectBitmap("INSPECT_BITMAP");
#else
#include "bitmaps/save.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/solve.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/options.xpm"
#include "bitmaps/inspect.xpm"
  wxBitmap saveBitmap(save_xpm);
  wxBitmap printBitmap(print_xpm);
  wxBitmap solveBitmap(solve_xpm);
  wxBitmap helpBitmap(help_xpm);
  wxBitmap optionsBitmap(options_xpm);
  wxBitmap inspectBitmap(inspect_xpm);
#endif // __WXMSW__

  SetMargins(2, 2);
#ifdef __WXMSW__
  SetToolBitmapSize(wxSize(33, 30));
#endif // __WXMSW__

  AddTool(NFG_FILE_SAVE, saveBitmap);
  AddTool(NFG_FILE_OUTPUT, printBitmap);
  AddSeparator();
  AddTool(NFG_SOLVE_STANDARD, solveBitmap);
  AddTool(NFG_VIEW_SOLUTIONS, inspectBitmap);
  AddSeparator();
  AddTool(NFG_OPTIONS_MENU, optionsBitmap);
  AddSeparator();
  AddTool(NFG_HELP_CONTENTS, helpBitmap);

  Realize();
  Show(true);
}

void NfgToolbar::OnMouseEnter(wxCommandEvent &p_event)
{
  if (p_event.GetSelection() > 0) {
    m_parent->SetStatusText(m_parent->GetMenuBar()->GetHelpString(p_event.GetSelection()));
  }
  else {
    m_parent->SetStatusText("");
  }
}

//----------------------------------------------------------------------
//                   class NfgShow: Member functions
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(NfgShow, wxFrame)
  EVT_MENU(NFG_FILE_SAVE, NfgShow::OnFileSave)
  EVT_MENU(NFG_FILE_OUTPUT, NfgShow::OnFileOutput)
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
  EVT_MENU(NFG_PREFS_FONT, NfgShow::OnPrefsFont)
  EVT_MENU(NFG_PREFS_COLORS, NfgShow::OnPrefsColors)
  EVT_MENU(NFG_PREFS_ACCELS, NfgShow::OnPrefsAccels)
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
  EVT_SASH_DRAGGED_RANGE(idPANELWINDOW, idSOLUTIONWINDOW, NfgShow::OnSashDrag)
  EVT_LIST_ITEM_SELECTED(idNFG_SOLUTION_LIST, NfgShow::OnSolutionSelected)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//                      class NfgShow: Lifecycle
//----------------------------------------------------------------------

NfgShow::NfgShow(Nfg &p_nfg, EfgNfgInterface *efg, wxFrame *p_frame)
  : wxFrame(p_frame, -1, "", wxDefaultPosition, wxSize(500, 500)),
    EfgNfgInterface(gNFG, efg),
    m_nfg(p_nfg),
    m_panel(0), m_toolbar(0), m_table(0), m_solutionTable(0),
    m_panelSashWindow(0), m_solutionSashWindow(0),
    m_rowPlayer(1), m_colPlayer(2)
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
  CreateStatusBar(3);
  m_toolbar = new NfgToolbar(this);

  m_panelSashWindow = new wxSashWindow(this, idPANELWINDOW,
				       wxPoint(0, 40), wxSize(200, 200),
				       wxNO_BORDER | wxSW_3D);
  m_panelSashWindow->SetSashVisible(wxSASH_RIGHT, true);

  m_panel = new NfgPanel(this, m_panelSashWindow);
  m_panel->SetSize(200, 200);

  m_table = new NfgTable(this);
  
  m_solutionSashWindow = new wxSashWindow(this, idSOLUTIONWINDOW,
					  wxDefaultPosition,
					  wxSize(600, 100));
  m_solutionSashWindow->SetSashVisible(wxSASH_TOP, true);

  m_solutionTable = new NfgProfileList(this, m_solutionSashWindow);
  m_solutionTable->Show(true);
  m_solutionSashWindow->Show(false);

  m_panelSashWindow->Show(true);

  // Create the accelerators
  //  ReadAccelerators(accelerators, "NfgAccelerators", wxGetApp().ResourceFile());
  
  SetPlayers(1, 2);

  m_nfg.SetIsDirty(false);
  UpdateMenus();
  Show(true);
}

NfgShow::~NfgShow()
{ 
  delete &m_nfg;
}

//----------------------------------------------------------------------
//               class NfgShow: Private member functions
//----------------------------------------------------------------------

void NfgShow::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(NFG_FILE_SAVE, "&Save", "Save the game");
  fileMenu->Append(NFG_FILE_OUTPUT, "Out&put", "Output to any device");
  fileMenu->Append(NFG_FILE_CLOSE, "&Close", "Close the window");
  
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
  prefsMenu->Append(NFG_PREFS_FONT, "&Font", "Set font");
  prefsMenu->Append(NFG_PREFS_COLORS, "&Colors", "Set player colors");
  prefsMenu->Append(NFG_PREFS_ACCELS, "&Accels", "Edit accelerators");
  prefsMenu->AppendSeparator();
  prefsMenu->Append(NFG_PREFS_SAVE, "&Save", "Save current configuration");
  prefsMenu->Append(NFG_PREFS_LOAD, "&Load", "Load configuration");

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(NFG_HELP_ABOUT, "&About");
  helpMenu->Append(NFG_HELP_CONTENTS, "&Contents");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(supportsMenu, "S&upports");
  menuBar->Append(solveMenu, "&Solve");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(prefsMenu, "&Prefs");
  menuBar->Append(helpMenu, "&Help");

  viewMenu->Check(NFG_VIEW_OUTCOMES, !m_drawSettings.OutcomeValues());
  
  SetMenuBar(menuBar);
}

void NfgShow::UpdateMenus(void)
{
  wxMenuBar *menu = GetMenuBar();
  gArray<int> profile(GetProfile());

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

  InterfaceDied();
  Show(false);
  Destroy();
}

void NfgShow::AdjustSizes(void)
{
  const int toolbarHeight = 40;
  int width, height;
  GetClientSize(&width, &height);
  if (m_toolbar) {
    m_toolbar->SetSize(0, 0, width, toolbarHeight);
  }
  if (m_solutionSashWindow && m_solutionSashWindow->IsShown()) {
    int solnHeight = gmax(100, height / 3);
    m_solutionSashWindow->SetSize(0, height - solnHeight, width, solnHeight);
    height -= solnHeight;
  }
  if (m_panelSashWindow) {
    m_panelSashWindow->SetSize(0, 40, 200, height - 40);
  }
  if (m_table) {
    m_table->SetSize(200, 40, width - 200, height - 40);
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
  case idPANELWINDOW:
    m_table->SetSize(p_event.GetDragRect().width,
		     m_table->GetRect().y,
		     clientWidth - p_event.GetDragRect().width,
		     m_table->GetRect().height);
    m_panelSashWindow->SetSize(m_panelSashWindow->GetRect().x,
			       m_panelSashWindow->GetRect().y,
			       p_event.GetDragRect().width,
			       m_panelSashWindow->GetRect().height);
    break;
  case idSOLUTIONWINDOW:
    m_table->SetSize(m_table->GetRect().x, m_table->GetRect().y,
		     m_table->GetRect().width,
		     clientHeight - p_event.GetDragRect().height - 40);
    m_panelSashWindow->SetSize(m_panelSashWindow->GetRect().x,
			       m_panelSashWindow->GetRect().y,
			       m_panelSashWindow->GetRect().width,
			       clientHeight - p_event.GetDragRect().height - 40);
    m_solutionSashWindow->SetSize(0, clientHeight - p_event.GetDragRect().height,
				  clientWidth, p_event.GetDragRect().width);
    break;
  }

}

//----------------------------------------------------------------------
//                 class NfgShow: Menu event handlers
//----------------------------------------------------------------------

void NfgShow::OnFileOutput(wxCommandEvent &)
{
  wxOutputDialogBox dialog;

  if (dialog.ShowModal() == wxID_OK) {
    //m_table->Print(print_dialog.GetMedia(), print_dialog.GetOption());
  }
}

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
    m_nfg.SetOutcome(m_panel->GetProfile(), dialog.GetOutcome());
    InterfaceDied();
    m_table->OnChangeValues();
  }
}

void NfgShow::OnEditOutcomeDetach(wxCommandEvent &)
{
  m_nfg.SetOutcome(m_panel->GetProfile(), 0);
  InterfaceDied();
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
    InterfaceDied();
  }
}

void NfgShow::OnEditOutcomeDelete(wxCommandEvent &)
{
  if (m_nfg.NumOutcomes() == 0)
    return;

  dialogNfgOutcomeSelect dialog(m_nfg, this);
    
  if (dialog.ShowModal() == wxID_OK) {
    m_nfg.DeleteOutcome(dialog.GetOutcome());
    InterfaceDied();
    m_table->OnChangeValues();
  }
}

void NfgShow::OnEditOutcomePayoffs(wxCommandEvent &)
{
  if (m_table->GetCursorRow() < m_currentSupport->NumStrats(m_rowPlayer) &&
      m_table->GetCursorColumn() < m_currentSupport->NumStrats(m_colPlayer)) {
    OutcomePayoffs(m_table->GetCursorRow() + 1,
		   m_table->GetCursorColumn() + 1, false);
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
      SetPlayers(m_rowPlayer, m_colPlayer);
    }
    else {
      m_table->MakeDomDisp();
    }

    m_table->OnChangeValues();
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
      SetPlayers(m_rowPlayer, m_colPlayer);
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
      SetPlayers(m_rowPlayer, m_colPlayer);
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
	SetPlayers(m_rowPlayer, m_colPlayer);
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
      SetPlayers(m_rowPlayer, m_colPlayer);
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
  SetPlayers(m_rowPlayer, m_colPlayer);
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
  SetPlayers(m_rowPlayer, m_colPlayer);
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
    *m_solutionTable += solver->Solve(support);
    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), this);
    wxEndBusyCursor();
  }
    
  delete solver;

  if (old_max_soln != m_solutionTable->Length()) {
    // Now, transfer the NEW solutions to extensive form if requested
    /*
    if (NSD.GetExtensive()) {
      for (int i = old_max_soln+1; i <= solns.Length(); i++) 
    SolutionToExtensive(solns[i]);
    }
    */
    if (!m_table->HaveProbs()) {
      m_table->MakeProbDisp();
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
    *m_solutionTable += solver->Solve(support);
    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), this);
    wxEndBusyCursor();
  }
    
  delete solver;

  if (old_max_soln != m_solutionTable->Length()) {
    // Now, transfer the NEW solutions to extensive form if requested
    /*
    if (NSD.GetExtensive()) {
      for (int i = old_max_soln+1; i <= solns.Length(); i++) 
    SolutionToExtensive(solns[i]);
    }
    */
    if (!m_table->HaveProbs()) {
      m_table->MakeProbDisp();
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
  if (m_table->HaveDom()) {
    m_table->RemoveDomDisp();
  }
  else {
    m_table->MakeDomDisp();
  }
}

void NfgShow::OnViewProbabilities(wxCommandEvent &)
{
  if (m_table->HaveProbs()) {
    m_table->RemoveProbDisp();
    GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, false);
  }
  else {
    m_table->MakeProbDisp();
    GetMenuBar()->Check(NFG_VIEW_PROBABILITIES, true);
  }
  m_table->OnChangeValues();
}

void NfgShow::OnViewValues(wxCommandEvent &)
{
  if (m_table->HaveVal()) {
    m_table->RemoveValDisp();
    GetMenuBar()->Check(NFG_VIEW_VALUES, false);
  }
  else {
    m_table->MakeValDisp();
    GetMenuBar()->Check(NFG_VIEW_VALUES, true);
  }
  m_table->OnChangeValues();
}

void NfgShow::OnViewOutcomes(wxCommandEvent &)
{
  m_drawSettings.SetOutcomeValues(1 - m_drawSettings.OutcomeValues());
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
    m_solutionTable->Append(dialog.GetProfile());
    ChangeSolution(m_solutionTable->Length());
    UpdateMenus();
  }
}

void NfgShow::OnProfilesClone(wxCommandEvent &)
{
  MixedSolution profile((*m_solutionTable)[m_currentSolution]);
  
  dialogMixedEditor dialog(this, profile);
  if (dialog.ShowModal() == wxID_OK) {
    m_solutionTable->Append(dialog.GetProfile());
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
    for (int i = 1; i <= m_currentSupport->NumStrats(m_colPlayer); i++) {
      m_table->SetColumnWidth(i - 1, dialog.GetValue());
    }
  }
}

void NfgShow::OnPrefsDisplayDecimals(wxCommandEvent &)
{
  guiSliderDialog dialog(this, "Decimal places", 0, 25, GetDecimals());

  if (dialog.ShowModal() == wxID_OK) {
    SetDecimals(dialog.GetValue());
    m_table->OnChangeValues();
  }
}

void NfgShow::OnPrefsFont(wxCommandEvent &)
{
  wxFontData data;
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_drawSettings.SetDataFont(dialog.GetFontData().GetChosenFont());
    m_table->SetCellTextFont(dialog.GetFontData().GetChosenFont());
  }
}

void NfgShow::OnPrefsColors(wxCommandEvent &)
{
}

void NfgShow::OnPrefsAccels(wxCommandEvent &)
{
  EditAccelerators(m_accelerators, MakeEventNames());
  //  WriteAccelerators(m_accelerators, "NfgAccelerators", wxGetApp().ResourceFile());
}


void NfgShow::OnPrefsSave(wxCommandEvent &)
{
  m_drawSettings.SaveSettings();
}

void NfgShow::OnPrefsLoad(wxCommandEvent &)
{
  m_drawSettings.LoadSettings();
}

//----------------------------------------------------------------------
//                class NfgShow: Public member functions
//----------------------------------------------------------------------

gArray<int> NfgShow::GetProfile(void) const
{
  return m_panel->GetProfile();
}

void NfgShow::SetStrategy(int p_player, int p_strategy)
{
  m_panel->SetStrategy(p_player, p_strategy);
  m_table->SetStrategy(p_player, p_strategy);
}

void NfgShow::UpdateProfile(gArray<int> &profile)
{
  //  m_table->OnChangeValues();
}

void NfgShow::ChangeSolution(int sol)
{
  m_currentSolution = sol;
    
  m_table->OnChangeValues();
  if (m_solutionTable) {
    m_solutionTable->UpdateValues();
  }
}

void NfgShow::OnSolutionSelected(wxListEvent &p_event)
{
  m_currentSolution = p_event.m_itemIndex + 1;
  m_table->OnChangeValues();
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
  if (!InterfaceOk()) {  // we better have someone to send solutions to
    return;
  }
  
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
}

void NfgShow::SetPlayers(int p_rowPlayer, int p_colPlayer)
{
  m_rowPlayer = p_rowPlayer;
  m_colPlayer = p_colPlayer;
  
  SetTitle((char *) (m_nfg.GetTitle() + " : " + 
		     m_nfg.Players()[m_rowPlayer]->GetName() +
		     " x " + m_nfg.Players()[m_colPlayer]->GetName()));

  m_table->SetPlayers(m_rowPlayer, m_colPlayer);
  SetStrategy(m_rowPlayer, 1);
  SetStrategy(m_colPlayer, 1);
  m_table->OnChangeLabels();
  m_panel->SetSupport(*m_currentSupport);
  m_table->OnChangeValues();
}

void NfgShow::OutcomePayoffs(int st1, int st2, bool next)
{
  gArray<int> profile(m_panel->GetProfile());
  profile[m_rowPlayer] = st1;
  profile[m_colPlayer] = st2;

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
    InterfaceDied();
  }
}



#include "nfgaccl.h"

// These events include those for NormShow and those for SpreadSheet3D
gArray<AccelEvent> NfgShow::MakeEventNames(void)
{
  gArray<AccelEvent> events(NUM_NFG_EVENTS);

  for (int i = 0; i < NUM_NFG_EVENTS; i++)
    events[i+1] = nfg_events[i];

  return events;
}


int NfgShow::CheckAccelerators(wxKeyEvent &ev)
{
  int id = ::CheckAccelerators(m_accelerators, ev);

#ifdef NOT_PORTED_YET
  if (id) 
    m_table->OnMenuCommand(id);
#endif  // NOT_PORTED_YET

  return id;
}

const gList<MixedSolution> &NfgShow::Solutions(void) const
{
  return *m_solutionTable;
}

//-----------------------------------------------------------------------
//               class NormalDrawSettings: Member functions
//-----------------------------------------------------------------------

NormalDrawSettings::NormalDrawSettings(void)
  : m_decimals(2), m_dataFont(*wxNORMAL_FONT)
{
  LoadSettings();
}

NormalDrawSettings::~NormalDrawSettings()
{ }

void NormalDrawSettings::LoadSettings(void)
{
  wxConfig config("Gambit");
  config.Read("NfgDisplay/Display-Precision", &m_decimals, 2);
  config.Read("NfgDisplay/Outcome-Values", &m_outcomeValues, 1);
}

void NormalDrawSettings::SaveSettings(void) const
{
  wxConfig config("Gambit");
  config.Write("NfgDisplay/Display-Precision", (long) m_decimals);
  config.Write("NfgDisplay/Outcome-Values", (long) m_outcomeValues);
}

#include "glist.imp"
template class gList<NFSupport *>;

