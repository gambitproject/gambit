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
  wxChoice *m_outcome;

public:
  // Lifecycle
  dialogEditNode(wxWindow *p_parent, Node *p_node);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetNodeName(void) const { return m_nodeName->GetValue(); }
  int GetOutcome(void) const { return m_outcome->GetSelection(); }
};

#endif   // DLEDITNODE_H
