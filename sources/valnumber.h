//
// FILE: valnumber.h -- declaration of validator specialization to gNumber
//
// $Id$
//

#ifndef VALNUMBER_H
#define VALNUMBER_H

#ifdef __GNUG__
#pragma interface "valnumber.h"
#endif  // __GNUG__

#include "wx/validate.h"

class gNumberValidator : public wxValidator {
protected:
  wxString *m_stringValue;
  
  // Event handlers
  void OnChar(wxKeyEvent &);

public:
  gNumberValidator(wxString *);
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
