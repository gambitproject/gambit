//
// FILE: dialogefgstandard.h -- Standard extensive form solutions
//
// $Id$
//

#ifndef DIALOGEFGSTANDARD_H
#define DIALOGEFGSTANDARD_H

typedef enum {
  efgSTANDARD_NASH = 0, efgSTANDARD_PERFECT = 1, efgSTANDARD_SEQUENTIAL = 2
} efgStandardType;

typedef enum {
  efgSTANDARD_ONE = 0, efgSTANDARD_TWO = 1, efgSTANDARD_ALL = 2
} efgStandardNum;

class dialogEfgStandard : public guiAutoDialog {
private:
  wxRadioBox *m_standardType, *m_standardNum, *m_precision;
  wxTextCtrl *m_description;
  const Efg &m_efg;

  void OnChange(void);

  const char *HelpString(void) const { return "EFG Standard Solutions"; }

public:
  dialogEfgStandard(wxWindow *p_parent, const Efg &p_efg);
  virtual ~dialogEfgStandard();

  efgStandardType Type(void) const;
  efgStandardNum Number(void) const;
  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }

  DECLARE_EVENT_TABLE()
};

#endif   // DIALOGEFGSTANDARD_H
