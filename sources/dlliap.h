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
  wxIntegerItem *m_accuracy, *m_nTries;
  wxRadioBox *m_startOption;

  const char *HelpString(void) const { return "Liap"; }

  void AlgorithmFields(void);

public:
  dialogLiap(wxWindow *p_parent, bool p_subgames = false,
	     bool p_vianfg = false);
  virtual ~dialogLiap();

  int StopAfter(void) const;

  int Accuracy(void) const { return m_accuracy->GetInteger(); }
  int NumTries(void) const  { return m_nTries->GetInteger(); }

  int StartOption(void) const { return m_startOption->GetSelection(); }
};

#endif  // DLLIAP_H
