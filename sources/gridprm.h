//
// FILE: gridprm.h -- Parameter selection for GobitGridSolve
//
// $Id$
//

#ifndef GRIDPRM_H
#define GRIDPRM_H

#include "algdlgs.h"

class GridSolveParamsDialog : public dialogAlgorithm {
private:
  wxText *m_minLam, *m_maxLam, *m_delLam, *m_delp1, *m_tol1, *m_delp2, *m_tol2;
  wxCheckBox *m_multiGrid;

  void AlgorithmFields(void);

public:
  GridSolveParamsDialog(wxWindow *p_parent, const gText &p_filename);
  virtual ~GridSolveParamsDialog() { }

  double MinLam(void) const
    { return ToNumber(m_minLam->GetValue()); }
  double MaxLam(void) const
    { return ToNumber(m_maxLam->GetValue()); }
  double DelLam(void) const
    { return ToNumber(m_delLam->GetValue()); }
  double Tol1(void) const
    { return ToNumber(m_tol1->GetValue()); }
  double DelP1(void) const
    { return ToNumber(m_delp1->GetValue()); }
  double Tol2(void) const
    { return ToNumber(m_tol2->GetValue()); }
  double DelP2(void) const
    { return ToNumber(m_delp2->GetValue()); }

  bool MultiGrid(void) const
    { return m_multiGrid->GetValue(); }
};

#endif // GRIDPRM_H
