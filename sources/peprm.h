//
// FILE: peprm.h -- Parameter selection for PolEnumSolve
//
// $Id$
//

#ifndef PEPRM_H
#define PEPRM_H

#include "algdlgs.h"

class guiPolEnumParamsDialog : public OutputParamsDialog {
private:
  wxText *m_stopAfter;

  void AlgorithmFields(void);

public:
  guiPolEnumParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false,
			 bool p_vianfg = false);
  virtual ~guiPolEnumParamsDialog() { }

  int StopAfter(void) const
    { return (int) ToDouble(m_stopAfter->GetValue()); }
};

#endif  // PEPRM_H

