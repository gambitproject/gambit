//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlefglegend.h
// Dialog for setting legends for extensive form display
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

#ifndef GAMBIT_GUI_DLEFGLEGEND_H
#define GAMBIT_GUI_DLEFGLEGEND_H

#include "style.h"

namespace Gambit::GUI {
class LegendDialog final : public wxDialog {
  wxChoice *m_nodeAbove, *m_nodeBelow, *m_actionAbove, *m_actionBelow;

public:
  // Lifecycle
  LegendDialog(wxWindow *, const TreeRenderConfig &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  NodeLabelStyle GetNodeAbove() const
  {
    return static_cast<NodeLabelStyle>(m_nodeAbove->GetSelection());
  }
  NodeLabelStyle GetNodeBelow() const
  {
    return static_cast<NodeLabelStyle>(m_nodeBelow->GetSelection());
  }

  BranchLabelStyle GetBranchAbove() const
  {
    return static_cast<BranchLabelStyle>(m_actionAbove->GetSelection());
  }
  BranchLabelStyle GetBranchBelow() const
  {
    return static_cast<BranchLabelStyle>(m_actionBelow->GetSelection());
  }
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLEFGLEGEND_H
