//
// FILE: efgprofile.h -- Declaration of extensive form profile list
//
// $Id$
//

#ifndef EFGPROFILE_H
#define EFGPROFILE_H

#include "wx/grid.h"
#include "efgshow.h"
#include "behavfilter.h"

class EfgProfileList : public wxGrid, public gList<BehavSolution> {
private:
  EfgShow *m_parent;
  wxMenu *m_menu;
  gBlock<int> m_displayOrder;

protected:
  BehavListFilter m_options;

  // Private members
  void Resort(void);

  // Event handlers
  void OnSortFilter(wxCommandEvent &);
  void OnSelectCell(wxGridEvent &);
  void OnLeftClick(wxGridEvent &);
  void OnRightClick(wxGridEvent &);
  void OnColumnClick(wxGridEvent &);
  void OnColumnDoubleClick(wxGridEvent &);
  
  // Overriding this to turn off cell highlight
  virtual void DrawCellHighlight(wxDC &, const wxGridCellAttr *) { }

public:
  EfgProfileList(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgProfileList();

  void UpdateValues(void);

  // Overriding gSortList<T> member functions
  virtual int Append(const BehavSolution &);  

  DECLARE_EVENT_TABLE()
};

#endif  // EFGPROFILE_H
