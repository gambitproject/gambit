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
  wxListBox *m_branchList;

public:
  dialogEfgDelete(wxWindow *, Node *);
  virtual ~dialogEfgDelete() { }

  Node *KeepNode(void) const
    { return m_node->GetChild(m_branchList->GetSelection() + 1); }
  bool DeleteTree(void) const { return false; }
};

#endif  // DLEFGDELETE_H
