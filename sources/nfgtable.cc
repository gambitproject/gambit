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

void NfgTable::OnLeftDoubleClick(wxGridEvent &p_event)
{
  m_parent->OutcomePayoffs(p_event.GetRow()+1, p_event.GetCol()+1); 
}

