//
// FILE: dialoglp.h -- Parameter selection for LpSolve
//
// $Id$
//

#ifndef DIALOGLP_H
#define DIALOGLP_H

#include "dialogalgorithm.h"

class dialogLp : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

  gText HelpTopic(void) const { return "Lp"; }

public:
  dialogLp(wxWindow *p_parent, bool p_subgames, bool p_vianfg);
  virtual ~dialogLp();

  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DIALOGLP_H





