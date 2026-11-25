//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/treestyle.cc
// How a game is drawn, as plain data
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

#include <algorithm>
#include <array>
#include <iterator>

#include "treestyle.h"

namespace Gambit::GUI {

namespace {

const std::array<Rgb, 8> c_defaultPlayerColors = {
    Rgb{255, 0, 0}, Rgb{0, 0, 255},   Rgb{0, 128, 0}, Rgb{255, 128, 0},
    Rgb{0, 0, 64},  Rgb{128, 0, 255}, Rgb{64, 0, 0},  Rgb{255, 128, 255}};

const std::string c_nodeTokens[] = {"line", "box", "circle", "diamond", "dot"};
const std::string c_branchStyles[] = {"line", "forktine"};
const std::string c_branchLabels[] = {"horizontal", "rotated"};
const std::string c_infosetStyles[] = {"lines", "circles"};
const std::string c_nodeLabels[] = {"none",   "label",      "player",     "isetlabel",
                                    "isetid", "realizprob", "beliefprob", "value"};
const std::string c_branchLabelStyles[] = {"none", "label", "probs", "value"};

// The position of p_value in p_names, as an enumerator; p_fallback if it is not there
template <class E, std::size_t N>
E Find(const std::string &p_value, const std::string (&p_names)[N], E p_fallback)
{
  const auto iter = std::ranges::find(p_names, p_value);
  return iter == std::end(p_names) ? p_fallback : static_cast<E>(iter - std::begin(p_names));
}

std::uint8_t Component(int p_value)
{
  return static_cast<std::uint8_t>(std::clamp(p_value, 0, 255));
}

} // namespace

bool TreeStyle::operator==(const TreeStyle &p_other) const
{
  if (m_nodeSize != p_other.m_nodeSize || m_terminalSpacing != p_other.m_terminalSpacing ||
      m_chanceToken != p_other.m_chanceToken || m_playerToken != p_other.m_playerToken ||
      m_terminalToken != p_other.m_terminalToken || m_branchLength != p_other.m_branchLength ||
      m_tineLength != p_other.m_tineLength || m_branchStyle != p_other.m_branchStyle ||
      m_branchLabels != p_other.m_branchLabels || m_infosetJoin != p_other.m_infosetJoin ||
      m_nodeAboveLabel != p_other.m_nodeAboveLabel ||
      m_nodeBelowLabel != p_other.m_nodeBelowLabel ||
      m_branchAboveLabel != p_other.m_branchAboveLabel ||
      m_branchBelowLabel != p_other.m_branchBelowLabel || m_font != p_other.m_font ||
      m_chanceColor != p_other.m_chanceColor || m_terminalColor != p_other.m_terminalColor ||
      m_numDecimals != p_other.m_numDecimals) {
    return false;
  }
  // A player given its default colour explicitly is drawn as one never given a colour
  const auto sameColor = [this, &p_other](const auto &p_entry) {
    return GetPlayerColor(p_entry.first) == p_other.GetPlayerColor(p_entry.first);
  };
  return std::ranges::all_of(m_playerColors, sameColor) &&
         std::ranges::all_of(p_other.m_playerColors, sameColor);
}

Rgb TreeStyle::GetPlayerColor(int p_player) const
{
  if (const auto entry = m_playerColors.find(p_player); entry != m_playerColors.end()) {
    return entry->second;
  }
  return c_defaultPlayerColors[static_cast<std::size_t>(std::max(p_player, 1) - 1) %
                               c_defaultPlayerColors.size()];
}

Rgb TreeStyle::GetPlayerColor(const GamePlayer &p_player) const
{
  if (!p_player) {
    return m_terminalColor;
  }
  if (p_player->IsChance()) {
    return m_chanceColor;
  }
  return GetPlayerColor(p_player->GetNumber());
}

void TreeStyle::SetPlayerColor(int p_player, const Rgb &p_color)
{
  m_playerColors[p_player] = p_color;
}

void TreeStyle::Save(WorkspaceFile &p_workspace) const
{
  const auto color = [](int p_player, const Rgb &p_value) {
    return WorkspaceFile::Color{
        .player = p_player, .red = p_value.red, .green = p_value.green, .blue = p_value.blue};
  };
  p_workspace.colors = {color(-1, m_terminalColor), color(0, m_chanceColor)};
  for (const auto &[player, value] : m_playerColors) {
    p_workspace.colors.push_back(color(player, value));
  }

  if (m_font) {
    p_workspace.font = WorkspaceFile::Font{.size = m_font->size,
                                           .family = m_font->family,
                                           .face = m_font->face,
                                           .style = m_font->style,
                                           .weight = m_font->weight};
  }
  p_workspace.layout = WorkspaceFile::Layout{.node_size = m_nodeSize,
                                             .terminal_spacing = m_terminalSpacing,
                                             .chance_token = c_nodeTokens[m_chanceToken],
                                             .player_token = c_nodeTokens[m_playerToken],
                                             .terminal_token = c_nodeTokens[m_terminalToken],
                                             .branch_length = m_branchLength,
                                             .tine_length = m_tineLength,
                                             .branch_style = c_branchStyles[m_branchStyle],
                                             .branch_labels = c_branchLabels[m_branchLabels],
                                             .infoset_style = c_infosetStyles[m_infosetJoin]};
  p_workspace.labels =
      WorkspaceFile::Labels{.node_above = c_nodeLabels[m_nodeAboveLabel],
                            .node_below = c_nodeLabels[m_nodeBelowLabel],
                            .branch_above = c_branchLabelStyles[m_branchAboveLabel],
                            .branch_below = c_branchLabelStyles[m_branchBelowLabel]};
  p_workspace.decimals = m_numDecimals;
}

void TreeStyle::Load(const WorkspaceFile &p_workspace)
{
  for (const auto &color : p_workspace.colors) {
    const Rgb value{Component(color.red), Component(color.green), Component(color.blue)};
    if (color.player > 0) {
      SetPlayerColor(color.player, value);
    }
    else if (color.player == 0) {
      m_chanceColor = value;
    }
    else if (color.player == -1) {
      m_terminalColor = value;
    }
  }
  if (p_workspace.font) {
    const auto &font = *p_workspace.font;
    m_font = TreeFont{.size = font.size,
                      .family = font.family,
                      .face = font.face,
                      .style = font.style,
                      .weight = font.weight};
  }
  if (p_workspace.layout) {
    const auto &layout = *p_workspace.layout;
    m_nodeSize = layout.node_size;
    m_terminalSpacing = layout.terminal_spacing;
    m_chanceToken = Find(layout.chance_token, c_nodeTokens, m_chanceToken);
    m_playerToken = Find(layout.player_token, c_nodeTokens, m_playerToken);
    m_terminalToken = Find(layout.terminal_token, c_nodeTokens, m_terminalToken);
    m_branchLength = layout.branch_length;
    m_tineLength = layout.tine_length;
    m_branchStyle = Find(layout.branch_style, c_branchStyles, m_branchStyle);
    m_branchLabels = Find(layout.branch_labels, c_branchLabels, m_branchLabels);
    m_infosetJoin = Find(layout.infoset_style, c_infosetStyles, m_infosetJoin);
  }
  if (p_workspace.labels) {
    const auto &labels = *p_workspace.labels;
    m_nodeAboveLabel = Find(labels.node_above, c_nodeLabels, m_nodeAboveLabel);
    m_nodeBelowLabel = Find(labels.node_below, c_nodeLabels, m_nodeBelowLabel);
    m_branchAboveLabel = Find(labels.branch_above, c_branchLabelStyles, m_branchAboveLabel);
    m_branchBelowLabel = Find(labels.branch_below, c_branchLabelStyles, m_branchBelowLabel);
  }
  if (p_workspace.decimals) {
    m_numDecimals = *p_workspace.decimals;
  }
}

} // namespace Gambit::GUI
