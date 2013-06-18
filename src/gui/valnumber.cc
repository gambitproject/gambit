//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/valnumber.cc
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

#include <cctype>
#include <cstring>
#include <cstdlib>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "libgambit/libgambit.h"
#include "valnumber.h"

BEGIN_EVENT_TABLE(gbtNumberValidator, wxValidator)
  EVT_CHAR(gbtNumberValidator::OnChar)
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
//               class gbtNumberValidator: Member functions
//------------------------------------------------------------------------

gbtNumberValidator::gbtNumberValidator(wxString *p_value)
  : m_stringValue(p_value), m_hasMin(false), m_hasMax(false)
{ }

gbtNumberValidator::gbtNumberValidator(wxString *p_value,
				       const Gambit::Rational &p_minValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(false),
    m_minValue(p_minValue)
{ }

gbtNumberValidator::gbtNumberValidator(wxString *p_value,
				       const Gambit::Rational &p_minValue,
				       const Gambit::Rational &p_maxValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(true),
    m_minValue(p_minValue), m_maxValue(p_maxValue)
{ }

gbtNumberValidator::gbtNumberValidator(const gbtNumberValidator &p_validator)
{
  Copy(p_validator);
}

bool gbtNumberValidator::Copy(const gbtNumberValidator &p_validator)
{
  wxValidator::Copy(p_validator);
  m_stringValue = p_validator.m_stringValue;
  m_hasMin = p_validator.m_hasMin;
  m_hasMax = p_validator.m_hasMax;
  m_minValue = p_validator.m_minValue;
  m_maxValue = p_validator.m_maxValue;
  return true;
}

bool gbtNumberValidator::Validate(wxWindow *p_parent)
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
    wxMessageBox(_T("The value ") + value + _T(" in ") +
		 m_validatorWindow->GetName() + _T(" is not a valid number."),
		 _("Error"), wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  if ((m_hasMin && Gambit::lexical_cast<Gambit::Rational>(std::string((const char *) value.mb_str())) < m_minValue) ||
      (m_hasMax && Gambit::lexical_cast<Gambit::Rational>(std::string((const char *) value.mb_str())) > m_maxValue)) {
    wxMessageBox(_T("The value ") + value + _T(" in ") +
		 m_validatorWindow->GetName() + _T(" is out of the range [") +
		 wxString(Gambit::lexical_cast<std::string>(m_minValue).c_str(), *wxConvCurrent) + _T(", ") + 
		 wxString(Gambit::lexical_cast<std::string>(m_maxValue).c_str(), *wxConvCurrent) + _T("]."),
		 _("Error"), wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  return true;
}

bool gbtNumberValidator::TransferToWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  control->SetValue(*m_stringValue);

  return true;
}

bool gbtNumberValidator::TransferFromWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  *m_stringValue = control->GetValue();

  return TRUE;
}

void gbtNumberValidator::OnChar(wxKeyEvent &p_event)
{
  if (m_validatorWindow) {
    int keyCode = (int) p_event.GetKeyCode();

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

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
    wxString value = control->GetValue();

    if ((keyCode == '.' || keyCode == '/') && 
	(value.Find('.') != -1 || value.Find('/') != -1)) {
      // At most one slash or decimal point is allowed
      if (!wxValidator::IsSilent())  wxBell();
      return;
    }

    if (keyCode == '/' && 
	(control->GetInsertionPoint() == 0 ||
	 (control->GetInsertionPoint() == 1 && value == wxT("-")))) {
      // Can't start with a slash
      if (!wxValidator::IsSilent())  wxBell();
      return;
    }

    if (keyCode == '-') {
      // Only permit minus signs at the start of the text
      long start, end;
      control->GetSelection(&start, &end);
      
      if (start == end) {
	// No selection; just see if inserting is OK
	if (control->GetInsertionPoint() != 0) {
	  if (!wxValidator::IsSilent())  wxBell();
	  return;
	}
      }
      else {
	// There is a selection; is selection at beginning?
	if (start != 0) {
	  if (!wxValidator::IsSilent())  wxBell();
	  return;
	}	
      }
    }
  }

  p_event.Skip();
}




