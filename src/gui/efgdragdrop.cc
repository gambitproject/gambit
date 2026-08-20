//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgdragdrop.cc
// Drag-and-drop of nodes and outcomes within the tree display
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/dnd.h>

#include "games.h"

#include "efgdisplay.h"
#include "dlexcept.h"

namespace Gambit::GUI {

#include "bitmaps/move.xpm"

//--------------------------------------------------------------------------
//                      class TreeDropTarget
//--------------------------------------------------------------------------

class TreeDropTarget : public wxTextDropTarget {
  EfgDisplay *m_owner;
  GameDocument *m_model;

  bool OnDropOutcome(const GameNode &p_node, const wxString &p_text, const wxPoint &p_pos);
  bool OnDropTreeNode(const GameNode &p_node, const wxString &p_text, const wxPoint &p_pos);

public:
  explicit TreeDropTarget(EfgDisplay *p_owner) : m_owner(p_owner), m_model(p_owner->GetDocument())
  {
  }

  bool OnDropText(wxCoord x, wxCoord y, const wxString &p_text) override;
  // Live feedback while dragging: highlights the node under the cursor as a valid or
  // invalid drop target. Only does this for node (tree) drags for now -- outcome drags
  // fall through to the default behavior unchanged.
  wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;
  void OnLeave() override;
};

//
// This recurses the subtree starting at 'p_node' looking for a node
// with the ID 'p_id'.
//
static GameNode GetNode(const GameNode &p_node, int p_id)
{
  if (p_node->GetNumber() == p_id) {
    return p_node;
  }
  if (p_node->IsTerminal()) {
    return nullptr;
  }
  for (const auto &child : p_node->GetChildren()) {
    if (const auto node = GetNode(child, p_id)) {
      return node;
    }
  }
  return nullptr;
}

bool TreeDropTarget::OnDropOutcome(const GameNode &p_node, const wxString &p_text,
                                   const wxPoint &p_pos)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);

  const GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode || srcNode == p_node || !srcNode->GetOutcome()) {
    return false;
  }

  return m_owner->ShowOutcomeDropMenu(p_node, srcNode, p_pos);
}

bool TreeDropTarget::OnDropTreeNode(const GameNode &p_node, const wxString &p_text,
                                    const wxPoint &p_pos)
{
  long n;
  p_text.Right(p_text.Length() - 1).ToLong(&n);

  const GameNode srcNode = GetNode(m_model->GetGame()->GetRoot(), n);
  if (!srcNode || srcNode == p_node || srcNode->IsTerminal()) {
    return false;
  }

  return m_owner->ShowTreeDropMenu(p_node, srcNode, p_pos);
}

bool TreeDropTarget::OnDropText(wxCoord p_x, wxCoord p_y, const wxString &p_text)
{
  const Game efg = m_owner->GetDocument()->GetGame();

  int x, y;
  m_owner->CalcUnscrolledPosition(p_x, p_y, &x, &y);

  x = m_owner->DeviceToLayout(x);
  y = m_owner->DeviceToLayout(y);

  const GameNode node = m_owner->GetLayout().NodeHitTest(x, y);
  if (!node) {
    return false;
  }

  try {
    switch (static_cast<char>(p_text[0])) {
    case 'N':
      return OnDropTreeNode(node, p_text, wxPoint(p_x, p_y));
    case 'O':
      return OnDropOutcome(node, p_text, wxPoint(p_x, p_y));
    default:
      return false;
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(m_owner, ex.what()).ShowModal();
    return false;
  }
}

wxDragResult TreeDropTarget::OnDragOver(wxCoord p_x, wxCoord p_y, wxDragResult p_def)
{
  const GameNode treeSource = m_owner->GetDraggedNode();
  const GameNode outcomeSource = m_owner->GetDraggedOutcomeNode();
  if (!treeSource && !outcomeSource) {
    // Neither drag is in progress -- shouldn't normally happen, since dragging is the only
    // way to get here, but fall back to the default behavior just in case.
    return p_def;
  }

  int x, y;
  m_owner->CalcUnscrolledPosition(p_x, p_y, &x, &y);
  x = m_owner->DeviceToLayout(x);
  y = m_owner->DeviceToLayout(y);

  const GameNode node = m_owner->GetLayout().NodeHitTest(x, y);
  const bool valid = treeSource ? m_owner->CanDropTreeOn(node, treeSource)
                                : m_owner->CanDropOutcomeOn(node, outcomeSource);
  m_owner->SetDragOverNode(node, valid);
  return valid ? wxDragMove : wxDragNone;
}

void TreeDropTarget::OnLeave() { m_owner->SetDragOverNode(nullptr, false); }

//----------------------------------------------------------------------
//              EfgDisplay: drag-and-drop members
//----------------------------------------------------------------------

void EfgDisplay::InitDropTarget() { wxWindow::SetDropTarget(new TreeDropTarget(this)); }

// True if p_node is p_ancestor itself or lies anywhere below it in the tree.
static bool IsDescendantOf(const GameNode &p_node, const GameNode &p_ancestor)
{
  for (GameNode node = p_node; node; node = node->GetParent()) {
    if (node == p_ancestor) {
      return true;
    }
  }
  return false;
}

bool EfgDisplay::CanDropTreeOn(const GameNode &p_target, const GameNode &p_source) const
{
  if (!p_target || !p_source || p_source->IsTerminal() || p_target == p_source) {
    return false;
  }
  // Copying (but not moving -- see ShowTreeDropMenu) onto a node's own descendant is
  // well-defined: CopyTree acts on the tree as it stood at the start of the operation, so
  // it doesn't recurse. Whether *that* is available is exactly canCopyOrMoveTree below, so
  // no separate exclusion is needed here.
  const bool canCopyOrMoveTree = p_target->IsTerminal();
  const bool canUseSameInfoset = (!p_target->IsTerminal() && p_target->GetChildren().size() ==
                                                                 p_source->GetChildren().size()) ||
                                 p_target->IsTerminal();
  return canCopyOrMoveTree || canUseSameInfoset;
}

void EfgDisplay::SetDragOverNode(const GameNode &p_node, bool p_valid)
{
  if (p_node == m_dragOverNode && p_valid == m_dragOverValid) {
    return;
  }
  m_dragOverNode = p_node;
  m_dragOverValid = p_valid;
  Refresh();
}

void EfgDisplay::DrawDragOverHighlight(wxDC &p_dc)
{
  if (!m_dragOverNode) {
    return;
  }
  const auto entry = m_layout.GetNodeEntry(m_dragOverNode);
  if (!entry) {
    return;
  }

  constexpr int padding = 10;
  const wxColour color = m_dragOverValid ? wxColour(0, 170, 0) : wxColour(210, 0, 0);
  p_dc.SetPen(wxPen(color, 3, wxPENSTYLE_SOLID));
  p_dc.SetBrush(*wxTRANSPARENT_BRUSH);
  p_dc.DrawEllipse(entry->GetTokenExtent().Inflate(padding, padding));
}

bool EfgDisplay::ShowTreeDropMenu(const GameNode &p_targetNode, const GameNode &p_sourceNode,
                                  const wxPoint &p_pos)
{
  if (!CanDropTreeOn(p_targetNode, p_sourceNode)) {
    return false;
  }

  const bool canCopyTree = p_targetNode->IsTerminal();
  // Moving, unlike copying, is only well-defined when the target isn't part of the
  // source's own subtree: CopyTree acts on a snapshot of the tree as it stood at the
  // start of the operation, so copying onto a descendant doesn't recurse, but moving
  // the subtree there would be asking to attach it to a node that only continues to
  // exist as part of the very subtree being relocated.
  const bool canMoveTree = canCopyTree && !IsDescendantOf(p_targetNode, p_sourceNode);
  const bool canUseSameInfoset =
      (!p_targetNode->IsTerminal() &&
       p_targetNode->GetChildren().size() == p_sourceNode->GetChildren().size()) ||
      p_targetNode->IsTerminal();

  const int copyTreeId = wxWindow::NewControlId();
  const int moveTreeId = wxWindow::NewControlId();
  const int infosetId = wxWindow::NewControlId();

  wxMenu menu;

  if (canCopyTree) {
    menu.Append(copyTreeId, _("Copy subtree here"));
    menu.Append(moveTreeId, _("Move subtree here"));
    menu.Enable(moveTreeId, canMoveTree);
  }

  if (canUseSameInfoset) {
    if (!menu.GetMenuItems().empty()) {
      menu.AppendSeparator();
    }

    if (p_targetNode->IsTerminal()) {
      menu.Append(infosetId, _("Insert move using same information set"));
    }
    else {
      menu.Append(infosetId, _("Put node in same information set"));
    }
  }

  const int selection = GetPopupMenuSelectionFromUser(menu, p_pos);

  try {
    if (selection == copyTreeId) {
      m_doc->DoCopyTree(p_targetNode, p_sourceNode);
      return true;
    }
    if (selection == moveTreeId) {
      m_doc->DoMoveTree(p_targetNode, p_sourceNode);
      return true;
    }
    if (selection == infosetId) {
      if (!p_targetNode->IsTerminal()) {
        m_doc->DoSetInfoset(p_targetNode, p_sourceNode->GetInfoset());
      }
      else {
        m_doc->DoAppendMove(p_targetNode, p_sourceNode->GetInfoset());
      }
      return true;
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }

  return false;
}

bool EfgDisplay::CanDropOutcomeOn(const GameNode &p_target, const GameNode &p_source) const
{
  return p_target && p_source && p_target != p_source && p_source->GetOutcome();
}

bool EfgDisplay::ShowOutcomeDropMenu(const GameNode &p_targetNode, const GameNode &p_sourceNode,
                                     const wxPoint &p_pos)
{
  if (!CanDropOutcomeOn(p_targetNode, p_sourceNode)) {
    return false;
  }

  const int useSameOutcomeId = wxWindow::NewControlId();
  const int moveOutcomeId = wxWindow::NewControlId();

  wxMenu menu;
  menu.Append(useSameOutcomeId, _("Use same outcome here"));
  menu.Append(moveOutcomeId, _("Move outcome here"));

  const int selection = GetPopupMenuSelectionFromUser(menu, p_pos);

  try {
    if (selection == useSameOutcomeId) {
      m_doc->DoSetOutcome(p_targetNode, p_sourceNode->GetOutcome());
      return true;
    }
    if (selection == moveOutcomeId) {
      m_doc->DoSetOutcome(p_targetNode, p_sourceNode->GetOutcome());
      m_doc->DoSetOutcome(p_sourceNode, nullptr);
      return true;
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }

  return false;
}

void EfgDisplay::OnMouseMotion(wxMouseEvent &p_event)
{
  using enum HitRegion;
  if (p_event.LeftIsDown() && p_event.Dragging()) {
    DismissNodeInfo();

    int x, y;
    CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
    x = DeviceToLayout(x);
    y = DeviceToLayout(y);

    const auto hit = m_layout.HitTest(x, y);

    if (hit.region == Node && !hit.node->IsTerminal()) {
      wxString label;
      label << "N" << hit.node->GetNumber();
      wxTextDataObject textData(label);

      // Tracked only for live drag-over feedback (CanDropTreeOn/DrawDragOverHighlight);
      // the actual drop still resolves its source node independently, by decoding the
      // dragged text -- see TreeDropTarget::OnDropTreeNode.
      m_draggedNode = hit.node;
      // wxDropSource's cursor-bundle parameters are wxIcon on GTK/MSW but wxCursor on
      // macOS/Cocoa -- wxDROP_ICON(move) (needing move_xpm in scope, from the #include
      // above) is wx's own portable macro for this, expanding to the right type per
      // port. On macOS specifically, this ends up with no visible effect regardless: as
      // of wxWidgets 3.3, its DoDragDrop ignores the cursor argument entirely and always
      // shows a fixed placeholder drag image instead (see src/osx/cocoa/dnd.mm's
      // wxDropSource::DoDragDrop, which draws a hardcoded white square with a "TODO:
      // proper drag image for data" comment) -- the drag-over highlight
      // (DrawDragOverHighlight) compensates there by giving feedback drawn on the canvas
      // itself, independent of the native drag image.
      wxDropSource source(textData, this, wxDROP_ICON(move), wxDROP_ICON(move));
      source.DoDragDrop(wxDrag_DefaultMove);
      m_draggedNode = nullptr;
      SetDragOverNode(nullptr, false);
      return;
    }

    if ((hit.region == Outcome || hit.region == Payoff) && hit.node->GetOutcome()) {
      wxString label;
      label << "O" << hit.node->GetNumber();
      wxTextDataObject textData(label);

      // Tracked only for live drag-over feedback -- see the node-drag case above.
      m_draggedOutcomeNode = hit.node;
      wxDropSource source(textData, this);
      source.DoDragDrop(wxDrag_DefaultMove);
      m_draggedOutcomeNode = nullptr;
      SetDragOverNode(nullptr, false);
    }
    return;
  }

  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = DeviceToLayout(x);
  y = DeviceToLayout(y);

  const auto hit = m_layout.HitTest(x, y);
  const GameNode node = (hit.region == Node) ? hit.node : nullptr;

  if (node == m_hoverNode) {
    return;
  }

  DismissNodeInfo();
  m_hoverNode = node;
  if (m_hoverNode) {
    m_hoverTimer.StartOnce(400);
  }
}
} // namespace Gambit::GUI
