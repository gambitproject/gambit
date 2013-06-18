//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

#ifndef VALNUMBER_H
#define VALNUMBER_H

#include <wx/validate.h>
#include "libgambit/libgambit.h"

class gbtNumberValidator : public wxValidator {
protected:
  wxString *m_stringValue;
  bool m_hasMin, m_hasMax;
  Gambit::Rational m_minValue, m_maxValue;
  
  // Event handlers
  void OnChar(wxKeyEvent &);

public:
  gbtNumberValidator(wxString *);
  gbtNumberValidator(wxString *, const Gambit::Rational &);
  gbtNumberValidator(wxString *, const Gambit::Rational &, const Gambit::Rational &);
  gbtNumberValidator(const gbtNumberValidator &);
  virtual ~gbtNumberValidator() { }

  virtual wxObject *Clone(void) const { return new gbtNumberValidator(*this); }
  bool Copy(const gbtNumberValidator &);

  virtual bool Validate(wxWindow *parent);
  virtual bool TransferToWindow(void);
  virtual bool TransferFromWindow(void);

  DECLARE_EVENT_TABLE()
};

#endif  // VALNUMBER_H
