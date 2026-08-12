//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/renratio.cc
// Implementation of wxGrid renderer/editor for rational numbers
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

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "renratio.h"
#include "valnumber.h"

namespace Gambit::GUI {
//----------------------------------------------------------------------------
//                   class RationalCellRenderer
//----------------------------------------------------------------------------

void RationalCellRenderer::SetTextColoursAndFont(wxGrid &grid, const wxGridCellAttr &attr,
                                                 wxDC &dc, bool isSelected)
{
  dc.SetBackgroundMode(wxTRANSPARENT);

  if (grid.IsEnabled()) {
    if (isSelected) {
      dc.SetTextBackground(grid.GetSelectionBackground());
      dc.SetTextForeground(grid.GetSelectionForeground());
    }
    else {
      dc.SetTextBackground(attr.GetBackgroundColour());
      dc.SetTextForeground(attr.GetTextColour());
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
  p_dc.SetFont(wxFont(7, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

  wxString num;
  int i = 0;
  while (p_value[i] != '/') {
    num += p_value[i++];
  }
  i++;
  wxString den;
  while (p_value[i] != '\0') {
    den += p_value[i++];
  }

  int numWidth, numHeight;
  p_dc.GetTextExtent(num, &numWidth, &numHeight);

  int denWidth, denHeight;
  p_dc.GetTextExtent(den, &denWidth, &denHeight);

  const int width = ((numWidth > denWidth) ? numWidth : denWidth);

  return {width + 4, numHeight + denHeight};
}

wxSize RationalCellRenderer::DoGetBestSize(wxGrid &grid, const wxGridCellAttr &attr, wxDC &dc,
                                           const wxString &text)
{
  if (text.Find('/') != -1) {
    return GetFractionExtent(dc, text);
  }

  wxArrayString lines;
  long w = 0, h = 0;
  grid.StringToLines(text, lines);
  if (!lines.empty()) {
    dc.SetFont(attr.GetFont());
    grid.GetTextBoxSize(dc, lines, &w, &h);
  }

  return {static_cast<int>(w), static_cast<int>(h)};
}

wxSize RationalCellRenderer::GetBestSize(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, int row,
                                         int col)
{
  return DoGetBestSize(grid, attr, dc, grid.GetTable()->GetValue(row, col));
}

static void DrawFraction(wxDC &p_dc, wxRect p_rect, const wxString &p_value)
{
  p_dc.SetFont(wxFont(7, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

  wxString num;
  int i = 0;
  while (p_value[i] != '/') {
    num += p_value[i++];
  }
  i++;
  wxString den;
  while (p_value[i] != '\0') {
    den += p_value[i++];
  }

  int numWidth, numHeight;
  p_dc.GetTextExtent(num, &numWidth, &numHeight);

  int denWidth, denHeight;
  p_dc.GetTextExtent(den, &denWidth, &denHeight);

  const int width = ((numWidth > denWidth) ? numWidth : denWidth);

  const wxPoint point(p_rect.x, p_rect.y + p_rect.height / 2);

  p_dc.SetPen(wxPen(p_dc.GetTextForeground(), 1, wxPENSTYLE_SOLID));
  p_dc.DrawText(num, point.x + (p_rect.width - numWidth) / 2, point.y - numHeight);
  p_dc.DrawText(den, point.x + (p_rect.width - denWidth) / 2, point.y);
  p_dc.DrawLine(point.x + (p_rect.width - width) / 2 - 2, point.y,
                point.x + (p_rect.width - width) / 2 + width + 2, point.y);
}

void RationalCellRenderer::Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc,
                                const wxRect &rectCell, int row, int col, bool isSelected)
{
  // Erase the cell's background before drawing over it.
  dc.SetBackgroundMode(wxSOLID);
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(wxBrush(isSelected ? grid.GetSelectionBackground() : attr.GetBackgroundColour()));
  dc.DrawRectangle(rectCell);

  wxRect rect = rectCell;
  rect.Inflate(-1);

  const wxString value = grid.GetTable()->GetValue(row, col);

  SetTextColoursAndFont(grid, attr, dc, isSelected);

  if (value.Find('/') != -1) {
    DrawFraction(dc, rect, value);
  }
  else {
    int hAlign, vAlign;
    attr.GetAlignment(&hAlign, &vAlign);
    grid.DrawTextRectangle(dc, value, rect, hAlign, vAlign);
  }
}

//----------------------------------------------------------------------------
//                   class RationalCellEditor
//----------------------------------------------------------------------------

void RationalCellEditor::Create(wxWindow *parent, wxWindowID id, wxEvtHandler *evtHandler)
{
  // This implementation parallels the generic text editor, except adds centering of the text.
  DoCreate(parent, id, evtHandler, wxTE_PROCESS_TAB | wxTE_CENTER | wxBORDER_NONE);
  SetValidator(NumberValidator(nullptr));
}

bool RationalCellEditor::IsAcceptedKey(wxKeyEvent &p_event)
{
  if (wxGridCellEditor::IsAcceptedKey(p_event)) {
    const int keycode = p_event.GetKeyCode();
    char tmpbuf[2];
    tmpbuf[0] = static_cast<char>(keycode);
    tmpbuf[1] = '\0';
    const wxString strbuf(tmpbuf, *wxConvCurrent);
#if wxUSE_INTL
    const bool is_decimal_point =
        (strbuf == wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER));
#else
    bool is_decimal_point = (strbuf == _T("."));
#endif

    if ((keycode < 128) && (wxIsdigit(keycode) || is_decimal_point || keycode == '-')) {
      return true;
    }
  }

  return false;
}

void RationalCellEditor::StartingKey(wxKeyEvent &event)
{
  const int keycode = event.GetKeyCode();
  char tmpbuf[2];
  tmpbuf[0] = static_cast<char>(keycode);
  tmpbuf[1] = '\0';
  const wxString strbuf(tmpbuf, *wxConvCurrent);
#if wxUSE_INTL
  const bool is_decimal_point =
      (strbuf == wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER));
#else
  const bool is_decimal_point = (strbuf == _T("."));
#endif
  if (wxIsdigit(keycode) || keycode == '-' || is_decimal_point) {
    wxGridCellTextEditor::StartingKey(event);
    return;
  }

  event.Skip();
}

} // namespace Gambit::GUI
