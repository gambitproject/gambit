//
// FILE: nfgtable.cc -- Implementation of normal form table class
//
// $Id$
//

#include "wx/wx.h"
#include "wx/config.h"
#include "nfgshow.h"
#include "nfgtable.h"
#include "nfgconst.h"

class NfgGridTable : public wxGridTableBase {
private:
  Nfg *m_nfg;

public:
  NfgGridTable(Nfg *p_nfg);
  virtual ~NfgGridTable() { }

  int GetNumberRows(void) { return m_nfg->NumStrats(1); }
  int GetNumberCols(void) { return m_nfg->NumStrats(2); }
  wxString GetValue(int row, int col) {
    gArray<int> strategy(m_nfg->NumPlayers());
    for (int pl = 1; pl <= strategy.Length(); pl++) {
      strategy[pl] = 1;
    }
    strategy[1] = row + 1;
    strategy[2] = col + 1;

    NFOutcome *outcome = m_nfg->GetOutcome(strategy);
    return wxString::Format("%s,%s",
			    (char *) ToText(m_nfg->Payoff(outcome, 1)),
			    (char *) ToText(m_nfg->Payoff(outcome, 2)));
  }

  wxString GetRowLabelValue(int);
  wxString GetColLabelValue(int);
  void SetValue(int, int, const wxString &) { /* ignore */ }
  bool IsEmptyCell(int, int) { return false; }
};

NfgGridTable::NfgGridTable(Nfg *p_nfg)
  : m_nfg(p_nfg)
{ }

wxString NfgGridTable::GetRowLabelValue(int p_row)
{
  return (char *) m_nfg->Strategies(1)[p_row+1]->Name();
}

wxString NfgGridTable::GetColLabelValue(int p_col)
{
  return (char *) m_nfg->Strategies(2)[p_col+1]->Name();
}

class ColoredStringRenderer : public wxGridCellRenderer {
public:
  // draw the string
  virtual void Draw(wxGrid& grid,
		    wxGridCellAttr& attr,
		    wxDC& dc,
		    const wxRect& rect,
		    int row, int col,
		    bool isSelected);

  // return the string extent
  virtual wxSize GetBestSize(wxGrid& grid,
			     wxGridCellAttr& attr,
			     wxDC& dc,
			     int row, int col);

  virtual wxGridCellRenderer *Clone() const
    { return new ColoredStringRenderer; }

protected:
  // set the text colours before drawing
  void SetTextColoursAndFont(wxGrid& grid,
			     wxGridCellAttr& attr,
			     wxDC& dc,
			     bool isSelected);

  // calc the string extent for given string/font
  wxSize DoGetBestSize(wxGridCellAttr& attr,
		       wxDC& dc,
		       const wxString& text);
};

void ColoredStringRenderer::SetTextColoursAndFont(wxGrid& grid,
                                                     wxGridCellAttr& attr,
                                                     wxDC& dc,
                                                     bool isSelected)
{
    dc.SetBackgroundMode( wxTRANSPARENT );

    // TODO some special colours for attr.IsReadOnly() case?

    if ( isSelected )
    {
        dc.SetTextBackground( grid.GetSelectionBackground() );
        dc.SetTextForeground( grid.GetSelectionForeground() );
    }
    else
    {
        dc.SetTextBackground( attr.GetBackgroundColour() );
        dc.SetTextForeground( attr.GetTextColour() );
    }

    dc.SetFont( attr.GetFont() );
}

wxSize ColoredStringRenderer::DoGetBestSize(wxGridCellAttr& attr,
					    wxDC& dc,
					    const wxString& text)
{
  wxCoord x = 0, y = 0;
  dc.SetFont(attr.GetFont());
  dc.GetTextExtent(text, &x, &y);
  return wxSize(x, y);
}

wxSize ColoredStringRenderer::GetBestSize(wxGrid& grid,
                                             wxGridCellAttr& attr,
                                             wxDC& dc,
                                             int row, int col)
{
  return DoGetBestSize(attr, dc, grid.GetCellValue(row, col));
}

void ColoredStringRenderer::Draw(wxGrid& grid,
                                    wxGridCellAttr& attr,
                                    wxDC& dc,
                                    const wxRect& rectCell,
                                    int row, int col,
                                    bool isSelected)
{
  wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

  // now we only have to draw the text
  SetTextColoursAndFont(grid, attr, dc, isSelected);

  wxRect rect = rectCell;
  rect.Inflate(-1);

  wxCoord x, y;
  grid.DrawTextRectangle(dc, "(", rect);
  dc.GetTextExtent("(", &x, &y);
  rect.x += x;

  wxString text = grid.GetCellValue(row, col);
  dc.SetTextForeground(*wxRED);
  for (unsigned int i = 0; i < text.Length(); i++) {
    if (text[i] == ',') {
      wxColour color = dc.GetTextForeground();
      dc.SetTextForeground(*wxBLACK);
      grid.DrawTextRectangle(dc, ",", rect);
      dc.GetTextExtent(",", &x, &y);
      rect.x += x;

      if (color == *wxRED) {
	dc.SetTextForeground(*wxBLUE);
      }
      else {
	dc.SetTextForeground(*wxRED);
      }
    }
    else {
      grid.DrawTextRectangle(dc, text[i], rect);
      dc.GetTextExtent(text[i], &x, &y);
      rect.x += x;
    }
  }
  
  dc.SetTextForeground(*wxBLACK);
  grid.DrawTextRectangle(dc, ")", rect); 
}

//======================================================================
//                   class NfgTable: Member functions
//======================================================================

BEGIN_EVENT_TABLE(NfgTable, wxPanel)
  EVT_GRID_CELL_LEFT_CLICK(NfgTable::OnLeftClick)
  EVT_GRID_CELL_LEFT_DCLICK(NfgTable::OnLeftDoubleClick)
  EVT_GRID_LABEL_LEFT_CLICK(NfgTable::OnLabelLeftClick)
  EVT_CHOICE(idSTRATEGY_CHOICE, NfgTable::OnStrategyChange)
  EVT_CHOICE(idROWPLAYER_CHOICE, NfgTable::OnRowPlayerChange)
  EVT_CHOICE(idCOLPLAYER_CHOICE, NfgTable::OnColPlayerChange)
END_EVENT_TABLE()

NfgTable::NfgTable(Nfg &p_nfg, wxWindow *p_parent)
  : wxPanel(p_parent, -1), m_nfg(p_nfg), m_parent(p_parent), 
    m_editable(true), m_rowPlayer(1), m_colPlayer(2),
    m_support(m_nfg), m_solution(0),
    m_showProb(0), m_showDom(0), m_showValue(0)
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

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    wxString playerName = (char *) (ToText(pl) + ": " +
				    m_nfg.Players()[pl]->GetName());
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

  m_stratProfile = new wxChoice *[m_nfg.NumPlayers()];
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_stratProfile[pl-1] = new wxChoice(this, idSTRATEGY_CHOICE);
    
    NFPlayer *player = m_nfg.Players()[pl];
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

  m_grid = new wxGrid(this, -1, wxDefaultPosition, wxDefaultSize);
  //  m_grid->SetTable(new NfgGridTable(&m_nfg), true);
  m_grid->CreateGrid(m_support.NumStrats(1),
  		     m_support.NumStrats(2));
  m_grid->SetGridCursor(0, 0);
  m_grid->SetEditable(false);
  m_grid->DisableDragRowSize();
  m_grid->AdjustScrollbars();
  m_grid->SetDefaultRenderer(new ColoredStringRenderer);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(navPanelSizer, 0, wxALL, 5);
  topSizer->Add(m_grid, 1, wxALL | wxEXPAND | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();

  OnChangeLabels();
  OnChangeValues();
  Show(true);
}

void NfgTable::SetProfile(const gArray<int> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    m_stratProfile[i-1]->SetSelection(p_profile[i] - 1);
  }

  OnChangeLabels();
  OnChangeValues();
  m_grid->SetGridCursor(p_profile[GetRowPlayer()] - 1,
			p_profile[GetColPlayer()] - 1);
}

gArray<int> NfgTable::GetProfile(void) const
{
  gArray<int> profile(m_nfg.NumPlayers());
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = m_stratProfile[i-1]->GetSelection() + 1;
  }
  return profile;
}

void NfgTable::SetPlayers(int p_rowPlayer, int p_colPlayer)
{ 
  m_rowPlayer = p_rowPlayer;
  m_colPlayer = p_colPlayer;
  SetStrategy(m_rowPlayer, 1);
  SetStrategy(m_colPlayer, 1);

  if (m_support.NumStrats(p_rowPlayer) + m_showProb + m_showDom + m_showValue
      > m_grid->GetRows()) {
    m_grid->InsertRows(0, m_support.NumStrats(p_rowPlayer) - m_grid->GetRows() +
		       m_showProb + m_showDom + m_showValue);
  }
  else if (m_support.NumStrats(p_rowPlayer) + m_showProb + m_showDom +
	   m_showValue < m_grid->GetRows()) {
    m_grid->DeleteRows(0, m_grid->GetRows() - m_support.NumStrats(p_rowPlayer) - m_showProb -
		       m_showDom - m_showValue);
  }

  if (m_support.NumStrats(p_colPlayer) + m_showProb + m_showDom + m_showValue
      > m_grid->GetCols()) {
    m_grid->InsertCols(0, m_support.NumStrats(p_colPlayer) - m_grid->GetCols() + m_showProb +
		       m_showDom + m_showValue);
  }
  else if (m_support.NumStrats(p_colPlayer) + m_showProb + m_showDom +
	   m_showValue < m_grid->GetCols()) {
    m_grid->DeleteCols(0, m_grid->GetCols() - m_support.NumStrats(p_colPlayer) - m_showProb -
		       m_showDom - m_showValue);
  }

  m_grid->AdjustScrollbars();
  OnChangeLabels();
  OnChangeValues();
}

void NfgTable::SetStrategy(int p_player, int p_strategy)
{
  m_stratProfile[p_player-1]->SetSelection(p_strategy-1);

  if (p_player == GetRowPlayer()) {
    m_grid->SetGridCursor(p_strategy - 1, m_grid->GetCursorColumn());
  }
  else if (p_player == GetColPlayer()) {
    m_grid->SetGridCursor(m_grid->GetCursorRow(), p_strategy - 1);
  }
}

void NfgTable::OnChangeLabels(void)
{
  gArray<int> profile = GetProfile();
  int rowPlayer = GetRowPlayer();
  int colPlayer = GetColPlayer();

  for (int i = 1; i <= m_support.NumStrats(colPlayer); i++) {
    m_grid->SetLabelValue(wxHORIZONTAL,
			  (char *) m_support.Strategies(colPlayer)[i]->Name(),
			  i - 1);
  }
  
  for (int i = 1; i <= m_support.NumStrats(rowPlayer); i++) {
    m_grid->SetLabelValue(wxVERTICAL,
			  (char *) m_support.Strategies(rowPlayer)[i]->Name(),
			  i - 1);
  }

  if (m_showProb) {
    m_grid->SetLabelValue(wxHORIZONTAL, "Prob", m_support.NumStrats(colPlayer));
    m_grid->SetLabelValue(wxVERTICAL, "Prob", m_support.NumStrats(rowPlayer));
  }

  if (m_showDom) {
    m_grid->SetLabelValue(wxHORIZONTAL, "Domin",
			  m_support.NumStrats(colPlayer) + m_showProb);
    m_grid->SetLabelValue(wxVERTICAL, "Domin",
			  m_support.NumStrats(rowPlayer) + m_showProb);
  }

  if (m_showValue) {
    m_grid->SetLabelValue(wxHORIZONTAL, "Value", m_support.NumStrats(colPlayer) + 
			  m_showProb + m_showDom);
    m_grid->SetLabelValue(wxVERTICAL, "Value", m_support.NumStrats(rowPlayer) +
			  m_showProb + m_showDom);
  }
}

void NfgTable::OnChangeValues(void)
{
  const Nfg &nfg = m_nfg;
  int rowPlayer = GetRowPlayer();
  int colPlayer = GetColPlayer();

  m_grid->ClearGrid();
  m_grid->SetCellBackgroundColour(*wxWHITE);

  NfgIter iterator(m_support);
  iterator.Set(GetProfile());

  for (int i = 1; i <= m_support.NumStrats(GetRowPlayer()); i++) {
    for (int j = 1; j <= m_support.NumStrats(GetColPlayer()); j++) {
      iterator.Set(GetRowPlayer(), i);
      iterator.Set(GetColPlayer(), j);
      gText pay_str;
      NFOutcome *outcome = iterator.GetOutcome();

      if (OutcomeValues()) {
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
      Strategy *strategy = m_support.Strategies(GetRowPlayer())[i];
      if (m_support.IsDominated(strategy, true)) {
	m_grid->SetCellValue("S", i - 1, dom_pos - 1);
      }
      else if (m_support.IsDominated(strategy, false)) {
	m_grid->SetCellValue("W", i - 1, dom_pos - 1);
      }
      else {
        m_grid->SetCellValue("N", i - 1, dom_pos - 1);
      }
    }
  }

  if (ShowDominance()) {
    int dom_pos = m_grid->GetRows() - ShowValues();

    for (int j = 1; j <= m_support.NumStrats(GetColPlayer()); j++) {
      Strategy *strategy = m_support.Strategies(GetColPlayer())[j];
      if (m_support.IsDominated(strategy, true)) { 
	m_grid->SetCellValue("S", dom_pos - 1, j - 1);
      }
      else if (m_support.IsDominated(strategy, false)) {
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

  if (m_solution == 0) {
    return;
  }

  gNumber t_max;
  gArray<int> profile(nfg.NumPlayers());

  // Figure out the index in the current support,
  // then map it onto the full support
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    profile[pl] = 1;
    t_max = (*m_solution)(nfg.Players()[pl]->Strategies()[1]);

    for (int st1 = 1; st1 <= m_support.NumStrats(pl); st1++) {
      if ((*m_solution)(m_support.Strategies(pl)[st1]) > t_max) {
	profile[pl] = st1;
	t_max = (*m_solution)(m_support.Strategies(pl)[st1]);
      }
    }
  }

  if (ShowProbs()) {
    gNumber cont_prob(1);

    for (int i = 1; i <= nfg.NumPlayers(); i++) {
      if (i != rowPlayer && i != colPlayer) {
	NFPlayer *player = nfg.Players()[i];
	cont_prob *= (*m_solution)(player->Strategies()[profile[i]]);
      }
    }

    m_grid->SetCellValue((char *) ToText(cont_prob, GetDecimals()),
			 m_support.NumStrats(rowPlayer),
			 m_support.NumStrats(colPlayer));
  }

  // Hilight the cells w/ nonzero prob
  gNumber eps;
  gEpsilon(eps, GetDecimals());
 
  if (ShowProbs()) {
    for (int i = 1; i <= m_support.NumStrats(rowPlayer); i++)
      m_grid->SetCellValue((char *) ToText((*m_solution)(m_support.Strategies(rowPlayer)[i]),
					   GetDecimals()),
			   i-1, m_support.NumStrats(colPlayer));

    for (int i = 1; i <= m_support.NumStrats(colPlayer); i++)
      m_grid->SetCellValue((char *) ToText((*m_solution)(m_support.Strategies(colPlayer)[i]),
					   GetDecimals()),
			   m_support.NumStrats(rowPlayer), i-1);

    for (int i = 0; i < m_grid->GetRows(); i++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, i,
				      m_support.NumStrats(colPlayer));
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, 
				      m_support.NumStrats(rowPlayer), i);
    }
  }

  if (ShowValues()) {
    for (int i = 1; i <= m_support.NumStrats(rowPlayer); i++) {
      m_grid->SetCellValue((char *) ToText(m_solution->Payoff(nfg.Players()[rowPlayer],
						       m_support.Strategies(rowPlayer)[i]),
					   GetDecimals()),
			   i-1, m_support.NumStrats(colPlayer)+ShowProbs()+ShowDominance());
    }
    
    for (int j = 1; j <= m_support.NumStrats(colPlayer); j++) {
      m_grid->SetCellValue((char *) ToText(m_solution->Payoff(nfg.Players()[colPlayer],
						       m_support.Strategies(colPlayer)[j]),
					   GetDecimals()),
			   m_support.NumStrats(rowPlayer)+ShowProbs()+ShowDominance(), j-1);

    }
    
    for (int i = 0; i < m_grid->GetRows(); i++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY, i,
				      m_support.NumStrats(colPlayer)+ShowProbs()+ShowDominance());
    }
    for (int i = 0; i < m_grid->GetCols(); i++) {
      m_grid->SetCellBackgroundColour(*wxLIGHT_GREY,
				      m_support.NumStrats(rowPlayer)+ShowProbs()+ShowDominance(),
				      i);
    }
  }
}

void NfgTable::ToggleProbs(void)
{
  if (m_showProb) {
    m_grid->DeleteRows(m_grid->GetRows() - 1);
    m_grid->DeleteCols(m_grid->GetCols() - 1);
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
    m_grid->DeleteRows(m_grid->GetRows() - 1);
    m_grid->DeleteCols(m_grid->GetCols() - 1);
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
    m_grid->DeleteRows(m_grid->GetRows() - 1);
    m_grid->DeleteCols(m_grid->GetCols() - 1);
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
  if (p_event.GetRow() >= m_support.NumStrats(GetRowPlayer()) ||
      p_event.GetCol() >= m_support.NumStrats(GetColPlayer())) {
    p_event.Veto();
  }
  else {
    SetStrategy(GetRowPlayer(), p_event.GetRow() + 1);
    SetStrategy(GetColPlayer(), p_event.GetCol() + 1);
  }
}

void NfgTable::OnLeftDoubleClick(wxGridEvent &p_event)
{
  if (m_editable &&
      p_event.GetRow() < m_support.NumStrats(GetRowPlayer()) &&
      p_event.GetCol() < m_support.NumStrats(GetColPlayer())) {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,
			 NFG_EDIT_OUTCOMES_PAYOFFS);
    m_parent->AddPendingEvent(event);
  }
}

void NfgTable::OnLabelLeftClick(wxGridEvent &p_event)
{
  // for the moment, just veto it
  p_event.Veto();
}

void NfgTable::OnStrategyChange(wxCommandEvent &)
{
  SetProfile(GetProfile());
}

void NfgTable::OnRowPlayerChange(wxCommandEvent &)
{
  int oldRowPlayer = GetRowPlayer();
  int newRowPlayer = m_rowChoice->GetSelection() + 1;

  if (newRowPlayer == oldRowPlayer) {
    return;
  }

  if (newRowPlayer == m_colChoice->GetSelection() + 1) {
    m_colChoice->SetSelection(oldRowPlayer - 1);
    SetPlayers(newRowPlayer, oldRowPlayer);
  }
  else {
    SetPlayers(newRowPlayer, m_colChoice->GetSelection() + 1);
  }
}

void NfgTable::OnColPlayerChange(wxCommandEvent &)
{
  int oldColPlayer = GetColPlayer();
  int newColPlayer = m_colChoice->GetSelection() + 1;

  if (newColPlayer == oldColPlayer) {
    return;
  }

  if (newColPlayer == m_rowChoice->GetSelection() + 1) {
    m_rowChoice->SetSelection(oldColPlayer - 1);
    SetPlayers(oldColPlayer, newColPlayer);
  }
  else {
    SetPlayers(m_rowChoice->GetSelection() + 1, newColPlayer);
  }
}

void NfgTable::SetSupport(const NFSupport &p_support)
{
  m_support = p_support;

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_stratProfile[pl-1]->Clear();
    NFPlayer *player = m_nfg.Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      if (m_support.Find(player->Strategies()[st])) {
	m_stratProfile[pl-1]->Append((char *) (ToText(st) + ": " +
					       player->Strategies()[st]->Name()));
      }
    }
    m_stratProfile[pl-1]->SetSelection(0);
  }
  SetPlayers(m_rowPlayer, m_colPlayer);
}

void NfgTable::SetSolution(const MixedSolution &p_solution)
{
  if (m_solution) {
    delete m_solution;
  }
  m_solution = new MixedSolution(p_solution);
  OnChangeValues();
}

void NfgTable::ClearSolution(void)
{
  if (m_solution) {
    delete m_solution;
    m_solution = 0;
  }
}

//-----------------------------------------------------------------------
//               class NfgTable::Settings: Member functions
//-----------------------------------------------------------------------

NfgTable::Settings::Settings(void)
  : m_decimals(2)
{
  LoadSettings();
}

NfgTable::Settings::~Settings()
{ }

void NfgTable::Settings::LoadSettings(void)
{
  wxConfig config("Gambit");
  config.Read("NfgDisplay/Display-Precision", &m_decimals, 2);
  config.Read("NfgDisplay/Outcome-Values", &m_outcomeValues, 1);
}

void NfgTable::Settings::SaveSettings(void) const
{
  wxConfig config("Gambit");
  config.Write("NfgDisplay/Display-Precision", (long) m_decimals);
  config.Write("NfgDisplay/Outcome-Values", (long) m_outcomeValues);
}

