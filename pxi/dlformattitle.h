//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog for setting title formatting
//

#ifndef DLFORMATTITLE_H
#define DLFORMATTITLE_H

#include "pxicanvas.h"   // for PxiTitleProperties

class dialogFormatTitle : public wxDialog {
private:
  wxTextCtrl *m_title;
  wxButton *m_fontButton, *m_colorButton;

  void OnFontButton(wxCommandEvent &);
  void OnColorButton(wxCommandEvent &);

public:
  dialogFormatTitle(wxWindow *p_parent, const PxiTitleProperties &);

  PxiTitleProperties GetProperties(void) const;

  DECLARE_EVENT_TABLE()
};


#endif  // DLFORMATTITLE_H
