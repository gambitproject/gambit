//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a node
//

#ifndef DLNODEPROPERTIES_H
#define DLNODEPROPERTIES_H

#include "wx/notebook.h"

class dialogNodeProperties : public wxDialog {
private:
  Node *m_node;
  wxNotebook *m_notebook;

public:
  // Lifecycle
  dialogNodeProperties(wxWindow *p_parent, Node *p_node);

  // Data access (only valid when ShowModal() returns with wxID_OK
  wxString GetNodeName(void) const;
  int GetOutcome(void) const;

  wxString GetInfosetName(void) const;
  gText GetActionName(int) const;
};

#endif   // DLNODEPROPERTIES_H
