//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for computing quantal response equilibria
//

#ifndef DLNFGQRE_H
#define DLNFGQRE_H

class dialogNfgQre : public wxDialog {
private:


public:
  // Lifecycle
  dialogNfgQre(wxWindow *, const NFSupport &);

  // Data access (only valid when ShowModal() returns with wxID_OK)

  DECLARE_EVENT_TABLE()
};


#endif  // DLNFGQRE_H
