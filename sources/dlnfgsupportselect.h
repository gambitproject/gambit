//
// FILE: dlnfgsupportselect.h -- Dialog for selecting NFSupport
//
// $Id$
//

#ifndef DLNFGSUPPORTSELECT_H
#define DLNFGSUPPORTSELECT_H

class dialogNfgSupportSelect : public guiAutoDialog {
private:
  wxListBox *m_supportList;

  const char *HelpString(void) const { return ""; }

public:
  dialogNfgSupportSelect(const gList<NFSupport *> &p_supports, 
			 NFSupport *p_current, const gText &p_caption,
			 wxWindow *p_parent = 0);
  virtual ~dialogNfgSupportSelect() { }

  int Selected(void) const { return (m_supportList->GetSelection() + 1); }
};

#endif  // DLNFGSUPPORTSELECT_H

