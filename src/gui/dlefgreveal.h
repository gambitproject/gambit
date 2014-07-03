//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlefgreveal.h
// Dialog for revealing actions to players
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

#ifndef DLEFGREVEAL_H
#define DLEFGREVEAL_H

#include "gamedoc.h"

class gbtRevealMoveDialog : public wxDialog {
private:
  gbtGameDocument *m_doc;
  Gambit::Array<wxCheckBox *> m_players;

public:
  // Lifecycle
  gbtRevealMoveDialog(wxWindow *, gbtGameDocument *);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  Gambit::Array<Gambit::GamePlayer> GetPlayers(void) const;
};

#endif  // DLEFGREVEAL_H
