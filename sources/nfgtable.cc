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

BEGIN_EVENT_TABLE(NfgTable, wxGrid)
  EVT_GRID_CELL_LEFT_CLICK(NfgTable::OnLeftClick)
  EVT_GRID_CELL_LEFT_DCLICK(NfgTable::OnLeftDoubleClick)
END_EVENT_TABLE()

NfgTable::NfgTable(NfgShow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_parent)
{
  CreateGrid(p_parent->CurrentSupport()->NumStrats(1),
	     p_parent->CurrentSupport()->NumStrats(2));
  SetGridCursor(0, 0);
  SetEditable(false);
  DisableDragRowSize();
  AdjustScrollbars();
  Show(true);
}

void NfgTable::SetProfile(const gArray<int> &p_profile)
{
  OnChangeLabels();
  OnChangeValues();
  SetGridCursor(p_profile[m_parent->GetRowPlayer()] - 1,
		p_profile[m_parent->GetColPlayer()] - 1);
}

void NfgTable::SetPlayers(int p_rowPlayer, int p_colPlayer)
{ 
  const NFSupport &support = *m_parent->CurrentSupport();

  if (support.NumStrats(p_rowPlayer) + m_showProb + m_showDom + m_showValue
      > GetRows()) {
    InsertRows(0, support.NumStrats(p_rowPlayer) - GetRows() +
	       m_showProb + m_showDom + m_showValue);
  }
  else if (support.NumStrats(p_rowPlayer) + m_showProb + m_showDom +
	   m_showValue < GetRows()) {
    DeleteRows(0, GetRows() - support.NumStrats(p_rowPlayer) - m_showProb -
	       m_showDom - m_showValue);
  }

  if (support.NumStrats(p_colPlayer) + m_showProb + m_showDom + m_showValue
      > GetCols()) {
    InsertCols(0, support.NumStrats(p_colPlayer) - GetCols() + m_showProb +
	       m_showDom + m_showValue);
  }
  else if (support.NumStrats(p_colPlayer) + m_showProb + m_showDom +
	   m_showValue < GetCols()) {
    DeleteCols(0, GetCols() - support.NumStrats(p_colPlayer) - m_showProb -
	       m_showDom - m_showValue);
  }

  AdjustScrollbars();
  OnChangeLabels();
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

  if (m_showProb) {
    SetLabelValue(wxHORIZONTAL, "Prob", support->NumStrats(colPlayer));
    SetLabelValue(wxVERTICAL, "Prob", support->NumStrats(rowPlayer));
  }

  if (m_showDom) {
    SetLabelValue(wxHORIZONTAL, "Domin",
		  support->NumStrats(colPlayer) + m_showProb);
    SetLabelValue(wxVERTICAL, "Domin",
		  support->NumStrats(rowPlayer) + m_showProb);
  }

  if (m_showValue) {
    SetLabelValue(wxHORIZONTAL, "Value", support->NumStrats(colPlayer) + 
		  m_showProb + m_showDom);
    SetLabelValue(wxVERTICAL, "Value", support->NumStrats(rowPlayer) +
		  m_showProb + m_showDom);
  }
}

void NfgTable::OnChangeValues(void)
{
  const Nfg &nfg = m_parent->Game();
  const NFSupport &support = *m_parent->CurrentSupport();
  int rowPlayer = m_parent->GetRowPlayer();
  int colPlayer = m_parent->GetColPlayer();

  ClearGrid();
  SetCellBackgroundColour(*wxWHITE);

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

      SetCellValue((char *) pay_str, i - 1, j - 1);
    }

    if (ShowDominance()) { 
      int dom_pos = GetCols() - ShowValues();
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
   
  if (ShowDominance()) {
    int dom_pos = GetRows() - ShowValues();

    for (int j = 1; j <= support.NumStrats(m_parent->GetColPlayer()); j++) {
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

    for (int j = 0; j < GetRows(); j++) {
      SetCellBackgroundColour(*wxLIGHT_GREY, j, dom_pos - 1);
    }
    for (int j = 0; j < GetCols(); j++) {
      SetCellBackgroundColour(*wxLIGHT_GREY, dom_pos - 1, j);
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

    SetCellValue((char *) ToText(cont_prob, GetDecimals()),
		 support.NumStrats(rowPlayer),
		 support.NumStrats(colPlayer));
  }

  // Hilight the cells w/ nonzero prob
  gNumber eps;
  gEpsilon(eps, GetDecimals());
 
  if (ShowProbs()) {
    for (int i = 1; i <= support.NumStrats(rowPlayer); i++)
      SetCellValue((char *) ToText(soln(support.Strategies(rowPlayer)[i]),
				   GetDecimals()),
		   i-1, support.NumStrats(colPlayer));

    for (int i = 1; i <= support.NumStrats(colPlayer); i++)
      SetCellValue((char *) ToText(soln(support.Strategies(colPlayer)[i]),
				   GetDecimals()),
		   support.NumStrats(rowPlayer), i-1);

    for (int i = 0; i < GetRows(); i++) {
      SetCellBackgroundColour(*wxLIGHT_GREY, i, support.NumStrats(colPlayer));
      SetCellBackgroundColour(*wxLIGHT_GREY, support.NumStrats(rowPlayer), i);
    }
  }

  if (ShowValues()) {
    for (int i = 1; i <= support.NumStrats(rowPlayer); i++) {
      SetCellValue((char *) ToText(soln.Payoff(nfg.Players()[rowPlayer],
					       support.Strategies(rowPlayer)[i]),
					   GetDecimals()),
			   i-1, support.NumStrats(colPlayer)+ShowProbs()+ShowDominance());
    }
    
    for (int j = 1; j <= support.NumStrats(colPlayer); j++) {
      SetCellValue((char *) ToText(soln.Payoff(nfg.Players()[colPlayer],
					       support.Strategies(colPlayer)[j]),
				   GetDecimals()),
		   support.NumStrats(rowPlayer)+ShowProbs()+ShowDominance(), j-1);

    }
    
    for (int i = 0; i < GetRows(); i++) {
      SetCellBackgroundColour(*wxLIGHT_GREY, i,
			      support.NumStrats(colPlayer)+ShowProbs()+ShowDominance());
    }
    for (int i = 0; i < GetCols(); i++) {
      SetCellBackgroundColour(*wxLIGHT_GREY,
			      support.NumStrats(rowPlayer)+ShowProbs()+ShowDominance(),
			      i);
    }
  }
}

void NfgTable::ToggleProbs(void)
{
  if (m_showProb) {
    DeleteRows();
    DeleteCols();
    m_showProb = 0;
  }
  else {
    InsertRows();
    InsertCols();
    m_showProb = 1;
  }

  AdjustScrollbars();
  OnChangeLabels();
  OnChangeValues();
}

void NfgTable::ToggleDominance(void)
{
  if (m_showDom) {
    DeleteRows();
    DeleteCols();
    m_showDom = 0;
  }
  else {
    InsertRows();
    InsertCols();
    m_showDom = 1;
  }

  AdjustScrollbars();
  OnChangeLabels();
  OnChangeValues();
}


void NfgTable::ToggleValues(void)
{
  if (m_showValue) {
    DeleteRows();
    DeleteCols();
    m_showValue = 0;
  }
  else {
    InsertRows();
    InsertCols();
    m_showValue = 1;
  }

  AdjustScrollbars();
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


