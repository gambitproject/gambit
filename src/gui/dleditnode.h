//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dleditnode.h
// Dialog for viewing and editing properties of a node
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

#ifndef GAMBIT_GUI_DLEDITNODE_H
#define GAMBIT_GUI_DLEDITNODE_H

namespace Gambit::GUI {
class EditNodeDialog final : public wxDialog {
  GameNode m_node;
  wxTextCtrl *m_nodeName;
  wxChoice *m_outcome, *m_infoset;
  Array<GameInfoset> m_infosetList;

public:
  // Lifecycle
  EditNodeDialog(wxWindow *p_parent, const GameNode &p_node);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  wxString GetNodeName() const { return m_nodeName->GetValue(); }
  int GetOutcome() const { return m_outcome->GetSelection(); }
  GameInfoset GetInfoset() const;
};
} // namespace Gambit::GUI
#endif // GAMBIT_GUI_DLEDITNODE_H
