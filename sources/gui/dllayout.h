//
// FILE: dllayout.h -- Declaration of tree-layout parameter dialog
//
// $Id$
//

#ifndef DLLAYOUT_H
#define DLLAYOUT_H

#include "wx/spinctrl.h"

class dialogLayout : public guiAutoDialog {
private:
  wxSpinCtrl *m_nodeLength, *m_branchLength, *m_ySpacing;
  wxRadioBox *m_infosetStyle;

  const char *HelpString(void) const { return "Prefs Menu (efg)"; }

public:
  dialogLayout(wxWindow *p_parent,
	       int p_nodeLength, int p_branchLength,
	       int p_ySpacing, int p_infosetStyle);
  virtual ~dialogLayout() { }

  int NodeLength(void) const { return m_nodeLength->GetValue(); }
  int BranchLength(void) const { return m_branchLength->GetValue(); }
  int YSpacing(void) const { return m_ySpacing->GetValue(); }

  int InfosetStyle(void) const { return m_infosetStyle->GetSelection(); }
};

#endif  // DLLAYOUT_H
