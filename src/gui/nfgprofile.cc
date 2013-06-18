//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/nfgprofile.cc
// Normal form mixed profile window
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

#include "nfgprofile.h"
#include "menuconst.h"
#include "renratio.h"        // for rational number rendering

//-------------------------------------------------------------------------
//              class gbtMixedProfileList: Member functions
//-------------------------------------------------------------------------

gbtMixedProfileList::gbtMixedProfileList(wxWindow *p_parent, 
					 gbtGameDocument *p_doc)
  : wxSheet(p_parent, -1), gbtGameView(p_doc),
    m_showProbs(1), m_showPayoff(0)
{
  CreateGrid(0, 0);
  SetRowLabelWidth(40);
  SetColLabelHeight(25);

  Connect(GetId(), wxEVT_SHEET_LABEL_LEFT_DOWN,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&gbtMixedProfileList::OnLabelClick)));

  Connect(GetId(), wxEVT_SHEET_CELL_LEFT_DOWN,
	  (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetEventFunction, wxSheetEventFunction(&gbtMixedProfileList::OnCellClick)));
}

gbtMixedProfileList::~gbtMixedProfileList()
{ }

void gbtMixedProfileList::OnLabelClick(wxSheetEvent &p_event)
{
  if (p_event.GetCol() == -1) {
    m_doc->SetCurrentProfile(RowToProfile(p_event.GetRow()));
  }
}

void gbtMixedProfileList::OnCellClick(wxSheetEvent &p_event)
{
  m_doc->SetCurrentProfile(RowToProfile(p_event.GetRow()));
}

#ifdef UNUSED
static Gambit::GameStrategy GetStrategy(gbtGameDocument *p_doc, int p_index)
{
  int index = 0;
  for (int pl = 1; pl <= p_doc->GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer player = p_doc->GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (index++ == p_index) {
	return player->GetStrategy(st);
      }
    }
  }
  return 0;
}
#endif

wxString gbtMixedProfileList::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    return wxString::Format(wxT("%d"), RowToProfile(p_coords.GetRow()));
  }
  else if (IsColLabelCell(p_coords)) {
    int index = 0;
    for (int pl = 1; pl <= m_doc->GetGame()->NumPlayers(); pl++) {
      Gambit::GamePlayer player = m_doc->GetGame()->GetPlayer(pl);
      for (int st = 1; st <= player->NumStrategies(); st++) {
	if (index++ == p_coords.GetCol()) {
	  return (wxString::Format(wxT("%d: "), pl) +
		  wxString(player->GetStrategy(st)->GetLabel().c_str(),
			   *wxConvCurrent));
	}
      }
    }
    return wxT("");
  }
  else if (IsCornerLabelCell(p_coords)) {
    return wxT("#");
  }

  int profile = RowToProfile(p_coords.GetRow());

  if (IsProbabilityRow(p_coords.GetRow())) {
    return wxString(m_doc->GetProfiles().GetStrategyProb(p_coords.GetCol()+1,
							 profile).c_str(),
		    *wxConvCurrent);
  }
  else {
    return wxString(m_doc->GetProfiles().GetStrategyValue(p_coords.GetCol()+1,
							  profile).c_str(),
		    *wxConvCurrent);
  }
}


static wxColour GetPlayerColor(gbtGameDocument *p_doc, int p_index)
{
  int index = 0;
  for (int pl = 1; pl <= p_doc->GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer player = p_doc->GetGame()->GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      if (index++ == p_index) {
	return p_doc->GetStyle().GetPlayerColor(pl);
      }
    }
  }
  return *wxBLACK;
}

wxSheetCellAttr gbtMixedProfileList::GetAttr(const wxSheetCoords &p_coords, 
					     wxSheetAttr_Type) const
{
  int currentProfile = m_doc->GetCurrentProfile();

  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultRowLabelAttr);
    if (RowToProfile(p_coords.GetRow()) == currentProfile) {
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
    attr.SetForegroundColour(GetPlayerColor(m_doc, p_coords.GetCol()));
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    return GetSheetRefData()->m_defaultCornerLabelAttr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  if (RowToProfile(p_coords.GetRow()) == currentProfile) {
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  }
  else {
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  }
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetRenderer(wxSheetCellRenderer(new gbtRationalRendererRefData()));
  attr.SetForegroundColour(GetPlayerColor(m_doc, p_coords.GetCol()));
			   
  attr.SetReadOnly(true);
  return attr;
}


void gbtMixedProfileList::OnUpdate(void)
{
  if (m_doc->NumProfileLists() == 0) {
    DeleteRows(0, GetNumberRows());
    return;
  }

  const gbtAnalysisOutput &profiles = m_doc->GetProfiles();

  BeginBatch();

  int newRows = profiles.NumProfiles() * (m_showProbs + m_showPayoff);
  DeleteRows(0, GetNumberRows());
  InsertRows(0, newRows);
	     
  int profileLength = m_doc->GetGame()->MixedProfileLength();
  int newCols = profileLength;
  DeleteCols(0, GetNumberCols());
  InsertCols(0, newCols);

  for (int row = 0; row < GetNumberRows(); row += m_showProbs + m_showPayoff) {
    SetCellSpan(wxSheetCoords(row, -1), 
		wxSheetCoords(m_showProbs + m_showPayoff, 1));
  }

  AutoSizeRows();
  AutoSizeCols();

  int colSize = GetColWidth(0);
  for (int col = 1; col < GetNumberCols(); col++) {
    if (GetColWidth(col) > colSize) {
      colSize = GetColWidth(col);
    }
  }

  for (int col = 0; col < GetNumberCols(); SetColWidth(col++, colSize));

  EndBatch();
}

