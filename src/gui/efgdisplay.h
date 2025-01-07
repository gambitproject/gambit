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

#ifndef EFGDISPLAY_H
#define EFGDISPLAY_H

#include "gamedoc.h"
#include "efglayout.h"

class gbtPayoffEditor : public wxTextCtrl {
private:
  gbtNodeEntry *m_entry{nullptr};
  Gambit::GameOutcome m_outcome;
  int m_player{0};

  /// @name Event handlers
  //@{
  void OnChar(wxKeyEvent &);
  //@}

public:
  explicit gbtPayoffEditor(wxWindow *p_parent);

  void BeginEdit(gbtNodeEntry *p_node, int p_player);
  void EndEdit();

  bool IsEditing() const { return IsShown(); }

  gbtNodeEntry *GetNodeEntry() const { return m_entry; }
  Gambit::GameOutcome GetOutcome() const { return m_outcome; }
  int GetPlayer() const { return m_player; }

  DECLARE_EVENT_TABLE()
};

class gbtEfgDisplay : public wxScrolledWindow, public gbtGameView {
private:
  gbtTreeLayout m_layout;
  int m_zoom;
  wxMenu *m_nodeMenu{nullptr};
  gbtPayoffEditor *m_payoffEditor;

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

  /// @name Overriding gbtGameView members
  //@{
  void OnUpdate() override;
  void PostPendingChanges() override;
  //@}

  void RefreshTree();

public:
  gbtEfgDisplay(wxWindow *p_parent, gbtGameDocument *p_doc);

  void OnDraw(wxDC &dc) override;
  void OnDraw(wxDC &, double);

  int GetZoom() const { return m_zoom; }
  void SetZoom(int p_zoom);
  void FitZoom();

  const gbtTreeLayout &GetLayout() const { return m_layout; }

  void EnsureNodeVisible(const Gambit::GameNode &);

  DECLARE_EVENT_TABLE()
};

#endif // EFGDISPLAY_H
