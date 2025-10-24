//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgdisplay.h
// Declaration of window class to display extensive form tree
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

#ifndef GAMBIT_GUI_EFGDISPLAY_H
#define GAMBIT_GUI_EFGDISPLAY_H

#include "gamedoc.h"
#include "efglayout.h"

namespace Gambit::GUI {
class TreePayoffEditor final : public wxTextCtrl {
  std::shared_ptr<NodeEntry> m_entry{nullptr};
  GameOutcome m_outcome;
  int m_player{0};

  /// @name Event handlers
  //@{
  void OnChar(wxKeyEvent &);
  //@}

public:
  explicit TreePayoffEditor(wxWindow *p_parent);

  void BeginEdit(const std::shared_ptr<NodeEntry> &p_node, int p_player);
  void EndEdit();

  bool IsEditing() const { return IsShown(); }

  std::shared_ptr<NodeEntry> GetNodeEntry() const { return m_entry; }
  GameOutcome GetOutcome() const { return m_outcome; }
  int GetPlayer() const { return m_player; }

  DECLARE_EVENT_TABLE()
};

class EfgDisplay final : public wxScrolledWindow, public GameView {
  TreeLayout m_layout;
  int m_zoom;
  wxMenu *m_nodeMenu{nullptr};
  TreePayoffEditor *m_payoffEditor;

  // Private Functions
  void MakeMenus();
  void AdjustScrollbarSteps();

  /// @name Event handlers
  //@{
  void OnMouseMotion(wxMouseEvent &);
  void OnLeftClick(wxMouseEvent &);
  void OnRightClick(wxMouseEvent &);
  void OnLeftDoubleClick(wxMouseEvent &);
  void OnKeyEvent(wxKeyEvent &);
  /// Payoff editor changes accepted with enter
  void OnAcceptPayoffEdit(wxCommandEvent &);
  //@}

  /// @name Overriding GameView members
  //@{
  void OnUpdate() override;
  void PostPendingChanges() override;
  //@}

  void RefreshTree();

public:
  EfgDisplay(wxWindow *p_parent, GameDocument *p_doc);

  void OnDraw(wxDC &dc) override;
  void OnDraw(wxDC &, double);

  int GetZoom() const { return m_zoom; }
  void SetZoom(int p_zoom);
  void FitZoom();

  const TreeLayout &GetLayout() const { return m_layout; }

  void EnsureNodeVisible(const GameNode &);

  DECLARE_EVENT_TABLE()
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_EFGDISPLAY_H
