//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlgameprop.h
// Dialog for viewing and editing properties of a game
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

#ifndef GAMBIT_GUI_DLGAMEPROP_H
#define GAMBIT_GUI_DLGAMEPROP_H

namespace Gambit::GUI {

class PlayerLabelPanel;

class GamePropertiesDialog final : public wxDialog {
  GameDocument *m_doc;
  wxTextCtrl *m_title, *m_comment;
  PlayerLabelPanel *m_playerPanel;
  wxStaticText *m_errorText;

  void UpdateValidation();

public:
  // Lifecycle
  GamePropertiesDialog(wxWindow *p_parent, GameDocument *p_doc);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetTitle() const override { return m_title->GetValue(); }
  wxString GetDescription() const { return m_comment->GetValue(); }

  // Player (and chance) labels and colors, as edited on the Players page. Row 0
  // is chance; the rest are the game's personal players. Both labels and
  // colors are staged only within the dialog and are not applied to the
  // document until ShowModal() returns wxID_OK and the caller commits them.
  int NumRows() const;
  GamePlayer GetPlayer(int p_index) const;
  wxString GetPlayerLabel(int p_index) const;
  wxColour GetPlayerColor(int p_index) const;
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLGAMEPROP_H
