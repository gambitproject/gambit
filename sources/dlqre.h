//
// FILE: dlqre.h -- Parameter selection for QreSolve
//
// $Id$
//

#ifndef DLQRE_H
#define DLQRE_H

#include "algdlgs.h"

class dialogQre : public dialogPxi {
private:
  wxNumberItem *m_minLam, *m_maxLam, *m_delLam;
  wxIntegerItem *m_tol1, *m_tolN, *m_maxits1, *m_maxitsN;
  wxRadioBox *m_startOption;

  void AlgorithmFields(void);

public:
  dialogQre(wxWindow *parent, const gText &filename, bool p_vianfg = false);
  virtual ~dialogQre();

  double MinLam(void) const
    { return m_minLam->GetNumber(); }
  double MaxLam(void) const
    { return m_maxLam->GetNumber(); }
  double DelLam(void) const
    { return m_delLam->GetNumber(); }
  int Tol1D(void) const
    { return m_tol1->GetInteger(); }
  int TolND(void) const
    { return m_tolN->GetInteger(); }
  int Maxits1D(void) const
    { return m_maxits1->GetInteger(); }
  int MaxitsND(void) const
    { return m_maxitsN->GetInteger(); }

  int StartOption(void) const
    { return m_startOption->GetSelection(); }
};

#endif  // DLQRE_H
