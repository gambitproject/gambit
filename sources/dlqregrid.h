//
// FILE: dlqregrid.h -- Parameter selection for QreGridSolve
//
// $Id$
//

#ifndef DLQREGRID_H
#define DLQREGRID_H

#include "algdlgs.h"

class dialogQreGrid : public dialogPxi {
private:
  wxNumberItem *m_minLam, *m_maxLam, *m_delLam;
  wxNumberItem *m_delp1, *m_tol1, *m_delp2, *m_tol2;
  wxCheckBox *m_multiGrid;
  wxRadioBox *m_startOption;

  void AlgorithmFields(void);

public:
  dialogQreGrid(wxWindow *p_parent, const gText &p_filename);
  virtual ~dialogQreGrid();

  double MinLam(void) const
    { return m_minLam->GetNumber(); }
  double MaxLam(void) const
    { return m_maxLam->GetNumber(); }
  double DelLam(void) const
    { return m_delLam->GetNumber(); }
  double Tol1(void) const
    { return m_tol1->GetNumber(); }
  double DelP1(void) const
    { return m_delp1->GetNumber(); }
  double Tol2(void) const
    { return m_tol2->GetNumber(); }
  double DelP2(void) const
    { return m_delp2->GetNumber(); }

  bool MultiGrid(void) const
    { return m_multiGrid->GetValue(); }
  int StartOption(void) const
    { return m_startOption->GetSelection(); }
};

#endif // DLQREGRID_H
