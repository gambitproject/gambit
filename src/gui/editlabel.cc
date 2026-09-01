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

bool LabelTextCtrl::IsLabelWhitespace(wxUniChar p_char)
{
  // Tab/CR/LF/VT/FF are ASCII control characters, not Unicode space separators;
  // they are treated as whitespace here (rather than rejected outright) purely as
  // a live-typing convenience, so that e.g. pasting text containing a tab
  // normalizes to a single space instead of being silently dropped.
  if (p_char == '\t' || p_char == '\r' || p_char == '\n' || p_char == '\v' || p_char == '\f') {
    return true;
  }
  // Unicode space separators (category Zs) -- normalized the same way as the
  // literal ASCII space, matching Gambit::IsValidLabel in src/games/game.h.
  const auto value = static_cast<unsigned long>(p_char);
  switch (value) {
  case 0x0020: // SPACE
  case 0x00a0: // NO-BREAK SPACE
  case 0x1680: // OGHAM SPACE MARK
  case 0x2000: // EN QUAD
  case 0x2001: // EM QUAD
  case 0x2002: // EN SPACE
  case 0x2003: // EM SPACE
  case 0x2004: // THREE-PER-EM SPACE
  case 0x2005: // FOUR-PER-EM SPACE
  case 0x2006: // SIX-PER-EM SPACE
  case 0x2007: // FIGURE SPACE
  case 0x2008: // PUNCTUATION SPACE
  case 0x2009: // THIN SPACE
  case 0x200a: // HAIR SPACE
  case 0x202f: // NARROW NO-BREAK SPACE
  case 0x205f: // MEDIUM MATHEMATICAL SPACE
  case 0x3000: // IDEOGRAPHIC SPACE
    return true;
  default:
    return false;
  }
}

bool LabelTextCtrl::IsControlCharacter(wxUniChar p_char)
{
  const auto value = static_cast<unsigned long>(p_char);
  return value <= 0x1f || value == 0x7f || (value >= 0x80 && value <= 0x9f) || value == 0x2028 ||
         value == 0x2029;
}

bool LabelTextCtrl::IsAllowedNonWhitespace(wxUniChar p_char)
{
  return !IsLabelWhitespace(p_char) && !IsControlCharacter(p_char);
}

wxString LabelTextCtrl::Normalize(const wxString &p_value, bool p_stripTrailing)
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
                             const wxPoint &p_pos, const wxSize &p_size, long p_style)
  : wxTextCtrl(p_parent, p_id, wxEmptyString, p_pos, p_size, p_style)
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
