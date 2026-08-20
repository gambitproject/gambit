//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

#include <vector>

#include "gamedoc.h"
#include "efglayout.h"

namespace Gambit::GUI {

class OutcomeEditorPopup;
class NodeInfoPopup;

// Number of IDs reserved (starting at GBT_MENU_EDIT_SET_PLAYER_BASE) for
// entries in the node menu's "set player" submenu; one per player, plus chance.
constexpr int gbtSetPlayerMenuCount = 64;

class EfgDisplay final : public wxScrolledWindow, public GameView {
  TreeLayout m_layout;
  int m_zoom;
  wxMenu *m_nodeMenu{nullptr};
  // The node the right-click (or double-click) node menu was last shown for -- the target
  // of whichever of that menu's commands the user picks. Purely local to the tree display;
  // there is no persistent "selected node" elsewhere (see UpdateNodeMenu()).
  GameNode m_contextNode;
  wxMenu *m_setPlayerMenu{nullptr};
  wxMenuItem *m_setPlayerItem{nullptr};
  std::vector<GamePlayer> m_setPlayerList;
  OutcomeEditorPopup *m_outcomeEditor{nullptr};
  NodeInfoPopup *m_nodeInfoPopup{nullptr};
  wxTimer m_hoverTimer;
  GameNode m_hoverNode;
  bool m_pendingInitialZoom{true};

  // Set for the duration of a node move/copy drag (see OnMouseMotion) to the node being
  // dragged; used only for live drag-over feedback, not for the drop itself.
  GameNode m_draggedNode;
  // Likewise, set for the duration of an outcome drag to the node whose outcome is being
  // dragged. At most one of m_draggedNode/m_draggedOutcomeNode is set at a time.
  GameNode m_draggedOutcomeNode;
  // The node currently under the cursor during either kind of drag, and whether dropping
  // there is possible, for DrawDragOverHighlight()'s use; null/false when nothing is being
  // dragged.
  GameNode m_dragOverNode;
  bool m_dragOverValid{false};

  // Constructs m_outcomeEditor/m_nodeInfoPopup (efgtooltip.cc) and installs the tree drop
  // target (efgdragdrop.cc), respectively. Called from the constructor body -- rather than
  // its initializer list -- since OutcomeEditorPopup/NodeInfoPopup/TreeDropTarget are only
  // forward-declared here; InitPopups() must run before OnUpdate(), which dereferences
  // m_nodeInfoPopup via DismissNodeInfo().
  void InitPopups();
  void InitDropTarget();

  void MakeMenus();
  // Points m_nodeMenu at p_node (see m_contextNode) and refreshes which of its items are
  // enabled accordingly. Called right before the menu is popped up, since nothing else
  // keeps it live anymore.
  void UpdateNodeMenu(const GameNode &p_node);
  void UpdateSetPlayerMenu();
  void AdjustScrollbarSteps();
  // Clears the hovered-node tracking and hides the node info popup, if shown.
  void DismissNodeInfo();
  // Forwards to m_outcomeEditor->BeginEdit(); lets OnLeftDoubleClick (efgnodemenu.cc) start
  // an outcome edit without needing OutcomeEditorPopup's complete type.
  void BeginEditOutcome(const GameNode &p_node, int p_initialPlayer = 0);
  void DrawDragOverHighlight(wxDC &p_dc);

  /// @name Event handlers
  //@{
  void OnSize(wxSizeEvent &);
  void OnMouseMotion(wxMouseEvent &);
  void OnLeaveWindow(wxMouseEvent &);
  void OnLeftClick(wxMouseEvent &);
  void OnRightClick(wxMouseEvent &);
  void OnLeftDoubleClick(wxMouseEvent &);
  void OnMagnify(wxMouseEvent &);
  void OnSetPlayerMenu(wxCommandEvent &);
  void OnHoverTimer(wxTimerEvent &);

  // Node menu command handlers -- operate on m_contextNode (the node the menu was shown
  // for), formerly on GameDocument's now-removed persistent selection.
  void OnEditInsertMove(wxCommandEvent &);
  void OnEditDeleteTree(wxCommandEvent &);
  void OnEditDeleteParent(wxCommandEvent &);
  void OnEditRemoveOutcome(wxCommandEvent &);
  void OnEditReveal(wxCommandEvent &);
  void OnEditNode(wxCommandEvent &);
  void OnEditMove(wxCommandEvent &);
  //@}

  /// @name Overriding GameView members
  //@{
  void OnUpdate() override;
  void PostPendingChanges() override;
  //@}

  void RefreshTree();

  /// @brief Scroll the viewport such that the node is at the specified fraction of the viewport
  void FocusNode(const GameNode &p_node, double p_xFrac = 0.5, double p_yFrac = 0.5);

  void ZoomByFactor(double p_factor, const wxPoint &p_clientPoint);

public:
  EfgDisplay(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc);

  void OnDraw(wxDC &dc) override;
  void OnDraw(wxDC &, double);

  int GetZoom() const { return m_zoom; }
  void SetZoom(int p_zoom);
  void FitZoom();

  double GetScale() const { return 0.01 * m_zoom; }
  int LayoutToDevice(int p_value) const { return static_cast<int>(p_value * GetScale()); }
  int DeviceToLayout(int p_value) const
  {
    return static_cast<int>(static_cast<double>(p_value) / GetScale());
  }

  const TreeLayout &GetLayout() const { return m_layout; }

  bool ShowTreeDropMenu(const GameNode &p_targetNode, const GameNode &p_sourceNode,
                        const wxPoint &p_pos);
  bool ShowOutcomeDropMenu(const GameNode &p_targetNode, const GameNode &p_sourceNode,
                           const wxPoint &p_pos);

  // Whether dropping a moved/copied subtree rooted at p_source onto p_target is possible
  // (some action would be offered); used both to gate ShowTreeDropMenu's contents and to
  // give live drag-over feedback while the drag is still in progress.
  bool CanDropTreeOn(const GameNode &p_target, const GameNode &p_source) const;
  // Whether dropping p_source's outcome onto p_target is possible; used both to gate
  // ShowOutcomeDropMenu's contents and for live drag-over feedback.
  bool CanDropOutcomeOn(const GameNode &p_target, const GameNode &p_source) const;
  // The node being dragged, if a node (tree) move/copy drag is currently in progress;
  // null otherwise (including during an outcome drag).
  GameNode GetDraggedNode() const { return m_draggedNode; }
  // The node whose outcome is being dragged, if an outcome drag is currently in progress;
  // null otherwise (including during a node drag).
  GameNode GetDraggedOutcomeNode() const { return m_draggedOutcomeNode; }
  // Updates the live drag-over highlight (see m_dragOverNode) and repaints if it changed.
  void SetDragOverNode(const GameNode &p_node, bool p_valid);

  DECLARE_EVENT_TABLE()
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_EFGDISPLAY_H
