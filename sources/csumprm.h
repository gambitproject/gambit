//
// FILE: csumprm.h -- Parameter selection for LpSolve
//
// $Id$
//

#ifndef CSUMPRM_H
#define CSUMPRM_H

#include "algdlgs.h"

class LPSolveParamsDialog : public OutputParamsDialog {
private:
  wxText *m_stopAfter;
  wxRadioBox *m_precision;

  void AlgorithmFields(void);

public:
  LPSolveParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false,
		      bool p_vianfg = false);
  virtual ~LPSolveParamsDialog();

  int StopAfter(void) const
    { return (int) ToDouble(m_stopAfter->GetValue()); }
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // CSUMPRM_H

