//
// FILE: enumprm.h -- Dialog boxes for parameters to EnumMixedSolve
//
// $Id$
//

#ifndef ENUMPRM_H
#define ENUMPRM_H

#include "algdlgs.h"
#include "enum.h"

class EnumSolveParamsDialog : public dialogAlgorithm {
private:
  wxText *m_stopAfter;
  wxRadioBox *m_precision;

  void AlgorithmFields(void);

public:
  EnumSolveParamsDialog(wxWindow *parent = 0, bool subgames = false,
			bool vianfg = false);
  virtual ~EnumSolveParamsDialog();

  int StopAfter(void) const
    { return (int) ToDouble(m_stopAfter->GetValue()); }
  gPrecision Precision(void) const
    { return (m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL; }
};

//
// Customized progress indicator
//
class wxEnumStatus : public wxStatus {
private:
  int pass;

public:
  wxEnumStatus(wxFrame *p_parent);
  void SetProgress(double p_value);
};

#endif  // ENUMPRM_H

