//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Interface to navigation window for extensive forms
// 

#ifndef EFGNAVIGATE_H
#define EFGNAVIGATE_H

#include "wx/grid.h"
#include "efgshow.h"

class EfgNavigateWindow : public wxGrid {
private:
  EfgShow *m_parent;
  const Node *m_cursor;

  void OnEditorShown(wxGridEvent &);

public:
  EfgNavigateWindow(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgNavigateWindow() { }

  void Set(const Node *p_cursor);
};

#endif  // EFGNAVIGATE_H
