//
// FILE: dlactionselect.h -- Select an action for an information set
//
// $Id$
//

#ifndef DLACTIONSELECT_H
#define DLACTIONSELECT_H

class dialogActionSelect : public wxDialogBox {
private:
  Infoset *m_infoset;
  int m_completed;
  wxListBox *m_actionList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogActionSelect *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogActionSelect *) p_object.GetClientData())->OnCancel(); }
  static void CallbackHelp(wxButton &p_object, wxEvent &)
    { ((dialogActionSelect *) p_object.GetClientData())->OnHelp(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);
  void OnHelp(void);

public:
  dialogActionSelect(Infoset *, wxWindow *);
  virtual ~dialogActionSelect() { }

  int Completed(void) const { return m_completed; }
  Action *GetAction(void) const 
    { return m_infoset->Actions()[m_actionList->GetSelection() + 1]; }
};

#endif  // DLACTIONSELECT_H
