//
// FILE: valinteger.cc -- wxValidator specialization to integers
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation "valinteger.h"
#endif  // __GNUG__

#include "wx/wx.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif  // __BORLANDC__

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

gIntegerValidator::gIntegerValidator(wxString *val)
{
  m_stringValue = val ;
}

gIntegerValidator::gIntegerValidator(const gIntegerValidator& val)
{
  Copy(val);
}

bool gIntegerValidator::Copy(const gIntegerValidator& val)
{
  wxValidator::Copy(val);

  m_stringValue = val.m_stringValue;

  return true;
}

bool gIntegerValidator::Validate(wxWindow *parent)
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
    m_validatorWindow->SetFocus();

    wxMessageBox("Numeric value out of range", "Validation conflict",
		 wxOK | wxICON_EXCLAMATION, parent);
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



