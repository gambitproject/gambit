//
// FILE: dlnodedelete.h -- Dialog for Edit->Node->Delete
//
// $Id$
//

#ifndef DLNODEDELETE_H
#define DLNODEDELETE_H

class dialogNodeDelete : public guiAutoDialog {
private:
  Node *m_node;
  wxListBox *m_branchList;

  const char *HelpString(void) const  { return "Node Menu"; }

public:
  dialogNodeDelete(Node *, wxWindow *);
  virtual ~dialogNodeDelete() { }

  Node *KeepNode(void) const
    { return m_node->GetChild(m_branchList->GetSelection() + 1); }
};

#endif  // DLNODEDELETE_H
