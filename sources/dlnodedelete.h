//
// FILE: dlnodedelete.h -- Dialog for Edit->Node->Delete
//
//
//

#ifndef DLNODEDELETE_H
#define DLNODEDELETE_H

class dialogNodeDelete : public wxDialogBox {
private:
  Node *m_node;
  int m_completed;
  wxListBox *m_branchList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogNodeDelete *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogNodeDelete *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogNodeDelete(Node *, wxWindow *);
  virtual ~dialogNodeDelete() { }

  int Completed(void) const { return m_completed; }
  Node *KeepNode(void) const
    { return m_node->GetChild(m_branchList->GetSelection() + 1); }
};

#endif  // DLNODEDELETE_H
