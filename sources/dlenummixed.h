//
// FILE: dlenummixed.h -- EnumMixedSolve interface
//
// $Id$
//

#ifndef DLENUMMIXED_H
#define DLENUMMIXED_H

#include "algdlgs.h"

class dialogEnumMixed : public dialogAlgorithm {
private:
  void AlgorithmFields(void);

  const char *HelpString(void) const { return "EnumMixed"; }
  
public:
  dialogEnumMixed(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~dialogEnumMixed();

  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#endif  // DLENUMMIXED_H


