//
// FILE: seqfprm.h -- definition of params dialog for LCPSolve[EFG]
//
// $Id$
//

#ifndef SEQFPRM_H
#define SEQFPRM_H

#include "algdlgs.h"

class LcpSolveDialog : public OutputParamsDialog {
private:
  wxText *m_maxDepth, *m_stopAfter;
  wxRadioBox *m_precision;

  void AlgorithmFields(void);

public:
  LcpSolveDialog(wxWindow *p_parent = 0, bool p_subgames = false,
		 bool p_vianfg = false);
  virtual ~LcpSolveDialog() { }

  int MaxDepth(void) const
    { return (int) ToDouble(m_maxDepth->GetValue()); }
  int StopAfter(void) const
    { return (int) ToDouble(m_stopAfter->GetValue()); }
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // SEQFPRM_H
