//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Declaration of normal form outcome palette window
//

#ifndef NFGOUTCOME_H
#define NFGOUTCOME_H

#include "wx/grid.h"
#include "nfgshow.h"

class NfgOutcomeWindow : public wxGrid {
private:
  NfgShow *m_parent;
  wxMenu *m_menu;

  void OnCellChanged(wxGridEvent &);
  void OnCellRightClick(wxGridEvent &);
  void OnLabelRightClick(wxGridEvent &);

  void OnPopupOutcomeNew(wxCommandEvent &);
  void OnPopupOutcomeDelete(wxCommandEvent &);
  void OnPopupOutcomeAttach(wxCommandEvent &);
  void OnPopupOutcomeDetach(wxCommandEvent &);

public:
  NfgOutcomeWindow(NfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~NfgOutcomeWindow() { }

  void UpdateValues(void);

  DECLARE_EVENT_TABLE()
};


#endif  // NFGOUTCOME_H
