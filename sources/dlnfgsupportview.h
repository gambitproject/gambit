//
// FILE: dlnfgsupportview.h -- Dialog for selecting NFSupport
//
// $Id$
//

#ifndef DLNFGSUPPORTVIEW_H
#define DLNFGSUPPORTVIEW_H

class dialogNfgSupportView : public guiAutoDialog {
private:
  wxListBox *m_supportList;

  const char *HelpString(void) const { return ""; }

public:
  dialogNfgSupportView(const gList<NFSupport *> &p_supports, 
		       NFSupport *p_current,
		       wxWindow *p_parent = 0);
  virtual ~dialogNfgSupportView() { }

  int Selected(void) const { return (m_supportList->GetSelection() + 1); }
};

#endif  // DLNFGSUPPORTVIEW_H

