//
// FILE: dllp.h -- Parameter selection for LpSolve
//
// $Id$
//

#ifndef DLLP_H
#define DLLP_H

#include "algdlgs.h"

class dialogLp : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

public:
  dialogLp(wxWindow *p_parent = 0, bool p_subgames = false,
	   bool p_vianfg = false);
  virtual ~dialogLp();

  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DLLP_H





