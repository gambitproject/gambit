//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a normal form game
//

#ifndef DLNFGPROPERTIES_H
#define DLNFGPROPERTIES_H

#include "wx/notebook.h"

class dialogNfgProperties : public wxDialog {
private:
  Nfg &m_nfg;
  wxNotebook *m_notebook;

  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  dialogNfgProperties(wxWindow *p_parent, Nfg &p_nfg, const wxString &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetGameTitle(void) const;
  wxString GetComment(void) const;

  int NumPlayers(void) const;
  wxString GetPlayerName(int pl) const;

  DECLARE_EVENT_TABLE()
};


#endif  // DLEFGPROPERTIES_H

