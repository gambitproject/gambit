//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of an extensive form game
//

#ifndef DLEFGPROPERTIES_H
#define DLEFGPROPERTIES_H

#include "wx/notebook.h"

class dialogEfgProperties : public wxDialog {
private:
  FullEfg &m_efg;
  wxNotebook *m_notebook;

  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogEfgProperties(wxWindow *p_parent, FullEfg &p_efg, const wxString &);

  // Data access (only valid when ShowModal() returns with wxID_OK
  wxString GetGameTitle(void) const;
  wxString GetComment(void) const;

  int NumPlayers(void) const;
  wxString GetPlayerName(int pl) const;

  DECLARE_EVENT_TABLE()
};


#endif  // DLEFGPROPERTIES_H

