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
  wxTextCtrl *m_minLam, *m_maxLam, *m_delLam;
  wxTextCtrl *m_delp1, *m_tol1, *m_delp2, *m_tol2;
  wxCheckBox *m_multiGrid;
  wxRadioBox *m_startOption;

  wxString m_minLamValue, m_maxLamValue, m_delLamValue;
  wxString m_delp1Value, m_tol1Value, m_delp2Value, m_tol2Value;

  const char *HelpString(void) const  { return "QRE Grid"; }

  void AlgorithmFields(void);

  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  dialogQreGrid(wxWindow *p_parent, const gText &p_filename);
  virtual ~dialogQreGrid();

  double MinLam(void) const
    { return ToNumber(m_minLam->GetValue().c_str()); }
  double MaxLam(void) const
    { return ToNumber(m_maxLam->GetValue().c_str()); }
  double DelLam(void) const
    { return ToNumber(m_delLam->GetValue().c_str()); }
  double Tol1(void) const
    { return ToNumber(m_tol1->GetValue().c_str()); }
  double DelP1(void) const
    { return ToNumber(m_delp1->GetValue().c_str()); }
  double Tol2(void) const
    { return ToNumber(m_tol2->GetValue().c_str()); }
  double DelP2(void) const
    { return ToNumber(m_delp2->GetValue().c_str()); }

  bool MultiGrid(void) const
    { return m_multiGrid->GetValue(); }
  int StartOption(void) const
    { return m_startOption->GetSelection(); }

  DECLARE_EVENT_TABLE()
};

#endif // DLQREGRID_H
