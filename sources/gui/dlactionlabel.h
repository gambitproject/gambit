//
// FILE: dlactionlabel.h -- Dialog for Edit->Action->Label
//
// $Id$
//

#ifndef DLACTIONLABEL_H
#define DLACTIONLABEL_H

#include "wx/grid.h"

class dialogActionLabel : public guiPagedDialog {
private:
  Infoset *m_infoset;

  const char *HelpString(void) const { return "Action Menu"; }
  
public:
  dialogActionLabel(Infoset *, wxWindow *);
  virtual ~dialogActionLabel() { }

  gText GetActionLabel(int p_action) const
    { return GetValue(p_action); }
};

#endif  // DLACTIONLABEL_H
