//
// FILE: dialogsupportselect.h -- Dialog for selecting supports
//
// $Id$
//

#ifndef DIALOGSUPPORTSELECT_H
#define DIALOGSUPPORTSELECT_H

class dialogSupportSelect : public guiAutoDialog {
private:
  wxListBox *m_supportList;

  const char *HelpString(void) const { return ""; }

public:
  dialogSupportSelect(wxWindow *p_parent, 
		      const gList<NFSupport *> &p_supports, 
		      int p_current, const gText &p_caption);
  /*
  dialogSupportSelect(wxWindow *p_parent,
		      const gList<EFSupport *> &p_supports, 
		      int p_current, const gText &p_caption);
  */
  virtual ~dialogSupportSelect() { }

  int Selected(void) const { return (m_supportList->GetSelection() + 1); }
};

#endif  // DIALOGSUPPORTSELECT_H

