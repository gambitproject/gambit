//
// FILE: dllcp.h -- Parameter selection for LcpSolve
//
// $Id$
//

#ifndef DLLCP_H
#define DLLCP_H

#include "algdlgs.h"

class dialogLcp : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

  const char *HelpString(void) const { return "Lp"; }

public:
  dialogLcp(wxWindow *p_parent = 0,
	    bool p_subgames = false, bool p_vianfg = false);
  virtual ~dialogLcp();

  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DLLCP_H
