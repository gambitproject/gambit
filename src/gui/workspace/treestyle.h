//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/treestyle.h
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

#ifndef GAMBIT_GUI_WORKSPACE_TREESTYLE_H
#define GAMBIT_GUI_WORKSPACE_TREESTYLE_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "games/game.h"
#include "games/workspace.h"

namespace Gambit::GUI {

/// A colour, as red, green and blue components
struct Rgb {
  std::uint8_t red{0}, green{0}, blue{0};

  constexpr Rgb() = default;
  constexpr Rgb(std::uint8_t p_red, std::uint8_t p_green, std::uint8_t p_blue)
    : red(p_red), green(p_green), blue(p_blue)
  {
  }

  bool operator==(const Rgb &) const = default;
};

/// A font, described as in the workspace file
struct TreeFont {
  int size{10};
  int family{0};
  std::string face;
  int style{0};
  int weight{0};

  bool operator==(const TreeFont &) const = default;
};

enum NodeTokenStyle {
  GBT_NODE_TOKEN_LINE,
  GBT_NODE_TOKEN_BOX,
  GBT_NODE_TOKEN_CIRCLE,
  GBT_NODE_TOKEN_DIAMOND,
  GBT_NODE_TOKEN_DOT
};

enum BranchLineStyle { GBT_BRANCH_STYLE_LINE, GBT_BRANCH_STYLE_FORKTINE };

enum BranchLabelOrientationStyle {
  GBT_BRANCH_LABEL_ORIENT_HORIZONTAL,
  GBT_BRANCH_LABEL_ORIENT_ROTATED
};

enum InfosetJoinStyle { GBT_INFOSET_JOIN_LINES, GBT_INFOSET_JOIN_CIRCLES };

enum NodeLabelStyle {
  GBT_NODE_LABEL_NOTHING,
  GBT_NODE_LABEL_LABEL,
  GBT_NODE_LABEL_PLAYER,
  GBT_NODE_LABEL_ISETLABEL,
  GBT_NODE_LABEL_ISETID,
  GBT_NODE_LABEL_REALIZPROB,
  GBT_NODE_LABEL_BELIEFPROB,
  GBT_NODE_LABEL_VALUE
};

enum BranchLabelStyle {
  GBT_BRANCH_LABEL_NOTHING,
  GBT_BRANCH_LABEL_LABEL,
  GBT_BRANCH_LABEL_PROBS,
  GBT_BRANCH_LABEL_VALUE
};

/// How a game is drawn: layout, labels, font, colours and the number of decimals shown
class TreeStyle {
  // Node styling
  int m_nodeSize{10}, m_terminalSpacing{50};
  NodeTokenStyle m_chanceToken{GBT_NODE_TOKEN_DOT}, m_playerToken{GBT_NODE_TOKEN_DOT},
      m_terminalToken{GBT_NODE_TOKEN_DOT};

  // Branch styling
  int m_branchLength{60}, m_tineLength{20};
  BranchLineStyle m_branchStyle{GBT_BRANCH_STYLE_FORKTINE};
  BranchLabelOrientationStyle m_branchLabels{GBT_BRANCH_LABEL_ORIENT_HORIZONTAL};

  // Information set styling
  InfosetJoinStyle m_infosetJoin{GBT_INFOSET_JOIN_CIRCLES};

  // Legend styling
  NodeLabelStyle m_nodeAboveLabel{GBT_NODE_LABEL_LABEL}, m_nodeBelowLabel{GBT_NODE_LABEL_ISETID};
  BranchLabelStyle m_branchAboveLabel{GBT_BRANCH_LABEL_LABEL},
      m_branchBelowLabel{GBT_BRANCH_LABEL_PROBS};

  // The font for labels; unset means the interface's default
  std::optional<TreeFont> m_font;

  // Colours; players without a colour of their own are drawn in a default one
  Rgb m_chanceColor{154, 205, 50}, m_terminalColor{0, 0, 0};
  // Colours given explicitly, by player number
  std::map<int, Rgb> m_playerColors;

  int m_numDecimals{4};

public:
  /// Whether the two styles draw a game the same way
  bool operator==(const TreeStyle &p_other) const;

  // Node styling
  int GetNodeSize() const { return m_nodeSize; }
  void SetNodeSize(int p_nodeSize) { m_nodeSize = p_nodeSize; }

  int GetTerminalSpacing() const { return m_terminalSpacing; }
  void SetTerminalSpacing(int p_spacing) { m_terminalSpacing = p_spacing; }

  NodeTokenStyle GetChanceToken() const { return m_chanceToken; }
  void SetChanceToken(NodeTokenStyle p_token) { m_chanceToken = p_token; }

  NodeTokenStyle GetPlayerToken() const { return m_playerToken; }
  void SetPlayerToken(NodeTokenStyle p_token) { m_playerToken = p_token; }

  NodeTokenStyle GetTerminalToken() const { return m_terminalToken; }
  void SetTerminalToken(NodeTokenStyle p_token) { m_terminalToken = p_token; }

  // Branch styling
  int GetBranchLength() const { return m_branchLength; }
  void SetBranchLength(int p_length) { m_branchLength = p_length; }

  int GetTineLength() const { return m_tineLength; }
  void SetTineLength(int p_length) { m_tineLength = p_length; }

  BranchLineStyle GetBranchStyle() const { return m_branchStyle; }
  void SetBranchStyle(BranchLineStyle p_style) { m_branchStyle = p_style; }

  /// The horizontal distance from a node to its children
  int GetNodeLevelLength() const
  {
    if (m_branchStyle == GBT_BRANCH_STYLE_LINE) {
      return m_nodeSize + m_branchLength;
    }
    return m_nodeSize + m_branchLength + m_tineLength;
  }

  BranchLabelOrientationStyle GetBranchLabels() const { return m_branchLabels; }
  void SetBranchLabels(BranchLabelOrientationStyle p_labels) { m_branchLabels = p_labels; }

  InfosetJoinStyle GetInfosetJoin() const { return m_infosetJoin; }
  void SetInfosetJoin(InfosetJoinStyle p_join) { m_infosetJoin = p_join; }

  // Legends
  NodeLabelStyle GetNodeAboveLabel() const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(NodeLabelStyle p_label) { m_nodeAboveLabel = p_label; }

  NodeLabelStyle GetNodeBelowLabel() const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(NodeLabelStyle p_label) { m_nodeBelowLabel = p_label; }

  BranchLabelStyle GetBranchAboveLabel() const { return m_branchAboveLabel; }
  void SetBranchAboveLabel(BranchLabelStyle p_label) { m_branchAboveLabel = p_label; }

  BranchLabelStyle GetBranchBelowLabel() const { return m_branchBelowLabel; }
  void SetBranchBelowLabel(BranchLabelStyle p_label) { m_branchBelowLabel = p_label; }

  // Font
  /// The font for labels, or nothing if the interface's default is used
  const std::optional<TreeFont> &GetFont() const { return m_font; }
  void SetFont(const TreeFont &p_font) { m_font = p_font; }

  // Colours
  Rgb ChanceColor() const { return m_chanceColor; }
  void SetChanceColor(const Rgb &p_color) { m_chanceColor = p_color; }

  Rgb TerminalColor() const { return m_terminalColor; }
  void SetTerminalColor(const Rgb &p_color) { m_terminalColor = p_color; }

  /// The colour of the player numbered `p_player` (from 1)
  Rgb GetPlayerColor(int p_player) const;
  /// The colour of a player; chance has the chance colour, and the null player the terminal one
  Rgb GetPlayerColor(const GamePlayer &p_player) const;
  /// Sets the colour of the player numbered `p_player` (from 1)
  void SetPlayerColor(int p_player, const Rgb &p_color);

  // Decimals
  int NumDecimals() const { return m_numDecimals; }
  void SetNumDecimals(int p_decimals) { m_numDecimals = p_decimals; }

  /// Writes the style into a workspace file
  void Save(WorkspaceFile &p_workspace) const;
  /// Reads the settings a workspace file gives, leaving the others as they are
  void Load(const WorkspaceFile &p_workspace);
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_WORKSPACE_TREESTYLE_H
