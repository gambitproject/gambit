//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog to set tree layout parameters
//

#ifndef DLEFGLAYOUT_H
#define DLEFGLAYOUT_H

#include "wx/notebook.h"
#include "treedraw.h"

class dialogLayout : public wxDialog {
private:
  wxNotebook *m_notebook;

public:
  // Lifecycle
  dialogLayout(wxWindow *p_parent, const TreeDrawSettings &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  void GetSettings(TreeDrawSettings &);
};

#endif  // DLEFGLAYOUT_H

