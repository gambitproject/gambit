//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/editlabel.h
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

#ifndef EDITLABEL_H
#define EDITLABEL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include <string>
#include <vector>

#include "labellist.h"

namespace Gambit::GUI {

class LabelTextCtrl final : public wxTextCtrl {
  bool m_normalizing{false};

  static bool IsLabelWhitespace(wxUniChar p_char);
  static bool IsControlCharacter(wxUniChar p_char);
  static bool IsAllowedNonWhitespace(wxUniChar p_char);

  wxString NormalizeValue(const wxString &p_value, bool p_stripTrailing) const;
  void NormalizeInPlace(bool p_stripTrailing);

  void OnText(wxCommandEvent &p_event);
  void OnKillFocus(wxFocusEvent &p_event);

public:
  static wxString Normalize(const wxString &p_value, bool p_stripTrailing);

  LabelTextCtrl(wxWindow *p_parent, wxWindowID p_id, const wxString &p_value,
                const wxPoint &p_pos = wxDefaultPosition, const wxSize &p_size = wxDefaultSize,
                long p_style = 0);

  wxString GetNormalizedValue();
};

/// Shows the state of a row of a label list in its label control: new (bold, green), renamed
/// from its stable label (italic, blue, with a tooltip naming the old label), or unchanged
void StyleLabelRow(LabelTextCtrl *p_ctrl, bool p_isNew, const std::string &p_stableLabel,
                   const std::string &p_label);

/// Shades the label controls of the rows that `p_validation` finds invalid (`p_controls` holds
/// one control per row, in order), and returns the message for the first problem, or an empty
/// string if there is none
wxString ValidateLabels(const LabelValidation &p_validation,
                        const std::vector<LabelTextCtrl *> &p_controls,
                        const wxColour &p_defaultBg, const wxString &p_emptyMessage,
                        const wxString &p_duplicateMessage);

} // namespace Gambit::GUI

#endif // EDITLABEL_H
