//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Normal form mixed profile window
//

#ifndef NFGPROFILE_H
#define NFGPROFILE_H

#include "wx/listctrl.h"
#include "nfgshow.h"

class NfgProfileList : public wxListCtrl {
private:
  NfgShow *m_parent;
  wxMenu *m_menu;

  // Event handlers
  void OnRightClick(wxMouseEvent &);
  void OnColumnClick(wxListEvent &);

public:
  NfgProfileList(NfgShow *p_nfgShow, wxWindow *p_parent);
  virtual ~NfgProfileList();

  void UpdateValues(void);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGPROFILE_H
