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

class PlayerPanel;

class GamePropertiesDialog final : public wxDialog {
  std::shared_ptr<GameDocument> m_doc;
  wxTextCtrl *m_title, *m_comment;
  PlayerPanel *m_playerPanel;
  wxStaticText *m_errorText;
  // Chance's colour, staged like everything on the Players page; unused if the game isn't a
  // tree. Chance itself is never added, deleted, or reordered, so it's kept out of
  // PlayerPanel entirely rather than as a fixed, non-editable row there.
  wxColour m_chanceColor;
  wxBitmapButton *m_chanceColorButton = nullptr;

  void UpdateValidation();
  void OnSetChanceColor();
  void RecreateChanceColorButton();
  wxBitmapButton *MakeChanceColorButton(wxWindow *p_parent);

public:
  // Lifecycle
  GamePropertiesDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetTitle() const override { return m_title->GetValue(); }
  wxString GetDescription() const { return m_comment->GetValue(); }

  // Chance's colour as edited on the Players page; meaningful only if the game is a tree.
  wxColour GetChanceColor() const { return m_chanceColor; }

  // The game's players (excluding chance) as edited on the Players page: adding, deleting,
  // reordering, relabeling, and recoloring are all staged only within the dialog and are not
  // applied to the document until ShowModal() returns wxID_OK and the caller commits them.
  // Row `p_index` may be a current player, kept or marked for deletion, or a brand-new one
  // added in this dialog; a deleted row should be excluded from the operation committed.
  int NumPlayerRows() const;
  bool IsPlayerDeleted(int p_index) const;
  // The label the player at position `p_index` had before this edit, or the placeholder
  // assigned if the row was newly added -- identifies the player for `Game::SetPlayers`,
  // independent of whatever's currently typed into its label field.
  std::string GetPlayerStableLabel(int p_index) const;
  wxString GetPlayerLabel(int p_index) const;
  wxColour GetPlayerColor(int p_index) const;
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLGAMEPROP_H
