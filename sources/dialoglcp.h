//
// FILE: dialoglcp.h -- Parameter selection for LcpSolve
//
// $Id$
//

#ifndef DIALOGLCP_H
#define DIALOGLCP_H

#include "dialogalgorithm.h"

class dialogLcp : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

  gText HelpTopic(void) const { return "Lcp"; }

public:
  dialogLcp(wxWindow *p_parent, bool p_subgames, bool p_vianfg);
  virtual ~dialogLcp();

  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DIALOGLCP_H
