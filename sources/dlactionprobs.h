//
// FILE: dlactionprobs.h -- Dialog for Edit->Action->Probabilities
//
// $Id$
//

#ifndef DLACTIONPROBS_H
#define DLACTIONPROBS_H

class dialogActionProbs : public guiPagedDialog {
private:
  Infoset *m_infoset;

  const char *HelpString(void) const { return "Action Menu"; }

public:
  dialogActionProbs(Infoset *, wxWindow *);
  virtual ~dialogActionProbs() { }

  gNumber GetActionProb(int p_action) const
    { return ToNumber(GetValue(p_action)); }
};

#endif  // DLACTIONPROBS_H

