//
// FILE: dlefgoutcome.h -- Outcome selection dialog
//
// $Id$
//

#ifndef DLEFGOUTCOME_H
#define DLEFGOUTCOME_H

class dialogEfgOutcomeSelect : public guiAutoDialog {
private:
  FullEfg &m_efg;
  wxListBox *m_outcomeList;

  const char *HelpString(void) const { return "Outcome Select Dialog"; }

public:
  dialogEfgOutcomeSelect(FullEfg &, wxWindow * = 0);
  virtual ~dialogEfgOutcomeSelect() { }

  efgOutcome GetOutcome(void); 
};

#endif  // DLEFGOUTCOME_H
