//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Class to store user-configurable settings
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef PREFS_H
#define PREFS_H

#include "wx/font.h"
#include "wx/button.h"
#include "wx/config.h"

#include "base/base.h"

typedef enum {
  GBT_NODE_TOKEN_LINE = 0,
  GBT_NODE_TOKEN_BOX = 1,
  GBT_NODE_TOKEN_CIRCLE = 2,
  GBT_NODE_TOKEN_DIAMOND = 3
};

typedef enum {
  GBT_BRANCH_STYLE_LINE = 0,
  GBT_BRANCH_STYLE_FORKTINE = 1
};

typedef enum {
  GBT_BRANCH_LABEL_HORIZONTAL = 0,
  GBT_BRANCH_LABEL_ROTATED = 1
};

typedef enum {
  GBT_INFOSET_JOIN_LINES = 0,
  GBT_INFOSET_JOIN_CIRCLES = 1
};

typedef enum {
  GBT_INFOSET_CONNECT_NONE = 0,
  GBT_INFOSET_CONNECT_SAMELEVEL = 1,
  GBT_INFOSET_CONNECT_ALL = 2
};

typedef enum {
  GBT_SUBGAME_HIDDEN = 0,
  GBT_SUBGAME_ARC = 1
};

typedef enum {
  GBT_NODE_LABEL_NOTHING = 0,
  GBT_NODE_LABEL_LABEL = 1,
  GBT_NODE_LABEL_PLAYER = 2,
  GBT_NODE_LABEL_ISETLABEL = 3,
  GBT_NODE_LABEL_ISETID = 4,
  GBT_NODE_LABEL_OUTCOME = 5,
  GBT_NODE_LABEL_REALIZPROB = 6,
  GBT_NODE_LABEL_BELIEFPROB = 7,
  GBT_NODE_LABEL_VALUE = 8
};

typedef enum {
  GBT_OUTCOME_LABEL_PAYOFFS = 0,
  GBT_OUTCOME_LABEL_LABEL = 1
};

typedef enum {
  GBT_BRANCH_LABEL_NOTHING = 0,
  GBT_BRANCH_LABEL_LABEL = 1,
  GBT_BRANCH_LABEL_PROBS = 2,
  GBT_BRANCH_LABEL_VALUE = 3
};

class gbtPreferences {
private:
  // Node styling
  long m_nodeSize, m_terminalSpacing;
  long m_chanceToken, m_playerToken, m_terminalToken;
  bool m_rootReachable;

  // Branch styling
  long m_branchLength, m_tineLength;
  long m_branchStyle, m_branchLabels;

  // Information set styling
  long m_infosetConnect, m_infosetJoin;

  // Subgame styling
  long m_subgameStyle;

  // Legend styling
  long m_nodeAboveLabel, m_nodeBelowLabel;
  long m_outcomeLabel;
  long m_branchAboveLabel, m_branchBelowLabel;

  // Fonts for legends
  wxFont m_nodeAboveFont, m_nodeBelowFont, m_nodeRightFont;
  wxFont m_branchAboveFont, m_branchBelowFont;

  // Colors for nodes
  wxColour m_chanceColor, m_terminalColor, m_playerColor[8];

  // Decimal places to display
  long m_numDecimals;

  // Fonts for normal form display
  wxFont m_dataFont, m_labelFont;

  static void LoadFont(const wxString &, const wxConfig &, wxFont &);
  static void SaveFont(const wxString &, wxConfig &, const wxFont &);

  static void LoadColor(const wxString &, const wxConfig &, wxColour &);
  static void SaveColor(const wxString &, wxConfig &, const wxColour &);
  
public:
  // Lifecycle
  gbtPreferences(void);
  
  // Node styling
  long NodeSize(void) const { return m_nodeSize; }
  void SetNodeSize(long p_nodeSize) { m_nodeSize = p_nodeSize; }

  long TerminalSpacing(void) const { return m_terminalSpacing; }
  void SetTerminalSpacing(long p_spacing) { m_terminalSpacing = p_spacing; }

  long ChanceToken(void) const { return m_chanceToken; }
  void SetChanceToken(long p_token) { m_chanceToken = p_token; }

  long PlayerToken(void) const { return m_playerToken; }
  void SetPlayerToken(long p_token) { m_playerToken = p_token; }

  long TerminalToken(void) const { return m_terminalToken; }
  void SetTerminalToken(long p_token) { m_terminalToken = p_token; }

  bool RootReachable(void) const { return m_rootReachable; }
  void SetRootReachable(bool p_reachable) { m_rootReachable = p_reachable; }


  // Branch styling
  long BranchLength(void) const { return m_branchLength; }
  void SetBranchLength(long p_length) { m_branchLength = p_length; }

  long TineLength(void) const { return m_tineLength; }
  void SetTineLength(long p_length) { m_tineLength = p_length; }

  long BranchStyle(void) const { return m_branchStyle; }
  void SetBranchStyle(long p_style) { m_branchStyle = p_style; }

  long BranchLabels(void) const { return m_branchLabels; }
  void SetBranchLabels(long p_labels) { m_branchLabels = p_labels; }


  // Information set styling
  long InfosetConnect(void) const { return m_infosetConnect; }
  void SetInfosetConnect(long p_connect) { m_infosetConnect = p_connect; }

  long InfosetJoin(void) const { return m_infosetJoin; }
  void SetInfosetJoin(long p_join) { m_infosetJoin = p_join; }


  // Subgame styling
  long SubgameStyle(void) const { return m_subgameStyle; }
  void SetSubgameStyle(long p_style) { m_subgameStyle = p_style; }


  // Legends
  long NodeAboveLabel(void) const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(long p_label) { m_nodeAboveLabel = p_label; }

  long NodeBelowLabel(void) const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(long p_label) { m_nodeBelowLabel = p_label; }

  long OutcomeLabel(void) const { return m_outcomeLabel; }
  void SetOutcomeLabel(long p_label) { m_outcomeLabel = p_label; }

  long BranchAboveLabel(void) const { return m_branchAboveLabel; }
  void SetBranchAboveLabel(long p_label) { m_branchAboveLabel = p_label; }

  long BranchBelowLabel(void) const { return m_branchBelowLabel; }
  void SetBranchBelowLabel(long p_label) { m_branchBelowLabel = p_label; }

  // Fonts 
  const wxFont &NodeAboveFont(void) const { return m_nodeAboveFont; }
  void SetNodeAboveFont(const wxFont &p_font) { m_nodeAboveFont = p_font; }

  const wxFont &NodeBelowFont(void) const { return m_nodeBelowFont; }
  void SetNodeBelowFont(const wxFont &p_font) { m_nodeBelowFont = p_font; }

  const wxFont &NodeRightFont(void) const { return m_nodeRightFont; }
  void SetNodeRightFont(const wxFont &p_font) { m_nodeRightFont = p_font; }

  const wxFont &BranchAboveFont(void) const { return m_branchAboveFont; }
  void SetBranchAboveFont(const wxFont &p_font) { m_branchAboveFont = p_font; }

  const wxFont &BranchBelowFont(void) const { return m_branchBelowFont; }
  void SetBranchBelowFont(const wxFont &p_font) { m_branchBelowFont = p_font; }

  // Colors
  const wxColour &ChanceColor(void) const { return m_chanceColor; }
  void SetChanceColor(const wxColour &p_color) { m_chanceColor = p_color; }

  const wxColour &TerminalColor(void) const { return m_terminalColor; }
  void SetTerminalColor(const wxColour &p_color) { m_terminalColor = p_color; }

  const wxColour &PlayerColor(long pl) const { return m_playerColor[pl-1]; }
  void SetPlayerColor(long pl, const wxColour &p_color)
  { m_playerColor[pl-1] = p_color; }

  // Decimals
  long NumDecimals(void) const { return m_numDecimals; }
  void SetNumDecimals(long p_decimals) { m_numDecimals = p_decimals; }

  // Normal form display
  void SetDataFont(const wxFont &p_font) { m_dataFont = p_font; }
  const wxFont &GetDataFont(void) const { return m_dataFont; }

  void SetLabelFont(const wxFont &p_font) { m_labelFont = p_font; }
  const wxFont &GetLabelFont(void) const { return m_labelFont; }


  // Read and write config files
  void SaveOptions(void) const;
  void LoadOptions(void);
};

#endif  // PREFS_H
