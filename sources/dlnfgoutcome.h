//
// FILE: dlnfgoutcome.h -- Outcome selection dialog
//
// $Id$
//

#ifndef DLNFGOUTCOME_H
#define DLNFGOUTCOME_H

class dialogNfgOutcomeSelect : public guiAutoDialog {
private:
  Nfg &m_nfg;
  wxListBox *m_outcomeList;

  const char *HelpString(void) const { return "Outcome Menu"; }

public:
  dialogNfgOutcomeSelect(Nfg &, wxWindow * = 0);
  virtual ~dialogNfgOutcomeSelect() { }

  NFOutcome *GetOutcome(void); 
};

#endif  // DLNFGOUTCOME_H
