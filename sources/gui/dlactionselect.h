//
// FILE: dlactionselect.h -- Select an action for an information set
//
// $Id$
//

#ifndef DLACTIONSELECT_H
#define DLACTIONSELECT_H

class dialogActionSelect : public guiAutoDialog {
private:
  Infoset *m_infoset;
  wxListBox *m_actionList;

  const char *HelpString(void) const { return "Edit Menu (efg)"; }

public:
  dialogActionSelect(Infoset *, const gText &, const gText &, wxWindow *);
  virtual ~dialogActionSelect() { }

  Action *GetAction(void) const 
    { return m_infoset->Actions()[m_actionList->GetSelection() + 1]; }
};

#endif  // DLACTIONSELECT_H
