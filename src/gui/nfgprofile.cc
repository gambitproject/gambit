//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#endif // WX_PRECOMP

#include "nfgprofile.h"
#include "renratio.h" // for rational number rendering

namespace Gambit::GUI {
//-------------------------------------------------------------------------
//              class MixedProfileList: Member functions
//-------------------------------------------------------------------------

MixedProfileList::MixedProfileList(wxWindow *p_parent, GameDocument *p_doc)
  : wxSheet(p_parent, wxID_ANY), GameView(p_doc), m_showProbs(1), m_showPayoff(0)
{
  CreateGrid(0, 0);
  SetRowLabelWidth(40);
  SetColLabelHeight(25);

  Connect(GetId(), wxEVT_SHEET_LABEL_LEFT_DOWN,
          (wxObjectEventFunction) reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction,
              static_cast<wxSheetEventFunction>(&MixedProfileList::OnLabelClick))));

  Connect(GetId(), wxEVT_SHEET_CELL_LEFT_DOWN,
          (wxObjectEventFunction) reinterpret_cast<wxEventFunction>(wxStaticCastEvent(
              wxSheetEventFunction,
              static_cast<wxSheetEventFunction>(&MixedProfileList::OnCellClick))));
}

MixedProfileList::~MixedProfileList() = default;

void MixedProfileList::OnLabelClick(wxSheetEvent &p_event)
{
  if (p_event.GetCol() == -1) {
    m_doc->SetCurrentProfile(RowToProfile(p_event.GetRow()));
  }
}

void MixedProfileList::OnCellClick(wxSheetEvent &p_event)
{
  m_doc->SetCurrentProfile(RowToProfile(p_event.GetRow()));
}

#ifdef UNUSED
static Gambit::GameStrategy GetStrategy(GameDocument *p_doc, int p_index)
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

wxString MixedProfileList::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    return wxString::Format(wxT("%d"), RowToProfile(p_coords.GetRow()));
  }
  else if (IsColLabelCell(p_coords)) {
    int index = 0;
    for (const auto &player : m_doc->GetGame()->GetPlayers()) {
      for (const auto &strategy : player->GetStrategies()) {
        if (index++ == p_coords.GetCol()) {
          return (wxString::Format(wxT("%d: "), player->GetNumber()) +
                  wxString(strategy->GetLabel().c_str(), *wxConvCurrent));
        }
      }
    }
    return wxT("");
  }
  else if (IsCornerLabelCell(p_coords)) {
    return wxT("#");
  }

  const int profile = RowToProfile(p_coords.GetRow());

  if (IsProbabilityRow(p_coords.GetRow())) {
    return {m_doc->GetProfiles().GetStrategyProb(p_coords.GetCol() + 1, profile).c_str(),
            *wxConvCurrent};
  }
  else {
    return {m_doc->GetProfiles().GetStrategyValue(p_coords.GetCol() + 1, profile).c_str(),
            *wxConvCurrent};
  }
}

static wxColour GetPlayerColor(const GameDocument *p_doc, int p_index)
{
  int index = 0;
  for (const auto &player : p_doc->GetGame()->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      if (index++ == p_index) {
        return p_doc->GetStyle().GetPlayerColor(player);
      }
    }
  }
  return *wxBLACK;
}

wxSheetCellAttr MixedProfileList::GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const
{
  const int currentProfile = m_doc->GetCurrentProfile();

  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultRowLabelAttr);
    if (RowToProfile(p_coords.GetRow()) == currentProfile) {
      attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    }
    else {
      attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    }
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultColLabelAttr);
    attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
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
    attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  }
  else {
    attr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  }
  attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  attr.SetRenderer(wxSheetCellRenderer(new RationalRendererRefData()));
  attr.SetForegroundColour(GetPlayerColor(m_doc, p_coords.GetCol()));

  attr.SetReadOnly(true);
  return attr;
}

void MixedProfileList::OnUpdate()
{
  if (m_doc->NumProfileLists() == 0) {
    DeleteRows(0, GetNumberRows());
    return;
  }

  const AnalysisOutput &profiles = m_doc->GetProfiles();

  BeginBatch();

  const int newRows = profiles.NumProfiles() * (m_showProbs + m_showPayoff);
  DeleteRows(0, GetNumberRows());
  InsertRows(0, newRows);

  const int profileLength = m_doc->GetGame()->MixedProfileLength();
  const int newCols = profileLength;
  DeleteCols(0, GetNumberCols());
  InsertCols(0, newCols);

  for (int row = 0; row < GetNumberRows(); row += m_showProbs + m_showPayoff) {
    SetCellSpan(wxSheetCoords(row, -1), wxSheetCoords(m_showProbs + m_showPayoff, 1));
  }

  AutoSizeRows();
  AutoSizeCols();

  int colSize = GetColWidth(0);
  for (int col = 1; col < GetNumberCols(); col++) {
    if (GetColWidth(col) > colSize) {
      colSize = GetColWidth(col);
    }
  }

  for (int col = 0; col < GetNumberCols(); SetColWidth(col++, colSize))
    ;

  EndBatch();
}
} // namespace Gambit::GUI
