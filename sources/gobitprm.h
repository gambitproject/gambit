//
// FILE: gobitprm.h -- Parameter selection for QreSolve
//
// $Id$
//

#ifndef GOBITPRM_H
#define GOBITPRM_H

#include "algdlgs.h"

class QreSolveParamsDialog : public PxiParamsDialog {
private:
  wxText *m_minLam, *m_maxLam, *m_delLam, *m_tol1, *m_tolN;
  wxText *m_maxits1, *m_maxitsN;
  wxRadioBox *m_startOption;

  void AlgorithmFields(void);

public:
  QreSolveParamsDialog(wxWindow *parent, const gText filename,
		       bool p_vianfg = false);
  virtual ~QreSolveParamsDialog() { }

  double MinLam(void) const
    { return ToNumber(m_minLam->GetValue()); }
  double MaxLam(void) const
    { return ToNumber(m_maxLam->GetValue()); }
  double DelLam(void) const
    { return ToNumber(m_delLam->GetValue()); }
  double Tol1D(void) const
    { return ToNumber(m_tol1->GetValue()); }
  double TolND(void) const
    { return ToNumber(m_tolN->GetValue()); }
  int Maxits1D(void) const
    { return (int) ToNumber(m_maxits1->GetValue()); }
  int MaxitsND(void) const
    { return (int) ToNumber(m_maxitsN->GetValue()); }

  int StartOption(void) const
    { return m_startOption->GetSelection(); }
};

#endif  // GOBITPRM_H
