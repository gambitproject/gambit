//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of "about" dialog
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
  int m_rowPlayer, m_colPlayer;

  // Implementation of gbtGameView members
  void OnUpdate(void);

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  void SetCellValue(const wxSheetCoords &, const wxString &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;

  // Overriding wxSheet members for customized behavior
  wxRect CellToRect(const wxSheetCoords &, bool = false) const;
  void DrawCellBorder(wxDC &, const wxSheetCoords &);
  void DrawRowLabels(wxDC &, const wxArrayInt &);
  void DrawColLabels(wxDC &, const wxArrayInt &);

public:
  gbtSchellingMatrix(wxWindow *p_parent, gbtGameDocument *p_doc);
};

gbtSchellingMatrix::gbtSchellingMatrix(wxWindow *p_parent,
				       gbtGameDocument *p_doc)
  : wxSheet(p_parent, -1), gbtGameView(p_doc),
    m_rowPlayer(1), m_colPlayer(2)
{
  CreateGrid(m_doc->GetGame()->GetPlayer(m_rowPlayer)->NumStrategies() * 2,
	     m_doc->GetGame()->GetPlayer(m_colPlayer)->NumStrategies() * 2);

  EnableEditing(true);
  DisableDragRowSize();
  DisableDragColSize();
  SetCursorCellHighlightColour(*wxWHITE);

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  AdjustScrollbars();

}

void gbtSchellingMatrix::OnUpdate(void)
{ 
  BeginBatch();
  int stratRows = GetNumberRows() / 2;
  int stratCols = GetNumberCols() / 2;

  const gbtGame &game = m_doc->GetGame();

  if (game->GetPlayer(m_rowPlayer)->NumStrategies() < stratRows) {
    DeleteRows(0, 2 * (stratRows - game->GetPlayer(m_rowPlayer)->NumStrategies()));
  }
  else if (game->GetPlayer(m_rowPlayer)->NumStrategies() > stratRows) {
    InsertRows(0, 2 * (game->GetPlayer(m_rowPlayer)->NumStrategies() - stratRows)); 
  }

  if (game->GetPlayer(m_colPlayer)->NumStrategies() < stratCols) {
    DeleteCols(0, 2 * (stratCols - game->GetPlayer(m_colPlayer)->NumStrategies()));
  }
  else if (game->GetPlayer(m_colPlayer)->NumStrategies() > stratCols) {
    InsertCols(0, 2 * (game->GetPlayer(m_colPlayer)->NumStrategies() - stratCols));
  }

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  EndBatch();
  AdjustScrollbars();
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
    return m_doc->GetGame()->GetPlayer(m_rowPlayer)->GetStrategy(p_coords.GetRow() / 2 + 1)->GetLabel().c_str();
  }
  else if (IsColLabelCell(p_coords)) {
    return m_doc->GetGame()->GetPlayer(m_colPlayer)->GetStrategy(p_coords.GetCol() / 2 + 1)->GetLabel().c_str();
  }
  else if (IsCornerLabelCell(p_coords)) {
    return "";
  }

  int rowStrat = p_coords.GetRow() / 2;
  int colStrat = p_coords.GetCol() / 2;

  bool rowSubcell = (p_coords.GetRow() % 2 == 1 && p_coords.GetCol() % 2 == 0);
  bool colSubcell = (p_coords.GetRow() % 2 == 0 && p_coords.GetCol() % 2 == 1);

  gbtGameContingency profile = m_doc->GetGame()->NewContingency();

  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_rowPlayer)->GetStrategy(rowStrat + 1));
  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_colPlayer)->GetStrategy(colStrat + 1));

  if (rowSubcell) {
    return ToText(profile->GetPayoff(m_doc->GetGame()->GetPlayer(m_rowPlayer))).c_str();
  }
  else if (colSubcell) {
    return ToText(profile->GetPayoff(m_doc->GetGame()->GetPlayer(m_colPlayer))).c_str();
  }
  else {
    return "";
  }
}

void gbtSchellingMatrix::SetCellValue(const wxSheetCoords &p_coords,
				      const wxString &p_value)
{
  if (IsLabelCell(p_coords))  return;

  int rowStrat = p_coords.GetRow() / 2;
  int colStrat = p_coords.GetCol() / 2;

  bool rowSubcell = (p_coords.GetRow() % 2 == 1 && p_coords.GetCol() % 2 == 0);
  bool colSubcell = (p_coords.GetRow() % 2 == 0 && p_coords.GetCol() % 2 == 1);

  gbtGameContingency profile = m_doc->GetGame()->NewContingency();

  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_rowPlayer)->GetStrategy(rowStrat + 1));
  profile->SetStrategy(m_doc->GetGame()->GetPlayer(m_colPlayer)->GetStrategy(colStrat + 1));

  gbtGameOutcome outcome = profile->GetOutcome();

  if (outcome.IsNull()) {
    outcome = m_doc->NewOutcome();
    profile->SetOutcome(outcome);
  }

  if (rowSubcell) {
    gbtRational r;
    m_doc->SetPayoff(outcome, 
		     m_doc->GetGame()->GetPlayer(m_rowPlayer),
		     FromText(p_value.c_str(), r));
  }
  else if (colSubcell) {
    gbtRational r;
    m_doc->SetPayoff(outcome,
		     m_doc->GetGame()->GetPlayer(m_colPlayer),
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
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_rowPlayer));
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(FALSE);
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_colPlayer));
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
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_rowPlayer));
    attr.SetReadOnly(FALSE);
  }
  else if (colSubcell) {
    attr.SetForegroundColour(m_doc->GetPlayerColor(m_colPlayer));
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

gbtTableSchelling::gbtTableSchelling(wxWindow *p_parent, 
				     gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{ 
  gbtSchellingMatrix *matrix = new gbtSchellingMatrix(this, p_doc);
  
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(matrix, 1, wxEXPAND, 0);
  SetSizer(sizer);
  Layout();
}

void gbtTableSchelling::OnUpdate(void)
{ }
