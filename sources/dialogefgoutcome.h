//
// FILE: dlefgoutcome.h -- Dialog for editing outcome payoffs
//
// $Id$
//

#ifndef DIALOGEFGOUTCOME_H
#define DIALOGEFGOUTCOME_H

#include "dialogpaged.h"

class dialogEfgOutcome : public guiPagedDialog {
private:
  EFOutcome *m_outcome;
  const Efg &m_efg;
  gText m_name;
  wxTextCtrl *m_outcomeName;

public:
  dialogEfgOutcome(wxWindow *, const Efg &, EFOutcome *);
  virtual ~dialogEfgOutcome() { }

  gArray<gNumber> Payoffs(void) const;
  gText Name(void) const;
};

#endif  // DIALOGEFGOUTCOME_H
