//
// FILE: nfgsolutions.h -- Normal form solutions display
//
// $Id$
//

#ifndef NFGSOLUTIONS_H
#define NFGSOLUTIONS_H

#include "wx/listctrl.h"

class guiNfgSolutions : public wxListCtrl {
private:
  Nfg &m_nfg;
  guiNfgFrame *m_parent;
  gList<MixedSolution> m_solutions;

  void OnSelected(wxListEvent &);

public:
  guiNfgSolutions(guiNfgFrame *m_parent, wxWindow *p_window, Nfg &p_nfg);
  virtual ~guiNfgSolutions() { }

  //  void OnChangeLabels(void);
  //void OnSelectCell(int, int);
  void AddSolutions(const gList<MixedSolution> &);

  //int CurrentSolution(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // NFGSOLUTIONS_H
