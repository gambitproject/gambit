//
// FILE: efgprofile.h -- Declaration of extensive form profile list
//
// $Id$
//

#ifndef EFGPROFILE_H
#define EFGPROFILE_H

#include "wx/listctrl.h"
#include "efgshow.h"

class EfgProfileList : public wxListCtrl, public gSortList<BehavSolution> {
private:
  EfgShow *m_parent;
  wxMenu *m_menu;

protected:
  BSolnSortFilterOptions m_options;
  //  virtual void  SortFilter(bool inter = true);

  // Event handlers
  void OnRightClick(wxMouseEvent &);

public:
  EfgProfileList(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgProfileList();

  void UpdateValues(void);

  // Overriding gSortList<T> member functions
  virtual int Append(const BehavSolution &);  

  DECLARE_EVENT_TABLE()
};

#endif  // EFGPROFILE_H
