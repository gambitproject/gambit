//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog to insert move into a tree

#ifndef DLINSERTMOVE_H
#define DLINSERTMOVE_H

#include "wx/spinctrl.h"

class dialogInsertMove : public wxDialog {
private:
  efgGame &m_efg;

  wxListBox *m_playerItem, *m_infosetItem;
  wxSpinCtrl *m_actions;

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);

public:
  // Constructor
  dialogInsertMove(wxWindow *, efgGame &);

  // Data access (only valid if ShowModal() returns wxID_OK.
  // If GetInfoset() returns null, user selected "new infoset"
  EFPlayer *GetPlayer(void) const;
  Infoset *GetInfoset(void) const;
  int GetActions(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLMOVEADD_H



