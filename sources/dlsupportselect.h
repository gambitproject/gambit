//
// FILE: dlsupportselect.h -- Dialog for selecting supports
//
// $Id$
//

#ifndef DLSUPPORTSELECT_H
#define DLSUPPORTSELECT_H

class dialogSupportSelect : public guiAutoDialog {
private:
  wxListBox *m_supportList;

  const char *HelpString(void) const { return "Supports"; }

public:
  dialogSupportSelect(const gList<NFSupport *> &p_supports, 
		      NFSupport *p_current, const gText &p_caption,
		      wxWindow *p_parent = 0);
  dialogSupportSelect(const gList<EFSupport *> &p_supports, 
		      EFSupport *p_current, const gText &p_caption,
		      wxWindow *p_parent = 0);
  virtual ~dialogSupportSelect() { }

  int Selected(void) const { return (m_supportList->GetSelection() + 1); }
};

#endif  // DLNFGSUPPORTSELECT_H

