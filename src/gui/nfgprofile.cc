//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit::GUI {
//-------------------------------------------------------------------------
//              class MixedProfileList: Member functions
//-------------------------------------------------------------------------

MixedStrategyProfileList::MixedStrategyProfileList(wxWindow *p_parent, GameDocument *p_doc)
  : wxGrid(p_parent, wxID_ANY), GameView(p_doc)
{
  CreateGrid(0, 0);

  SetRowLabelSize(40);
  SetColLabelSize(25);
  SetCornerLabelValue(wxT("#"));

  EnableEditing(false);
  EnableDragGridSize(false);
  EnableDragRowSize(false);
  EnableDragColSize(false);

  SetCellHighlightPenWidth(0);
  SetCellHighlightROPenWidth(0);

  Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &MixedStrategyProfileList::OnLabelClick, this);
  Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MixedStrategyProfileList::OnCellClick, this);
  Bind(wxEVT_GRID_SELECT_CELL, &MixedStrategyProfileList::OnSelectCell, this);
}

MixedStrategyProfileList::~MixedStrategyProfileList() = default;

void MixedStrategyProfileList::OnLabelClick(wxGridEvent &p_event)
{
  if (p_event.GetCol() == -1) {
    m_doc->SetCurrentProfile(p_event.GetRow() + 1);
  }

  ClearSelection();
}

void MixedStrategyProfileList::OnCellClick(wxGridEvent &p_event)
{
  m_doc->SetCurrentProfile(p_event.GetRow() + 1);
  ClearSelection();
}

void MixedStrategyProfileList::OnSelectCell(wxGridEvent &p_event)
{
  p_event.Veto();
  ClearSelection();
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

void MixedStrategyProfileList::ResizeGrid(int p_rows, int p_cols)
{
  if (GetNumberRows() > p_rows) {
    DeleteRows(p_rows, GetNumberRows() - p_rows);
  }
  else if (GetNumberRows() < p_rows) {
    AppendRows(p_rows - GetNumberRows());
  }

  if (GetNumberCols() > p_cols) {
    DeleteCols(p_cols, GetNumberCols() - p_cols);
  }
  else if (GetNumberCols() < p_cols) {
    AppendCols(p_cols - GetNumberCols());
  }
}

void MixedStrategyProfileList::UpdateLabels()
{
  for (int row = 0; row < GetNumberRows(); ++row) {
    wxString label;
    label << (row + 1);
    SetRowLabelValue(row, label);
  }

  int index = 0;
  for (const auto &player : m_doc->GetGame()->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      wxString label;
      label << player->GetNumber() << ": " << strategy->GetLabel();
      SetColLabelValue(index++, label);
    }
  }
}

void MixedStrategyProfileList::UpdateCells()
{
  const int currentProfile = m_doc->GetCurrentProfile();

  const wxFont normalFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  const wxFont boldFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

  for (int row = 0; row < GetNumberRows(); ++row) {
    const int profile = row + 1;

    for (int col = 0; col < GetNumberCols(); ++col) {
      SetCellValue(row, col,
                   wxString(m_doc->GetProfiles().GetStrategyProb(col + 1, profile).c_str(),
                            *wxConvCurrent));

      wxGridCellAttr *attr = new wxGridCellAttr;
      attr->SetFont(profile == currentProfile ? boldFont : normalFont);
      attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
      attr->SetTextColour(GetPlayerColor(m_doc, col));
      attr->SetReadOnly(true);

      SetAttr(row, col, attr);
    }
  }
}

void MixedStrategyProfileList::OnUpdate()
{
  if (m_doc->NumProfileLists() == 0) {
    if (GetNumberRows() > 0) {
      DeleteRows(0, GetNumberRows());
    }
    if (GetNumberCols() > 0) {
      DeleteCols(0, GetNumberCols());
    }
    return;
  }

  const AnalysisOutput &profiles = m_doc->GetProfiles();

  BeginBatch();

  const int newRows = profiles.NumProfiles();
  const int newCols = m_doc->GetGame()->GetStrategies().size();

  ResizeGrid(newRows, newCols);
  UpdateLabels();
  UpdateCells();

  AutoSizeRows();
  AutoSizeColumns();

  int colSize = 0;
  for (int col = 0; col < GetNumberCols(); ++col) {
    if (GetColSize(col) > colSize) {
      colSize = GetColSize(col);
    }
  }

  for (int col = 0; col < GetNumberCols(); ++col) {
    SetColSize(col, colSize);
  }

  ClearSelection();

  EndBatch();
}
} // namespace Gambit::GUI
