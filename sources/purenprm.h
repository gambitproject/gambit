//
// FILE: purenprm.h -- Parameter selection for EnumPureSolve
//
// $Id$
//

#ifndef PURENPRM_H
#define PURENPRM_H

#include "algdlgs.h"

class PureNashSolveParamsDialog : public dialogAlgorithm {
private:
  wxText *m_stopAfter;

  void AlgorithmFields(void);

public:
  PureNashSolveParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false,
			    bool p_vianfg = false);
  virtual ~PureNashSolveParamsDialog() { }

  int StopAfter(void) const
    { return (int) ToDouble(m_stopAfter->GetValue()); }
};

#endif  // PURENPRM_H
