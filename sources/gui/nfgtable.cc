//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of normal form table class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/config.h"
#include "nfgshow.h"
#include "nfgtable.h"
#include "nfgconst.h"

//-----------------------------------------------------------------------
//               class NfgTableSettings: Member functions
//-----------------------------------------------------------------------

NfgTableSettings::NfgTableSettings(void)
{
  LoadSettings();
}

void NfgTableSettings::SaveFont(const wxString &p_prefix, 
				wxConfig &p_config, const wxFont &p_font)
{
  p_config.Write(p_prefix + "Size", (long) p_font.GetPointSize());
  p_config.Write(p_prefix + "Family", (long) p_font.GetFamily());
  p_config.Write(p_prefix + "Face", p_font.GetFaceName());
  p_config.Write(p_prefix + "Style", (long) p_font.GetStyle());
  p_config.Write(p_prefix + "Weight", (long) p_font.GetWeight());
}

void NfgTableSettings::LoadFont(const wxString &p_prefix,
				const wxConfig &p_config, wxFont &p_font)
{
  long size, family, style, weight;
  wxString face;
  p_config.Read(p_prefix + "Size", &size, 10);
  p_config.Read(p_prefix + "Family", &family, wxMODERN);
  p_config.Read(p_prefix + "Face", &face, "");
  p_config.Read(p_prefix + "Style", &style, wxNORMAL);
  p_config.Read(p_prefix + "Weight", &weight, wxNORMAL);

  p_font = *wxTheFontList->FindOrCreateFont(size, family, style, weight,
					    false, face);
}

void NfgTableSettings::LoadSettings(void)
{
  wxConfig config("Gambit");
  config.Read("/NfgDisplay/DisplayPrecision", &m_decimals, 2);
  config.Read("/NfgDisplay/OutcomeValues", &m_outcomeValues, true);

  LoadFont("/NfgDisplay/DataFont", config, m_dataFont);
  LoadFont("/NfgDisplay/LabelFont", config, m_labelFont);
}

void NfgTableSettings::SaveSettings(void) const
{
  wxConfig config("Gambit");
  config.Write("/NfgDisplay/DisplayPrecision", (long) m_decimals);
  config.Write("/NfgDisplay/OutcomeValues", (long) m_outcomeValues);

  SaveFont("/NfgDisplay/DataFont", config, m_dataFont);
  SaveFont("/NfgDisplay/LabelFont", config, m_labelFont);
}

//---------------------------------------------------------------------
//                       class NfgGridTable
//---------------------------------------------------------------------

class NfgGridTable : public wxGridTableBase {
private:
  Nfg *m_nfg;
  NfgTable *m_table;

public:
  NfgGridTable(NfgTable *p_table, Nfg *p_nfg);
  virtual ~NfgGridTable() { }

  int GetNumberRows(void);
  int GetNumberCols(void);
  wxString GetValue(int row, int col);
  wxString GetRowLabelValue(int);
  wxString GetColLabelValue(int);
  void SetValue(int, int, const wxString &);
  bool IsEmptyCell(int, int) { return false; }

  bool InsertRows(size_t pos = 0, size_t numRows = 1);
  bool AppendRows(size_t numRows = 1);
  bool DeleteRows(size_t pos = 0, size_t numRows = 1);
  bool InsertCols(size_t pos = 0, size_t numCols = 1);
  bool AppendCols(size_t numCols = 1);
  bool DeleteCols(size_t pos = 0, size_t numCols = 1);

  wxGridCellAttr *GetAttr(int row, int col);
};

NfgGridTable::NfgGridTable(NfgTable *p_table, Nfg *p_nfg)
  : m_nfg(p_nfg), m_table(p_table)
{ }

int NfgGridTable::GetNumberRows(void)
{
  return (m_table->GetSupport().NumStrats(m_table->GetRowPlayer()) +
	  m_table->ShowProbs() + m_table->ShowDominance() +
	  m_table->ShowValues());
}

int NfgGridTable::GetNumberCols(void)
{
  return (m_table->GetSupport().NumStrats(m_table->GetColPlayer()) +
	  m_table->ShowProbs() + m_table->ShowDominance() + 
	  m_table->ShowValues());
}

wxString NfgGridTable::GetRowLabelValue(int p_row)
{
  int numStrats = m_table->GetSupport().NumStrats(m_table->GetRowPlayer());
  if (p_row + 1 <= numStrats) {
    return (char *) m_table->GetSupport().Strategies(m_table->GetRowPlayer())[p_row+1]->Name();
  }
  else if (p_row + 1 == numStrats + m_table->ShowDominance()) {
    return "Dom";
  }
  else if (p_row + 1 == 
	   numStrats + m_table->ShowDominance() + m_table->ShowProbs()) {
    return "Prob";
  }
  else {
    return "Val";
  }
}

wxString NfgGridTable::GetColLabelValue(int p_col)
{
  int numStrats = m_table->GetSupport().NumStrats(m_table->GetColPlayer());
  if (p_col + 1 <= numStrats) {
    return (char *) m_table->GetSupport().Strategies(m_table->GetColPlayer())[p_col+1]->Name();
  }
  else if (p_col + 1 == numStrats + m_table->ShowDominance()) {
    return "Dom";
  }
  else if (p_col + 1 == 
	   numStrats + m_table->ShowDominance() + m_table->ShowProbs()) {
    return "Prob";
  }
  else {
    return "Val";
  }
}

wxString NfgGridTable::GetValue(int row, int col)
{
  int rowPlayer = m_table->GetRowPlayer();
  int colPlayer = m_table->GetColPlayer();
  const gbtNfgSupport &support = m_table->GetSupport();
  int numRowStrats = support.NumStrats(rowPlayer);
  int numColStrats = support.NumStrats(colPlayer);

  if (row < numRowStrats && col < numColStrats) {
    gArray<int> strategy(m_table->GetContingency());
    strategy[m_table->GetRowPlayer()] = row + 1;
    strategy[m_table->GetColPlayer()] = col + 1;
    
    StrategyProfile profile(*m_nfg);
    for (int pl = 1; pl <= strategy.Length(); pl++) {
      profile.Set(pl, support.GetStrategy(pl, strategy[pl]));
    }

    gbtNfgOutcome outcome = m_nfg->GetOutcome(profile);
    if (m_table->GetSettings().OutcomeValues()) {
      wxString ret = "(";
      for (int pl = 1; pl <= strategy.Length(); pl++) {
	ret += wxString::Format("%s",
				(char *) ToText(m_nfg->Payoff(outcome, pl),
						m_table->GetSettings().GetDecimals()));
	if (pl < strategy.Length()) {
	  ret += wxString(",");
	}
      }
      ret += ")";
      return ret;
    }
    else {
      if (!outcome.IsNull()) {
	wxString ret = (char *) outcome.GetLabel();
	if (ret == "") {
	  ret = (char *) (gText("Outcome") + ToText(outcome.GetId()));
	}
	return ret;
      }
      else {
	return "Null";
      }
    }
  }
  else if (row < numRowStrats &&
	   col == numColStrats + m_table->ShowDominance() - 1) {
    Strategy *strategy = support.GetStrategy(rowPlayer, row + 1);
    if (support.IsDominated(strategy, true)) {
      return "S";
    }
    else if (support.IsDominated(strategy, false)) {
      return "W";
    }
    else {
      return "N";
    }
  }
  else if (row == numRowStrats + m_table->ShowDominance() - 1 &&
	   col < numColStrats) {
    Strategy *strategy = support.GetStrategy(colPlayer, col + 1);
    if (support.IsDominated(strategy, true)) {
      return "S";
    }
    else if (support.IsDominated(strategy, false)) {
      return "W";
    }
    else {
      return "N";
    }
  }
  else if (row < numRowStrats && 
	   col == numColStrats + m_table->ShowDominance() + m_table->ShowProbs() - 1) {
    Strategy *strategy = support.GetStrategy(rowPlayer, row + 1);
    return ((char *) ToText(m_table->GetProfile()(strategy)));
  }
  else if (row == numRowStrats + m_table->ShowDominance() + m_table->ShowProbs() - 1 && 
	   col < numColStrats) {
    Strategy *strategy = support.GetStrategy(colPlayer, col + 1);
    return ((char *) ToText(m_table->GetProfile()(strategy)));
  }
  else if (row < numRowStrats && 
	   col == numColStrats + m_table->ShowDominance() + m_table->ShowProbs() + m_table->ShowValues() - 1) {
    Strategy *strategy = support.GetStrategy(rowPlayer, row + 1);
    return ((char *) ToText(m_table->GetProfile().Payoff(strategy->GetPlayer(), strategy)));
  }
  else if (row == numRowStrats + m_table->ShowDominance() + m_table->ShowProbs() + m_table->ShowValues() - 1 && 
	   col < numColStrats) {
    Strategy *strategy = support.GetStrategy(colPlayer, col + 1);
    return ((char *) ToText(m_table->GetProfile().Payoff(strategy->GetPlayer(), strategy)));
  }

  return "";
}

void NfgGridTable::SetValue(int row, int col, const wxString &)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES, row, col);
  GetView()->ProcessTableMessage(msg);
}
	
bool NfgGridTable::InsertRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::AppendRows(size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::DeleteRows(size_t pos, size_t numRows)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED,
			 pos, numRows);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::InsertCols(size_t pos, size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_INSERTED,
			 pos, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::AppendCols(size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

bool NfgGridTable::DeleteCols(size_t pos, size_t numCols)
{
  wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_DELETED,
			 pos, numCols);
  GetView()->ProcessTableMessage(msg);
  return true;
}

wxGridCellAttr *NfgGridTable::GetAttr(int row, int col)
{
  wxGridCellAttr *attr = new wxGridCellAttr;

  if (row >= m_table->GetSupport().NumStrats(m_table->GetRowPlayer()) &&
      col >= m_table->GetSupport().NumStrats(m_table->GetColPlayer())) {
    attr->SetBackgroundColour(*wxBLACK);
  }
  else if (row >= m_table->GetSupport().NumStrats(m_table->GetRowPlayer()) ||
	   col >= m_table->GetSupport().NumStrats(m_table->GetColPlayer())) {
    attr->SetBackgroundColour(*wxLIGHT_GREY);
  }
  else {
    attr->SetBackgroundColour(*wxWHITE);
  }

  attr->SetAlignment(wxCENTER, wxCENTER);

  return attr;
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
  grid.DrawTextRectangle(dc, wxString("("), rect);
  dc.GetTextExtent("(", &x, &y);
  rect.x += x;

  wxString text = grid.GetCellValue(row, col);
  dc.SetTextForeground(*wxRED);
  for (unsigned int i = 0; i < text.Length(); i++) {
    if (text[i] == ',') {
      wxColour color = dc.GetTextForeground();
      dc.SetTextForeground(*wxBLACK);
      grid.DrawTextRectangle(dc, wxString(","), rect);
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
      grid.DrawTextRectangle(dc, wxString(text[i]), rect);
      dc.GetTextExtent(text[i], &x, &y);
      rect.x += x;
    }
  }
  
  dc.SetTextForeground(*wxBLACK);
  grid.DrawTextRectangle(dc, wxString(")"), rect); 
}

//======================================================================
//                   class NfgTable: Member functions
//======================================================================

BEGIN_EVENT_TABLE(NfgTable, wxPanel)
  EVT_GRID_SELECT_CELL(NfgTable::OnCellSelect)
  EVT_GRID_CELL_LEFT_DCLICK(NfgTable::OnLeftDoubleClick)
  EVT_GRID_LABEL_LEFT_CLICK(NfgTable::OnLabelLeftClick)
END_EVENT_TABLE()

NfgTable::NfgTable(Nfg &p_nfg, wxWindow *p_parent)
  : wxPanel(p_parent, -1), m_nfg(p_nfg), m_parent(p_parent), 
    m_editable(true), m_cursorMoving(false), m_rowPlayer(1), m_colPlayer(2),
    m_support(m_nfg), m_profile(0),
    m_showProb(0), m_showDom(0), m_showValue(0)
{
  SetAutoLayout(true);

  m_grid = new wxGrid(this, -1, wxDefaultPosition, wxDefaultSize);
  m_grid->SetTable(new NfgGridTable(this, &m_nfg), true);
  m_grid->SetGridCursor(0, 0);
  m_grid->SetEditable(false);
  m_grid->SetDefaultCellFont(m_settings.GetDataFont());
  m_grid->SetLabelFont(m_settings.GetLabelFont());
  m_grid->SetDefaultCellAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  m_grid->DisableDragRowSize();
  m_grid->DisableDragColSize();
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
  m_grid->AdjustScrollbars();

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(m_grid, 1, wxALL | wxEXPAND | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  Show(true);
}

void NfgTable::SetContingency(const gArray<int> &p_profile)
{
  m_grid->SetGridCursor(p_profile[GetRowPlayer()] - 1,
			p_profile[GetColPlayer()] - 1);
  RefreshTable();
}

gArray<int> NfgTable::GetContingency(void) const
{
  return ((NfgShow *) m_parent)->GetContingency();
}

void NfgTable::SetPlayers(int p_rowPlayer, int p_colPlayer)
{ 
  m_grid->BeginBatch();
  m_rowPlayer = p_rowPlayer;
  m_colPlayer = p_colPlayer;
  int stratRows = m_grid->GetRows() - m_showProb - m_showDom - m_showValue;
  int stratCols = m_grid->GetCols() - m_showProb - m_showDom - m_showValue;

  if (m_support.NumStrats(p_rowPlayer) < stratRows) {
    m_grid->DeleteRows(0, stratRows - m_support.NumStrats(p_rowPlayer));
  }
  else if (m_support.NumStrats(p_rowPlayer) > stratRows) {
    m_grid->InsertRows(0, m_support.NumStrats(p_rowPlayer) - stratRows); 
  }

  if (m_support.NumStrats(p_colPlayer) < stratCols) {
    m_grid->DeleteCols(0, stratCols - m_support.NumStrats(p_colPlayer));
  }
  else if (m_support.NumStrats(p_colPlayer) > stratCols) {
    m_grid->InsertCols(0, m_support.NumStrats(p_colPlayer) - stratCols);
  }

  ((NfgShow *) m_parent)->SetStrategy(m_rowPlayer, 1);
  ((NfgShow *) m_parent)->SetStrategy(m_colPlayer, 1);
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
  m_grid->EndBatch();
  m_grid->AdjustScrollbars();
  RefreshTable();
}

void NfgTable::SetStrategy(int p_player, int p_strategy)
{
  if (!m_cursorMoving) {
    // prevents reentry
    if (p_player == GetRowPlayer()) {
      m_grid->SetGridCursor(p_strategy - 1, m_grid->GetCursorColumn());
    }
    else if (p_player == GetColPlayer()) {
      m_grid->SetGridCursor(m_grid->GetCursorRow(), p_strategy - 1);
    }
  }
}

void NfgTable::ToggleProbs(void)
{
  m_showProb = 1 - m_showProb;
  if (m_showProb) {
    m_grid->AppendCols();
    m_grid->AppendRows();
  }
  else {
    m_grid->DeleteCols();
    m_grid->DeleteRows();
  }
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
  m_grid->AdjustScrollbars();
  m_grid->Refresh();
}

void NfgTable::ToggleDominance(void)
{
  m_showDom = 1 - m_showDom;
  if (m_showDom) {
    m_grid->AppendCols();
    m_grid->AppendRows();
  }
  else {
    m_grid->DeleteCols();
    m_grid->DeleteRows();
  }
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
  m_grid->AdjustScrollbars();
  m_grid->Refresh();
}

void NfgTable::ToggleValues(void)
{
  m_showValue = 1 - m_showValue;
  if (m_showValue) {
    m_grid->AppendCols();
    m_grid->AppendRows();
  }
  else {
    m_grid->DeleteCols();
    m_grid->DeleteRows();
  }
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
  m_grid->AdjustScrollbars();
  m_grid->Refresh();
}

void NfgTable::SetDataFont(const wxFont &p_font)
{ 
  m_settings.SetDataFont(p_font);
  m_settings.SaveSettings();
  m_grid->SetDefaultCellFont(p_font);
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
}

void NfgTable::SetLabelFont(const wxFont &p_font) 
{ 
  m_settings.SetLabelFont(p_font); 
  m_settings.SaveSettings();
  m_grid->SetLabelFont(p_font);
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
}

void NfgTable::SetOutcomeValues(bool p_values)
{
  m_settings.SetOutcomeValues(p_values);
  m_settings.SaveSettings();
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
}

void NfgTable::OnCellSelect(wxGridEvent &p_event)
{
  if (p_event.GetRow() >= m_support.NumStrats(GetRowPlayer()) ||
      p_event.GetCol() >= m_support.NumStrats(GetColPlayer())) {
    p_event.Veto();
  }
  else {
    m_cursorMoving = true;  // this prevents re-entry
    ((NfgShow *) m_parent)->SetStrategy(GetRowPlayer(), p_event.GetRow() + 1);
    ((NfgShow *) m_parent)->SetStrategy(GetColPlayer(), p_event.GetCol() + 1);
    m_cursorMoving = false;
    // now continue with the default behavior (i.e., highlight the new cell)
    p_event.Skip(); 
  }
}

void NfgTable::OnLeftDoubleClick(wxGridEvent &p_event)
{
  if (m_editable &&
      p_event.GetRow() < m_support.NumStrats(GetRowPlayer()) &&
      p_event.GetCol() < m_support.NumStrats(GetColPlayer())) {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, NFG_EDIT_CONTINGENCY);
    m_parent->AddPendingEvent(event);
  }
}

void NfgTable::OnLabelLeftClick(wxGridEvent &p_event)
{
  // for the moment, just veto it
  p_event.Veto();
}

void NfgTable::SetSupport(const gbtNfgSupport &p_support)
{
  m_support = p_support;
  SetPlayers(m_rowPlayer, m_colPlayer);
  RefreshTable();
}

void NfgTable::SetProfile(const MixedSolution &p_solution)
{
  if (m_profile) {
    delete m_profile;
  }
  m_profile = new MixedSolution(p_solution);
  RefreshTable();
}

void NfgTable::ClearProfile(void)
{
  if (m_profile) {
    delete m_profile;
    m_profile = 0;
    RefreshTable();
  }
}

void NfgTable::RefreshTable(void)
{
  m_grid->ForceRefresh();
  m_grid->AutoSizeRows();
  m_grid->AutoSizeColumns();
}


