//
// FILE: dlactionlabel.h -- Dialog for Edit->Action->Label
//
//
//

#ifndef DLACTIONLABEL_H
#define DLACTIONLABEL_H

class dialogActionLabel : public wxDialogBox {
private:
  Infoset *m_infoset;
  int m_completed, m_lastSelection;
  gArray<gText> m_actionNames;
  wxListBox *m_actionList;
  wxText *m_actionName;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData())->OnCancel(); }
  static void CallbackAction(wxListBox &p_object, wxEvent &)
    { ((dialogActionLabel *) p_object.GetClientData(0))->OnAction(); }

  void OnOK(void);
  void OnCancel(void);
  void OnAction(void);
  Bool OnClose(void);

public:
  dialogActionLabel(Infoset *, wxWindow *);
  virtual ~dialogActionLabel() { }

  int Completed(void) const { return m_completed; }
  const gText &GetActionLabel(int p_action) const
    { return m_actionNames[p_action]; }
};

#endif  // DLACTIONLABEL_H
