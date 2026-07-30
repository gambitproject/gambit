//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/labelcell.cc
// Implementation of wxSheet editor for Gambit labels
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

#include "labelcell.h"

#include "wx/sheet/sheet.h"

namespace Gambit::GUI {

IMPLEMENT_DYNAMIC_CLASS(LabelEditorRefData, wxSheetCellTextEditorRefData)

LabelEditorRefData::LabelEditorRefData(LabelCharacterPolicy p_policy) : m_policy(p_policy) {}

void LabelEditorRefData::CreateEditor(wxWindow *parent, wxWindowID id, wxEvtHandler *evtHandler,
                                      wxSheet *sheet)
{
  auto *textCtrl =
      new LabelTextCtrl(parent, id, wxEmptyString, m_policy, wxDefaultPosition, wxDefaultSize,
                        wxTE_PROCESS_TAB | wxTE_CENTER | wxBORDER_NONE);
  SetControl(textCtrl);

  textCtrl->Bind(wxEVT_KILL_FOCUS, [sheet](wxFocusEvent &event) {
    if (!sheet->IsTabTraversing()) {
      sheet->CallAfter([sheet]() {
        if (!sheet->IsTabTraversing() && sheet->IsCellEditControlShown()) {
          sheet->DisableCellEditControl(true);
          sheet->Refresh();
        }
      });
    }
    event.Skip();
  });

  if (m_maxChars != 0) {
    textCtrl->SetMaxLength(m_maxChars);
  }

  wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler, sheet);
}

bool LabelEditorRefData::Copy(const LabelEditorRefData &p_other)
{
  m_policy = p_other.m_policy;
  return wxSheetCellTextEditorRefData::Copy(p_other);
}

bool LabelEditorRefData::IsAcceptedKey(wxKeyEvent &p_event)
{
  if (!wxSheetCellEditorRefData::IsAcceptedKey(p_event)) {
    return false;
  }

  const int keycode = p_event.GetKeyCode();

  // Let the editor start on ordinary printable ASCII characters.  The
  // LabelTextCtrl itself performs full normalization and filtering, so this
  // does not need to duplicate the complete label policy.
  if (m_policy == LabelCharacterPolicy::AsciiOnly) {
    return keycode >= 0x20 && keycode <= 0x7e;
  }

  // For the future Unicode policy, accept the key here and let LabelTextCtrl
  // normalize/filter the resulting text.
  return true;
}

void LabelEditorRefData::StartingKey(wxKeyEvent &p_event)
{
  const int keycode = p_event.GetKeyCode();

  if (m_policy == LabelCharacterPolicy::AsciiOnly && (keycode < 0x20 || keycode > 0x7e)) {
    p_event.Skip();
    return;
  }

  wxSheetCellTextEditorRefData::StartingKey(p_event);
}

bool LabelEditorRefData::EndEdit(const wxSheetCoords &p_coords, wxSheet *p_sheet)
{
  auto *textCtrl = wxStaticCast(GetTextCtrl(), LabelTextCtrl);
  const wxString value = textCtrl->GetNormalizedValue();

  if (value.empty()) {
    wxBell();
    textCtrl->SetFocus();
    return false;
  }

  if (value == p_sheet->GetCellValue(p_coords)) {
    return false;
  }

  p_sheet->SetCellValue(p_coords, value);
  return true;
}

} // namespace Gambit::GUI
