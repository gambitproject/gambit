//
// FILE: dlactionprobs.h -- Dialog for Edit->Action->Probabilities
//
//
//

#ifndef DLACTIONPROBS_H
#define DLACTIONPROBS_H

class dialogActionProbs : public wxDialogBox {
private:
  Infoset *m_infoset;
  int m_completed, m_lastSelection;
  gArray<gNumber> m_actionProbs;
  wxListBox *m_actionList;
  wxText *m_actionProb;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogActionProbs *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogActionProbs *) p_object.GetClientData())->OnCancel(); }
  static void CallbackAction(wxListBox &p_object, wxEvent &)
    { ((dialogActionProbs *) p_object.GetClientData(0))->OnAction(); }

  void OnOK(void);
  void OnCancel(void);
  void OnAction(void);
  Bool OnClose(void);

public:
  dialogActionProbs(Infoset *, wxWindow *);
  virtual ~dialogActionProbs() { }

  int Completed(void) const { return m_completed; }
  const gNumber &GetActionProb(int p_action) const
    { return m_actionProbs[p_action]; }
};

#endif  // DLACTIONPROBS_H

