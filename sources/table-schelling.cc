//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of view of normal form with Schelling-style payoffs
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "table-schelling.h"

#include "sheet.h"     // the wxSheet widget

class gbtSchellingMatrix : public wxSheet, public gbtGameView {
private:
  gbtTableSchelling *m_view;

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  void SetCellValue(const wxSheetCoords &, const wxString &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;

  // Overriding wxSheet members for customized behavior
  wxRect CellToRect(const wxSheetCoords &, bool = false) const;
  void DrawCellBorder(wxDC &, const wxSheetCoords &);
  void DrawRowLabels(wxDC &, const wxArrayInt &);
  void DrawColLabels(wxDC &, const wxArrayInt &);

  // Event handlers
  void OnLabelRightDown(wxSheetEvent &);
  void OnMenuAddPlayer(wxCommandEvent &);
  void OnMenuRowStrategyBefore(wxCommandEvent &);
  void OnMenuRowStrategyAfter(wxCommandEvent &);
  void OnMenuColStrategyBefore(wxCommandEvent &);
  void OnMenuColStrategyAfter(wxCommandEvent &);

public:
  gbtSchellingMatrix(gbtTableSchelling *p_view, gbtGameDocument *p_doc);

  // Implementation of gbtGameView members
  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

const int GBT_MENU_ADD_PLAYER = 2999;
const int GBT_SHEET_SCHELLING = 3000;
const int GBT_MENU_ROW_STRATEGY_BEFORE = 3001;
const int GBT_MENU_ROW_STRATEGY_AFTER = 4001;
const int GBT_MENU_COL_STRATEGY_BEFORE = 5001;
const int GBT_MENU_COL_STRATEGY_AFTER = 6001;

BEGIN_EVENT_TABLE(gbtSchellingMatrix, wxSheet)
  EVT_SHEET_LABEL_RIGHT_DOWN(GBT_SHEET_SCHELLING,
			     gbtSchellingMatrix::OnLabelRightDown)
  EVT_MENU(GBT_MENU_ADD_PLAYER, gbtSchellingMatrix::OnMenuAddPlayer)
  EVT_MENU_RANGE(GBT_MENU_ROW_STRATEGY_BEFORE,
		 GBT_MENU_ROW_STRATEGY_BEFORE + 1000,
		 gbtSchellingMatrix::OnMenuRowStrategyBefore)
  EVT_MENU_RANGE(GBT_MENU_ROW_STRATEGY_AFTER,
		 GBT_MENU_ROW_STRATEGY_AFTER + 1000,
		 gbtSchellingMatrix::OnMenuRowStrategyAfter)
  EVT_MENU_RANGE(GBT_MENU_COL_STRATEGY_BEFORE,
		 GBT_MENU_COL_STRATEGY_BEFORE + 1000,
		 gbtSchellingMatrix::OnMenuColStrategyBefore)
  EVT_MENU_RANGE(GBT_MENU_COL_STRATEGY_AFTER,
		 GBT_MENU_COL_STRATEGY_AFTER + 1000,
		 gbtSchellingMatrix::OnMenuColStrategyAfter)
END_EVENT_TABLE()

gbtSchellingMatrix::gbtSchellingMatrix(gbtTableSchelling *p_view,
				       gbtGameDocument *p_doc)
  : wxSheet(p_view, GBT_SHEET_SCHELLING), gbtGameView(p_doc), m_view(p_view)
{
  CreateGrid(m_doc->GetGame()->GetPlayer(p_view->GetRowPlayer())->NumStrategies() * 2,
	     m_doc->GetGame()->GetPlayer(p_view->GetColPlayer())->NumStrategies() * 2);

  EnableEditing(!m_doc->GetGame()->HasTree());
  DisableDragRowSize();
  DisableDragColSize();
  SetCursorCellHighlightColour(*wxWHITE);

  /*
  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  */
  AdjustScrollbars();

}

void gbtSchellingMatrix::OnUpdate(void)
{ 
  BeginBatch();
  int stratRows = GetNumberRows() / 2;
  int stratCols = GetNumberCols() / 2;

  const gbtGame &game = m_doc->GetGame();

  if (game->GetPlayer(m_view->GetRowPlayer())->NumStrategies() < stratRows) {
    DeleteRows(0, 2 * (stratRows - game->GetPlayer(m_view->GetRowPlayer())->NumStrategies()));
  }
  else if (game->GetPlayer(m_view->GetRowPlayer())->NumStrategies() > stratRows) {
    InsertRows(0, 2 * (game->GetPlayer(m_view->GetRowPlayer())->NumStrategies() - stratRows)); 
  }

  if (game->GetPlayer(m_view->GetColPlayer())->NumStrategies() < stratCols) {
    DeleteCols(0, 2 * (stratCols - game->GetPlayer(m_view->GetColPlayer())->NumStrategies()));
  }
  else if (game->GetPlayer(m_view->GetColPlayer())->NumStrategies() > stratCols) {
    InsertCols(0, 2 * (game->GetPlayer(m_view->GetColPlayer())->NumStrategies() - stratCols));
  }

  /*
  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  */
  EndBatch();
  AdjustScrollbars();
}

void gbtSchellingMatrix::OnLabelRightDown(wxSheetEvent &p_event)
{
  wxMenu *menu = new wxMenu;
  if (IsCornerLabelCell(p_event.GetCoords())) {
    menu->Append(GBT_MENU_ADD_PLAYER, "Add a new player");
  }
  else if (IsRowLabelCell(p_event.GetCoords())) {
    menu->Append(GBT_MENU_ROW_STRATEGY_BEFORE + (p_event.GetRow() / 2) + 1, 
		 "Add strategy before");
    menu->Append(GBT_MENU_ROW_STRATEGY_AFTER + (p_event.GetRow() / 2) + 1, 
		 "Add strategy after");
  }
  else {
    menu->Append(GBT_MENU_COL_STRATEGY_BEFORE + (p_event.GetCol() / 2) + 1, 
		 "Add strategy before");
    menu->Append(GBT_MENU_COL_STRATEGY_AFTER + (p_event.GetCol() / 2) + 1, 
		 "Add strategy after");
  }

  PopupMenu(menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void gbtSchellingMatrix::OnMenuAddPlayer(wxCommandEvent &)
{
  m_doc->NewPlayer();
}

void gbtSchellingMatrix::OnMenuRowStrategyBefore(wxCommandEvent &p_event)
{
  m_doc->InsertStrategy(m_view->GetRowPlayer(),
			p_event.GetId() - GBT_MENU_ROW_STRATEGY_BEFORE);
}

void gbtSchellingMatrix::OnMenuRowStrategyAfter(wxCommandEvent &p_event)
{
  m_doc->InsertStrategy(m_view->GetRowPlayer(),
			p_event.GetId() - GBT_MENU_ROW_STRATEGY_AFTER + 1);
}

void gbtSchellingMatrix::OnMenuColStrategyBefore(wxCommandEvent &p_event)
{
  m_doc->InsertStrategy(m_view->GetColPlayer(),
			p_event.GetId() - GBT_MENU_COL_STRATEGY_BEFORE);
}

void gbtSchellingMatrix::OnMenuColStrategyAfter(wxCommandEvent &p_event)
{
  m_doc->InsertStrategy(m_view->GetColPlayer(),
			p_event.GetId() - GBT_MENU_COL_STRATEGY_AFTER + 1);
}

//
// For Schelling-style payoffs, each strategy corresponds to a 2x2 block
// of cells.  The upper-right cell shows the column player payoffs,
// and the lower-left cell shows the row player payoffs.
//
wxString gbtSchellingMatrix::GetCellValue(const wxSheetCoords &p_coords)
{
  // Labels
  if (IsRowLabelCell(p_coords)) {
    return m_doc->GetGame()->GetPlayer(m_view->GetRowPlayer())->GetStrategy(p_coords.GetRow() / 2 + 1)->GetLabel().c_str();
  }
  else if (IsColLabelCell(p_coords)) {
    return m_doc->GetGame()->GetPlayer(m_view->GetColPlayer())->GetStrategy(p_coords.GetCol() / 2 + 1)->GetLabel().c_str();
  }
  else if (IsCornerLabelCell(p_coords)) {
    return "";
  }

  int rowStrat = p_coords.GetRow() / 2;
  int colStrat = p_coords.GetCol() / 2;

  bool rowSubcell = (p_coords.GetRow() % 2 == 1 && p_coords.GetCol() % 2 == 0);
  bool colSubcell = (p_coords.GetRow() % 2 == 0 && p_coords.GetCol() % 2 == 1);

  gbtGameContingency profile = m_doc->GetGame()->NewContingency();

  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_view->GetRowPlayer())->GetStrategy(rowStrat + 1));
  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_view->GetColPlayer())->GetStrategy(colStrat + 1));

  if (rowSubcell) {
    return ToText(profile->GetPayoff(m_doc->GetGame()->GetPlayer(m_view->GetRowPlayer()))).c_str();
  }
  else if (colSubcell) {
    return ToText(profile->GetPayoff(m_doc->GetGame()->GetPlayer(m_view->GetColPlayer()))).c_str();
  }
  else {
    return "";
  }
}

void gbtSchellingMatrix::SetCellValue(const wxSheetCoords &p_coords,
				      const wxString &p_value)
{
  if (IsCornerLabelCell(p_coords))  {
    return;
  }
  else if (IsRowLabelCell(p_coords)) {
    int rowStrat = p_coords.GetRow() / 2;
    gbtGame game = m_doc->GetGame();
    gbtGamePlayer player = game->GetPlayer(m_view->GetRowPlayer());
    m_doc->SetStrategyLabel(player->GetStrategy(rowStrat + 1),
			    p_value.c_str());
    return;
  }
  else if (IsColLabelCell(p_coords)) {
    gbtGame game = m_doc->GetGame();
    int colStrat = p_coords.GetCol() / 2;
    gbtGamePlayer player = game->GetPlayer(m_view->GetColPlayer());
    m_doc->SetStrategyLabel(player->GetStrategy(colStrat + 1),
			    p_value.c_str());
    return;
  }

  int rowStrat = p_coords.GetRow() / 2;
  int colStrat = p_coords.GetCol() / 2;

  bool rowSubcell = (p_coords.GetRow() % 2 == 1 && p_coords.GetCol() % 2 == 0);
  bool colSubcell = (p_coords.GetRow() % 2 == 0 && p_coords.GetCol() % 2 == 1);

  gbtGameContingency profile = m_doc->GetGame()->NewContingency();

  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_view->GetRowPlayer())->GetStrategy(rowStrat + 1));
  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_view->GetColPlayer())->GetStrategy(colStrat + 1));

  gbtGameOutcome outcome = profile->GetOutcome();

  if (outcome.IsNull()) {
    outcome = m_doc->NewOutcome();
    profile->SetOutcome(outcome);
  }

  if (rowSubcell) {
    gbtRational r;
    m_doc->SetPayoff(outcome, 
		     m_doc->GetGame()->GetPlayer(m_view->GetRowPlayer()),
		     FromText(p_value.c_str(), r));
  }
  else if (colSubcell) {
    gbtRational r;
    m_doc->SetPayoff(outcome,
		     m_doc->GetGame()->GetPlayer(m_view->GetColPlayer()),
		     FromText(p_value.c_str(), r));
  }
  ForceRefresh();
}

wxSheetCellAttr gbtSchellingMatrix::GetAttr(const wxSheetCoords &p_coords,
					    wxSheetAttr_Type) const
{
  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(FALSE);
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_view->GetRowPlayer()));
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(FALSE);
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_view->GetColPlayer()));
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    return GetSheetRefData()->m_defaultGridCellAttr;
  }

  int rowStrat = p_coords.GetRow() / 2;
  int colStrat = p_coords.GetCol() / 2;

  bool rowSubcell = (p_coords.GetRow() % 2 == 1 && p_coords.GetCol() % 2 == 0);
  bool colSubcell = (p_coords.GetRow() % 2 == 0 && p_coords.GetCol() % 2 == 1);

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetReadOnly(TRUE);
  if (rowSubcell) {
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_view->GetRowPlayer()));
    attr.SetReadOnly(FALSE);
  }
  else if (colSubcell) {
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_view->GetColPlayer()));
    attr.SetReadOnly(FALSE);
  }
  return attr;
}

//----------------------------------------------------------------------------
//          class gbtSchellingMatrix: Customizing wxSheet behavior
//----------------------------------------------------------------------------

//
// This overrides cell rectangle calculations for row and column labels
//
wxRect gbtSchellingMatrix::CellToRect(const wxSheetCoords &p_coords,
				      bool p_getDeviceRect) const
{
  if (p_coords.GetRow() >= 0 && p_coords.GetCol() == -1) {
    int virtrow = p_coords.GetRow() - (p_coords.GetRow() % 2);
    wxSheetBlock block(virtrow, -1, 2, 1);
    wxRect rect(GetColLeft(block.GetLeft()), GetRowTop(block.GetTop()), 0, 0);
    rect.width  = GetColRight( block.GetRight() ) - rect.x; // + 1;
    rect.height = GetRowBottom(block.GetBottom()) - rect.y; // + 1;
    
    if (GridLinesEnabled()) {
      rect.width--;
      rect.height--;
    }
    
    int dummy;
    CalcScrolledPosition(rect.x, 0, &rect.x, &dummy);
    return rect;
  }
  else if (p_coords.GetRow() == -1 && p_coords.GetCol() >= 0) {
    int virtcol = p_coords.GetCol() - (p_coords.GetCol() % 2);
    wxSheetBlock block(-1, virtcol, 1, 2);

    wxRect rect(GetColLeft(block.GetLeft()), GetRowTop(block.GetTop()), 0, 0);
    rect.width  = GetColRight( block.GetRight() ) - rect.x; // + 1;
    rect.height = GetRowBottom(block.GetBottom()) - rect.y; // + 1;
    
    if (GridLinesEnabled()) {
      rect.width--;
      rect.height--;
    }
    
    int dummy;
    CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
    return rect;

  }
  else {
    return wxSheet::CellToRect(p_coords, p_getDeviceRect);
  }
}

static bool wxRectIsEmpty(const wxRect &rect)
{
  return (rect.width < 1) || (rect.height < 1);
}

void gbtSchellingMatrix::DrawCellBorder(wxDC& dc, const wxSheetCoords &coords)
{
  wxRect rect(CellToRect( coords ));
  if ( wxRectIsEmpty(rect) )  // !IsCellShown
    return;

  dc.SetPen( wxPen(GetGridLineColour(), 1, wxSOLID) );
  // right hand border
  // the right border gets drawn only for odd-numbered columns
  if (coords.GetCol() % 2 == 1) {
    dc.DrawLine( rect.x + rect.width, rect.y,
		 rect.x + rect.width, rect.y + rect.height + 1 );
  }

  // bottom border
  // the bottom border gets drawn only for odd-numbered rows
  if (coords.GetRow() % 2 == 1) {
    dc.DrawLine( rect.x - 1,          rect.y + rect.height,
		 rect.x + rect.width, rect.y + rect.height);
  }
}

// This is identical to the wxSheet implementation, except
// row bottoms are only drawn for the odd-numbered rows
void gbtSchellingMatrix::DrawRowLabels( wxDC& dc, const wxArrayInt& rows )
{
  size_t numLabels = rows.GetCount();
  if (GetNumberRows() == 0 || !numLabels) return;

  for (size_t i = 0; i < numLabels; i++) {
    DrawCell(dc, wxSheetCoords(rows[i], -1));
  }
    
  // Now draw the dividing lines
  dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW),
		  1, wxSOLID));
  int top, dummy;
  CalcUnscrolledPosition(0, 0, &dummy, &top);
  int bottom = GetRowBottom(rows[numLabels-1]);
  // left of row labels
  dc.DrawLine(0, top, 0, bottom);
  // right of row labels
  dc.DrawLine(GetRowLabelWidth()-1, top, GetRowLabelWidth()-1, bottom);
  // draw bottoms
  for (size_t i = 0; i < numLabels; i++) {
    if (rows[i] % 2 == 1) {
      bottom = GetRowBottom(rows[i])-1;
      dc.DrawLine(0, bottom, GetRowLabelWidth()-1, bottom);
    }
  }
}

// This is identical to the wxSheet implementation, except the
// right sides of only odd-numbered columns are drawn
void gbtSchellingMatrix::DrawColLabels(wxDC &dc, const wxArrayInt &cols)
{
  size_t i, numLabels = cols.GetCount();
  if (GetNumberCols() == 0 || !numLabels ) return;

  for ( i = 0; i < numLabels; i++ )
    DrawCell( dc, wxSheetCoords(-1, cols[i]) );

  // Now draw the dividing lines
  dc.SetPen(wxPen(GetLabelOutlineColour(), 1, wxSOLID));
  int left, dummy;
  CalcUnscrolledPosition(0, 0, &left, &dummy);
  int right = GetColRight(cols[numLabels-1]); 
  // top of col labels
  dc.DrawLine(left, 0, right, 0);
  // bottom of col labels
  dc.DrawLine(left, GetColLabelHeight()-1, right, GetColLabelHeight()-1);
  // draw rights
  for (i = 0; i < numLabels; i++) {
    if (cols[i] % 2 == 1) {
      right = GetColRight(cols[i])-1;
      dc.DrawLine(right, 0, right, GetColLabelHeight()-1);
    }
  }
}

BEGIN_EVENT_TABLE(gbtTableSchelling, wxPanel)
  EVT_SIZE(gbtTableSchelling::OnSize)
  EVT_ROW_PLAYER_CHANGE(gbtTableSchelling::OnSetRowPlayer)
  EVT_COL_PLAYER_CHANGE(gbtTableSchelling::OnSetColPlayer)
END_EVENT_TABLE()

gbtTableSchelling::gbtTableSchelling(wxWindow *p_parent, 
				     gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc),
    m_sheet(0), m_rowPlayer(0), m_colPlayer(0)
{ 
  m_colPlayer = new gbtPlayerLabelCtrl(this, p_doc, false);
  m_colPlayer->SetPlayer(2);
  m_rowPlayer = new gbtPlayerLabelCtrl(this, p_doc, true);
  m_rowPlayer->SetPlayer(1);
  m_sheet = new gbtSchellingMatrix(this, p_doc);

  RefreshLayout();
}

void gbtTableSchelling::OnSetRowPlayer(wxCommandEvent &p_event)
{
  if (p_event.GetInt() == m_colPlayer->GetPlayer()) {
    m_colPlayer->SetPlayer(m_rowPlayer->GetPlayer());
  }
  m_rowPlayer->SetPlayer(p_event.GetInt());
  m_sheet->OnUpdate();
}

void gbtTableSchelling::OnSetColPlayer(wxCommandEvent &p_event)
{
  if (p_event.GetInt() == m_rowPlayer->GetPlayer()) {
    m_rowPlayer->SetPlayer(m_colPlayer->GetPlayer());
  }
  m_colPlayer->SetPlayer(p_event.GetInt());
  m_sheet->OnUpdate();
}

void gbtTableSchelling::OnSize(wxSizeEvent &)
{ RefreshLayout(); }

void gbtTableSchelling::RefreshLayout(void)
{
  if (!m_sheet || !m_rowPlayer || !m_colPlayer) return;

  int width = m_sheet->GetBestSize().GetWidth() + 20;
  if (width > GetClientSize().GetWidth() - 40) {
    width = GetClientSize().GetWidth() - 40;
  }

  int height = m_sheet->GetBestSize().GetHeight() + 20;
  int topMargin = 0;
  if (height > GetClientSize().GetHeight() - topMargin) {
    height = GetClientSize().GetHeight() - topMargin;
  }

  m_rowPlayer->SetBackgroundColour(m_sheet->GetCornerLabelAttr().GetBackgroundColour());
  m_rowPlayer->SetSize(0, 40, 40, height); 

  m_colPlayer->SetBackgroundColour(m_sheet->GetCornerLabelAttr().GetBackgroundColour());
  m_colPlayer->SetSize(40, 0, width, 40);

  SetBackgroundColour(m_sheet->GetCornerLabelAttr().GetBackgroundColour());
  m_sheet->SetSize(40, 40, 
		   GetClientSize().GetWidth() - 40,
		   GetClientSize().GetHeight() - 40);
}

void gbtTableSchelling::OnUpdate(void)
{ }
