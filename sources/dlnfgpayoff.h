//
// FILE: dlnfgpayoff.h -- Dialog for editing outcome payoffs
//
// $Id$
//

#ifndef DLNFGPAYOFF_H
#define DLNFGPAYOFF_H

class dialogNfgPayoffs : public guiPagedDialog {
private:
  NFOutcome *m_outcome;
  const Nfg &m_nfg;
  wxText *m_outcomeName;

  const char *HelpString(void) const { return "Outcome Menu"; }

public:
  dialogNfgPayoffs(const Nfg &, NFOutcome *, wxWindow *parent);
  virtual ~dialogNfgPayoffs() { }

  gArray<gNumber> Payoffs(void) const;
  gText Name(void) const;
};

#endif  // DLNFGPAYOFF_H
