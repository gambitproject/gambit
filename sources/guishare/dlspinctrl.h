//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Generic dialog featuring just a wxSpinCtrl
//

#ifndef DLSPINCTRL_H
#define DLSPINCTRL_H

#include "wx/spinctrl.h"

class dialogSpinCtrl : public wxDialog {
private:
  wxSpinCtrl *m_spin;

public:
  dialogSpinCtrl(wxWindow *p_parent, const wxString &p_caption,
		 int p_min, int p_max, int p_default);

  int GetValue(void) const { return m_spin->GetValue(); }
};

#endif // DLSPINCTRL_H








