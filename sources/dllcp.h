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
  bool m_lemkeHowson;
  wxIntegerItem *m_maxDepth;
  wxCheckBox *m_dupStrat;

  void AlgorithmFields(void);

public:
  dialogLcp(bool p_lemkeHowson, wxWindow *p_parent = 0,
	    bool p_subgames = false, bool p_vianfg = false);
  virtual ~dialogLcp();

  int MaxDepth(void) const  { return m_maxDepth->GetInteger(); }
  int StopAfter(void) const;
  bool DupStrat(void) const  { return m_dupStrat->GetValue(); }
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DLLCP_H
