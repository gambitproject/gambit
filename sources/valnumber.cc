//
// FILE: valnumber.cc -- wxValidator specialization to gNumber
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation "valnumber.h"
#endif

#include "wx/wx.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "valnumber.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

BEGIN_EVENT_TABLE(gNumberValidator, wxValidator)
  EVT_CHAR(gNumberValidator::OnChar)
END_EVENT_TABLE()

static bool IsNumeric(const wxString &p_value)
{
  for (int i = 0; i < (int) p_value.Length(); i++) {
    if (isdigit(p_value[i]) || p_value[i] == '.') {
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
//               class gNumberValidator: Member functions
//------------------------------------------------------------------------

gNumberValidator::gNumberValidator(wxString *val)
{
  m_stringValue = val ;
}

gNumberValidator::gNumberValidator(const gNumberValidator& val)
{
  Copy(val);
}

bool gNumberValidator::Copy(const gNumberValidator& val)
{
  wxValidator::Copy(val);

  m_stringValue = val.m_stringValue;

  return true;
}

bool gNumberValidator::Validate(wxWindow *parent)
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
    m_validatorWindow->SetFocus();

    wxMessageBox("Numeric value out of range", "Validation conflict",
		 wxOK | wxICON_EXCLAMATION, parent);
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
  control->SetValue(* m_stringValue);

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
	(!isdigit(keyCode) && keyCode != '.' && keyCode != '-')) {
      if (!wxValidator::IsSilent()) {
	wxBell();
      }

      return;
    }
  }

  p_event.Skip();
}



