//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to select colors for players
//

#ifndef DLEFGCOLOR_H
#define DLEFGCOLOR_H

class dialogEfgColor : public wxDialog {
private:
  wxButton *m_chanceButton, *m_terminalButton, *m_playerButton[8];

  // Event handlers
  void OnTerminalColor(wxCommandEvent &);
  void OnChanceColor(wxCommandEvent &);
  void OnPlayerColor(wxCommandEvent &);

public:
  dialogEfgColor(wxWindow *p_parent, const TreeDrawSettings &);

  wxColour GetTerminalColor(void) const
  { return m_terminalButton->GetBackgroundColour(); }
  wxColour GetChanceColor(void) const
  { return m_chanceButton->GetBackgroundColour(); }
  wxColour GetPlayerColor(int p_player) const
  { return m_playerButton[p_player-1]->GetBackgroundColour(); }

  DECLARE_EVENT_TABLE()
};

#endif  // DLEFGCOLOR_H
