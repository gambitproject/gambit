//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog for setting legend formatting
//

#ifndef DLFORMATLEGEND_H
#define DLFORMATLEGEND_H

#include "pxiplot.h"   // for PxiTitleProperties

class dialogFormatLegend : public wxDialog {
private:
  wxCheckBox *m_showLegend;
  wxButton *m_fontButton, *m_colorButton;

  void OnFontButton(wxCommandEvent &);
  void OnColorButton(wxCommandEvent &);

public:
  dialogFormatLegend(wxWindow *p_parent, const PxiLegendProperties &);

  PxiLegendProperties GetProperties(void) const;

  DECLARE_EVENT_TABLE()
};


#endif  // DLFORMATTITLE_H
