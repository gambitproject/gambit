//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Class to store settings related to extensive form layout and rendering
//

#ifndef TREEDRAW_H
#define TREEDRAW_H

#include "wx/font.h"
#include "wx/button.h"
#include "wx/config.h"

#include "base/base.h"

const int NODE_TOKEN_LINE = 0;
const int NODE_TOKEN_BOX = 1;
const int NODE_TOKEN_CIRCLE = 2;
const int NODE_TOKEN_DIAMOND = 3;

const int SHOWISET_NONE = 0;
const int SHOWISET_SAME = 1;
const int SHOWISET_ALL = 2;


class TreeDrawSettings {
private:
  // Node styling
  int m_nodeSize, m_terminalSpacing;
  int m_chanceToken, m_playerToken, m_terminalToken;
  bool m_rootReachable;

  // Branch styling
  int m_branchLength, m_tineLength;
  int m_branchStyle, m_branchLabels;

  // Information set styling
  int m_infosetConnect, m_infosetJoin;

  // Subgame styling
  int m_subgameStyle;

  // Legend styling
  int m_nodeAboveLabel, m_nodeBelowLabel, m_nodeRightLabel;
  int m_branchAboveLabel, m_branchBelowLabel;

  // Fonts for legends
  wxFont m_nodeAboveFont, m_nodeBelowFont, m_nodeRightFont;
  wxFont m_branchAboveFont, m_branchBelowFont;

  // Decimal places to display
  int m_numDecimals;
  
  static void LoadFont(const wxString &, const wxConfig &, wxFont &);
  static void SaveFont(const wxString &, wxConfig &, const wxFont &);
  
public:
  TreeDrawSettings(void);
  
  // Node styling
  int NodeSize(void) const { return m_nodeSize; }
  void SetNodeSize(int p_nodeSize) { m_nodeSize = p_nodeSize; }

  int TerminalSpacing(void) const { return m_terminalSpacing; }
  void SetTerminalSpacing(int p_spacing) { m_terminalSpacing = p_spacing; }

  int ChanceToken(void) const { return m_chanceToken; }
  void SetChanceToken(int p_token) { m_chanceToken = p_token; }

  int PlayerToken(void) const { return m_playerToken; }
  void SetPlayerToken(int p_token) { m_playerToken = p_token; }

  int TerminalToken(void) const { return m_terminalToken; }
  void SetTerminalToken(int p_token) { m_terminalToken = p_token; }

  bool RootReachable(void) const { return m_rootReachable; }
  void SetRootReachable(bool p_reachable) { m_rootReachable = p_reachable; }


  // Branch styling
  int BranchLength(void) const { return m_branchLength; }
  void SetBranchLength(int p_length) { m_branchLength = p_length; }

  int TineLength(void) const { return m_tineLength; }
  void SetTineLength(int p_length) { m_tineLength = p_length; }

  int BranchStyle(void) const { return m_branchStyle; }
  void SetBranchStyle(int p_style) { m_branchStyle = p_style; }

  int BranchLabels(void) const { return m_branchLabels; }
  void SetBranchLabels(int p_labels) { m_branchLabels = p_labels; }


  // Information set styling
  int InfosetConnect(void) const { return m_infosetConnect; }
  void SetInfosetConnect(int p_connect) { m_infosetConnect = p_connect; }

  int InfosetJoin(void) const { return m_infosetJoin; }
  void SetInfosetJoin(int p_join) { m_infosetJoin = p_join; }


  // Subgame styling
  int SubgameStyle(void) const { return m_subgameStyle; }
  void SetSubgameStyle(int p_style) { m_subgameStyle = p_style; }


  // Legends
  int NodeAboveLabel(void) const { return m_nodeAboveLabel; }
  void SetNodeAboveLabel(int p_label) { m_nodeAboveLabel = p_label; }

  int NodeBelowLabel(void) const { return m_nodeBelowLabel; }
  void SetNodeBelowLabel(int p_label) { m_nodeBelowLabel = p_label; }

  int NodeRightLabel(void) const { return m_nodeRightLabel; }
  void SetNodeRightLabel(int p_label) { m_nodeRightLabel = p_label; }

  int BranchAboveLabel(void) const { return m_branchAboveLabel; }
  void SetBranchAboveLabel(int p_label) { m_branchAboveLabel = p_label; }

  int BranchBelowLabel(void) const { return m_branchBelowLabel; }
  void SetBranchBelowLabel(int p_label) { m_branchBelowLabel = p_label; }

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

  // Decimals
  int NumDecimals(void) const { return m_numDecimals; }
  void SetNumDecimals(int p_decimals) { m_numDecimals = p_decimals; }

  // Read and write config files
  void SaveOptions(void) const;
  void LoadOptions(void);
};

#endif  // TREEDRAW_H
