//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dleditmove.h
// Dialog for viewing and editing properties of a move
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

#ifndef DLEDITMOVE_H
#define DLEDITMOVE_H

#include "wx/sheet/sheet.h"

class gbtEditMoveDialog : public wxDialog {
private:
  Gambit::GameInfoset m_infoset;
  wxChoice *m_player;
  wxTextCtrl *m_infosetName;
  wxSheet *m_actionSheet;

public:
  // Lifecycle
  gbtEditMoveDialog(wxWindow *p_parent, Gambit::GameInfoset p_infoset);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetInfosetName(void) const { return m_infosetName->GetValue(); }
  int GetPlayer(void) const { return (m_player->GetSelection() + 1); }

  int NumActions(void) const { return m_actionSheet->GetNumberRows(); }
  //  const Gambit::Array<gbtEfgAction *> &GetActions(void) const 
  //  { return m_infoset->Actions(); }
  wxString GetActionName(int p_act) const;
  wxString GetActionProb(int p_act) const; 
};

#endif   // DLEDITMOVE_H
