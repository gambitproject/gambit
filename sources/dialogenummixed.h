//
// FILE: dialogenummixed.h -- EnumMixedSolve interface
//
// $Id$
//

#ifndef DIALOGENUMMIXED_H
#define DIALOGENUMMIXED_H

#include "dialogalgorithm.h"

class dialogEnumMixed : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

  gText HelpTopic(void) const { return "EnumMixed"; }
  
public:
  dialogEnumMixed(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~dialogEnumMixed();

  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DLENUMMIXED_H


