//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for deleting moves/subtrees from extensive form
//

#ifndef DLEFGDELETE_H
#define DLEFGDELETE_H

class dialogEfgDelete : public wxDialog {
private:
  Node *m_node;
  wxRadioBox *m_deleteTree;
  wxListBox *m_branchList;

  void OnDeleteTree(wxCommandEvent &);

public:
  dialogEfgDelete(wxWindow *, Node *);
  virtual ~dialogEfgDelete() { }

  Node *KeepNode(void) const
    { return m_node->GetChild(m_branchList->GetSelection() + 1); }
  bool DeleteTree(void) const 
    { return (m_deleteTree->GetSelection() == 0); }

  DECLARE_EVENT_TABLE()
};

#endif  // DLEFGDELETE_H
