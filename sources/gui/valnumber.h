//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// wxValidator specialization to gNumber
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifdef __GNUG__
#pragma interface "valnumber.h"
#endif  // __GNUG__

#include "wx/validate.h"
#include "math/gnumber.h"

class gNumberValidator : public wxValidator {
protected:
  wxString *m_stringValue;
  bool m_hasMin, m_hasMax;
  gNumber m_minValue, m_maxValue;
  
  // Event handlers
  void OnChar(wxKeyEvent &);

public:
  gNumberValidator(wxString *);
  gNumberValidator(wxString *, const gNumber &);
  gNumberValidator(wxString *, const gNumber &, const gNumber &);
  gNumberValidator(const gNumberValidator &);
  virtual ~gNumberValidator() { }

  virtual wxObject *Clone(void) const { return new gNumberValidator(*this); }
  bool Copy(const gNumberValidator &);

  virtual bool Validate(wxWindow *parent);
  virtual bool TransferToWindow(void);
  virtual bool TransferFromWindow(void);

  DECLARE_EVENT_TABLE()
};

#endif  // VALNUMBER_H
