//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for revealing actions to players
//

#ifndef DLEFGREVEAL_H
#define DLEFGREVEAL_H

class dialogInfosetReveal : public wxDialog {
private:
  const efgGame &m_efg;
  wxCheckBox **m_players;

public:
  // Lifecycle
  dialogInfosetReveal(wxWindow *, const efgGame &);
  virtual ~dialogInfosetReveal();

  // Data access (only valid when ShowModal() returns with wxID_OK)
  gArray<EFPlayer *> GetPlayers(void) const;
};

#endif  // DLEFGREVEAL_H
