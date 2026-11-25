//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

#ifndef GAMBIT_GUI_DLEDITMOVE_H
#define GAMBIT_GUI_DLEDITMOVE_H

#include <wx/bmpcbox.h>

#include "editlabel.h"

namespace Gambit::GUI {
class ActionPanel;

class EditMoveDialog final : public wxDialog {
  std::shared_ptr<GameDocument> m_doc;
  GameInfoset m_infoset;
  wxPanel *m_headerPanel;
  wxBitmapComboBox *m_player; // shows each player's colour swatch alongside their name
  LabelTextCtrl *m_infosetLabel;
  wxColour m_infosetLabelDefaultBg;
  ActionPanel *m_actionPanel;
  wxStaticText *m_errorText;

  void UpdateValidation();
  void OnOK(wxCommandEvent &);

public:
  // Lifecycle
  EditMoveDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                 const GameInfoset &p_infoset);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  // The information set label, player, and actions (added, deleted, reordered, relabelled,
  // and their probabilities at a chance move) as edited
  EditMove GetEdit() const;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLEDITMOVE_H
