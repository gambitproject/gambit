//
// FILE: dlefgpayoff.h -- Dialog for editing outcome payoffs
//
// $Id$
//

#ifndef DLEFGPAYOFF_H
#define DLEFGPAYOFF_H

class dialogEfgPayoffs : public guiPagedDialog {
private:
  EFOutcome *m_outcome;
  const FullEfg &m_efg;
  gText m_name;
  wxText *m_outcomeName;

public:
  dialogEfgPayoffs(const FullEfg &, EFOutcome *, wxWindow *parent);
  virtual ~dialogEfgPayoffs() { }

  gArray<gNumber> Payoffs(void) const;
  gText Name(void) const;
};

#endif  // DLEFGPAYOFF_H
