//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgnodemenu.cc
// The tree display's right-click node menu and its editing commands
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

#include "games.h"
#include "games/gametree.h"

#include "efgdisplay.h"
#include "menuconst.h"
#include "dialogstyle.h"
#include "dleditnode.h"
#include "dleditmove.h"

namespace Gambit::GUI {

void EfgDisplay::MakeMenus()
{
  m_nodeMenu = new wxMenu;

  // Player submenus: create a new move for a player, or reassign the existing one. Labels get
  // their mnemonics set here (UpdateNewMoveMenu only ever changes the text between "Append"
  // and "Insert"); "Assign" uses 's' rather than 'A' so it never collides with "Append"/
  // "Insert", both of which are visible (if not always enabled) in the same menu.
  m_newMoveMenu = new wxMenu;
  m_newMoveItem = m_nodeMenu->AppendSubMenu(m_newMoveMenu, _("&Append move for"),
                                            _("Create a new move for the chosen player"));

  m_reassignPlayerMenu = new wxMenu;
  m_reassignPlayerItem = m_nodeMenu->AppendSubMenu(m_reassignPlayerMenu, _("As&sign this move to"),
                                                   _("Set the player who moves at this node"));
  m_nodeMenu->AppendSeparator();

  // Properties dialogs for the node and the move at it, plus the one quick action that also
  // operates on the move's infoset (refining the information sets below it by its action).
  m_nodeMenu->Append(GBT_MENU_EDIT_NODE, _("&Node properties..."),
                     _("Edit properties of the node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_MOVE, _("&Move properties..."),
                     _("Edit properties of the move"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REFINE_INFOSETS, _("&Refine information sets by this move..."),
                     _("Refine the information sets below this node by the action taken here"));
  m_nodeMenu->AppendSeparator();

  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_TREE, _("&Delete subtree"),
                     _("Delete the subtree starting at the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_PARENT, _("Delete &parent"),
                     _("Delete the node directly before the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REMOVE_OUTCOME, _("Remove &outcome"),
                     _("Remove the outcome from the selected node"));
  m_nodeMenu->AppendSeparator();

  m_nodeMenu->Append(GBT_MENU_EDIT_GAME, _("&Game properties..."),
                     _("Edit properties of the game"));
}

void EfgDisplay::UpdateNewMoveMenu()
{
  while (m_newMoveMenu->GetMenuItemCount() > 0) {
    m_newMoveMenu->Destroy(m_newMoveMenu->FindItemByPosition(0));
  }
  m_newMoveList.clear();

  if (!m_contextNode) {
    m_nodeMenu->Enable(m_newMoveItem->GetId(), false);
    return;
  }
  m_nodeMenu->Enable(m_newMoveItem->GetId(), true);

  const bool terminal = m_contextNode->IsTerminal();
  m_newMoveItem->SetItemLabel(terminal ? _("&Append move for") : _("&Insert move for"));

  const Game efg = m_doc->GetGame();
  for (const auto &player : efg->GetPlayersWithChance()) {
    if (static_cast<int>(m_newMoveList.size()) >= gbtPlayerMenuCount) {
      break;
    }
    const int id = GBT_MENU_EDIT_NEW_MOVE_BASE + static_cast<int>(m_newMoveList.size());
    m_newMoveList.push_back(player);

    wxString label = wxString::FromUTF8(player->GetLabel());
    if (label.empty()) {
      label = player->IsChance() ? wxString(_("Chance"))
                                 : wxString::Format(_("Player %d"), player->GetNumber());
    }

    auto *item = new wxMenuItem(m_newMoveMenu, id, label);
    item->SetBitmap(MakeColorSwatch(m_doc->GetPlayerColor(player)));
    m_newMoveMenu->Append(item);
  }
}

void EfgDisplay::UpdateReassignPlayerMenu()
{
  while (m_reassignPlayerMenu->GetMenuItemCount() > 0) {
    m_reassignPlayerMenu->Destroy(m_reassignPlayerMenu->FindItemByPosition(0));
  }
  m_reassignPlayerList.clear();

  if (!m_contextNode || m_contextNode->IsTerminal()) {
    m_nodeMenu->Enable(m_reassignPlayerItem->GetId(), false);
    return;
  }

  // Swapping a node between a chance move and a personal player's move is
  // not currently supported, so the operation isn't offered at chance nodes.
  if (m_contextNode->GetPlayer()->IsChance()) {
    m_nodeMenu->Enable(m_reassignPlayerItem->GetId(), false);
    return;
  }
  m_nodeMenu->Enable(m_reassignPlayerItem->GetId(), true);

  const Game efg = m_doc->GetGame();
  for (const auto &player : efg->GetPlayers()) { // excludes chance -- see the check above
    if (static_cast<int>(m_reassignPlayerList.size()) >= gbtPlayerMenuCount) {
      break;
    }
    const int id =
        GBT_MENU_EDIT_REASSIGN_PLAYER_BASE + static_cast<int>(m_reassignPlayerList.size());
    m_reassignPlayerList.push_back(player);

    wxString label = wxString::FromUTF8(player->GetLabel());
    if (label.empty()) {
      label = wxString::Format(_("Player %d"), player->GetNumber());
    }

    auto *item = new wxMenuItem(m_reassignPlayerMenu, id, label);
    item->SetBitmap(MakeColorSwatch(m_doc->GetPlayerColor(player)));
    m_reassignPlayerMenu->Append(item);

    if (m_contextNode->GetPlayer() == player) {
      m_reassignPlayerMenu->Enable(id, false);
    }
  }
}

void EfgDisplay::OnNewMoveMenu(wxCommandEvent &p_event)
{
  if (!m_contextNode) {
    return;
  }

  const size_t index = p_event.GetId() - GBT_MENU_EDIT_NEW_MOVE_BASE;
  if (index >= m_newMoveList.size()) {
    return;
  }
  const GamePlayer player = m_newMoveList[index];

  try {
    if (m_contextNode->IsTerminal()) {
      BeginAppendMove(m_contextNode, player);
    }
    else {
      BeginInsertMove(m_contextNode, player);
    }
  }
  catch (std::exception &ex) {
    ShowInternalError(this, ex.what());
  }
}

void EfgDisplay::OnReassignPlayerMenu(wxCommandEvent &p_event)
{
  if (!m_contextNode) {
    return;
  }

  const size_t index = p_event.GetId() - GBT_MENU_EDIT_REASSIGN_PLAYER_BASE;
  if (index >= m_reassignPlayerList.size()) {
    return;
  }
  const GamePlayer player = m_reassignPlayerList[index];

  try {
    m_doc->Apply(SetMovePlayer{.node = m_contextNode, .player = player});
  }
  catch (std::exception &ex) {
    ShowInternalError(this, ex.what());
  }
}

void EfgDisplay::UpdateNodeMenu(const GameNode &p_node)
{
  m_contextNode = p_node;

  m_nodeMenu->Enable(
      GBT_MENU_EDIT_REFINE_INFOSETS,
      p_node && m_doc->CanApply(RefineInfosets{.infoset = p_node->GetInfoset(), .players = {}}));
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_TREE, m_doc->CanApply(DeleteTree{.node = p_node}));
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_PARENT, m_doc->CanApply(DeleteParent{.node = p_node}));
  m_nodeMenu->Enable(GBT_MENU_EDIT_REMOVE_OUTCOME,
                     m_doc->CanApply(RemoveNodeOutcome{.node = p_node}));
  m_nodeMenu->Enable(GBT_MENU_EDIT_NODE, static_cast<bool>(p_node));
  m_nodeMenu->Enable(GBT_MENU_EDIT_MOVE, p_node && p_node->GetInfoset());

  UpdateNewMoveMenu();
  UpdateReassignPlayerMenu();
}

//
// Left mouse button double-click: brings up the properties dialog for whatever was hit.
//
void EfgDisplay::OnLeftDoubleClick(wxMouseEvent &p_event)
{
  using enum HitRegion;
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = DeviceToLayout(x);
  y = DeviceToLayout(y);

  const auto hit = m_layout.HitTest(x, y);

  if (hit.region == Node) {
    m_contextNode = hit.node;
    const wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_NODE);
    wxPostEvent(this, event);
    return;
  }

  if (hit.region == Outcome || hit.region == Payoff) {
    const int initialPlayer = (hit.region == Payoff) ? hit.payoffPlayer : 0;
    BeginEditOutcome(hit.node, initialPlayer);
    return;
  }

  const bool aboveClickable = hit.region == BranchAbove &&
                              m_doc->GetStyle().GetBranchAboveLabel() == GBT_BRANCH_LABEL_LABEL;
  const bool belowClickable = hit.region == BranchBelow &&
                              m_doc->GetStyle().GetBranchBelowLabel() == GBT_BRANCH_LABEL_LABEL;
  if (aboveClickable || belowClickable) {
    m_contextNode = hit.node->GetParent();
    const wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_MENU_EDIT_MOVE);
    wxPostEvent(this, event);
  }
}

//
// Right mouse-button click: display the context-sensitive node menu, targeting
// whichever node (if any) was clicked.
//
void EfgDisplay::OnRightClick(wxMouseEvent &p_event)
{
  int x, y;
  CalcUnscrolledPosition(p_event.GetX(), p_event.GetY(), &x, &y);
  x = DeviceToLayout(x);
  y = DeviceToLayout(y);

  UpdateNodeMenu(m_layout.NodeHitTest(x, y));
  PopupMenu(m_nodeMenu);
}

//---------------------------------------------------------------------
//              EfgDisplay: Node menu command handlers
//---------------------------------------------------------------------

std::optional<std::vector<GamePlayer>> ChooseRefiningPlayers(wxWindow *p_parent,
                                                             const GameDocument &p_doc);

void EfgDisplay::OnEditDeleteTree(wxCommandEvent &)
{
  try {
    m_doc->Apply(DeleteTree{.node = m_contextNode});
  }
  catch (std::exception &ex) {
    ShowInternalError(this, ex.what());
  }
}

void EfgDisplay::OnEditDeleteParent(wxCommandEvent &)
{
  try {
    m_doc->Apply(DeleteParent{.node = m_contextNode});
  }
  catch (std::exception &ex) {
    ShowInternalError(this, ex.what());
  }
}

void EfgDisplay::OnEditRemoveOutcome(wxCommandEvent &)
{
  try {
    m_doc->Apply(RemoveNodeOutcome{.node = m_contextNode});
  }
  catch (std::exception &ex) {
    ShowInternalError(this, ex.what());
  }
}

void EfgDisplay::OnEditRefineInfosets(wxCommandEvent &)
{
  if (const auto players = ChooseRefiningPlayers(this, *m_doc); players) {
    try {
      m_doc->Apply(RefineInfosets{.infoset = m_contextNode->GetInfoset(), .players = *players});
    }
    catch (std::exception &ex) {
      ShowInternalError(this, ex.what());
    }
  }
}

void EfgDisplay::OnEditNode(wxCommandEvent &)
{
  EditNodeDialog dialog(this, m_contextNode);
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_doc->Apply(EditNode{.node = m_contextNode,
                            .label = dialog.GetNodeLabel().ToStdString(wxConvUTF8),
                            .outcome = (dialog.GetOutcome() > 0)
                                           ? m_doc->GetGame()->GetOutcome(dialog.GetOutcome())
                                           : GameOutcome(nullptr),
                            .infoset = dialog.GetInfoset()});
    }
    catch (std::exception &ex) {
      ShowInternalError(this, ex.what());
    }
  }
}

void EfgDisplay::OnEditMove(wxCommandEvent &)
{
  const GameInfoset infoset = m_contextNode->GetInfoset();
  if (!infoset) {
    return;
  }

  EditMoveDialog dialog(this, m_doc, infoset);
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_doc->Apply(dialog.GetEdit());
    }
    catch (std::exception &ex) {
      ShowInternalError(this, ex.what());
    }
  }
}
} // namespace Gambit::GUI
