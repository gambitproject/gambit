//
// FILE: dlactionprobs.h -- Dialog for Edit->Action->Probabilities
//
// $Id$
//

#ifndef DLACTIONPROBS_H
#define DLACTIONPROBS_H

class dialogActionProbs : public wxDialogBox {
private:
  Infoset *m_infoset;
  int m_completed, m_pageNumber;
  static int s_actionsPerDialog;
  wxNumberItem **m_probItems;
  gArray<gNumber> m_actionProbs;
  wxButton *m_okButton, *m_cancelButton, *m_backButton, *m_nextButton;

  static void CallbackNext(wxButton &p_object, wxEvent &)
    { ((dialogActionProbs *) p_object.GetClientData())->OnNext(); }
  static void CallbackBack(wxButton &p_object, wxEvent &)
    { ((dialogActionProbs *) p_object.GetClientData())->OnBack(); }
  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogActionProbs *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogActionProbs *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

  void OnNext(void);
  void OnBack(void);

public:
  dialogActionProbs(Infoset *, wxWindow *);
  virtual ~dialogActionProbs() { }

  int Completed(void) const { return m_completed; }
  const gNumber &GetActionProb(int p_action) const
    { return m_actionProbs[p_action]; }
};

#endif  // DLACTIONPROBS_H

