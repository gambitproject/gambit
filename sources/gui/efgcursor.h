//
// FILE: efgcursor.h -- Extensive form cursor property window
//
// $Id$
//

#ifndef EFGCURSOR_H
#define EFGCURSOR_H

#include "wx/grid.h"
#include "efgshow.h"

class EfgCursorWindow : public wxGrid {
private:
  EfgShow *m_parent;
  const Node *m_cursor;
  static const int s_numFeatures;
  static char *s_featureNames[];

public:
  EfgCursorWindow(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgCursorWindow() { }

  void Set(const Node *p_cursor);
};

#endif  // EFGCURSOR_H
