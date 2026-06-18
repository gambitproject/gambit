//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#endif // WX_PRECOMP

#include "efgprofile.h"

namespace Gambit::GUI {
//-------------------------------------------------------------------------
//              class BehaviorProfileList: Member functions
//-------------------------------------------------------------------------

MixedBehaviorProfileList::MixedBehaviorProfileList(wxWindow *p_parent, GameDocument *p_doc)
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

  Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &MixedBehaviorProfileList::OnLabelClick, this);
  Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MixedBehaviorProfileList::OnCellClick, this);
  Bind(wxEVT_GRID_SELECT_CELL, &MixedBehaviorProfileList::OnSelectCell, this);
}

MixedBehaviorProfileList::~MixedBehaviorProfileList() = default;

void MixedBehaviorProfileList::OnLabelClick(wxGridEvent &p_event)
{
  if (p_event.GetCol() == -1) {
    m_doc->SetCurrentProfile(p_event.GetRow() + 1);
  }
  else {
    // Clicking on an action column sets the selected node to the first
    // member of that information set.
    const GameAction action = m_doc->GetAction(p_event.GetCol() + 1);
    m_doc->SetSelectNode(action->GetInfoset()->GetMember(1));
  }

  ClearSelection();
}

void MixedBehaviorProfileList::OnCellClick(wxGridEvent &p_event)
{
  m_doc->SetCurrentProfile(p_event.GetRow() + 1);
  ClearSelection();
}

void MixedBehaviorProfileList::OnSelectCell(wxGridEvent &p_event)
{
  p_event.Veto();
  ClearSelection();
}

static wxColour GetPlayerColor(const GameDocument *p_doc, int p_index)
{
  const GameAction action = p_doc->GetAction(p_index + 1);
  return p_doc->GetStyle().GetPlayerColor(action->GetInfoset()->GetPlayer());
}

void MixedBehaviorProfileList::ResizeGrid(int p_rows, int p_cols)
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

void MixedBehaviorProfileList::UpdateLabels()
{
  for (int row = 0; row < GetNumberRows(); ++row) {
    wxString label;
    label << (row + 1);
    SetRowLabelValue(row, label);
  }

  for (int col = 0; col < GetNumberCols(); ++col) {
    const GameAction action = m_doc->GetAction(col + 1);

    wxString label;
    label << action->GetInfoset()->GetNumber() << ": " << action->GetLabel();
    SetColLabelValue(col, label);
  }
}

void MixedBehaviorProfileList::UpdateCells()
{
  const int currentProfile = m_doc->GetCurrentProfile();

  const wxFont normalFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  const wxFont boldFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

  for (int row = 0; row < GetNumberRows(); ++row) {
    for (int col = 0; col < GetNumberCols(); ++col) {
      SetCellValue(
          row, col,
          wxString(m_doc->GetProfiles().GetActionProb(col + 1, row + 1).c_str(), *wxConvCurrent));

      wxGridCellAttr *attr = new wxGridCellAttr;
      attr->SetFont(row + 1 == currentProfile ? boldFont : normalFont);
      attr->SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
      attr->SetReadOnly(true);

      try {
        const GameAction action = m_doc->GetAction(col + 1);
        attr->SetTextColour(m_doc->GetStyle().GetPlayerColor(action->GetInfoset()->GetPlayer()));

        if (action->GetInfoset()->GetNumber() % 2 == 0) {
          attr->SetBackgroundColour(wxColour(250, 250, 250));
        }
        else {
          attr->SetBackgroundColour(wxColour(225, 225, 225));
        }
      }
      catch (std::out_of_range &) {
        // If GetAction() throws this, just handle it silently; can occur
        // when solving a trivial game via the strategic form
      }

      SetAttr(row, col, attr);
    }
  }
}

void MixedBehaviorProfileList::OnUpdate()
{
  if (!m_doc->GetGame() || m_doc->NumProfileLists() == 0) {
    if (GetNumberRows() > 0) {
      DeleteRows(0, GetNumberRows());
    }
    if (GetNumberCols() > 0) {
      DeleteCols(0, GetNumberCols());
    }
    return;
  }

  const AnalysisOutput &profiles = m_doc->GetProfiles();
  const int profileLength = m_doc->GetGame()->BehavProfileLength();

  BeginBatch();

  ResizeGrid(profiles.NumProfiles(), profileLength);
  UpdateLabels();
  UpdateCells();

  AutoSizeRows();
  AutoSizeColumns();

  ClearSelection();

  EndBatch();
}
} // namespace Gambit::GUI
