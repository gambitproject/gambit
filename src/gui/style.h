//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/style.h
// Class to store settings related to graphical interface styling
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

#ifndef STYLE_H
#define STYLE_H

#include "gambit.h"
#include "core/tinyxml.h"

using namespace Gambit;

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

enum InfosetConnectStyle {
  GBT_INFOSET_CONNECT_NONE,
  GBT_INFOSET_CONNECT_SAMELEVEL,
  GBT_INFOSET_CONNECT_ALL
};

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

class wxFont;

class gbtStyle {
  // Node styling
  int m_nodeSize{10}, m_terminalSpacing{50};
  NodeTokenStyle m_chanceToken{GBT_NODE_TOKEN_DOT}, m_playerToken{GBT_NODE_TOKEN_DOT},
      m_terminalToken{GBT_NODE_TOKEN_DOT};
  bool m_rootReachable{false};

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

  // Fonts for legends
  wxFont m_font;

  // Colors for nodes
  wxColour m_chanceColor, m_terminalColor;
  mutable Gambit::Array<wxColour> m_playerColors;

  // Decimal places to display
  int m_numDecimals{4};

public:
  // Lifecycle
  gbtStyle();

  // Node styling
  int GetNodeSize() const { return m_nodeSize; }
  void SetNodeSize(int p_nodeSize) { m_nodeSize = p_nodeSize; }

  int TerminalSpacing() const { return m_terminalSpacing; }
  void SetTerminalSpacing(int p_spacing) { m_terminalSpacing = p_spacing; }

  NodeTokenStyle GetChanceToken() const { return m_chanceToken; }
  void SetChanceToken(NodeTokenStyle p_token) { m_chanceToken = p_token; }

  NodeTokenStyle GetPlayerToken() const { return m_playerToken; }
  void SetPlayerToken(NodeTokenStyle p_token) { m_playerToken = p_token; }

  NodeTokenStyle GetTerminalToken() const { return m_terminalToken; }
  void SetTerminalToken(NodeTokenStyle p_token) { m_terminalToken = p_token; }

  bool RootReachable() const { return m_rootReachable; }
  void SetRootReachable(bool p_reachable) { m_rootReachable = p_reachable; }

  // Branch styling
  int GetBranchLength() const { return m_branchLength; }
  void SetBranchLength(int p_length) { m_branchLength = p_length; }

  int GetTineLength() const { return m_tineLength; }
  void SetTineLength(int p_length) { m_tineLength = p_length; }

  BranchLineStyle GetBranchStyle() const { return m_branchStyle; }
  void SetBranchStyle(BranchLineStyle p_style) { m_branchStyle = p_style; }

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

  // Fonts
  const wxFont &GetFont() const { return m_font; }
  void SetFont(const wxFont &p_font) { m_font = p_font; }

  // Colors
  const wxColour &ChanceColor() const { return m_chanceColor; }
  void SetChanceColor(const wxColour &p_color) { m_chanceColor = p_color; }

  const wxColour &TerminalColor() const { return m_terminalColor; }
  void SetTerminalColor(const wxColour &p_color) { m_terminalColor = p_color; }

  const wxColour &GetPlayerColor(int pl) const;
  const wxColour &GetPlayerColor(const GamePlayer &) const;
  void SetPlayerColor(int pl, const wxColour &p_color) { m_playerColors[pl] = p_color; }

  // Decimals
  int NumDecimals() const { return m_numDecimals; }
  void SetNumDecimals(int p_decimals) { m_numDecimals = p_decimals; }

  // Reset to the "factory" defaults
  void SetDefaults();

  /// @name Reading and writing XML
  //@{
  /// Get the color settings as an XML entry
  std::string GetColorXML() const;
  /// Set the color settings from an XML entry
  void SetColorXML(TiXmlNode *p_node);

  /// Get the font settings as an XML entry
  std::string GetFontXML() const;
  /// Set the font settings from an XML entry
  void SetFontXML(TiXmlNode *p_node);

  /// Get the layout settings as an XML entry
  std::string GetLayoutXML() const;
  /// Set the layout settings from an XML entry
  void SetLayoutXML(TiXmlNode *p_node);

  /// Get the label settings as an XML entry
  std::string GetLabelXML() const;
  /// Set the label settings from an XML entry
  void SetLabelXML(TiXmlNode *p_node);
  //@}
};

#endif // STYLE_H
