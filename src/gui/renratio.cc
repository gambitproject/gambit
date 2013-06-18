//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/renratio.cc
// Implementation of wxSheet renderer for rational numbers
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

#include "renratio.h"

#include "wx/sheet/sheet.h"     // the wxSheet widget

//----------------------------------------------------------------------------
//                   class gbtRationalRendererRefData
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(gbtRationalRendererRefData, wxSheetCellRendererRefData)

void 
gbtRationalRendererRefData::SetTextColoursAndFont(wxSheet& grid,
						  const wxSheetCellAttr& attr,
						  wxDC& dc,
						  bool isSelected)
{
  dc.SetBackgroundMode(wxTRANSPARENT);

  if (grid.IsEnabled()) {
    if (isSelected) {
      dc.SetTextBackground(grid.GetSelectionBackground());
      dc.SetTextForeground(grid.GetSelectionForeground());
    }
    else {
      dc.SetTextBackground(attr.GetBackgroundColour());
      dc.SetTextForeground(attr.GetForegroundColour());
    }
  }
  else {
    dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
  }
    
  dc.SetFont(attr.GetFont());
}

static wxSize GetFractionExtent(wxDC &p_dc, const wxString &p_value)
{
  p_dc.SetFont(wxFont(7, wxSWISS, wxNORMAL, wxBOLD));
  
  wxString num;
  int i = 0;
  while (p_value[i] != '/')  num += p_value[i++];
  i++;
  wxString den;
  while (p_value[i] != '\0') den += p_value[i++];
   
  int numWidth, numHeight;
  p_dc.GetTextExtent(num, &numWidth, &numHeight);

  int denWidth, denHeight;
  p_dc.GetTextExtent(den, &denWidth, &denHeight);

  int width = ((numWidth > denWidth) ? numWidth : denWidth);

  return wxSize(width + 4, numHeight + denHeight);
}

wxSize gbtRationalRendererRefData::DoGetBestSize(wxSheet& grid,
						 const wxSheetCellAttr& attr,
						 wxDC& dc,
						 const wxString& text)
{
  if (text.Find('/') != -1) {
    return GetFractionExtent(dc, text);
  }

  wxArrayString lines;
  long w=0, h=0;
  if (grid.StringToLines(text, lines) > 0) {
    dc.SetFont(attr.GetFont());
    grid.GetTextBoxSize(dc, lines, &w, &h);
  }
    
  return wxSize(w, h);
}

wxSize 
gbtRationalRendererRefData::GetBestSize(wxSheet& grid,
					const wxSheetCellAttr& attr,
					wxDC& dc,
					const wxSheetCoords& coords)
{
  return DoGetBestSize(grid, attr, dc, grid.GetCellValue(coords));
}

void gbtRationalRendererRefData::Draw(wxSheet& grid,
				      const wxSheetCellAttr& attr,
				      wxDC& dc,
				      const wxRect& rectCell,
				      const wxSheetCoords& coords,
				      bool isSelected)
{    
  wxRect rect = rectCell;
  rect.Inflate(-1);

  // erase only this cells background, overflow cells should have been erased
  wxSheetCellRendererRefData::Draw(grid, attr, dc, rectCell, coords, isSelected);
  DoDraw(grid, attr, dc, rectCell, coords, isSelected);
}

static void DrawFraction(wxDC &p_dc, wxRect p_rect, const wxString &p_value)
{
  p_dc.SetFont(wxFont(7, wxSWISS, wxNORMAL, wxBOLD));
  
  wxString num;
  int i = 0;
  while (p_value[i] != '/')  num += p_value[i++];
  i++;
  wxString den;
  while (p_value[i] != '\0') den += p_value[i++];
   
  int numWidth, numHeight;
  p_dc.GetTextExtent(num, &numWidth, &numHeight);

  int denWidth, denHeight;
  p_dc.GetTextExtent(den, &denWidth, &denHeight);

  int width = ((numWidth > denWidth) ? numWidth : denWidth);

  wxPoint point(p_rect.x, p_rect.y + p_rect.height/2);

  p_dc.SetPen(wxPen(p_dc.GetTextForeground(), 1, wxSOLID));
  p_dc.DrawText(num,
		point.x + (p_rect.width-numWidth)/2,
		point.y - numHeight);
  p_dc.DrawText(den,
		point.x + (p_rect.width-denWidth)/2,
		point.y);
  p_dc.DrawLine(point.x + (p_rect.width-width)/2 - 2, point.y, 
		point.x + (p_rect.width-width)/2 + width + 2, point.y);
}

void gbtRationalRendererRefData::DoDraw(wxSheet& grid,
					const wxSheetCellAttr& attr,
					wxDC& dc,
					const wxRect& rectCell,
					const wxSheetCoords& coords,
					bool isSelected)
{        
  wxRect rect = rectCell;
  rect.Inflate(-1);
    
  int align = attr.GetAlignment();
  
  wxString value = grid.GetCellValue(coords);
  //int best_width = DoGetBestSize(grid, attr, dc, value).GetWidth();
  wxSheetCoords cellSpan(grid.GetCellSpan(coords)); // shouldn't get here if <=0
  //int cell_rows = cellSpan.m_row;
  //int cell_cols = cellSpan.m_col;
  
  //bool is_grid_cell = wxSheet::IsGridCell(coords);
  // no overflow for row/col/corner labels
  //int num_cols = grid.GetNumberCols();

  // Draw the text 
  if (value.Find('/') != -1) {
    SetTextColoursAndFont(grid, attr, dc, isSelected);
    DrawFraction(dc, rect, value);
  }
  else {
    SetTextColoursAndFont(grid, attr, dc, isSelected);
    grid.DrawTextRectangle(dc, value, rect, align);
  }
}

//----------------------------------------------------------------------------
//                   class gbtRationalEditorRefData
//----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(gbtRationalEditorRefData, wxSheetCellTextEditorRefData)

#include "valnumber.h"

void gbtRationalEditorRefData::CreateEditor(wxWindow* parent,
					    wxWindowID id,
					    wxEvtHandler* evtHandler,
					    wxSheet* sheet)
{
  wxSheetCellTextEditorRefData::CreateEditor(parent, id, evtHandler, sheet);
  GetTextCtrl()->SetValidator(gbtNumberValidator(0));
}

bool gbtRationalEditorRefData::Copy(const gbtRationalEditorRefData &p_other)
{
  return wxSheetCellTextEditorRefData::Copy(p_other);
}

void gbtRationalEditorRefData::StartingKey(wxKeyEvent &event)
{
  int keycode = event.GetKeyCode();
  char tmpbuf[2];
  tmpbuf[0] = (char) keycode;
  tmpbuf[1] = '\0';
  wxString strbuf(tmpbuf, *wxConvCurrent);
#if wxUSE_INTL        
  bool is_decimal_point = 
    (strbuf == wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER));
#else
  bool is_decimal_point = ( strbuf == _T(".") );
#endif
  if (wxIsdigit(keycode) || keycode == '-' || is_decimal_point) {
    wxSheetCellTextEditorRefData::StartingKey(event);
    return;
  }

  event.Skip();
}

bool gbtRationalEditorRefData::IsAcceptedKey(wxKeyEvent &p_event)
{
  if (wxSheetCellEditorRefData::IsAcceptedKey(p_event)) {
    int keycode = p_event.GetKeyCode();
    char tmpbuf[2];
    tmpbuf[0] = (char) keycode;
    tmpbuf[1] = '\0';
    wxString strbuf(tmpbuf, *wxConvCurrent);
#if wxUSE_INTL        
    bool is_decimal_point =
      (strbuf == wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT,
				    wxLOCALE_CAT_NUMBER));
#else
    bool is_decimal_point = ( strbuf == _T(".") );
#endif

    if ((keycode < 128) && 
	(wxIsdigit(keycode) || is_decimal_point || keycode == '-') )
      return true;
  }

  return false;
}
