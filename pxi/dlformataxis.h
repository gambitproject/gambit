//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog for setting axis formatting
//

#ifndef DLFORMATAXIS_H
#define DLFORMATAXIS_H

#include "pxicanvas.h"    // for PxiAxisProperties

class dialogFormatAxis : public wxDialog {
private:
  wxNotebook *m_notebook;
  class ScalePanel *m_scalePanel;
  class FontPanel *m_fontPanel;
  class ColorPanel *m_colorPanel;

  // Event handlers (mostly for validation)
  void OnOK(wxCommandEvent &);
  void OnNotebookPageChanging(wxNotebookEvent &);

public:
  dialogFormatAxis(wxWindow *p_parent, const PxiAxisProperties &);

  PxiAxisProperties GetProperties(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLFORMATAXIS_H
