//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/editlabel.cc
// Text control for editing valid labels
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

#include <algorithm>

#include "editlabel.h"

namespace Gambit::GUI {

bool LabelTextCtrl::IsAsciiPrintable(wxUniChar p_char)
{
  const auto value = static_cast<unsigned long>(p_char);
  return value >= 0x20 && value <= 0x7e;
}

bool LabelTextCtrl::IsLabelWhitespace(wxUniChar p_char)
{
  return p_char == ' ' || p_char == '\t' || p_char == '\r' || p_char == '\n' || p_char == '\v' ||
         p_char == '\f';
}

bool LabelTextCtrl::IsAllowedNonWhitespace(wxUniChar p_char) const
{
  switch (m_policy) {
  case LabelCharacterPolicy::AsciiOnly:
    return IsAsciiPrintable(p_char) && !IsLabelWhitespace(p_char);

  case LabelCharacterPolicy::Unicode:
    return !IsLabelWhitespace(p_char);

  default:
    return false;
  }
}

wxString LabelTextCtrl::Normalize(const wxString &p_value, bool p_stripTrailing) const
{
  wxString normalized;
  bool sawNonWhitespace = false;
  bool previousWasSpace = false;

  for (wxString::const_iterator iter = p_value.begin(); iter != p_value.end(); ++iter) {
    const wxUniChar ch = *iter;

    if (IsLabelWhitespace(ch)) {
      if (!sawNonWhitespace) {
        continue;
      }
      if (!previousWasSpace) {
        normalized << ' ';
        previousWasSpace = true;
      }
      continue;
    }

    if (!IsAllowedNonWhitespace(ch)) {
      continue;
    }

    normalized << ch;
    sawNonWhitespace = true;
    previousWasSpace = false;
  }

  if (p_stripTrailing && normalized.EndsWith(" ")) {
    normalized.RemoveLast();
  }

  return normalized;
}

void LabelTextCtrl::NormalizeInPlace(bool p_stripTrailing)
{
  if (m_normalizing) {
    return;
  }

  const wxString oldValue = GetValue();
  const wxString newValue = Normalize(oldValue, p_stripTrailing);
  if (oldValue == newValue) {
    return;
  }

  const long insertionPoint = GetInsertionPoint();

  m_normalizing = true;
  ChangeValue(newValue);
  SetInsertionPoint(std::min<long>(insertionPoint, newValue.length()));
  m_normalizing = false;
}

void LabelTextCtrl::OnText(wxCommandEvent &p_event)
{
  NormalizeInPlace(false);
  p_event.Skip();
}

void LabelTextCtrl::OnKillFocus(wxFocusEvent &p_event)
{
  NormalizeInPlace(true);
  p_event.Skip();
}

LabelTextCtrl::LabelTextCtrl(wxWindow *p_parent, wxWindowID p_id, const wxString &p_value,
                             LabelCharacterPolicy p_policy, const wxPoint &p_pos,
                             const wxSize &p_size, long p_style)
  : wxTextCtrl(p_parent, p_id, wxEmptyString, p_pos, p_size, p_style), m_policy(p_policy)
{
  ChangeValue(Normalize(p_value, true));

  Bind(wxEVT_TEXT, &LabelTextCtrl::OnText, this);
  Bind(wxEVT_KILL_FOCUS, &LabelTextCtrl::OnKillFocus, this);
}

wxString LabelTextCtrl::GetNormalizedValue()
{
  NormalizeInPlace(true);
  return GetValue();
}

} // namespace Gambit::GUI
