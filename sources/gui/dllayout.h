//
// FILE: dllayout.h -- Declaration of tree-layout parameter dialog
//
// $Id$
//

#ifndef DLLAYOUT_H
#define DLLAYOUT_H

class dialogLayout : public guiAutoDialog {
private:
  wxSlider *m_branchLength, *m_nodeLength, *m_forkLength, *m_ySpacing;
  wxRadioBox *m_infosetStyle;

  const char *HelpString(void) const { return "Prefs Menu (efg)"; }

public:
  dialogLayout(wxWindow *p_parent,
	       int p_branchLength, int p_nodeLength, int p_forkLength,
	       int p_ySpacing, int p_infosetStyle);
  virtual ~dialogLayout() { }

  int BranchLength(void) const { return m_branchLength->GetValue(); }
  int NodeLength(void) const { return m_nodeLength->GetValue(); }
  int ForkLength(void) const { return m_forkLength->GetValue(); }
  int YSpacing(void) const { return m_ySpacing->GetValue(); }

  int InfosetStyle(void) const { return m_infosetStyle->GetSelection(); }
};

#endif  // DLLAYOUT_H
