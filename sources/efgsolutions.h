//
// FILE: efgsolutions.h -- Extensive form solutions display
//
// $Id$
//

#ifndef EFGSOLUTIONS_H
#define EFGSOLUTIONS_H

#include "wx/listctrl.h"

class guiEfgSolutions : public wxListCtrl {
private:
  Efg &m_efg;
  guiEfgFrame *m_parent;
  gList<BehavSolution> m_solutions;

  void OnSelected(wxListEvent &);

public:
  guiEfgSolutions(guiEfgFrame *m_parent, wxWindow *p_window, Efg &p_efg);
  virtual ~guiEfgSolutions() { }

  //  void OnChangeLabels(void);
  //void OnSelectCell(int, int);
  void AddSolutions(const gList<BehavSolution> &);

  //int CurrentSolution(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // EFGSOLUTIONS_H
