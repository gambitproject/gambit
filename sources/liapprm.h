//
// FILE: liapprm.h -- definition of the parameter dialog for LiapSolve
//
// $Id$
//

#ifndef LIAPPRM_H
#define LIAPPRM_H

#include "algdlgs.h"

class LiapSolveParamsDialog : public dialogAlgorithm {
private:
  wxText *m_tol1D, *m_tolND, *m_maxits1D, *m_maxitsND, *m_nTries;
  wxRadioBox *m_startOption;

  void AlgorithmFields(void);

public:
  LiapSolveParamsDialog(wxWindow *p_parent, bool p_subgames = false,
			bool p_vianfg = false);
  virtual ~LiapSolveParamsDialog();

  double Tol1D(void) const
    { return ToNumber(m_tol1D->GetValue()); }
  double TolND(void) const
    { return ToNumber(m_tolND->GetValue()); }
  int Maxits1D(void) const
    { return (int) ToNumber(m_maxits1D->GetValue()); }
  int MaxitsND(void) const
    { return (int) ToNumber(m_maxitsND->GetValue()); }
  int NumTries(void) const
    { return (int) ToNumber(m_nTries->GetValue()); }

  int StartOption(void) const
    { return m_startOption->GetSelection(); }
};

#endif  // LIAPPRM_H
