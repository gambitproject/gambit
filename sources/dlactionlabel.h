//
// FILE: dlactionlabel.h -- Dialog for Edit->Action->Label
//
// $Id$
//

#ifndef DLACTIONLABEL_H
#define DLACTIONLABEL_H

class dialogActionLabel : public wxDialogBox {
private:
  Infoset *m_infoset;
  int m_completed, m_pageNumber;
  static int s_actionsPerDialog;
  wxText **m_actionLabels;
  wxButton *m_okButton, *m_cancelButton, *m_helpButton, *m_backButton, *m_nextButton;
  gArray<gText> m_actionNames;

  static void CallbackNext(wxButton &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData())->OnNext(); }
  static void CallbackBack(wxButton &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData())->OnBack(); }

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData())->OnCancel(); }
  static void CallbackHelp(wxButton &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData())->OnHelp(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);
  void OnHelp(void);

  void OnNext(void);
  void OnBack(void);

public:
  dialogActionLabel(Infoset *, wxWindow *);
  virtual ~dialogActionLabel() { }

  int Completed(void) const { return m_completed; }
  const gText &GetActionLabel(int p_action) const
    { return m_actionNames[p_action]; }
};

#endif  // DLACTIONLABEL_H
