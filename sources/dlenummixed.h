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

public:
  dialogEnumMixed(wxWindow *p_parent = 0, bool p_subgames = false);
  virtual ~dialogEnumMixed();

  int StopAfter(void) const;
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

#include "wxstatus.h"

class wxEnumStatus : public wxStatus {
private:
  int pass;

public:
  wxEnumStatus(wxFrame *p_parent);
  void SetProgress(double p_value);
};

#endif  // DLENUMMIXED_H


