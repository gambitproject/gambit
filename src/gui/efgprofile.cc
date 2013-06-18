//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/efgprofile.cc
// Extensive form behavior profile window
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

#include "efgprofile.h"
#include "menuconst.h"
#include "renratio.h"        // for rational number rendering

//-------------------------------------------------------------------------
//              class gbtBehavProfileList: Member functions
//-------------------------------------------------------------------------

gbtBehavProfileList::gbtBehavProfileList(wxWindow *p_parent, 
					 gbtGameDocument *p_doc)
  : wxSheet(p_parent, -1), gbtGameView(p_doc)
{
  CreateGrid(0, 0);
  SetRowLabelWidth(40);
  SetColLabelHeight(25);

  Connect(GetId(), wxEVT_SHEET_LABEL_LEFT_DOWN,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&gbtBehavProfileList::OnLabelClick)));

  Connect(GetId(), wxEVT_SHEET_CELL_LEFT_DOWN,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&gbtBehavProfileList::OnCellClick)));
}

gbtBehavProfileList::~gbtBehavProfileList()
{ }

void gbtBehavProfileList::OnLabelClick(wxSheetEvent &p_event)
{
  if (p_event.GetCol() == -1) {
    m_doc->SetCurrentProfile(p_event.GetRow() + 1);
  }
  else {
    // Clicking on an action column sets the selected node to the first
    // member of that information set.
    Gambit::GameAction action = m_doc->GetGame()->GetAction(p_event.GetCol()+1);
    m_doc->SetSelectNode(action->GetInfoset()->GetMember(1));
  }
}

void gbtBehavProfileList::OnCellClick(wxSheetEvent &p_event)
{
  m_doc->SetCurrentProfile(p_event.GetRow() + 1);
}

wxString gbtBehavProfileList::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    return wxString::Format(wxT("%d"), p_coords.GetRow() + 1);
  }
  else if (IsColLabelCell(p_coords)) {
    Gambit::GameAction action = m_doc->GetGame()->GetAction(p_coords.GetCol()+1);
    return (wxString::Format(wxT("%d: "), 
			     action->GetInfoset()->GetNumber()) +
	    wxString(action->GetLabel().c_str(), *wxConvCurrent));
  }
  else if (IsCornerLabelCell(p_coords)) {
    return wxT("#");
  }


  return wxString(m_doc->GetProfiles().GetActionProb(p_coords.GetCol()+1,
						     p_coords.GetRow()+1).c_str(),
		  *wxConvCurrent);
}

static wxColour GetPlayerColor(gbtGameDocument *p_doc, int p_index)
{
  Gambit::GameAction action = p_doc->GetGame()->GetAction(p_index+1);
  return p_doc->GetStyle().GetPlayerColor(action->GetInfoset()->GetPlayer()->GetNumber());
}

wxSheetCellAttr gbtBehavProfileList::GetAttr(const wxSheetCoords &p_coords, 
					     wxSheetAttr_Type) const
{
  int currentProfile = m_doc->GetCurrentProfile();

  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultRowLabelAttr);
    if (p_coords.GetRow()+1 == currentProfile) {
      attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    }
    else {
      attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
    }
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultColLabelAttr);
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    if (p_coords.GetRow() > 0) {
      attr.SetForegroundColour(GetPlayerColor(m_doc, p_coords.GetCol()));
    }
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    return GetSheetRefData()->m_defaultCornerLabelAttr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  if (p_coords.GetRow()+1 == currentProfile) {
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  }
  else {
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  }
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetRenderer(wxSheetCellRenderer(new gbtRationalRendererRefData()));

  try {
    Gambit::GameAction action = m_doc->GetGame()->GetAction(p_coords.GetCol()+1);
    attr.SetForegroundColour(m_doc->GetStyle().GetPlayerColor(action->GetInfoset()->GetPlayer()->GetNumber()));
    if (action->GetInfoset()->GetNumber() % 2 == 0) {
      attr.SetBackgroundColour(wxColour(250, 250, 250));
    }
    else {
      attr.SetBackgroundColour(wxColour(225, 225, 225));
    }
  }
  catch (Gambit::IndexException) {
    // If GetAction() throws this, just handle it silently; can occur
    // when solving a trivial game via the strategic form
  }
  attr.SetReadOnly(true);
  return attr;
}

void gbtBehavProfileList::OnUpdate(void)
{
  if (!m_doc->GetGame() || m_doc->NumProfileLists() == 0) {
    DeleteRows(0, GetNumberRows());
    return;
  }

  const gbtAnalysisOutput &profiles = m_doc->GetProfiles();
  int profileLength = m_doc->GetGame()->BehavProfileLength();

  BeginBatch();
  if (GetNumberRows() > profiles.NumProfiles()) {
    DeleteRows(0, GetNumberRows() - profiles.NumProfiles());
  }
  else if (GetNumberRows() < profiles.NumProfiles()) {
    InsertRows(0, profiles.NumProfiles() - GetNumberRows());
  }

  if (GetNumberCols() > profileLength) {
    DeleteCols(0, GetNumberCols() - profileLength);
  }
  else if (GetNumberCols() < profileLength) {
    InsertCols(0, profileLength - GetNumberCols());
  }

  AutoSizeRows();
  AutoSizeCols();

  EndBatch();
}

