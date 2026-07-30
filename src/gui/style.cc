//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/style.cc
// Display configuration class for the extensive form
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
// aint with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <sstream>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "style.h"

namespace Gambit::GUI {
//===========================================================================
//                 class TreeRenderConfig: Implementation
//===========================================================================

TreeRenderConfig::TreeRenderConfig()
  : m_font(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD))
{
  SetDefaults();
}

static wxColour s_defaultColors[8] = {
    wxColour(255, 0, 0), wxColour(0, 0, 255),   wxColour(0, 128, 0), wxColour(255, 128, 0),
    wxColour(0, 0, 64),  wxColour(128, 0, 255), wxColour(64, 0, 0),  wxColour(255, 128, 255)};

//!
//! Gets the player color assigned to player number 'pl'.
//! If this is the first request for that player's color, create the
//! default one.
//!
const wxColour &TreeRenderConfig::GetPlayerColor(const int pl) const
{
  while (pl > static_cast<int>(m_playerColors.size())) {
    m_playerColors.push_back(s_defaultColors[m_playerColors.size() % 8]);
  }
  return m_playerColors[pl];
}

const wxColour &TreeRenderConfig::GetPlayerColor(const GamePlayer &p_player) const
{
  if (!p_player) {
    return m_terminalColor;
  }
  if (p_player->IsChance()) {
    return m_chanceColor;
  }
  while (p_player->GetNumber() > static_cast<int>(m_playerColors.size())) {
    m_playerColors.push_back(s_defaultColors[m_playerColors.size() % 8]);
  }
  return m_playerColors[p_player->GetNumber()];
}

void TreeRenderConfig::SetDefaults()
{
  m_nodeSize = 10;
  m_terminalSpacing = 50;
  m_chanceToken = GBT_NODE_TOKEN_DOT;
  m_playerToken = GBT_NODE_TOKEN_DOT;
  m_terminalToken = GBT_NODE_TOKEN_DOT;
  m_branchLength = 60;
  m_tineLength = 20;
  m_branchStyle = GBT_BRANCH_STYLE_FORKTINE;
  m_branchLabels = GBT_BRANCH_LABEL_ORIENT_HORIZONTAL;
  m_infosetJoin = GBT_INFOSET_JOIN_CIRCLES;
  m_nodeAboveLabel = GBT_NODE_LABEL_LABEL;
  m_nodeBelowLabel = GBT_NODE_LABEL_ISETID;
  m_branchAboveLabel = GBT_BRANCH_LABEL_LABEL;
  m_branchBelowLabel = GBT_BRANCH_LABEL_PROBS;
  m_numDecimals = 4;

  m_font = wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

  m_chanceColor = wxColour(154, 205, 50);
  m_terminalColor = *wxBLACK;
  for (size_t pl = 1; pl <= m_playerColors.size(); pl++) {
    m_playerColors[pl] = s_defaultColors[(pl - 1) % 8];
  }
}

void TreeRenderConfig::Save(LegacyWorkspaceFile &p_workspace) const
{
  const auto color = [](int player, const wxColour &value) {
    return LegacyWorkspaceFile::Color{player, value.Red(), value.Green(), value.Blue()};
  };
  p_workspace.colors = {color(-1, m_terminalColor), color(0, m_chanceColor)};
  for (size_t player = 1; player <= m_playerColors.size(); ++player) {
    p_workspace.colors.push_back(color(player, m_playerColors[player]));
  }

  p_workspace.font = LegacyWorkspaceFile::Font{
      m_font.GetPointSize(), static_cast<int>(m_font.GetFamily()),
      m_font.GetFaceName().ToStdString(), static_cast<int>(m_font.GetStyle()),
      static_cast<int>(m_font.GetWeight())};
  static const std::string node_tokens[] = {"line", "box", "circle", "diamond", "dot"};
  static const std::string branch_styles[] = {"line", "forktine"};
  static const std::string branch_labels[] = {"horizontal", "rotated"};
  static const std::string infoset_styles[] = {"lines", "circles"};
  p_workspace.layout = LegacyWorkspaceFile::Layout{m_nodeSize,
                                                   m_terminalSpacing,
                                                   node_tokens[m_chanceToken],
                                                   node_tokens[m_playerToken],
                                                   node_tokens[m_terminalToken],
                                                   m_branchLength,
                                                   m_tineLength,
                                                   branch_styles[m_branchStyle],
                                                   branch_labels[m_branchLabels],
                                                   infoset_styles[m_infosetJoin]};

  static const std::string node_labels[] = {"none",   "label",      "player",     "isetlabel",
                                            "isetid", "realizprob", "beliefprob", "value"};
  static const std::string edge_labels[] = {"none", "label", "probs", "value"};
  p_workspace.labels = LegacyWorkspaceFile::Labels{
      node_labels[m_nodeAboveLabel], node_labels[m_nodeBelowLabel],
      edge_labels[m_branchAboveLabel], edge_labels[m_branchBelowLabel]};
  p_workspace.decimals = m_numDecimals;
}

void TreeRenderConfig::Load(const LegacyWorkspaceFile &p_workspace)
{
  for (const auto &color : p_workspace.colors) {
    const wxColour value(color.red, color.green, color.blue);
    if (color.player > 0) {
      GetPlayerColor(color.player);
      SetPlayerColor(color.player, value);
    }
    else if (color.player == 0) {
      SetChanceColor(value);
    }
    else if (color.player == -1) {
      SetTerminalColor(value);
    }
  }
  if (p_workspace.font) {
    const auto &font = *p_workspace.font;
    SetFont(wxFont(font.size, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
                   wxString(font.face.c_str(), *wxConvCurrent)));
  }
  const auto find = [](const std::string &value, const auto &names, auto fallback) {
    const auto iter = std::find(std::begin(names), std::end(names), value);
    return iter == std::end(names) ? fallback : static_cast<decltype(fallback)>(iter - names);
  };
  if (p_workspace.layout) {
    const auto &layout = *p_workspace.layout;
    static const std::string node_tokens[] = {"line", "box", "circle", "diamond", "dot"};
    static const std::string branch_styles[] = {"line", "forktine"};
    static const std::string branch_labels[] = {"horizontal", "rotated"};
    static const std::string infoset_styles[] = {"lines", "circles"};
    m_nodeSize = layout.node_size;
    m_terminalSpacing = layout.terminal_spacing;
    m_chanceToken = find(layout.chance_token, node_tokens, m_chanceToken);
    m_playerToken = find(layout.player_token, node_tokens, m_playerToken);
    m_terminalToken = find(layout.terminal_token, node_tokens, m_terminalToken);
    m_branchLength = layout.branch_length;
    m_tineLength = layout.tine_length;
    m_branchStyle = find(layout.branch_style, branch_styles, m_branchStyle);
    m_branchLabels = find(layout.branch_labels, branch_labels, m_branchLabels);
    m_infosetJoin = find(layout.infoset_style, infoset_styles, m_infosetJoin);
  }
  if (p_workspace.labels) {
    const auto &labels = *p_workspace.labels;
    static const std::string node_labels[] = {"none",   "label",      "player",     "isetlabel",
                                              "isetid", "realizprob", "beliefprob", "value"};
    static const std::string edge_labels[] = {"none", "label", "probs", "value"};
    m_nodeAboveLabel = find(labels.node_above, node_labels, m_nodeAboveLabel);
    m_nodeBelowLabel = find(labels.node_below, node_labels, m_nodeBelowLabel);
    m_branchAboveLabel = find(labels.branch_above, edge_labels, m_branchAboveLabel);
    m_branchBelowLabel = find(labels.branch_below, edge_labels, m_branchBelowLabel);
  }
  if (p_workspace.decimals) {
    m_numDecimals = *p_workspace.decimals;
  }
}
} // namespace Gambit::GUI
