//
// FILE: nfgsolvd.h -- the main dialog for running NormalForm solution
//                     algorithms.
//
// $Id$
//

#ifndef NFGSOLVD_H
#define NFGSOLVD_H

typedef enum {
  nfgSTANDARD_NASH = 0, nfgSTANDARD_PERFECT = 1
} nfgStandardType;

typedef enum {
  nfgSTANDARD_ONE = 0, nfgSTANDARD_TWO = 1, nfgSTANDARD_ALL = 2
} nfgStandardNum;

class dialogNfgSolveStandard : public guiAutoDialog {
private:
  wxRadioBox *m_standardType, *m_standardNum, *m_precision;
  wxTextCtrl *m_description;
  const Nfg &m_nfg;

  // Button event handler
  void OnChanged(wxCommandEvent &);

  const char *HelpString(void) const { return "NFG Standard Solutions"; }

public:
  dialogNfgSolveStandard(wxWindow *p_parent, const Nfg &p_nfg);
  virtual ~dialogNfgSolveStandard();

  nfgStandardType Type(void) const;
  nfgStandardNum Number(void) const;
  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }

  DECLARE_EVENT_TABLE()
};


class guiBadStandardSolve : public gException {
private:
  gText m_description;

public:
  guiBadStandardSolve(const gText &p_description)
    : m_description(p_description) { }
  virtual ~guiBadStandardSolve() { }

  gText Description(void) const { return m_description; }
};
  

#endif // NFGSOLVD_H
