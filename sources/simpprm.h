//
// FILE: simpprm.h -- Parameter selection for SimpdivSolve
//
// $Id$
//

#ifndef SIMPPRM_H
#define SIMPPRM_H

#include "algdlgs.h"

class SimpdivSolveParamsDialog : public dialogAlgorithm {
private:
  wxText *m_nRestarts, *m_leashLength, *m_stopAfter;
  wxRadioBox *m_precision;

  void AlgorithmFields(void);

public:
  SimpdivSolveParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~SimpdivSolveParamsDialog() { }

  int NumRestarts(void) const
    { return (int) ToNumber(m_nRestarts->GetValue()); }
  int LeashLength(void) const
    { return (int) ToNumber(m_leashLength->GetValue()); }
  int StopAfter(void) const
    { return (int) ToNumber(m_stopAfter->GetValue()); }
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // SIMPPRM_H
