//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Declaration of extensive form outcome palette window
//

#ifndef EFGOUTCOME_H
#define EFGOUTCOME_H

#include "wx/grid.h"
#include "efgshow.h"

class EfgOutcomeWindow : public wxGrid {
private:
  EfgShow *m_parent;
  wxMenu *m_menu;

  void OnCellChanged(wxGridEvent &);
  void OnCellRightClick(wxGridEvent &);
  void OnLabelRightClick(wxGridEvent &);

  void OnPopupOutcomeNew(wxCommandEvent &);
  void OnPopupOutcomeDelete(wxCommandEvent &);
  void OnPopupOutcomeAttach(wxCommandEvent &);
  void OnPopupOutcomeDetach(wxCommandEvent &);

public:
  EfgOutcomeWindow(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~EfgOutcomeWindow() { }

  void UpdateValues(void);

  DECLARE_EVENT_TABLE()
};

#endif   // EFGOUTCOME_H
