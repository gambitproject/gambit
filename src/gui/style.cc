//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/config.h>

#include "style.h"

//===========================================================================
//                     class gbtStyle: Implementation
//===========================================================================

gbtStyle::gbtStyle(void)
  : m_font(wxFont(10, wxSWISS, wxNORMAL, wxBOLD))
{
  SetDefaults();
}

static wxColour s_defaultColors[8] = {
  wxColour(255, 0, 0),
  wxColour(0, 0, 255),
  wxColour(0, 128, 0),
  wxColour(255, 128, 0),
  wxColour(0, 0, 64),
  wxColour(128, 0, 255),
  wxColour(64, 0, 0),
  wxColour(255, 128, 255)
};

//!
//! Gets the player color assigned to player number 'pl'.
//! If this is the first request for that player's color, create the
//! default one.
//!
const wxColour &gbtStyle::GetPlayerColor(int pl) const
{
  while (pl > m_playerColors.Length()) {
    m_playerColors.Append(s_defaultColors[m_playerColors.Length() % 8]);
  }

  return m_playerColors[pl];
}

void gbtStyle::SetDefaults(void)
{
  m_nodeSize = 10;
  m_terminalSpacing = 50;
  m_chanceToken = GBT_NODE_TOKEN_DOT;
  m_playerToken = GBT_NODE_TOKEN_DOT;
  m_terminalToken = GBT_NODE_TOKEN_DOT;
  m_rootReachable = 0;
  m_branchLength = 60;
  m_tineLength = 20;
  m_branchStyle = GBT_BRANCH_STYLE_FORKTINE;
  m_branchLabels = GBT_BRANCH_LABEL_HORIZONTAL;
  m_infosetConnect = GBT_INFOSET_CONNECT_ALL;
  m_infosetJoin = GBT_INFOSET_JOIN_CIRCLES;
  m_nodeAboveLabel = GBT_NODE_LABEL_LABEL;
  m_nodeBelowLabel = GBT_NODE_LABEL_ISETID;
  m_branchAboveLabel = GBT_BRANCH_LABEL_LABEL;
  m_branchBelowLabel = GBT_BRANCH_LABEL_PROBS;
  m_numDecimals = 4;

  m_font = wxFont(10, wxSWISS, wxNORMAL, wxBOLD);

  m_chanceColor = wxColour(154, 205, 50);
  m_terminalColor = *wxBLACK;
  for (int pl = 1; pl <= m_playerColors.Length(); pl++) {
    m_playerColors[pl] = s_defaultColors[(pl-1)%8];
  }
}

std::string gbtStyle::GetColorXML(void) const
{
  std::ostringstream s;

  s <<"<colors>\n";
  s <<"<player id=\"-1\" ";
  s <<"red=\"" << ((int) m_terminalColor.Red()) << "\" ";
  s <<"green=\"" << ((int) m_terminalColor.Green()) << "\" ";
  s <<"blue=\"" << ((int) m_terminalColor.Blue()) << "\" ";
  s <<"/>\n";

  s <<"<player id=\"0\" ";
  s <<"red=\"" << ((int) m_chanceColor.Red()) << "\" ";
  s <<"green=\"" << ((int) m_chanceColor.Green()) << "\" ";
  s <<"blue=\"" << ((int) m_chanceColor.Blue()) << "\" ";
  s <<"/>\n";

  for (int pl = 1; pl <= m_playerColors.Length(); pl++) {
    s <<"<player id=\"" << pl << "\" ";
    s <<"red=\"" << ((int) m_playerColors[pl].Red()) << "\" ";
    s <<"green=\"" << ((int) m_playerColors[pl].Green()) << "\" ";
    s <<"blue=\"" << ((int) m_playerColors[pl].Blue()) << "\" "; 
    s <<"/>\n";
  }

  s <<"</colors>\n";

  return s.str();
}

void gbtStyle::SetColorXML(TiXmlNode *p_colors)
{
  for (TiXmlNode *node = p_colors->FirstChild(); node;
       node = node->NextSiblingElement()) {
    int id = -2;
    node->ToElement()->QueryIntAttribute("id", &id);

    int red = 0, green = 0, blue = 0;
    node->ToElement()->QueryIntAttribute("red", &red);
    node->ToElement()->QueryIntAttribute("green", &green);
    node->ToElement()->QueryIntAttribute("blue", &blue);
    if (id > 0) {
      // This call ensures that the player appears in the color table
      GetPlayerColor(id);
      SetPlayerColor(id, wxColour(red, green, blue));
    }
    else if (id == 0) {
      SetChanceColor(wxColour(red, green, blue));
    }
    else if (id == -1) {
      SetTerminalColor(wxColour(red, green, blue));
    }
  }
}

std::string gbtStyle::GetFontXML(void) const
{
  std::ostringstream s;

  s << "<font size=\"" << (int) m_font.GetPointSize() << "\" ";
  s << "family=\"" << (int) m_font.GetFamily() << "\" ";
  s << "face=\"" << (const char *) m_font.GetFaceName().mb_str() << "\" ";
  s << "style=\"" << (int) m_font.GetStyle() << "\" ";
  s << "weight=\"" << (int) m_font.GetWeight() << "\" ";
  s << "/>\n";

  return s.str();
}

void gbtStyle::SetFontXML(TiXmlNode *p_font)
{
  int size, family, style, weight;
  p_font->ToElement()->QueryIntAttribute("size", &size);
  p_font->ToElement()->QueryIntAttribute("family", &family);
  p_font->ToElement()->QueryIntAttribute("style", &style);
  p_font->ToElement()->QueryIntAttribute("weight", &weight);
  SetFont(wxFont(size, family, style, weight, false,
		 wxString(p_font->ToElement()->Attribute("face"),
			  *wxConvCurrent)));
}

std::string gbtStyle::GetLayoutXML(void) const
{
  std::ostringstream s;
  s << "<autolayout>\n";

  s << "<nodes size=\"" << m_nodeSize << "\" spacing=\"" << m_terminalSpacing << "\" ";
  std::string nodeTokens[] = { "line", "box", "circle", "diamond", "dot" };
  s << "chance=\"" << nodeTokens[m_chanceToken] << "\" ";
  s << "player=\"" << nodeTokens[m_playerToken] << "\" ";
  s << "terminal=\"" << nodeTokens[m_terminalToken] << "\"/>\n";

  s << "<branches size=\"" << m_branchLength << "\" tine=\"" << m_tineLength << "\" ";
  std::string branchStyles[] = { "line", "forktine" };
  s << "branch=\"" << branchStyles[m_branchStyle] << "\" ";
  std::string branchLabels[] = { "horizontal", "rotated" };
  s << "labels=\"" << branchLabels[m_branchLabels] << "\"/>\n";
  
  std::string infosetConnect[] = { "none", "same", "all" };
  s << "<infosets connect=\"" << infosetConnect[m_infosetConnect] << "\" ";
  std::string infosetStyle[] = { "lines", "circles" };
  s << "style=\"" << infosetStyle[m_infosetJoin] << "\"/>\n";
  
  s << "</autolayout>\n";
  return s.str();
}

void gbtStyle::SetLayoutXML(TiXmlNode *p_node)
{
  TiXmlNode *nodes = p_node->FirstChild("nodes");
  if (nodes) {
    nodes->ToElement()->QueryIntAttribute("size", &m_nodeSize);
    nodes->ToElement()->QueryIntAttribute("spacing", &m_terminalSpacing);
    const char *chance = nodes->ToElement()->Attribute("chance");
    if (chance) {
      std::string s = chance;
      if (s == "line")           m_chanceToken = GBT_NODE_TOKEN_LINE;
      else if (s == "box")       m_chanceToken = GBT_NODE_TOKEN_BOX;
      else if (s == "circle")    m_chanceToken = GBT_NODE_TOKEN_CIRCLE;
      else if (s == "diamond")   m_chanceToken = GBT_NODE_TOKEN_DIAMOND;
      else if (s == "dot")       m_chanceToken = GBT_NODE_TOKEN_DOT;
    }

    const char *player = nodes->ToElement()->Attribute("player");
    if (player) {
      std::string s = player;
      if (s == "line")           m_playerToken = GBT_NODE_TOKEN_LINE;
      else if (s == "box")       m_playerToken = GBT_NODE_TOKEN_BOX;
      else if (s == "circle")    m_playerToken = GBT_NODE_TOKEN_CIRCLE;
      else if (s == "diamond")   m_playerToken = GBT_NODE_TOKEN_DIAMOND;
      else if (s == "dot")       m_playerToken = GBT_NODE_TOKEN_DOT;
    }

    const char *terminal = nodes->ToElement()->Attribute("terminal");
    if (terminal) {
      std::string s = terminal;
      if (s == "line")           m_terminalToken = GBT_NODE_TOKEN_LINE;
      else if (s == "box")       m_terminalToken = GBT_NODE_TOKEN_BOX;
      else if (s == "circle")    m_terminalToken = GBT_NODE_TOKEN_CIRCLE;
      else if (s == "diamond")   m_terminalToken = GBT_NODE_TOKEN_DIAMOND;
      else if (s == "dot")       m_terminalToken = GBT_NODE_TOKEN_DOT;
    }
  }

  TiXmlNode *branches = p_node->FirstChild("branches");
  if (branches) {
    branches->ToElement()->QueryIntAttribute("size", &m_branchLength);
    branches->ToElement()->QueryIntAttribute("tine", &m_tineLength);

    const char *branch = branches->ToElement()->Attribute("branch");
    if (branch) {
      std::string s = branch;
      if (s == "line")           m_branchStyle = GBT_BRANCH_STYLE_LINE;
      else if (s == "forktine")  m_branchStyle = GBT_BRANCH_STYLE_FORKTINE;
    }
   
    const char *labels = branches->ToElement()->Attribute("labels");
    if (labels) {
      std::string s = labels;
      if (s == "horizontal")     m_branchLabels = GBT_BRANCH_LABEL_HORIZONTAL;
      else if (s == "rotated")   m_branchLabels = GBT_BRANCH_LABEL_ROTATED;
    }
  }

  TiXmlNode *infosets = p_node->FirstChild("infosets");
  if (infosets) {
    const char *connect = infosets->ToElement()->Attribute("connect");
    if (connect) {
      std::string s = connect;
      if (s == "none")      m_infosetConnect = GBT_INFOSET_CONNECT_NONE;
      else if (s == "same") m_infosetConnect = GBT_INFOSET_CONNECT_SAMELEVEL;
      else if (s == "all")  m_infosetConnect = GBT_INFOSET_CONNECT_ALL;
    }

    const char *style = infosets->ToElement()->Attribute("style");
    if (style) {
      std::string s = style;
      if (s == "lines")         m_infosetJoin = GBT_INFOSET_JOIN_LINES;
      else if (s == "circles")  m_infosetJoin = GBT_INFOSET_JOIN_CIRCLES;
    }
  }
}

std::string gbtStyle::GetLabelXML(void) const
{
  std::ostringstream s;
  s << "<labels ";
  std::string nodeLabels[] = { "none", "label", "player",
			       "isetlabel", "isetid",
			       "realizprob", "beliefprob", "value" };
  s << "abovenode=\"" << nodeLabels[m_nodeAboveLabel] << "\" ";
  s << "belownode=\"" << nodeLabels[m_nodeBelowLabel] << "\" ";

  std::string branchLabels[] = { "none", "label", "probs", "value" };
  s << "abovebranch=\"" << branchLabels[m_branchAboveLabel] << "\" ";
  s << "belowbranch=\"" << branchLabels[m_branchBelowLabel] << "\" ";

  s << "/>\n";
  return s.str();
}

void gbtStyle::SetLabelXML(TiXmlNode *p_node)
{
  const char *abovenode = p_node->ToElement()->Attribute("abovenode");
  if (abovenode) {
    std::string s = abovenode;
    if (s == "none")             m_nodeAboveLabel = GBT_NODE_LABEL_NOTHING;
    else if (s == "label")       m_nodeAboveLabel = GBT_NODE_LABEL_LABEL;
    else if (s == "player")      m_nodeAboveLabel = GBT_NODE_LABEL_PLAYER;
    else if (s == "isetlabel")   m_nodeAboveLabel = GBT_NODE_LABEL_ISETLABEL;
    else if (s == "isetid")      m_nodeAboveLabel = GBT_NODE_LABEL_ISETID;
    else if (s == "realizprob")  m_nodeAboveLabel = GBT_NODE_LABEL_REALIZPROB;
    else if (s == "beliefprob")  m_nodeAboveLabel = GBT_NODE_LABEL_BELIEFPROB;
    else if (s == "value")       m_nodeAboveLabel = GBT_NODE_LABEL_VALUE;
  }

  const char *belownode = p_node->ToElement()->Attribute("belownode");
  if (belownode) {
    std::string s = belownode;
    if (s == "none")             m_nodeBelowLabel = GBT_NODE_LABEL_NOTHING;
    else if (s == "label")       m_nodeBelowLabel = GBT_NODE_LABEL_LABEL;
    else if (s == "player")      m_nodeBelowLabel = GBT_NODE_LABEL_PLAYER;
    else if (s == "isetlabel")   m_nodeBelowLabel = GBT_NODE_LABEL_ISETLABEL;
    else if (s == "isetid")      m_nodeBelowLabel = GBT_NODE_LABEL_ISETID;
    else if (s == "realizprob")  m_nodeBelowLabel = GBT_NODE_LABEL_REALIZPROB;
    else if (s == "beliefprob")  m_nodeBelowLabel = GBT_NODE_LABEL_BELIEFPROB;
    else if (s == "value")       m_nodeBelowLabel = GBT_NODE_LABEL_VALUE;
  }

  const char *abovebranch = p_node->ToElement()->Attribute("abovebranch");
  if (abovebranch) {
    std::string s = abovebranch;
    if (s == "none")             m_branchAboveLabel = GBT_BRANCH_LABEL_NOTHING;
    else if (s == "label")       m_branchAboveLabel = GBT_BRANCH_LABEL_LABEL;
    else if (s == "probs")       m_branchAboveLabel = GBT_BRANCH_LABEL_PROBS;
    else if (s == "value")       m_branchAboveLabel = GBT_BRANCH_LABEL_VALUE;
  }

  const char *belowbranch = p_node->ToElement()->Attribute("belowbranch");
  if (belowbranch) {
    std::string s = belowbranch;
    if (s == "none")             m_branchBelowLabel = GBT_BRANCH_LABEL_NOTHING;
    else if (s == "label")       m_branchBelowLabel = GBT_BRANCH_LABEL_LABEL;
    else if (s == "probs")       m_branchBelowLabel = GBT_BRANCH_LABEL_PROBS;
    else if (s == "value")       m_branchBelowLabel = GBT_BRANCH_LABEL_VALUE;
  }

}
