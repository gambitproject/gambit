//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Extensive form behavior profile window
//

#ifndef EFGPROFILE_H
#define EFGPROFILE_H

#include "wx/listctrl.h"
#include "efgshow.h"

class EfgProfileList : public wxListCtrl, public gList<BehavSolution> {
private:
  EfgShow *m_parent;
  wxMenu *m_menu;

  // Event handlers
  void OnSortFilter(wxCommandEvent &);
  void OnRightClick(wxMouseEvent &);
  void OnColumnClick(wxListEvent &);
  
public:
  EfgProfileList(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgProfileList();

  void UpdateValues(void);

  // Overriding gSortList<T> member functions
  virtual int Append(const BehavSolution &);  

  DECLARE_EVENT_TABLE()
};

#endif  // EFGPROFILE_H
