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

  if (features.prob) {
    SetLabelValue(wxHORIZONTAL, "Prob", support->NumStrats(colPlayer));
    SetLabelValue(wxVERTICAL, "Prob", support->NumStrats(rowPlayer));
  }

  if (features.dom) {
    SetLabelValue(wxHORIZONTAL, "Domin",
		  support->NumStrats(colPlayer) + features.prob);
    SetLabelValue(wxVERTICAL, "Domin",
		  support->NumStrats(rowPlayer) + features.prob);
  }

  if (features.val) {
    SetLabelValue(wxHORIZONTAL, "Value", support->NumStrats(colPlayer) + 
		  features.prob + features.dom);
    SetLabelValue(wxVERTICAL, "Value", support->NumStrats(rowPlayer) +
		  features.prob + features.dom);
  }
}

void NfgTable::OnChangeValues(void)
{
  const Nfg &nfg = m_parent->Game();
  const NFSupport &support = *m_parent->CurrentSupport();
  int rowPlayer = m_parent->GetRowPlayer();
  int colPlayer = m_parent->GetColPlayer();

  ClearGrid();

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

  if (HaveProbs()) {
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

  // Set the profile boxes to correct values if this is a pure equ
  SetProfile(profile);

  // Hilight the cells w/ nonzero prob
  gNumber eps;
  gEpsilon(eps, GetDecimals());
 
  if (HaveProbs()) {
    for (int i = 1; i <= support.NumStrats(rowPlayer); i++)
      SetCellValue((char *) ToText(soln(support.Strategies(rowPlayer)[i]),
				   GetDecimals()),
		   i-1, support.NumStrats(colPlayer));

    for (int i = 1; i <= support.NumStrats(colPlayer); i++)
      SetCellValue((char *) ToText(soln(support.Strategies(colPlayer)[i]),
				   GetDecimals()),
		   support.NumStrats(rowPlayer), i-1);
  }

  if (HaveVal()) {
    for (int i = 1; i <= support.NumStrats(rowPlayer); i++) {
      SetCellValue((char *) ToText(soln.Payoff(nfg.Players()[rowPlayer],
					       support.Strategies(rowPlayer)[i]),
					   GetDecimals()),
			   i-1, support.NumStrats(colPlayer)+HaveProbs()+HaveDom());
    }
    
    for (int j = 1; j <= support.NumStrats(colPlayer); j++) {
      SetCellValue((char *) ToText(soln.Payoff(nfg.Players()[colPlayer],
					       support.Strategies(colPlayer)[j]),
				   GetDecimals()),
		   support.NumStrats(rowPlayer)+HaveProbs()+HaveDom(), j-1);
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
  if (!features.prob) {
    InsertRows();
    InsertCols();
    features.prob = 1;
    AdjustScrollbars();
    OnChangeLabels();
    OnChangeValues();
  }
}


void NfgTable::RemoveProbDisp(void)
{
  if (features.prob) {
    DeleteRows();
    DeleteCols();
    features.prob = 0;
    AdjustScrollbars();
    OnChangeLabels();
    OnChangeValues();
  }
}

void NfgTable::MakeDomDisp(void)
{
  if (!features.dom) {
    features.dom = 1;
    AppendRows();
    AppendCols();
    AdjustScrollbars();
    OnChangeLabels();
    OnChangeValues();
  }
}


void NfgTable::RemoveDomDisp(void)
{
  if (features.dom) {
    features.dom = 0;
    DeleteRows();
    DeleteCols();
    AdjustScrollbars();
    OnChangeLabels();
    OnChangeValues();
  }
}

void NfgTable::MakeValDisp(void)
{
  if (!features.val) {
    InsertRows();
    InsertCols();
    features.val = 1;
    AdjustScrollbars();
    OnChangeLabels();
    OnChangeValues();
  }
}


void NfgTable::RemoveValDisp(void)
{
  if (features.val) {
    DeleteRows();
    DeleteCols();
    features.val = 0;
    AdjustScrollbars();
    OnChangeLabels();
    OnChangeValues();
  }
}

void NfgTable::OnLeftDoubleClick(wxGridEvent &p_event)
{
  if (p_event.GetRow() < m_parent->CurrentSupport()->NumStrats(m_parent->GetRowPlayer()) &&
      p_event.GetCol() < m_parent->CurrentSupport()->NumStrats(m_parent->GetColPlayer())) {
    m_parent->OutcomePayoffs(p_event.GetRow()+1, p_event.GetCol()+1); 
  }
}


