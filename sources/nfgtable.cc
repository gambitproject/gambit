//
// FILE: nfgtable.cc -- Implementation of normal form table class
//
// $Id$
//

#include "wx/wx.h"
#include "nfgshow.h"
#include "nfgtable.h"

//======================================================================
//                   class NfgTable: Member functions
//======================================================================

BEGIN_EVENT_TABLE(NfgTable, wxPanel)
  EVT_GRID_CELL_LEFT_CLICK(NfgTable::OnLeftClick)
  EVT_GRID_CELL_LEFT_DCLICK(NfgTable::OnLeftDoubleClick)
  EVT_CHOICE(idSTRATEGY_CHOICE, NfgTable::OnStrategyChange)
  EVT_CHOICE(idROWPLAYER_CHOICE, NfgTable::OnRowPlayerChange)
  EVT_CHOICE(idCOLPLAYER_CHOICE, NfgTable::OnColPlayerChange)
END_EVENT_TABLE()

NfgTable::NfgTable(NfgShow *p_parent)
  : wxPanel(p_parent, -1), m_parent(p_parent)
{
  SetAutoLayout(true);

  wxStaticBoxSizer *playerViewSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "View players"),
			 wxVERTICAL);

  wxBoxSizer *rowChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
  rowChoiceSizer->Add(new wxStaticText(this, -1, "Row player"),
		      1, wxALIGN_LEFT | wxRIGHT, 5);
  m_rowChoice = new wxChoice(this, idROWPLAYER_CHOICE);
  rowChoiceSizer->Add(m_rowChoice, 0, wxALL, 0);

  wxBoxSizer *colChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
  colChoiceSizer->Add(new wxStaticText(this, -1, "Column player"),
		      1, wxALIGN_LEFT | wxRIGHT, 5);
  m_colChoice = new wxChoice(this, idCOLPLAYER_CHOICE);
  colChoiceSizer->Add(m_colChoice, 0, wxALL, 0);

  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    wxString playerName = (char *) (ToText(pl) + ": " +
				    m_parent->Game().Players()[pl]->GetName());
    m_rowChoice->Append(playerName);
    m_colChoice->Append(playerName);
  }

  m_rowChoice->SetSelection(0);
  m_colChoice->SetSelection(1);

  playerViewSizer->Add(rowChoiceSizer, 0, wxALL | wxEXPAND, 5);
  playerViewSizer->Add(colChoiceSizer, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *contViewSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Current contingency"),
			 wxVERTICAL);

  m_stratProfile = new wxChoice *[m_parent->Game().NumPlayers()];
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    m_stratProfile[pl-1] = new wxChoice(this, idSTRATEGY_CHOICE);
    
    NFPlayer *player = m_parent->Game().Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					     player->Strategies()[st]->Name()));
    }
    m_stratProfile[pl-1]->SetSelection(0);

    wxBoxSizer *stratSizer = new wxBoxSizer(wxHORIZONTAL);
    stratSizer->Add(new wxStaticText(this, -1,
				     (char *) ("Player " + ToText(pl))),
		    1, wxALIGN_LEFT | wxRIGHT, 5);
    stratSizer->Add(m_stratProfile[pl-1], 0, wxALL, 0);
    contViewSizer->Add(stratSizer, 0, wxALL | wxEXPAND, 5);
  }

  wxBoxSizer *navPanelSizer = new wxBoxSizer(wxVERTICAL);
  navPanelSizer->Add(playerViewSizer, 0, wxALL | wxEXPAND, 10);
  navPanelSizer->Add(contViewSizer, 0, wxALL | wxEXPAND, 10);

  m_grid = new wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize);
  m_grid->CreateGrid(p_parent->CurrentSupport()->NumStrats(1),
		     p_parent->CurrentSupport()->NumStrats(2));
  m_grid->SetGridCursor(0, 0);
  m_grid->SetEditable(false);
  m_grid->DisableDragRowSize();
  m_grid->AdjustScrollbars();

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(navPanelSizer, 0, wxALL, 5);
  topSizer->Add(m_grid, 1, wxALL | wxEXPAND | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  Show(true);
}

void NfgTable::SetProfile(const gArray<int> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    m_stratProfile[i-1]->SetSelection(p_profile[i] - 1);
  }

  OnChangeLabels();
  OnChangeValues();
  m_grid->SetGridCursor(p_profile[m_parent->GetRowPlayer()] - 1,
			p_profile[m_parent->GetColPlayer()] - 1);
}

gArray<int> NfgTable::GetProfile(void) const
{
  gArray<int> profile(m_parent->Game().NumPlayers());
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = m_stratProfile[i-1]->GetSelection() + 1;
  }
  return profile;
}

void NfgTable::SetPlayers(int p_rowPlayer, int p_colPlayer)
{ 
  const NFSupport &support = *m_parent->CurrentSupport();

  if (support.NumStrats(p_rowPlayer) + m_showProb + m_showDom + m_showValue
      > m_grid->GetRows()) {
    m_grid->InsertRows(0, support.NumStrats(p_rowPlayer) - m_grid->GetRows() +
		       m_showProb + m_showDom + m_showValue);
  }
  else if (support.NumStrats(p_rowPlayer) + m_showProb + m_showDom +
	   m_showValue < m_grid->GetRows()) {
    m_grid->DeleteRows(0, m_grid->GetRows() - support.NumStrats(p_rowPlayer) - m_showProb -
		       m_showDom - m_showValue);
  }

  if (support.NumStrats(p_colPlayer) + m_showProb + m_showDom + m_showValue
      > m_grid->GetCols()) {
    m_grid->InsertCols(0, support.NumStrats(p_colPlayer) - m_grid->GetCols() + m_showProb +
		       m_showDom + m_showValue);
  }
  else if (support.NumStrats(p_colPlayer) + m_showProb + m_showDom +
	   m_showValue < m_grid->GetCols()) {
    m_grid->DeleteCols(0, m_grid->GetCols() - support.NumStrats(p_colPlayer) - m_showProb -
		       m_showDom - m_showValue);
  }

  m_grid->AdjustScrollbars();
  OnChangeLabels();
  OnChangeValues();
}

void NfgTable::SetStrategy(int p_player, int p_strategy)
{
  m_stratProfile[p_player-1]->SetSelection(p_strategy-1);

  if (p_player == m_parent->GetRowPlayer()) {
    m_grid->SetGridCursor(p_strategy - 1, m_grid->GetCursorColumn());
  }
  else if (p_player == m_parent->GetColPlayer()) {
    m_grid->SetGridCursor(m_grid->GetCursorRow(), p_strategy - 1);
  }
}

void NfgTable::OnChangeLabels(void)
{
  const NFSupport *support = m_parent->CurrentSupport();
  gArray<int> profile = m_parent->GetProfile();
  int rowPlayer = m_parent->GetRowPlayer();
  int colPlayer = m_parent->GetColPlayer();

  for (int i = 1; i <= support->NumStrats(colPlayer); i++) {
    m_grid->SetLabelValue(wxHORIZONTAL,
			  (char *) support->Strategies(colPlayer)[i]->Name(),
			  i - 1);
  }
  
  for (int i = 1; i <= support->NumStrats(rowPlayer); i++) {
    m_grid->SetLabelValue(wxVERTICAL,
			  (char *) support->Strategies(rowPlayer)[i]->Name(),
			  i - 1);
  }

  if (m_showProb) {
    m_grid->SetLabelValue(wxHORIZONTAL, "Prob", support->NumStrats(colPlayer));
    m_grid->SetLabelValue(wxVERTICAL, "Prob", support->NumStrats(rowPlayer));
  }

  if (m_showDom) {
    m_grid->SetLabelValue(wxHORIZONTAL, "Domin",
			  support->NumStrats(colPlayer) + m_showProb);
    m_grid->SetLabelValue(wxVERTICAL, "Domin",
			  support->NumStrats(rowPlayer) + m_showProb);
  }

  if (m_showValue) {
    m_grid->SetLabelValue(wxHORIZONTAL, "Value", support->NumStrats(colPlayer) + 
			  m_showProb + m_showDom);
    m_grid->SetLabelValue(wxVERTICAL, "Value", support->NumStrats(rowPlayer) +
			  m_showProb + m_showDom);
  }
}

void NfgTable::OnChangeValues(void)
{
  const Nfg &nfg = m_parent->Game();
  const NFSupport &support = *m_parent->CurrentSupport();
  int rowPlayer = m_parent->GetRowPlayer();
  int colPlayer = m_parent->GetColPlayer();

  m_grid->ClearGrid();
  m_grid->SetCellBackgroundColour(*wxWHITE);

  NfgIter iterator(support);
  iterator.Set(m_parent->GetProfile());

  for (int i = 1; i <= support.NumStrats(m_parent->GetRowPlayer()); i++) {
    for (int j = 1; j <= support.NumStrats(m_parent->GetColPlayer()); j++) {
      iterator.Set(m_parent->GetRowPlayer(), i);
      iterator.Set(m_parent->GetColPlayer(), j);
      gText pay_str;
      NFOutcome *outcome = iterator.GetOutcome();

      if (m_parent->OutcomeValues()) {
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

      m_grid->SetCellValue((char *) pay_str, i - 1, j - 1);
    }

    if (ShowDominance()) { 
      int dom_pos = m_grid->GetCols() - ShowValues();
      Strategy *strategy = support.Strategies(m_parent->GetRowPlayer())[i];
      if (support.IsDominated(strategy, true)) {
	m_grid->SetCellValue("S", i - 1, dom_pos - 1);
      }
      else if (support.IsDominated(strategy, false)) {
	m_grid->SetCellValue("W", i - 1, dom_pos - 1);
      }
      else {
        m_grid->SetCellValue("N", i - 1, dom_pos - 1);
      }
    }
  }
   
  if (ShowDominance()) {
    int dom_pos = m_grid->GetRows() - ShowValues();

    for (int j = 1; j <= support.NumStrats(m_parent->GetColPlayer()); j++) {
      Strategy *strategy = support.Strategies(m_parent->GetColPlayer())[j];
      if (support.IsDominated(strategy, true)) { 
	m_grid->SetCellValue("S", dom_pos - 1, j - 1);
      }
      else if (support.IsDominated(strategy, false)) {
	m_grid->SetCellValue("W", dom_pos - 1, j - 1);
      }
      else {
	m_grid->SetCellValue("N", dom_pos - 1, j - 1);
      }
    }

    for (int j = 0; j < m_grid->GetRows(); j++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, j, dom_pos - 1);
    }
    for (int j = 0; j < m_grid->GetCols(); j++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, dom_pos - 1, j);
    }
  }

  if (m_parent->CurrentSolution() == 0) {
    return;
  }

  MixedSolution soln = m_parent->Solutions()[m_parent->CurrentSolution()];
  gNumber t_max;
  gArray<int> profile(nfg.NumPlayers());

  // Figure out the index in the current support,
  // then map it onto the full support
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    profile[pl] = 1;
    t_max = soln(nfg.Players()[pl]->Strategies()[1]);

    for (int st1 = 1; st1 <= support.NumStrats(pl); st1++) {
      if (soln(support.Strategies(pl)[st1]) > t_max) {
	profile[pl] = st1;
	t_max = soln(support.Strategies(pl)[st1]);
      }
    }
  }

  if (ShowProbs()) {
    gNumber cont_prob(1);

    for (int i = 1; i <= nfg.NumPlayers(); i++) {
      if (i != rowPlayer && i != colPlayer) {
	NFPlayer *player = nfg.Players()[i];
	cont_prob *= soln(player->Strategies()[profile[i]]);
      }
    }

    m_grid->SetCellValue((char *) ToText(cont_prob, GetDecimals()),
			 support.NumStrats(rowPlayer),
			 support.NumStrats(colPlayer));
  }

  // Hilight the cells w/ nonzero prob
  gNumber eps;
  gEpsilon(eps, GetDecimals());
 
  if (ShowProbs()) {
    for (int i = 1; i <= support.NumStrats(rowPlayer); i++)
      m_grid->SetCellValue((char *) ToText(soln(support.Strategies(rowPlayer)[i]),
					   GetDecimals()),
			   i-1, support.NumStrats(colPlayer));

    for (int i = 1; i <= support.NumStrats(colPlayer); i++)
      m_grid->SetCellValue((char *) ToText(soln(support.Strategies(colPlayer)[i]),
					   GetDecimals()),
			   support.NumStrats(rowPlayer), i-1);

    for (int i = 0; i < m_grid->GetRows(); i++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, i,
				      support.NumStrats(colPlayer));
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, 
				      support.NumStrats(rowPlayer), i);
    }
  }

  if (ShowValues()) {
    for (int i = 1; i <= support.NumStrats(rowPlayer); i++) {
      m_grid->SetCellValue((char *) ToText(soln.Payoff(nfg.Players()[rowPlayer],
						       support.Strategies(rowPlayer)[i]),
					   GetDecimals()),
			   i-1, support.NumStrats(colPlayer)+ShowProbs()+ShowDominance());
    }
    
    for (int j = 1; j <= support.NumStrats(colPlayer); j++) {
      m_grid->SetCellValue((char *) ToText(soln.Payoff(nfg.Players()[colPlayer],
						       support.Strategies(colPlayer)[j]),
					   GetDecimals()),
			   support.NumStrats(rowPlayer)+ShowProbs()+ShowDominance(), j-1);

    }
    
    for (int i = 0; i < m_grid->GetRows(); i++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, i,
				      support.NumStrats(colPlayer)+ShowProbs()+ShowDominance());
    }
    for (int i = 0; i < m_grid->GetCols(); i++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY,
				      support.NumStrats(rowPlayer)+ShowProbs()+ShowDominance(),
				      i);
    }
  }
}

void NfgTable::ToggleProbs(void)
{
  if (m_showProb) {
    m_grid->DeleteRows();
    m_grid->DeleteCols();
    m_showProb = 0;
  }
  else {
    m_grid->InsertRows();
    m_grid->InsertCols();
    m_showProb = 1;
  }

  m_grid->AdjustScrollbars();
  OnChangeLabels();
  OnChangeValues();
}

void NfgTable::ToggleDominance(void)
{
  if (m_showDom) {
    m_grid->DeleteRows();
    m_grid->DeleteCols();
    m_showDom = 0;
  }
  else {
    m_grid->InsertRows();
    m_grid->InsertCols();
    m_showDom = 1;
  }

  m_grid->AdjustScrollbars();
  OnChangeLabels();
  OnChangeValues();
}


void NfgTable::ToggleValues(void)
{
  if (m_showValue) {
    m_grid->DeleteRows();
    m_grid->DeleteCols();
    m_showValue = 0;
  }
  else {
    m_grid->InsertRows();
    m_grid->InsertCols();
    m_showValue = 1;
  }

  m_grid->AdjustScrollbars();
  OnChangeLabels();
  OnChangeValues();
}

void NfgTable::OnLeftClick(wxGridEvent &p_event)
{
  if (p_event.GetRow() >= 
      m_parent->CurrentSupport()->NumStrats(m_parent->GetRowPlayer()) ||
      p_event.GetCol() >=
      m_parent->CurrentSupport()->NumStrats(m_parent->GetColPlayer())) {
    p_event.Veto();
  }
  else {
    m_parent->SetStrategy(m_parent->GetRowPlayer(), p_event.GetRow() + 1);
    m_parent->SetStrategy(m_parent->GetColPlayer(), p_event.GetCol() + 1);
  }
}

void NfgTable::OnLeftDoubleClick(wxGridEvent &p_event)
{
  if (p_event.GetRow() < m_parent->CurrentSupport()->NumStrats(m_parent->GetRowPlayer()) &&
      p_event.GetCol() < m_parent->CurrentSupport()->NumStrats(m_parent->GetColPlayer())) {
    m_parent->OutcomePayoffs(p_event.GetRow()+1, p_event.GetCol()+1); 
  }
}

void NfgTable::OnStrategyChange(wxCommandEvent &)
{
  m_parent->SetProfile(GetProfile());
}

void NfgTable::OnRowPlayerChange(wxCommandEvent &)
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

void NfgTable::OnColPlayerChange(wxCommandEvent &)
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
    m_parent->SetPlayers(m_rowChoice->GetSelection() + 1, newColPlayer);
  }
}

void NfgTable::SetSupport(const NFSupport &p_support)
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

