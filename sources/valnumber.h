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
#include "gnumber.h"

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
