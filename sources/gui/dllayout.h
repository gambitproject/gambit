//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Declaration of dialog to set layout parameters
//

#ifndef DLLAYOUT_H
#define DLLAYOUT_H

#include "wx/notebook.h"

class dialogLayout : public wxDialog {
private:
  wxNotebook *m_notebook;

public:
  dialogLayout(wxWindow *p_parent, const TreeDrawSettings &);
  virtual ~dialogLayout() { }

  void GetSettings(TreeDrawSettings &);
};

#endif  // DLLAYOUT_H
