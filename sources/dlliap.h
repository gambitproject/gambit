//
// FILE: dlliap.h -- Parameter selection for LiapSolve
//
// $Id$
//

#ifndef DLLIAP_H
#define DLLIAP_H

#include "algdlgs.h"

class dialogLiap : public dialogAlgorithm {
private:
  wxIntegerItem *m_tol1D, *m_tolND;
  wxIntegerItem *m_maxits1D, *m_maxitsND, *m_nTries;
  wxRadioBox *m_startOption;

  gText HelpTopic(void) const { return "Liap"; }

  void AlgorithmFields(void);

public:
  dialogLiap(wxWindow *p_parent, bool p_subgames = false,
			bool p_vianfg = false);
  virtual ~dialogLiap();

  int StopAfter(void) const;

  int Tol1D(void) const { return m_tol1D->GetInteger(); }
  int TolND(void) const { return m_tolND->GetInteger(); }
  int Maxits1D(void) const { return m_maxits1D->GetInteger(); }
  int MaxitsND(void) const { return m_maxitsND->GetInteger(); }
  int NumTries(void) const  { return m_nTries->GetInteger(); }

  int StartOption(void) const { return m_startOption->GetSelection(); }
};

#endif  // DLLIAP_H
