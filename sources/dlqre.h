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
  wxIntegerItem *m_tol1D, *m_tolND, *m_maxits1D, *m_maxitsND;
  wxRadioBox *m_startOption;

  gText HelpTopic(void) const  { return "Qre"; }
  
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
    { return m_tol1D->GetInteger(); }
  int TolND(void) const
    { return m_tolND->GetInteger(); }
  int Maxits1D(void) const
    { return m_maxits1D->GetInteger(); }
  int MaxitsND(void) const
    { return m_maxitsND->GetInteger(); }

  int StartOption(void) const
    { return m_startOption->GetSelection(); }
};

#endif  // DLQRE_H
