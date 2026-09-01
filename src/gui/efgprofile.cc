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

MixedBehaviorProfileList::MixedBehaviorProfileList(wxWindow *p_parent,
                                                   const std::shared_ptr<GameDocument> &p_doc)
  : wxGrid(p_parent, wxID_ANY), GameView(p_doc)
{
  CreateGrid(0, 0);

  // The table takes ownership of the provider
  m_labels = new ProfileLabelProvider;
  GetTable()->SetAttrProvider(m_labels);

  SetRowLabelSize(40);
  SetColLabelSize(25);
  SetCornerLabelValue(wxT("#"));

  EnableEditing(false);
  EnableDragGridSize(false);
  EnableDragRowSize(false);
  EnableDragColSize(false);

  SetCellHighlightPenWidth(0);
  SetCellHighlightROPenWidth(0);

#if wxCHECK_VERSION(3, 3, 0)
  // Suppress wxGrid's own label highlighting, which follows the grid cursor.
  // The cursor is meaningless here, and cannot be moved out of the way, as
  // wxGrid puts it back on the first cell whenever the grid is repainted;
  // the labels worth highlighting are chosen in OnUpdate() instead.
  // Versions before 3.3 do not highlight labels, so there is nothing to do.
  DisableOverlaySelection();
#endif

  // The column and corner labels sort the list, so show them as clickable
  GetGridColLabelWindow()->SetCursor(wxCursor(wxCURSOR_HAND));
  GetGridCornerLabelWindow()->SetCursor(wxCursor(wxCURSOR_HAND));

  GetGridColLabelWindow()->SetToolTip(_("Click an action to sort the profiles by its probability; "
                                        "click it again to reverse the order"));
  GetGridCornerLabelWindow()->SetToolTip(
      _("Click to list the profiles in the order in which they were computed"));

  Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &MixedBehaviorProfileList::OnLabelClick, this);
  Bind(wxEVT_GRID_CELL_LEFT_CLICK, &MixedBehaviorProfileList::OnCellClick, this);
  Bind(wxEVT_GRID_SELECT_CELL, &MixedBehaviorProfileList::OnSelectCell, this);
}

MixedBehaviorProfileList::~MixedBehaviorProfileList() = default;

void MixedBehaviorProfileList::OnLabelClick(wxGridEvent &p_event)
{
  if (p_event.GetCol() == -1 && p_event.GetRow() == -1) {
    // The corner label: restore the order in which the profiles were computed
    p_event.Veto();
    m_sortOrder.Reset();
    OnUpdate();
  }
  else if (p_event.GetCol() == -1) {
    m_doc->DoSelectProfile(m_sortOrder.GetProfile(p_event.GetRow()));
  }
  else if (p_event.GetRow() == -1) {
    // An action: sort the profiles on the probability it is played with
    p_event.Veto();
    m_sortOrder.ToggleColumn(p_event.GetCol() + 1);
    OnUpdate();
  }

  ClearSelection();
}

void MixedBehaviorProfileList::OnCellClick(wxGridEvent &p_event)
{
  m_doc->DoSelectProfile(m_sortOrder.GetProfile(p_event.GetRow()));
  ClearSelection();
}

void MixedBehaviorProfileList::OnSelectCell(wxGridEvent &p_event)
{
  p_event.Veto();
  ClearSelection();
}

static wxColour GetPlayerColor(const std::shared_ptr<GameDocument> &p_doc, int p_index)
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
  // Row labels identify the profile itself, so that a profile keeps its
  // number however the list happens to be sorted.
  for (int row = 0; row < GetNumberRows(); ++row) {
    wxString label;
    label << m_sortOrder.GetProfile(row);
    SetRowLabelValue(row, label);
  }

  for (int col = 0; col < GetNumberCols(); ++col) {
    const GameAction action = m_doc->GetAction(col + 1);

    wxString label;
    label << action->GetInfoset()->GetNumber() << ": " << action->GetLabel()
          << wxString::FromUTF8(m_sortOrder.GetColumnMarker(col + 1));
    SetColLabelValue(col, label);
  }
}

void MixedBehaviorProfileList::UpdateCells()
{
  const int currentProfile = m_doc->GetWorkspace().GetCurrentProfile();

  const wxFont normalFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  const wxFont boldFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

  for (int row = 0; row < GetNumberRows(); ++row) {
    const int profile = m_sortOrder.GetProfile(row);

    for (int col = 0; col < GetNumberCols(); ++col) {
      SetCellValue(
          row, col,
          wxString(m_doc->GetWorkspace().GetProfiles().GetActionProb(col + 1, profile).c_str(),
                   *wxConvCurrent));

      wxGridCellAttr *attr = new wxGridCellAttr;
      attr->SetFont(profile == currentProfile ? boldFont : normalFont);
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

void MixedBehaviorProfileList::UpdateHighlights()
{
  // GetColumn() numbers columns from one, and is zero when unsorted
  m_labels->SetSortedColumn(m_sortOrder.GetColumn() - 1);
  m_labels->SetSelectedRow(m_sortOrder.GetRow(m_doc->GetWorkspace().GetCurrentProfile()));
}

void MixedBehaviorProfileList::OnUpdate()
{
  if (!m_doc->GetGame() || m_doc->GetWorkspace().NumProfileLists() == 0) {
    if (GetNumberRows() > 0) {
      DeleteRows(0, GetNumberRows());
    }
    if (GetNumberCols() > 0) {
      DeleteCols(0, GetNumberCols());
    }
    return;
  }

  const AnalysisOutput &profiles = m_doc->GetWorkspace().GetProfiles();
  const int profileLength = m_doc->GetGame()->BehavProfileLength();

  BeginBatch();

  ResizeGrid(profiles.NumProfiles(), profileLength);
  m_sortOrder.Rebuild(profiles.NumProfiles(), profileLength,
                      [&](int p_col, int p_left, int p_right) {
                        return profiles.CompareActionProb(p_col, p_left, p_right);
                      });
  UpdateHighlights();
  UpdateLabels();
  UpdateCells();

  AutoSizeRows();
  AutoSizeColumns();

  ClearSelection();

  EndBatch();
}
} // namespace Gambit::GUI
