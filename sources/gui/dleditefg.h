//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of an extensive form game
//

#ifndef DLEDITEFG_H
#define DLEDITEFG_H

#include "wx/notebook.h"

class dialogEditEfg : public wxDialog {
private:
  FullEfg &m_efg;
  wxNotebook *m_notebook;

  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogEditEfg(wxWindow *p_parent, FullEfg &p_efg, const wxString &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetGameTitle(void) const;
  wxString GetComment(void) const;

  int NumPlayers(void) const;
  wxString GetPlayerName(int pl) const;

  DECLARE_EVENT_TABLE()
};


#endif  // DLEDITEFG_H

