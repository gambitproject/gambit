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

#include "efgdisplay.h"
#include "menuconst.h"
#include "dlexcept.h"
#include "dlinsertmove.h"
#include "dleditnode.h"
#include "dleditmove.h"

namespace Gambit::GUI {

void EfgDisplay::MakeMenus()
{
  m_nodeMenu = new wxMenu;
  m_nodeMenu->Append(GBT_MENU_EDIT_INSERT_MOVE, _("&Insert move"), _("Insert a move"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REVEAL, _("&Reveal"), _("Reveal choice at node"));

  m_setPlayerMenu = new wxMenu;
  m_setPlayerItem = m_nodeMenu->AppendSubMenu(m_setPlayerMenu, _("Assign this move to"),
                                              _("Set the player who moves at this node"));

  m_nodeMenu->AppendSeparator();

  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_TREE, _("&Delete subtree"),
                     _("Delete the subtree starting at the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_DELETE_PARENT, _("Delete &parent"),
                     _("Delete the node directly before the selected node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_REMOVE_OUTCOME, _("Remove &outcome"),
                     _("Remove the outcome from the selected node"));
  m_nodeMenu->AppendSeparator();

  m_nodeMenu->Append(GBT_MENU_EDIT_NODE, _("&Node properties"), _("Edit properties of the node"));
  m_nodeMenu->Append(GBT_MENU_EDIT_MOVE, _("&Edit move"), _("Edit properties of the move"));

  m_nodeMenu->AppendSeparator();
  m_nodeMenu->Append(GBT_MENU_EDIT_GAME, _("&Game properties"), _("Edit properties of the game"));
}

void EfgDisplay::UpdateSetPlayerMenu()
{
  while (m_setPlayerMenu->GetMenuItemCount() > 0) {
    m_setPlayerMenu->Destroy(m_setPlayerMenu->FindItemByPosition(0));
  }
  m_setPlayerList.clear();

  if (!m_contextNode) {
    m_nodeMenu->Enable(m_setPlayerItem->GetId(), false);
    return;
  }

  const bool terminal = m_contextNode->IsTerminal();

  // Swapping a node between a chance move and a personal player's move is
  // not currently supported, so the operation isn't offered at chance nodes.
  if (!terminal && m_contextNode->GetPlayer()->IsChance()) {
    m_nodeMenu->Enable(m_setPlayerItem->GetId(), false);
    return;
  }
  m_nodeMenu->Enable(m_setPlayerItem->GetId(), true);

  m_setPlayerItem->SetItemLabel(terminal ? _("Insert move for") : _("Assign this move to"));

  const Game efg = m_doc->GetGame();
  for (const auto &player : efg->GetPlayersWithChance()) {
    if (!terminal && player->IsChance()) {
      continue; // can't reassign an existing personal player's move to chance
    }
    if (static_cast<int>(m_setPlayerList.size()) >= gbtSetPlayerMenuCount) {
      break;
    }
    const int id = GBT_MENU_EDIT_SET_PLAYER_BASE + static_cast<int>(m_setPlayerList.size());
    m_setPlayerList.push_back(player);

    wxString label = wxString::FromUTF8(player->GetLabel());
    if (label.empty()) {
      label = player->IsChance() ? wxString(_("Chance"))
                                 : wxString::Format(_("Player %d"), player->GetNumber());
    }

    auto *item = new wxMenuItem(m_setPlayerMenu, id, label);
    item->SetBitmap(MakeColorSwatch(m_doc->GetStyle().GetPlayerColor(player)));
    m_setPlayerMenu->Append(item);

    if (!terminal && m_contextNode->GetPlayer() == player) {
      m_setPlayerMenu->Enable(id, false);
    }
  }
}

void EfgDisplay::OnSetPlayerMenu(wxCommandEvent &p_event)
{
  if (!m_contextNode) {
    return;
  }

  const size_t index = p_event.GetId() - GBT_MENU_EDIT_SET_PLAYER_BASE;
  if (index >= m_setPlayerList.size()) {
    return;
  }
  const GamePlayer player = m_setPlayerList[index];

  try {
    if (m_contextNode->IsTerminal()) {
      m_doc->DoInsertMove(m_contextNode, player, 2);
    }
    else {
      m_doc->DoSetPlayer(m_contextNode, player);
    }
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

void EfgDisplay::UpdateNodeMenu(const GameNode &p_node)
{
  m_contextNode = p_node;

  m_nodeMenu->Enable(GBT_MENU_EDIT_INSERT_MOVE, static_cast<bool>(p_node));
  m_nodeMenu->Enable(GBT_MENU_EDIT_REVEAL,
                     p_node && p_node->GetInfoset() &&
                         !m_doc->GetGame()->IsAbsentMinded(p_node->GetInfoset()));
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_TREE, p_node && !p_node->IsTerminal());
  m_nodeMenu->Enable(GBT_MENU_EDIT_DELETE_PARENT, p_node && p_node->GetParent());
  m_nodeMenu->Enable(GBT_MENU_EDIT_REMOVE_OUTCOME, p_node && p_node->GetOutcome());
  m_nodeMenu->Enable(GBT_MENU_EDIT_NODE, static_cast<bool>(p_node));
  m_nodeMenu->Enable(GBT_MENU_EDIT_MOVE, p_node && p_node->GetInfoset());

  UpdateSetPlayerMenu();
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
    // hit.node is the child whose incoming branch was hit; the old behavior
    // was to target the parent (the node whose move is being edited).
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

// Forward declaration only: defined in dlefgreveal.cc, with no header of its own -- this
// mirrors how it was previously forward-declared in gameframe.cc.
std::optional<std::vector<GamePlayer>> RevealMove(wxWindow *p_parent, const Game &p_game);

void EfgDisplay::OnEditInsertMove(wxCommandEvent &)
{
  InsertMoveDialog dialog(this, m_doc);
  if (dialog.ShowModal() == wxID_OK) {
    try {
      if (dialog.GetInfoset()) {
        m_doc->DoInsertMove(m_contextNode, dialog.GetInfoset());
      }
      else {
        GamePlayer player = dialog.GetPlayer();
        if (!player) {
          // "Insert move for a new player" was selected: no such player exists yet.
          player = m_doc->DoAddPlayer();
        }
        m_doc->DoInsertMove(m_contextNode, player, dialog.GetActions());
      }
    }
    catch (std::exception &ex) {
      ExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void EfgDisplay::OnEditDeleteTree(wxCommandEvent &)
{
  try {
    m_doc->DoDeleteTree(m_contextNode);
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

void EfgDisplay::OnEditDeleteParent(wxCommandEvent &)
{
  try {
    m_doc->DoDeleteParent(m_contextNode);
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

void EfgDisplay::OnEditRemoveOutcome(wxCommandEvent &)
{
  try {
    m_doc->DoRemoveOutcome(m_contextNode);
  }
  catch (std::exception &ex) {
    ExceptionDialog(this, ex.what()).ShowModal();
  }
}

void EfgDisplay::OnEditReveal(wxCommandEvent &)
{
  if (const auto players = RevealMove(this, m_doc->GetGame()); players) {
    try {
      const auto &infoset = m_contextNode->GetInfoset();
      for (const auto &player : *players) {
        m_doc->DoRevealAction(infoset, player);
      }
    }
    catch (std::exception &ex) {
      ExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void EfgDisplay::OnEditNode(wxCommandEvent &)
{
  EditNodeDialog dialog(this, m_contextNode);
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_doc->DoSetNodeLabel(m_contextNode, dialog.GetNodeLabel());
      if (dialog.GetOutcome() > 0) {
        m_doc->DoSetOutcome(m_contextNode, m_doc->GetGame()->GetOutcome(dialog.GetOutcome()));
      }
      else {
        m_doc->DoSetOutcome(m_contextNode, nullptr);
      }

      if (!m_contextNode->IsTerminal() && dialog.GetInfoset() != m_contextNode->GetInfoset()) {
        if (dialog.GetInfoset() == nullptr) {
          m_doc->DoLeaveInfoset(m_contextNode);
        }
        else {
          m_doc->DoSetInfoset(m_contextNode, dialog.GetInfoset());
        }
      }
    }
    catch (std::exception &ex) {
      ExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}

void EfgDisplay::OnEditMove(wxCommandEvent &)
{
  GameInfoset infoset = m_contextNode->GetInfoset();
  if (!infoset) {
    return;
  }

  EditMoveDialog dialog(this, infoset);
  if (dialog.ShowModal() == wxID_OK) {
    try {
      m_doc->DoSetInfosetLabel(infoset, dialog.GetInfosetLabel());

      if (!infoset->IsChanceInfoset() && dialog.GetPlayer() != infoset->GetPlayer()->GetNumber()) {
        m_doc->DoSetPlayer(infoset, m_doc->GetGame()->GetPlayer(dialog.GetPlayer()));
        // DoSetPlayer reforms the members into a brand-new information set (via
        // MakeInfoset), invalidating this handle; the node itself is still valid, so
        // re-fetch the information set through it.
        infoset = m_contextNode->GetInfoset();
      }

      std::vector<std::string> stableLabels, labels;
      std::vector<Number> probs;
      for (int i = 0; i < dialog.NumActions(); i++) {
        if (dialog.IsDeleted(i)) {
          continue;
        }
        stableLabels.push_back(dialog.GetStableLabel(i));
        labels.push_back(dialog.GetActionLabel(i).ToStdString(wxConvUTF8));
        if (infoset->IsChanceInfoset()) {
          probs.push_back(dialog.GetActionProb(i));
        }
      }
      m_doc->DoSetActions(infoset, stableLabels, labels, probs);
    }
    catch (std::exception &ex) {
      ExceptionDialog(this, ex.what()).ShowModal();
    }
  }
}
} // namespace Gambit::GUI
