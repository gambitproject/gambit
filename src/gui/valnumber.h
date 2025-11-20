//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/valnumber.h
// wxValidator specialization to gbtNumber
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

#ifndef GAMBIT_GUI_VALNUMBER_H
#define GAMBIT_GUI_VALNUMBER_H

#include <wx/validate.h>
#include "gambit.h"

namespace Gambit::GUI {
class NumberValidator final : public wxValidator {
protected:
  wxString *m_stringValue;
  bool m_hasMin, m_hasMax;
  Rational m_minValue, m_maxValue;

  // Event handlers
  void OnChar(wxKeyEvent &);

public:
  explicit NumberValidator(wxString *);
  NumberValidator(wxString *, const Rational &);
  NumberValidator(wxString *, const Rational &, const Rational &);
  NumberValidator(const NumberValidator &);
  ~NumberValidator() override = default;

  wxObject *Clone() const override { return new NumberValidator(*this); }
  bool Copy(const NumberValidator &);

  bool Validate(wxWindow *parent) override;
  bool TransferToWindow() override;
  bool TransferFromWindow() override;

  DECLARE_EVENT_TABLE()
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_VALNUMBER_H
