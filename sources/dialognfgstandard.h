//
// FILE: dialognfgstandard.h -- Select a standard normal form algorithm
//
// $Id$
//

#ifndef DIALOGNFGSTANDARD_H
#define DIALOGNFGSTANDARD_H

typedef enum {
  nfgSTANDARD_NASH = 0, nfgSTANDARD_PERFECT = 1
} nfgStandardType;

typedef enum {
  nfgSTANDARD_ONE = 0, nfgSTANDARD_TWO = 1, nfgSTANDARD_ALL = 2
} nfgStandardNum;

class dialogNfgStandard : public guiAutoDialog {
private:
  wxRadioBox *m_standardType, *m_standardNum, *m_precision;
  wxTextCtrl *m_description;
  const Nfg &m_nfg;

  void OnChange(void);

  const char *HelpString(void) const { return "NFG Standard Solutions"; }

public:
  dialogNfgStandard(wxWindow *p_parent, const Nfg &p_nfg);
  virtual ~dialogNfgStandard();

  nfgStandardType Type(void) const;
  nfgStandardNum Number(void) const;
  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }

  DECLARE_EVENT_TABLE()
};


#endif // DIALOGNFGSTANDARD_H
