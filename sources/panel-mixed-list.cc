//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of list of computed mixed strategy profiles
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
#include "sheet.h"    // for wxSheet
#include "panel-mixed-list.h"

DEFINE_EVENT_TYPE(GBT_MIXED_PROFILE_SELECTED)

const int GBT_ID_MIXED_PROFILE_CTRL = 2000;

BEGIN_EVENT_TABLE(gbtMixedProfileCtrl, wxSheet)
  EVT_SHEET_RANGE_SELECTED(GBT_ID_MIXED_PROFILE_CTRL,
			   gbtMixedProfileCtrl::OnRowSelected)
END_EVENT_TABLE()

gbtMixedProfileCtrl::gbtMixedProfileCtrl(wxWindow *p_parent,
					 gbtGameDocument *p_doc,
					 gbtList<gbtMixedProfile<double> > &p_eqa)
  : wxSheet(p_parent, GBT_ID_MIXED_PROFILE_CTRL), gbtGameView(p_doc),
    m_eqa(p_eqa)
{
  CreateGrid(m_eqa.Length(), p_doc->GetGame()->NumPlayers());

  EnableEditing(false);
  DisableDragRowSize();
  DisableDragColSize();
  SetCursorCellHighlightColour(*wxWHITE);
  SetSelectionMode(wxSHEET_SelectRows);

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  AdjustScrollbars();
}

wxString gbtMixedProfileCtrl::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    return wxString::Format(wxT("%d"), p_coords.GetRow() + 1);
  }
  else if (IsColLabelCell(p_coords)) {
    return wxString::Format(wxT("%s"),
			    m_doc->GetGame()->GetPlayer(p_coords.GetCol() + 1)->GetLabel().c_str());
  }
  else if (IsCornerLabelCell(p_coords)) {
    return wxT("#");
  }

  return wxString::Format(wxT("%s"),
			  m_eqa[p_coords.GetRow() + 1]->ToMyerson(m_doc->GetGame()->GetPlayer(p_coords.GetCol() + 1)).c_str());
}

wxSheetCellAttr gbtMixedProfileCtrl::GetAttr(const wxSheetCoords &p_coords,
					     wxSheetAttr_Type) const
{
  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(*wxBLACK);
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(m_doc->GetPlayerColor(p_coords.GetCol() + 1));
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(*wxBLACK);
    return attr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetReadOnly(true);
  attr.SetForegroundColour(m_doc->GetPlayerColor(p_coords.GetCol() + 1));
  return attr;
}


void gbtMixedProfileCtrl::OnUpdate(void)
{
  if (m_eqa.Length() > GetNumberRows()) {
    AppendRows(m_eqa.Length() - GetNumberRows());
  }
  else if (m_eqa.Length() < GetNumberRows()) {
    DeleteRows(GetNumberRows() - m_eqa.Length());
  }

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  AdjustScrollbars();
}

void gbtMixedProfileCtrl::OnRowSelected(wxSheetRangeSelectEvent &p_event)
{
  if (p_event.Selecting()) {
    wxCommandEvent event(GBT_MIXED_PROFILE_SELECTED);
    event.SetInt(p_event.GetTopRow() + 1);
    wxPostEvent(this, event);
  }
}


