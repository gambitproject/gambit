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
#include "nfgsoln.h"
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

class NfgPanel : public wxPanel {
private:
  NfgShow *m_parent;
  wxChoice *m_rowChoice, *m_colChoice, **m_stratProfile;

  // Event handlers
  void OnStrategyChange(wxCommandEvent &);
  void OnRowPlayerChange(wxCommandEvent &);
  void OnColPlayerChange(wxCommandEvent &);
  
public:
  NfgPanel(NfgShow *);
  virtual ~NfgPanel() { }

  void SetProfile(const gArray<int> &p_profile);
  gArray<int> GetProfile(void) const;

  void SetSupport(const NFSupport &);
  void SetStrategy(int p_player, int p_strategy);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(NfgPanel, wxPanel)
  EVT_CHOICE(idSTRATEGY_CHOICE, NfgPanel::OnStrategyChange)
  EVT_CHOICE(idROWPLAYER_CHOICE, NfgPanel::OnRowPlayerChange)
  EVT_CHOICE(idCOLPLAYER_CHOICE, NfgPanel::OnColPlayerChange)
END_EVENT_TABLE()

NfgPanel::NfgPanel(NfgShow *p_parent)
  : wxPanel(p_parent, -1, wxPoint(0, 40), wxSize(300, 100)),
	    m_parent(p_parent)
{
  SetAutoLayout(true);
  m_rowChoice = new wxChoice(this, idROWPLAYER_CHOICE);
  m_colChoice = new wxChoice(this, idCOLPLAYER_CHOICE);

  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    wxString playerName = (char *) (ToText(pl) + ": " +
				    m_parent->Game().Players()[pl]->GetName());
    m_rowChoice->Append(playerName);
    m_colChoice->Append(playerName);
  }

  m_rowChoice->SetSelection(0);
  m_colChoice->SetSelection(1);

  wxStaticText *rowLabel = new wxStaticText(this, -1, "Row player");
  wxStaticText *columnLabel = new wxStaticText(this, -1, "Column player");

  rowLabel->SetConstraints(new wxLayoutConstraints);
  rowLabel->GetConstraints()->centreY.SameAs(m_rowChoice, wxCentreY);
  rowLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
  rowLabel->GetConstraints()->width.SameAs(columnLabel, wxWidth);
  rowLabel->GetConstraints()->height.AsIs();
  
  m_rowChoice->SetConstraints(new wxLayoutConstraints);
  m_rowChoice->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_rowChoice->GetConstraints()->left.SameAs(rowLabel, wxRight, 10);
  m_rowChoice->GetConstraints()->width.AsIs();
  m_rowChoice->GetConstraints()->height.AsIs();

  columnLabel->SetConstraints(new wxLayoutConstraints);
  columnLabel->GetConstraints()->centreY.SameAs(m_colChoice, wxCentreY);
  columnLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
  columnLabel->GetConstraints()->width.AsIs();
  columnLabel->GetConstraints()->height.AsIs();

  m_colChoice->SetConstraints(new wxLayoutConstraints);
  m_colChoice->GetConstraints()->top.SameAs(m_rowChoice, wxBottom);
  m_colChoice->GetConstraints()->left.SameAs(columnLabel, wxRight, 10);
  m_colChoice->GetConstraints()->width.AsIs();
  m_colChoice->GetConstraints()->height.AsIs();

  m_stratProfile = new wxChoice *[m_parent->Game().NumPlayers()];
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    m_stratProfile[pl-1] = new wxChoice(this, idSTRATEGY_CHOICE);
    
    NFPlayer *player = m_parent->Game().Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					     player->Strategies()[st]->Name()));
    }
    m_stratProfile[pl-1]->SetSelection(0);

    wxStaticText *profileLabel = new wxStaticText(this, -1,
						  (char *) ("Player " + ToText(pl)));

    profileLabel->SetConstraints(new wxLayoutConstraints);
    profileLabel->GetConstraints()->left.SameAs(this, wxLeft, 10);
    profileLabel->GetConstraints()->centreY.SameAs(m_stratProfile[pl-1],
						   wxCentreY);
    profileLabel->GetConstraints()->width.SameAs(columnLabel, wxWidth);
    profileLabel->GetConstraints()->height.AsIs();

    m_stratProfile[pl-1]->SetConstraints(new wxLayoutConstraints);
    m_stratProfile[pl-1]->GetConstraints()->left.SameAs(m_rowChoice, wxLeft);
    if (pl > 1) {
      m_stratProfile[pl-1]->GetConstraints()->top.SameAs(m_stratProfile[pl-2],
							 wxBottom);
    }
    else {
      m_stratProfile[pl-1]->GetConstraints()->top.SameAs(m_colChoice,
							 wxBottom, 10);
    }
    m_stratProfile[pl-1]->GetConstraints()->width.AsIs();
    m_stratProfile[pl-1]->GetConstraints()->height.AsIs();
  }

  Layout();
  Show(true);
}

void NfgPanel::SetProfile(const gArray<int> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    m_stratProfile[i-1]->SetSelection(p_profile[i] - 1);
  }
}

gArray<int> NfgPanel::GetProfile(void) const
{
  gArray<int> profile(m_parent->Game().NumPlayers());
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = m_stratProfile[i-1]->GetSelection() + 1;
  }
  return profile;
}

void NfgPanel::SetStrategy(int p_player, int p_strategy)
{
  m_stratProfile[p_player-1]->SetSelection(p_strategy-1);
}

void NfgPanel::OnStrategyChange(wxCommandEvent &)
{
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    m_parent->SetStrategy(pl, m_stratProfile[pl-1]->GetSelection() + 1);
  }
}

void NfgPanel::OnRowPlayerChange(wxCommandEvent &)
{
  int oldRowPlayer = m_parent->GetRowPlayer();
  int newRowPlayer = m_rowChoice->GetSelection() + 1;

  if (newRowPlayer == oldRowPlayer) {
    return;
  }

  if (newRowPlayer == m_colChoice->GetSelection() + 1) {
    m_colChoice->SetSelection(oldRowPlayer - 1);
    m_parent->SetPlayers(newRowPlayer, oldRowPlayer);
  }
  else {
    m_parent->SetPlayers(newRowPlayer, m_colChoice->GetSelection() + 1);
  }
}

void NfgPanel::OnColPlayerChange(wxCommandEvent &)
{
  int oldColPlayer = m_parent->GetColPlayer();
  int newColPlayer = m_colChoice->GetSelection() + 1;

  if (newColPlayer == oldColPlayer) {
    return;
  }

  if (newColPlayer == m_rowChoice->GetSelection() + 1) {
    m_rowChoice->SetSelection(oldColPlayer - 1);
    m_parent->SetPlayers(oldColPlayer, newColPlayer);
  }
  else {
    m_parent->SetPlayers(m_colChoice->GetSelection() + 1, newColPlayer);
  }
}

void NfgPanel::SetSupport(const NFSupport &p_support)
{
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    m_stratProfile[pl-1]->Clear();
    NFPlayer *player = m_parent->Game().Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      if (p_support.Find(player->Strategies()[st])) {
	m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					       player->Strategies()[st]->Name()));
      }
    }
    m_stratProfile[pl-1]->SetSelection(0);
  }
}


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

//======================================================================
//                          class NfgTable
//======================================================================

class NfgTable : public wxGrid {
private:
  NfgShow *m_parent;

  struct ns_features_struct {
    int prob, dom, val; /* these are actually int, not bool 0 or 1 */
    bool verbose;
    ns_features_struct(void) :prob(0), dom(0), val(0), verbose(TRUE) { }
    ns_features_struct(const ns_features_struct &s): prob(s.prob), dom(s.dom),
      val(s.val), verbose(s.verbose) { }
  } features;

  // Event handlers
  void OnLeftDoubleClick(wxMouseEvent &);

  // Overriding wxGrid member functions
  void OnSelectCell(int p_row, int p_col);

public:
  NfgTable(NfgShow *);
  virtual ~NfgTable() { }

  void SetProfile(const gArray<int> &profile);
  void SetStrategy(int p_player, int p_strategy);

  void SetPlayers(int p_rowPlayer, int p_colPlayer);

  // Functions to create an extra row&col to display probs, dominators, values
  void MakeProbDisp(void);
  void RemoveProbDisp(void);
  int  HaveProbs(void) const { return features.prob; }
  void MakeDomDisp(void);
  void RemoveDomDisp(void);
  int  HaveDom(void) const { return features.dom; }
  void MakeValDisp(void);
  void RemoveValDisp(void);
  int  HaveVal(void) const { return features.val; }
  
  int GetDecimals(void) const { return m_parent->GetDecimals(); }

  void OnChangeValues(void);
  virtual void OnChangeLabels(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(NfgTable, wxGrid)
  EVT_LEFT_DCLICK(NfgTable::OnLeftDoubleClick)
END_EVENT_TABLE()

NfgTable::NfgTable(NfgShow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_parent)
{
  CreateGrid(p_parent->CurrentSupport()->NumStrats(1),
	     p_parent->CurrentSupport()->NumStrats(2));
  SetGridCursor(0, 0);
  SetEditable(false);
  AdjustScrollbars();
  Show(true);
}

void NfgTable::SetProfile(const gArray<int> &p_profile)
{
  SetGridCursor(p_profile[m_parent->GetRowPlayer()],
		p_profile[m_parent->GetColPlayer()]);
}

void NfgTable::SetPlayers(int p_rowPlayer, int p_colPlayer)
{ 
  const NFSupport &support = *m_parent->CurrentSupport();

  SetEditable(true);
  if (support.NumStrats(p_rowPlayer) > GetRows()) {
    InsertRows(0, support.NumStrats(p_rowPlayer) - GetRows() + features.dom);
  }
  else if (support.NumStrats(p_rowPlayer) < GetRows()) {
    DeleteRows(0, GetRows() - support.NumStrats(p_rowPlayer) - features.dom);
  }

  if (support.NumStrats(p_colPlayer) > GetCols()) {
    InsertCols(0, support.NumStrats(p_colPlayer) - GetCols() + features.dom);
  }
  else if (support.NumStrats(p_colPlayer) < GetCols()) {
    DeleteCols(0, GetCols() - support.NumStrats(p_colPlayer) - features.dom);
  }
  SetEditable(false);

  AdjustScrollbars();
  OnChangeValues();
}

void NfgTable::SetStrategy(int p_player, int p_strategy)
{
  if (p_player == m_parent->GetRowPlayer()) {
    SetGridCursor(p_strategy - 1, GetCursorColumn());
  }
  else if (p_player == m_parent->GetColPlayer()) {
    SetGridCursor(GetCursorRow(), p_strategy - 1);
  }
}

void NfgTable::OnSelectCell(int p_row, int p_col)
{
  if (p_row < m_parent->CurrentSupport()->NumStrats(m_parent->GetRowPlayer()))
    m_parent->SetStrategy(m_parent->GetRowPlayer(), p_row + 1);
  if (p_col < m_parent->CurrentSupport()->NumStrats(m_parent->GetColPlayer()))
    m_parent->SetStrategy(m_parent->GetColPlayer(), p_col + 1);
}

void NfgTable::OnChangeLabels(void)
{
  const NFSupport *support = m_parent->CurrentSupport();
  gArray<int> profile = m_parent->GetProfile();
  int rowPlayer = m_parent->GetRowPlayer();
  int colPlayer = m_parent->GetColPlayer();

  for (int i = 1; i <= support->NumStrats(colPlayer); i++) {
    SetLabelValue(wxHORIZONTAL,
		  (char *) support->Strategies(colPlayer)[i]->Name(),
		  i - 1);
  }
  
  for (int i = 1; i <= support->NumStrats(rowPlayer); i++) {
    SetLabelValue(wxVERTICAL,
		  (char *) support->Strategies(rowPlayer)[i]->Name(),
		  i - 1);
  }

  if (features.dom) {
    SetLabelValue(wxHORIZONTAL, "Domin",
		  support->NumStrats(colPlayer) + features.prob);
    SetLabelValue(wxVERTICAL, "Domin",
		  support->NumStrats(rowPlayer) + features.prob);
  }
}

void NfgTable::OnChangeValues(void)
{
  const Nfg &nfg = m_parent->Game();
  const NFSupport &support = *m_parent->CurrentSupport();

  NfgIter iterator(support);
  iterator.Set(m_parent->GetProfile());

  for (int i = 1; i <= support.NumStrats(m_parent->GetRowPlayer()); i++) {
    for (int j = 1; j <= support.NumStrats(m_parent->GetColPlayer()); j++) {
      iterator.Set(m_parent->GetRowPlayer(), i);
      iterator.Set(m_parent->GetColPlayer(), j);
      gText pay_str;
      NFOutcome *outcome = iterator.GetOutcome();

      if (m_parent->getNormalDrawSettings().OutcomeValues()) {
	for (int k = 1; k <= nfg.NumPlayers(); k++) {
	  pay_str += ToText(nfg.Payoff(outcome, k), GetDecimals());
	  
	  if (k != nfg.NumPlayers())
	    pay_str += ',';
	}
      }
      else {
	if (outcome) {
	  pay_str = outcome->GetName();

	  if (pay_str == "")
	    pay_str = "Outcome" + ToText(outcome->GetNumber());
	}
	else {
	  pay_str = "Null";
	}
      }

      SetCellValue((char *) pay_str, i - 1, j - 1);
    }

    if (HaveDom()) { 
      int dom_pos = GetCols() - HaveVal();
      Strategy *strategy = support.Strategies(m_parent->GetRowPlayer())[i];
      if (support.IsDominated(strategy, true)) {
	SetCellValue("S", i - 1, dom_pos - 1);
      }
      else if (support.IsDominated(strategy, false)) {
	SetCellValue("W", i - 1, dom_pos - 1);
      }
      else {
        SetCellValue("N", i - 1, dom_pos - 1);
      }
    }
  }
   
  if (HaveDom()) {
    for (int j = 1; j <= support.NumStrats(m_parent->GetColPlayer()); j++) {
      int dom_pos = GetRows() - HaveVal();
      Strategy *strategy = support.Strategies(m_parent->GetColPlayer())[j];
      if (support.IsDominated(strategy, true)) { 
	SetCellValue("S", dom_pos - 1, j - 1);
      }
      else if (support.IsDominated(strategy, false)) {
	SetCellValue("W", dom_pos - 1, j - 1);
      }
      else {
	SetCellValue("N", dom_pos - 1, j - 1);
      }
    }
  }
}



//************ extra features for displaying dominance, probs, vals **********
// these features each create an extra row and columns.  They always go in
// order: 1. Prob, 2. Domin, 3. Value.  Make sure to update the labels if a
// feature is turned on/off.  Note that if you turn on a feature that is
// already on, no new rows/cols will be created, but the labels will be
// updated.

void NfgTable::MakeProbDisp(void)
{
  /*
  int row = m_parent->CurrentSupport()->NumStrats(m_parent->GetRowPlayer()) + 1;
  int col = m_parent->CurrentSupport()->NumStrats(m_parent->GetColPlayer()) + 1;

  if (!features.prob) {
    AppendRows();
    //SetSelectableRow(row, FALSE);
    AppendCols();
v    //SetSelectableCol(col, FALSE);
  }

  // Note: this insures that Prob is always the FIRST extra after the
  // regular data, and Domin is AFTER the prob.
  SetLabelValue(wxVERTICAL, "Prob", row-1);
  SetLabelValue(wxHORIZONTAL, "Prob", col-1);
  features.prob = 1;
  */
}


void NfgTable::RemoveProbDisp(void)
{
  /* 
 if (features.prob) {
    int row = dimensionality[pl1] + 1;
    int col = dimensionality[m_colPlayer] + 1;
    DeleteRows(row);
    DeleteCols(col);
    features.prob = 0;
  }
  */
}

void NfgTable::MakeDomDisp(void)
{
  if (!features.dom) {
    features.dom = 1;
    SetEditable(true);
    AppendRows();
    AppendCols();
    UpdateDimensions();
    AdjustScrollbars();
    SetEditable(false);
    OnChangeValues();
  }
}


void NfgTable::RemoveDomDisp(void)
{
  if (features.dom) {
    features.dom = 0;
    SetEditable(true);
    DeleteRows();
    DeleteCols();
    UpdateDimensions();
    AdjustScrollbars();
    SetEditable(false);
    OnChangeValues();
  }
}

void NfgTable::MakeValDisp(void)
{
  /*
  int row = dimensionality[pl1] + features.prob + features.dom + 1;
  int col = dimensionality[m_colPlayer] + features.prob + features.dom + 1;

  if (!features.val) {
    InsertRows(row);
    InsertCols(col);
  }
  
  SetLabelValue(wxVERTICAL, "Value", row-1);
  SetLabelValue(wxHORIZONTAL, "Value", col-1);
  features.val = 1;
  */
}


void NfgTable::RemoveValDisp(void)
{
  /*
  if (features.val) {
    int row = dimensionality[pl1] + features.prob + features.dom + 1;
    int col = dimensionality[m_colPlayer] + features.prob + features.dom + 1;
    DeleteRows(row);
    DeleteCols(col);
    features.val = 0;
  }
  */
}

void NfgTable::OnLeftDoubleClick(wxMouseEvent &p_event)
{
#ifndef __WXMSW__
  int row, col;
  if (CellHitTest(p_event.GetX(), p_event.GetY(), &row, &col)) {
    m_parent->OutcomePayoffs(row+1, col+1); 
  }
#endif  // __WXMSW__
}

//======================================================================
//                           class NfgShow
//======================================================================

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
  EVT_SIZE(NfgShow::OnSize)
  EVT_CLOSE(NfgShow::OnCloseWindow)
END_EVENT_TABLE()

//======================================================================
//                 NfgShow: Constructor and destructor
//======================================================================

NfgShow::NfgShow(Nfg &p_nfg, EfgNfgInterface *efg, wxFrame *p_frame)
  : wxFrame(p_frame, -1, "", wxDefaultPosition, wxSize(500, 500)),
    EfgNfgInterface(gNFG, efg),
    m_nfg(p_nfg), m_rowPlayer(1), m_colPlayer(2)
{
#ifdef __WXMSW__
  SetIcon(wxIcon("nfg_icn"));
#else
#include "nfg.xbm"
  SetIcon(wxIcon(nfg_bits, nfg_width, nfg_height));
#endif  // __WXMSW__

  cur_soln = 0;
  m_currentSupport = new NFSupport(m_nfg);    // base support
  m_currentSupport->SetName("Full Support");
  supports.Append(m_currentSupport);

  MakeMenus();
  CreateStatusBar(3);
  m_toolbar = new NfgToolbar(this);
  m_panel = new NfgPanel(this);
  m_table = new NfgTable(this);
  m_solutionTable = 0;  // no solution inspect window yet.

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

void NfgShow::MakeMenus(void)
{
  wxMenu *file_menu = new wxMenu;
  file_menu->Append(NFG_FILE_SAVE, "&Save", "Save the game");
  file_menu->Append(NFG_FILE_OUTPUT, "Out&put", "Output to any device");
  file_menu->Append(NFG_FILE_CLOSE, "&Close", "Close the window");
  
  wxMenu *edit_menu = new wxMenu;
  edit_menu->Append(NFG_EDIT_LABEL, "&Label", "Set the label of the game");
  edit_menu->Append(NFG_EDIT_PLAYERS, "&Players", "Edit player names");
  edit_menu->Append(NFG_EDIT_STRATS, "&Strategies", "Edit strategy names");

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
  edit_menu->Append(NFG_EDIT_OUTCOMES,  "&Outcomes",  editOutcomesMenu,
		    "Set/Edit outcomes");

  wxMenu *supports_menu = new wxMenu;
  supports_menu->Append(NFG_SUPPORT_UNDOMINATED, "&Undominated",
			"Find undominated strategies");
  supports_menu->Append(NFG_SUPPORT_NEW, "&New",
			"Create a new support");
  supports_menu->Append(NFG_SUPPORT_EDIT, "&Edit",
			"Edit the currently displayed support");
  supports_menu->Append(NFG_SUPPORT_DELETE, "&Delete",
			"Delete a support");
  wxMenu *supportsSelectMenu = new wxMenu;
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_FROMLIST, "From &List...",
			     "Select a support from the list of defined supports");
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_PREVIOUS, "&Previous",
			     "Select the previous support from the list");
  supportsSelectMenu->Append(NFG_SUPPORT_SELECT_NEXT, "&Next",
			     "Select the next support from the list");
  supports_menu->Append(NFG_SUPPORT_SELECT, "&Select", supportsSelectMenu,
			"Change the current support");

  wxMenu *solve_menu = new wxMenu;
  solve_menu->Append(NFG_SOLVE_STANDARD,  "S&tandard...",
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
  solve_menu->Append(NFG_SOLVE_CUSTOM, "Custom", solveCustomMenu,
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

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(NFG_HELP_ABOUT, "&About");
  help_menu->Append(NFG_HELP_CONTENTS, "&Contents");

  wxMenuBar *tmp_menubar = new wxMenuBar;
  tmp_menubar->Append(file_menu,     "&File");
  tmp_menubar->Append(edit_menu,     "&Edit");
  tmp_menubar->Append(supports_menu, "S&upports");
  tmp_menubar->Append(solve_menu,    "&Solve");
  tmp_menubar->Append(viewMenu,  "&View");
  tmp_menubar->Append(prefsMenu,    "&Prefs");
  tmp_menubar->Append(help_menu,     "&Help");

  viewMenu->Check(NFG_VIEW_OUTCOMES, getNormalDrawSettings().OutcomeValues());
  
  SetMenuBar(tmp_menubar);
}

gArray<int> NfgShow::GetProfile(void) const
{
  return m_panel->GetProfile();
}

void NfgShow::SetStrategy(int p_player, int p_strategy)
{
  m_panel->SetStrategy(p_player, p_strategy);
  m_table->SetStrategy(p_player, p_strategy);
}

void NfgShow::UpdateSoln(void)
{
  if (!cur_soln)  return;

  // The profile is obvious for pure strategy: just set the display strat
  // to the nonzero solution strategy.  However, for mixed equs, we set
  // the display strategy to the highest soluton strat.  (Note that
  // MixedSolution.Pure() is not yet implemented :( Add support for
  // displaying solutions created for supports other than m_currentSupport

  MixedSolution soln = solns[cur_soln];
  gNumber t_max;
  gArray<int> profile(m_nfg.NumPlayers());

  // Figure out the index in the current support,
  // then map it onto the full support
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    profile[pl] = 1;
    t_max = soln(m_nfg.Players()[pl]->Strategies()[1]);

    for (int st1 = 1; st1 <= m_currentSupport->NumStrats(pl); st1++) {
      if (soln(m_currentSupport->Strategies(pl)[st1]) > t_max) {
	profile[pl] = st1;
	t_max = soln(m_currentSupport->Strategies(pl)[st1]);
      }
    }
  }

  UpdateProfile(profile);

  // Set the profile boxes to correct values if this is a pure equ
  m_table->SetProfile(profile);
  m_panel->SetProfile(profile);

  // Hilight the cells w/ nonzero prob
  gNumber eps;
  gEpsilon(eps, GetDecimals());
 
#ifdef NOT_PORTED_YET
  if (m_table->HaveProbs()) {
    // Print out the probability in the next column/row
    for (int i = 1; i <= m_currentSupport->NumStrats(m_rowPlayer); i++)
      m_table->SetCellValue((char *) ToText(soln(m_currentSupport->Strategies(m_rowPlayer)[i]),
					   GetDecimals()),
			   i-1, m_currentSupport->Num);

    for (int i = 1; i <= m_currentSupport->NumStrats(m_colPlayer); i++)
      m_table->SetCellValue((char *) ToText(soln(m_currentSupport->Strategies(m_colPlayer)[i]),
					   GetDecimals()),
			   rows, i-1);
  }

  if (m_table->HaveVal()) {
    // Print out the probability in the last column/row
    for (int i = 1; i <= rows; i++) {
      m_table->SetCellValue((char *) ToText(soln.Payoff(m_nfg.Players()[pl1],
						       m_currentSupport->Strategies(pl1)[i]),
					   GetDecimals()),
			   i-1, cols+m_table->HaveProbs()+m_table->HaveDom());
    }
    
    for (int j = 1; j <= cols; j++) {
      m_table->SetCellValue((char *) ToText(soln.Payoff(m_nfg.Players()[m_colPlayer],
						       m_currentSupport->Strategies(m_colPlayer)[j]),
					   GetDecimals()),
			   rows+m_table->HaveProbs()+m_table->HaveDom(), j-1);
    }
  }
#endif  // NOT_PORTED_YET
}



void NfgShow::UpdateContingencyProb(const gArray<int> &profile)
{
  if (!cur_soln || !m_table->HaveProbs()) 
    return;

  // The value in the maximum row&col cell corresponds to prob of being
  // at this contingency = Product(Prob(strat_here), all players except pl1, m_colPlayer)
  const MixedSolution &soln = solns[cur_soln];

  gNumber cont_prob(1);

  for (int i = 1; i <= m_nfg.NumPlayers(); i++) {
    if (i != m_rowPlayer && i != m_colPlayer) {
      NFPlayer *player = m_nfg.Players()[i];
      cont_prob *= soln(player->Strategies()[profile[i]]);
    }
  }

#ifdef NOT_PORTED_YET
  m_table->SetCellValue((char *) ToText(cont_prob, GetDecimals()),
		       rows, cols);
#endif  // NOT_PORTED_YET
}



void NfgShow::UpdateProfile(gArray<int> &profile)
{
  UpdateContingencyProb(profile);
  m_table->OnChangeValues();
}

gText NfgShow::UniqueSupportName(void) const
{
  int number = supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= supports.Length(); i++) {
      if (supports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

// Clear solutions-just updates the spreadsheet to remove any hilights
void NfgShow::ClearSolutions(void)
{
#ifdef NOT_PORTED_YET
  if (m_table->HaveProbs()) {
    for (int i = 1; i <= cols; i++) {
      m_table->SetCellValue("", rows+1, i);
    }
    for (int i = 1; i <= rows; i++) {
      m_table->SetCellValue("", i, cols+1);
    }
    m_table->SetCellValue("", rows+1, cols+1);
  }

  /*
  if (m_table->HaveDom()) {  // if there exist the dominance row/col
    int dom_pos = 1+m_table->HaveProbs();
    for (int i = 1; i <= cols; i++)
      m_table->SetCell(rows+dom_pos, i, "");

    for (int i = 1; i <= rows; i++)
      m_table->SetCell(i, cols+dom_pos, "");
  }
  */

  if (m_table->HaveVal()) {
    int val_pos = 1+m_table->HaveProbs()+m_table->HaveDom();
    for (int i = 1; i <= cols; i++)  {
      m_table->SetCellValue("", rows+val_pos, i);
    }
    for (int i = 1; i <= rows; i++) {
      m_table->SetCellValue("", i, cols+val_pos);
    }
  }
#endif  // NOT_PORTED_YET
}

void NfgShow::ChangeSolution(int sol)
{
  ClearSolutions();

  if (sol) {
    cur_soln = sol;
    
    if (cur_soln)
      UpdateSoln();
  }
  else {
    cur_soln = 0;
    m_table->RemoveProbDisp();
    m_table->RemoveValDisp();
    m_table->OnChangeValues();
  }
}


// Remove solutions-permanently removes any solutions
void NfgShow::RemoveSolutions(void)
{
#ifdef NOT_PORTED_YET
  if (m_solutionTable) {
    m_solutionTable->Show(FALSE);
    delete m_solutionTable;
    m_solutionTable = 0;
  }
#endif  // NOT_PORTED_YET

  ClearSolutions();

  cur_soln = 0;
  solns.Flush();
}



MixedSolution NfgShow::CreateSolution(void)
{
  return MixedSolution(MixedProfile<gNumber>(*m_currentSupport));
}

void NfgShow::SetFileName(const gText &s)
{
  if (s != "")
    filename = s;
  else 
    filename = "untitled.nfg";

  // Title the window
  SetTitle((char *) ("[" + filename + "] " + m_nfg.GetTitle()));
}

// how: 0-default, 1-saved, 2-query
MixedProfile<gNumber> NfgShow::CreateStartProfile(int how)
{
  MixedProfile<gNumber> start(*m_currentSupport);

  if (how == 0)
    start.Centroid();

#ifdef NOT_PORTED_YET
  if (how == 1 || how == 2) {
    if (starting_points.last == -1 || how == 2) {
      MSolnSortFilterOptions sf_opts; // no sort, filter

      if (starting_points.profiles.Length() == 0)
	starting_points.profiles += start;

      Nfg1SolnPicker *start_dialog = 
	new Nfg1SolnPicker(starting_points.profiles, 
			   nf.NumPlayers(), 
			   gmax(nf.NumStrats()), 0, 
			   draw_settings, sf_opts, this, m_table);
      
      m_table->Enable(FALSE);  // disable this window until the edit window is closed
      
      while (start_dialog->Completed() == wxRUNNING) 
	wxYield();
      
      m_table->Enable(TRUE);
      starting_points.last = start_dialog->Picked();
      delete start_dialog;
    }

    if (starting_points.last)
      start = starting_points.profiles[starting_points.last];
  }
#endif // NOT_PORTED_YET

  return start;
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
  UpdateSoln();
}

void NfgShow::OutcomePayoffs(int st1, int st2, bool next)
{
  gArray<int> profile(m_panel->GetProfile());
  profile[m_rowPlayer] = st1 + 1;
  profile[m_colPlayer] = st2 + 1;

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
    RemoveSolutions();
    InterfaceDied();
  }
}

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
  OutcomePayoffs(m_table->GetCursorRow(), m_table->GetCursorColumn(), false);
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
	    supports.Append(sup);
	  }
	}
	else {
	  if ((sup = sup->Undominated(dialog.DomStrong(), 
				      dialog.Players(), gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    supports.Append(sup);
	  }
	}
      }
      else {
	if (dialog.Iterative()) {
	  while ((sup = sup->MixedUndominated(dialog.DomStrong(), precRATIONAL,
					      dialog.Players(),
					      gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    supports.Append(sup);
	  }
	}
	else {
	  if ((sup = sup->MixedUndominated(dialog.DomStrong(), precRATIONAL,
					   dialog.Players(),
					   gnull, status)) != 0) {
	    sup->SetName(UniqueSupportName());
	    supports.Append(sup);
	  }
	}
      }
    }
    catch (gSignalBreak &) { }

    if (m_currentSupport != sup) {
      m_currentSupport = supports[supports.Length()]; // displaying the last created support
      SetPlayers(m_rowPlayer, m_colPlayer);
    }
    else {
      m_table->MakeDomDisp();
    }

    UpdateSoln();
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
      supports.Append(support);

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
  if (supports.Length() == 1)  return;

  dialogSupportSelect dialog(this, supports,
			     m_currentSupport, "Delete Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      delete supports.Remove(dialog.Selected());
      if (!supports.Find(m_currentSupport)) {
	m_currentSupport = supports[1];
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
  dialogSupportSelect dialog(this, supports,
			     m_currentSupport, "Select Support");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_currentSupport = supports[dialog.Selected()];
      SetPlayers(m_rowPlayer, m_colPlayer);
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), this);
    }
  }
}

void NfgShow::OnSupportSelectPrevious(wxCommandEvent &)
{
  int index = supports.Find(m_currentSupport);
  if (index == 1) {
    m_currentSupport = supports[supports.Length()];
  }
  else {
    m_currentSupport = supports[index - 1];
  }
  SetPlayers(m_rowPlayer, m_colPlayer);
}

void NfgShow::OnSupportSelectNext(wxCommandEvent &)
{
  int index = supports.Find(m_currentSupport);
  if (index == supports.Length()) {
    m_currentSupport = supports[1];
  }
  else {
    m_currentSupport = supports[index + 1];
  }
  SetPlayers(m_rowPlayer, m_colPlayer);
}

void NfgShow::OnSolveStandard(wxCommandEvent &)
{ 
  dialogNfgSolveStandard dialog(this, m_nfg);
  
  if (dialog.ShowModal() != wxID_OK)
    return;

  int old_max_soln = solns.Length();  // used for extensive update

  guiNfgSolution *solver = 0;

  switch (dialog.Number()) {
  case nfgSTANDARD_ONE:
    if (dialog.Type() == nfgSTANDARD_NASH) {
      if (m_nfg.NumPlayers() == 2) {
	if (IsConstSum(m_nfg))
	  solver = new guinfgLp(*m_currentSupport, this, 1, dialog.Precision(), true);
	else
	  solver = new guinfgLcp(*m_currentSupport, this, 1, dialog.Precision(), true);
      }
      else
	solver = new guinfgSimpdiv(*m_currentSupport, this, 1, dialog.Precision(), true);
    }
    else {  // nfgSTANDARD_PERFECT
      if (m_nfg.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(*m_currentSupport, this, 1, dialog.Precision(), true);
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
	solver = new guinfgEnumMixed(*m_currentSupport, this, 2, dialog.Precision(), false);
      else
	solver = new guinfgLiap(*m_currentSupport, this, 2, 10, false);
    }
    else {  // nfgSTANDARD_PERFECT
      if (m_nfg.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(*m_currentSupport, this, 2, dialog.Precision(), true);
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
	solver = new guinfgEnumMixed(*m_currentSupport, this, 0, dialog.Precision(), false);
      else {
	solver = new guinfgPolEnum(*m_currentSupport, this, 0, false);
      }
    }
    else {  // nfgSTANDARD_PERFECT
      if (m_nfg.NumPlayers() == 2) {
	solver = new guinfgEnumMixed(*m_currentSupport, this, 0, dialog.Precision(), true);
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
    solver->Eliminate();
    solns += solver->Solve();
    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), Frame());
    wxEndBusyCursor();
  }
    
  delete solver;

  if (old_max_soln != solns.Length()) {
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

    ChangeSolution(solns.VisibleLength());
  }  

}

void NfgShow::OnSolveCustom(wxCommandEvent &p_event)
{
  int id = p_event.GetInt();

  NFSupport *sup = (supports.Length() > 1) ? m_currentSupport : 0;

  if (!sup)
    sup = new NFSupport(m_nfg);

  int old_max_soln = solns.Length();  // used for extensive update

  guiNfgSolution *solver;

  switch (id) {
  case NFG_SOLVE_CUSTOM_ENUMPURE:
    solver = new guinfgEnumPure(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_ENUMMIXED:
    solver = new guinfgEnumMixed(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LCP:      
    solver = new guinfgLcp(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LP:       
    solver = new guinfgLp(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_LIAP:
    solver = new guinfgLiap(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_SIMPDIV:
    solver = new guinfgSimpdiv(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_POLENUM:
    solver = new guinfgPolEnum(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_QRE:
    solver = new guinfgQre(*sup, this);
    break;
  case NFG_SOLVE_CUSTOM_QREGRID:
    solver = new guinfgQreAll(*sup, this);
    break;
  default:
    // shouldn't happen.  we'll ignore silently
    return;
  }

  bool go = solver->SolveSetup();

  wxBeginBusyCursor();

  try {
    if (go) {
      solver->Eliminate();
      solns += solver->Solve();
    }
    wxEndBusyCursor();
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), Frame());
    wxEndBusyCursor();
  }
    
  delete solver;

  if (!go)  return;

  if (old_max_soln != solns.Length()) {
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

    ChangeSolution(solns.VisibleLength());
  }
}

void NfgShow::OnViewSolutions(wxCommandEvent &)
{
  if (solns.Length() == 0) {
    wxMessageBox("Solution list currently empty");
    return;
  }

  if (m_solutionTable) {
    delete m_solutionTable;
    m_solutionTable = 0;
  }
  else {
    m_solutionTable = new NfgSolnShow(solns, m_nfg.NumPlayers(), 
				gmax(m_nfg.NumStrats()), 
				cur_soln, draw_settings, 
				sf_options, this);
    m_solutionTable->Show(true);
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
  UpdateSoln();
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
  UpdateSoln();
}

void NfgShow::OnViewOutcomes(wxCommandEvent &)
{
  draw_settings.SetOutcomeValues(1 - draw_settings.OutcomeValues());
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
    draw_settings.SetDataFont(dialog.GetFontData().GetChosenFont());
    m_table->SetCellTextFont(dialog.GetFontData().GetChosenFont());
  }
}

void NfgShow::OnPrefsColors(wxCommandEvent &)
{
}

void NfgShow::OnPrefsAccels(wxCommandEvent &)
{
  EditAccelerators(accelerators, MakeEventNames());
  //  WriteAccelerators(accelerators, "NfgAccelerators", wxGetApp().ResourceFile());
}


void NfgShow::OnPrefsSave(wxCommandEvent &)
{
  draw_settings.SaveSettings();
}

void NfgShow::OnPrefsLoad(wxCommandEvent &)
{
  draw_settings.LoadSettings();
}


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

  menu->Enable(NFG_VIEW_PROBABILITIES, NumSolutions() > 0);
  menu->Enable(NFG_VIEW_VALUES, NumSolutions() > 0);

  SetStatusText((char *)
		("Support: " + CurrentSupport()->GetName()), 1);
  if (CurrentSolution() > 0) {
    SetStatusText((char *) ("Solution: " + 
			    ToText((int) Solutions()[CurrentSolution()].Id())),
		  2);
  }
  else {
    SetStatusText("No solution displayed", 2);
  }
}

void NfgShow::AdjustSizes(void)
{
  int width, height;
  GetClientSize(&width, &height);
  if (m_solutionTable && m_solutionTable->IsShown()) {
    int solnHeight = gmax(100, height / 3);
    m_solutionTable->SetSize(0, height - solnHeight, width, solnHeight);
    height -= solnHeight;
  }
  if (m_panel) {
    m_panel->SetSize(0, 40, 200, height - 40);
  }
  if (m_table) {
    m_table->SetSize(200, 40, width - 200, height - 40);
  }
  if (m_toolbar) {
    m_toolbar->SetSize(0, 0, width, 40);
  }
}


void NfgShow::OnSize(wxSizeEvent &)
{
  AdjustSizes();
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
  int id = ::CheckAccelerators(accelerators, ev);

#ifdef NOT_PORTED_YET
  if (id) 
    m_table->OnMenuCommand(id);
#endif  // NOT_PORTED_YET

  return id;
}

template class SolutionList<MixedSolution>;


//**********************************************************************
//                       NORMAL DRAW OPTIONS
//**********************************************************************

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

