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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#ifdef __GNUG__
#pragma implementation "valnumber.h"
#endif  // __GNUG__

#include "valnumber.h"
#include "base/base.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

BEGIN_EVENT_TABLE(gNumberValidator, wxValidator)
  EVT_CHAR(gNumberValidator::OnChar)
END_EVENT_TABLE()

static bool IsNumeric(const wxString &p_value)
{
  bool seenDigit = false, seenSeparator = false;

  for (int i = 0; i < (int) p_value.Length(); i++) {
    if (isdigit(p_value[i])) {
      seenDigit = true;
      continue;
    }
    else if ((p_value[i] == '.' && !seenSeparator) ||
	     (p_value[i] == '/' && !seenSeparator && seenDigit)) {
      seenSeparator = true;
      continue;
    }
    else if (i == 0 && p_value[i] == '-') {
      continue;
    }
    return false;
  }

  if (p_value[p_value.Length() - 1] == '/') {
    return false;
  }
  else {
    return true;
  }
}

//------------------------------------------------------------------------
//               class gNumberValidator: Member functions
//------------------------------------------------------------------------

gNumberValidator::gNumberValidator(wxString *p_value)
  : m_stringValue(p_value), m_hasMin(false), m_hasMax(false)
{ }

gNumberValidator::gNumberValidator(wxString *p_value,
				   const gNumber &p_minValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(false),
    m_minValue(p_minValue)
{ }

gNumberValidator::gNumberValidator(wxString *p_value,
				   const gNumber &p_minValue,
				   const gNumber &p_maxValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(true),
    m_minValue(p_minValue), m_maxValue(p_maxValue)
{ }

gNumberValidator::gNumberValidator(const gNumberValidator &p_validator)
{
  Copy(p_validator);
}

bool gNumberValidator::Copy(const gNumberValidator &p_validator)
{
  wxValidator::Copy(p_validator);
  m_stringValue = p_validator.m_stringValue;
  m_hasMin = p_validator.m_hasMin;
  m_hasMax = p_validator.m_hasMax;
  m_minValue = p_validator.m_minValue;
  m_maxValue = p_validator.m_maxValue;
  return true;
}

bool gNumberValidator::Validate(wxWindow *p_parent)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

  if (!control->IsEnabled()) {
    return true;
  }

  wxString value(control->GetValue());

  if (!IsNumeric(value)) {
    wxMessageBox(wxString("The value ") + value + " in " +
		 m_validatorWindow->GetName() + " is not a valid number.",
		 "Error", wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  if ((m_hasMin && ToNumber(value.c_str()) < m_minValue) ||
      (m_hasMax && ToNumber(value.c_str()) > m_maxValue)) {
    wxMessageBox(wxString("The value ") + value + " in " +
		 m_validatorWindow->GetName() + " is out of the range [" +
		 wxString((char *) ToText(m_minValue)) + ", " + 
		 wxString((char *) ToText(m_maxValue)) + "].",
		 "Error", wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  return true;
}

bool gNumberValidator::TransferToWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  control->SetValue(*m_stringValue);

  return true;
}

bool gNumberValidator::TransferFromWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  *m_stringValue = control->GetValue();

  return TRUE;
}

void gNumberValidator::OnChar(wxKeyEvent &p_event)
{
  if (m_validatorWindow) {
    int keyCode = (int) p_event.KeyCode();

    // we don't filter special keys and Delete
    if (!(keyCode < WXK_SPACE || keyCode == WXK_DELETE || 
	  keyCode > WXK_START) &&
	(!isdigit(keyCode) &&
	 keyCode != '.' && keyCode != '-' && keyCode != '/')) {
      if (!wxValidator::IsSilent()) {
	wxBell();
      }

      return;
    }
  }

  p_event.Skip();
}




