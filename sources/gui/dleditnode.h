//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a node
//

#ifndef DLEDITNODE_H
#define DLEDITNODE_H

class dialogEditNode : public wxDialog {
private:
  Node *m_node;
  wxTextCtrl *m_nodeName;
  wxCheckBox *m_markedSubgame;
  wxChoice *m_outcome, *m_infoset;
  gBlock<Infoset *> m_infosetList;

public:
  // Lifecycle
  dialogEditNode(wxWindow *p_parent, Node *p_node);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetNodeName(void) const { return m_nodeName->GetValue(); }
  int GetOutcome(void) const { return m_outcome->GetSelection(); }
  Infoset *GetInfoset(void) const;
  bool MarkedSubgame(void) const { return m_markedSubgame->GetValue(); }
};

#endif   // DLEDITNODE_H
