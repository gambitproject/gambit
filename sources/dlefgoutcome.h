//
// FILE: dlefgoutcome.h -- Outcome selection dialog
//
// $Id$
//

#ifndef DLEFGOUTCOME_H
#define DLEFGOUTCOME_H

class dialogEfgOutcomeSelect : public guiAutoDialog {
private:
  Efg &m_efg;
  wxListBox *m_outcomeList;

  const char *HelpString(void) const { return "Outcome Menu"; }

public:
  dialogEfgOutcomeSelect(Efg &, wxWindow * = 0);
  virtual ~dialogEfgOutcomeSelect() { }

  EFOutcome *GetOutcome(void); 
};

#endif  // DLEFGOUTCOME_H
