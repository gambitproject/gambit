//
// FILE: dlsimpdiv.h -- Parameter selection for SimpdivSolve
//
// $Id$
//

#ifndef DLSIMPDIV_H
#define DLSIMPDIV_H

#include "algdlgs.h"

class dialogSimpdiv : public dialogAlgorithm {
private:
  wxIntegerItem *m_nRestarts, *m_leashLength;

  gText HelpTopic(void) const { return "Simpdiv"; }

  void AlgorithmFields(void);

public:
  dialogSimpdiv(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~dialogSimpdiv();

  int NumRestarts(void) const { return m_nRestarts->GetInteger(); }
  int LeashLength(void) const { return m_leashLength->GetInteger(); }
  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DLSIMPDIV_H
