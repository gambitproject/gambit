//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of panel for showing details of a mixed strategy profile
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
#include "panel-mixed-detail.h"

gbtMixedProfileDetail::gbtMixedProfileDetail(wxWindow *p_parent,
					     gbtGameDocument *p_doc,
					     gbtList<gbtMixedProfile<double> > &p_eqa)
  : wxSheet(p_parent, -1), gbtGameView(p_doc),
    m_eqa(p_eqa), m_index(0)
{
  CreateGrid(1 + p_doc->GetGame()->NumPlayers(), 1);

  EnableEditing(false);
  DisableDragRowSize();
  DisableDragColSize();
  SetCursorCellHighlightColour(*wxWHITE);
  SetColLabelHeight(1);

  AutoSizeRows();
  AutoSizeColumns();
  for (int col = 0; col <= GetNumberCols(); col++) {
    if (GetColWidth(col) < GetRowHeight(col)) {
      SetColWidth(col, GetRowHeight(col));
    }
  }
  AdjustScrollbars();
}

void gbtMixedProfileDetail::OnUpdate(void)
{
  ForceRefresh();
}

wxString gbtMixedProfileDetail::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    if (p_coords.GetRow() == 0) {
      return wxT("Lyapunov");
    }
    else {
      gbtGamePlayer player = m_doc->GetGame()->GetPlayer(p_coords.GetRow());
      if (player->GetLabel() != "") {
	return wxString::Format(wxT("Payoff to %s"),
				player->GetLabel().c_str());
      }
      else {
	return wxString::Format(wxT("Payoff to Player %d"),
				p_coords.GetRow());
      }
    }
  }
  else if (IsColLabelCell(p_coords) || IsCornerLabelCell(p_coords)) {
    return wxT("");
  }

  if (m_index == 0)  return wxT("");

  if (p_coords.GetRow() == 0) {
    return wxString::Format(wxT("%f"), m_eqa[m_index]->GetLiapValue(false));
  }
  else {
    return wxString::Format(wxT("%f"), 
			    m_eqa[m_index]->GetPayoff(m_doc->GetGame()->GetPlayer(p_coords.GetRow())));
  }
}

wxSheetCellAttr gbtMixedProfileDetail::GetAttr(const wxSheetCoords &p_coords,
					       wxSheetAttr_Type) const
{
  if (IsColLabelCell(p_coords) || IsCornerLabelCell(p_coords)) {
    return GetSheetRefData()->m_defaultGridCellAttr;
  }

  if (p_coords.GetRow() == 0) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    if (IsRowLabelCell(p_coords)) {
      attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    }
    else {
      attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    }
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(*wxBLACK);
    return attr;
  }
  else {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
    if (IsRowLabelCell(p_coords)) {
      attr.SetFont(wxFont(12, wxSWISS, wxNORMAL, wxBOLD));
    }
    else {
      attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    }
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(m_doc->GetPlayerColor(p_coords.GetRow()));
    return attr;
  }
}

