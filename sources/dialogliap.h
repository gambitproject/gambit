//
// FILE: dialogliap.h -- Parameter selection for LiapSolve
//
// $Id$
//

#ifndef DIALOGLIAP_H
#define DIALOGLIAP_H

#include "dialogalgorithm.h"

class dialogLiap : public dialogAlgorithm {
private:
  wxTextCtrl *m_accuracy, *m_nTries;
  wxRadioBox *m_startOption;

  gText HelpTopic(void) const { return "Liap"; }

  void AlgorithmFields(void);

public:
  dialogLiap(wxWindow *p_parent, bool p_subgames, bool p_vianfg);
  virtual ~dialogLiap();

  int StopAfter(void) const;

  int Accuracy(void) const { return atoi(m_accuracy->GetValue()); }
  int NumTries(void) const { return atoi(m_nTries->GetValue()); }

  int StartOption(void) const { return m_startOption->GetSelection(); }
};

#endif  // DIALOGLIAP_H
