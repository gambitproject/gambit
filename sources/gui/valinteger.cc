//
// FILE: valinteger.cc -- wxValidator specialization to integers
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#ifdef __GNUG__
#pragma implementation "valinteger.h"
#endif  // __GNUG__

#include "valinteger.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

BEGIN_EVENT_TABLE(gIntegerValidator, wxValidator)
  EVT_CHAR(gIntegerValidator::OnChar)
END_EVENT_TABLE()

static bool IsInteger(const wxString &p_value)
{
  for (int i = 0; i < (int) p_value.Length(); i++) {
    if (isdigit(p_value[i])) {
      continue;
    }
    if (i == 0 && p_value[i] == '-') {
      continue;
    }
    return false;
  }

  return true;
}

//------------------------------------------------------------------------
//               class gIntegerValidator: Member functions
//------------------------------------------------------------------------

gIntegerValidator::gIntegerValidator(wxString *p_value)
  : m_stringValue(p_value), m_hasMin(false), m_hasMax(false)
{ }

gIntegerValidator::gIntegerValidator(wxString *p_value, int p_minValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(false),
    m_minValue(p_minValue)
{ }

gIntegerValidator::gIntegerValidator(wxString *p_value,
				     int p_minValue, int p_maxValue)
  : m_stringValue(p_value), m_hasMin(true), m_hasMax(true),
    m_minValue(p_minValue), m_maxValue(p_maxValue)
{ }

gIntegerValidator::gIntegerValidator(const gIntegerValidator &p_validator)
{
  Copy(p_validator);
}

bool gIntegerValidator::Copy(const gIntegerValidator &p_validator)
{
  wxValidator::Copy(p_validator);
  m_stringValue = p_validator.m_stringValue;
  m_hasMin = p_validator.m_hasMin;
  m_hasMax = p_validator.m_hasMax;
  m_minValue = p_validator.m_minValue;
  m_maxValue = p_validator.m_maxValue;
  return true;
}

bool gIntegerValidator::Validate(wxWindow *p_parent)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

  if (!control->IsEnabled()) {
    return true;
  }

  wxString value(control->GetValue());

  if (!IsInteger(value)) {
    wxMessageBox(wxString("The value ") + value + " in " +
		 m_validatorWindow->GetName() + " is not a valid integer.",
		 "Error", wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  if ((m_hasMin && atoi(value.c_str()) < m_minValue) ||
      (m_hasMax && atoi(value.c_str()) > m_maxValue)) {
    wxMessageBox(wxString("The value ") + value + " in " +
		 m_validatorWindow->GetName() + " is out of the range [" +
		 wxString::Format("%d", m_minValue) + ", " +
		 wxString::Format("%d", m_maxValue) + "].",
		 "Error", wxOK | wxICON_EXCLAMATION, p_parent);
    m_validatorWindow->SetFocus();
    return false;
  }

  return true;
}

bool gIntegerValidator::TransferToWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  control->SetValue(*m_stringValue);

  return true;
}

bool gIntegerValidator::TransferFromWindow(void)
{
  if (!m_stringValue) {
    return false;
  }

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
  *m_stringValue = control->GetValue();

  return TRUE;
}

void gIntegerValidator::OnChar(wxKeyEvent &p_event)
{
  if (m_validatorWindow) {
    int keyCode = (int) p_event.KeyCode();

    // we don't filter special keys and Delete
    if (!(keyCode < WXK_SPACE || keyCode == WXK_DELETE || 
	  keyCode > WXK_START) &&
	(!isdigit(keyCode) && keyCode != '-')) {
      if (!wxValidator::IsSilent()) {
	wxBell();
      }

      return;
    }
  }

  p_event.Skip();
}



