//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to select colors for players
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
  dialogEfgColor(wxWindow *p_parent, const gbtPreferences &);

  wxColour GetTerminalColor(void) const
  { return m_terminalButton->GetBackgroundColour(); }
  wxColour GetChanceColor(void) const
  { return m_chanceButton->GetBackgroundColour(); }
  wxColour GetPlayerColor(int p_player) const
  { return m_playerButton[p_player-1]->GetBackgroundColour(); }

  DECLARE_EVENT_TABLE()
};

#endif  // DLEFGCOLOR_H
