//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Display configuration class for the extensive form
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/spinctrl.h"

#include "gambit.h"
#include "treedraw.h"

//===========================================================================
//                 class TreeDrawSettings: Implementation
//===========================================================================

TreeDrawSettings::TreeDrawSettings(void)
  : m_nodeAboveFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_nodeBelowFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_nodeRightFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_branchAboveFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_branchBelowFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_chanceColor(*wxLIGHT_GREY), m_terminalColor(*wxBLACK)
{
  for (int pl = 0; pl < 8; pl++) {
    if (pl % 2 == 0) {
      m_playerColor[pl] = *wxRED;
    }
    else {
      m_playerColor[pl] = *wxBLUE;
    }
  }
  LoadOptions();
}

void TreeDrawSettings::SaveFont(const wxString &p_prefix, 
				wxConfig &p_config, const wxFont &p_font)
{
  p_config.Write(p_prefix + "Size", (long) p_font.GetPointSize());
  p_config.Write(p_prefix + "Family", (long) p_font.GetFamily());
  p_config.Write(p_prefix + "Face", p_font.GetFaceName());
  p_config.Write(p_prefix + "Style", (long) p_font.GetStyle());
  p_config.Write(p_prefix + "Weight", (long) p_font.GetWeight());
}

void TreeDrawSettings::LoadFont(const wxString &p_prefix,
				const wxConfig &p_config, wxFont &p_font)
{
  long size, family, style, weight;
  wxString face;
  p_config.Read(p_prefix + "Size", &size, 10);
  p_config.Read(p_prefix + "Family", &family, wxMODERN);
  p_config.Read(p_prefix + "Face", &face, "");
  p_config.Read(p_prefix + "Style", &style, wxNORMAL);
  p_config.Read(p_prefix + "Weight", &weight, wxNORMAL);

  p_font = *wxTheFontList->FindOrCreateFont(size, family, style, weight,
					    false, face);
}

void TreeDrawSettings::SaveColor(const wxString &p_prefix,
				 wxConfig &p_config, const wxColour &p_color)
{
  p_config.Write(p_prefix + "Red", (long) p_color.Red());
  p_config.Write(p_prefix + "Green", (long) p_color.Green());
  p_config.Write(p_prefix + "Blue", (long) p_color.Blue());
}

void TreeDrawSettings::LoadColor(const wxString &p_prefix,
				 const wxConfig &p_config, wxColour &p_color)
{
  long red, green, blue;
  p_config.Read(p_prefix + "Red", &red, p_color.Red());
  p_config.Read(p_prefix + "Green", &green, p_color.Green());
  p_config.Read(p_prefix + "Blue", &blue, p_color.Blue());
  p_color = wxColour(red, green, blue);
}

void TreeDrawSettings::SaveOptions(void) const
{
  wxConfig config("Gambit");

  config.Write("/TreeDisplay/NodeSize", (long) m_nodeSize);
  config.Write("/TreeDisplay/TerminalSpacing", (long) m_terminalSpacing);
  config.Write("/TreeDisplay/ChanceToken", (long) m_chanceToken);
  config.Write("/TreeDisplay/PlayerToken", (long) m_playerToken);
  config.Write("/TreeDisplay/TerminalToken", (long) m_terminalToken);
  config.Write("/TreeDisplay/RootReachable", (long) m_rootReachable);

  config.Write("/TreeDisplay/BranchLength", (long) m_branchLength);
  config.Write("/TreeDisplay/TineLength", (long) m_tineLength);
  config.Write("/TreeDisplay/BranchStyle", (long) m_branchStyle);
  config.Write("/TreeDisplay/BranchLabels", (long) m_branchLabels);

  config.Write("/TreeDisplay/InfosetConnect", (long) m_infosetConnect);
  config.Write("/TreeDisplay/InfosetJoin", (long) m_infosetJoin);

  config.Write("/TreeDisplay/SubgameStyle", (long) m_subgameStyle);

  config.Write("/TreeDisplay/NodeAboveLabel", (long) m_nodeAboveLabel);
  config.Write("/TreeDisplay/NodeBelowLabel", (long) m_nodeBelowLabel);
  config.Write("/TreeDisplay/NodeRightLabel", (long) m_nodeRightLabel);
  config.Write("/TreeDisplay/BranchAboveLabel", (long) m_branchAboveLabel);
  config.Write("/TreeDisplay/BranchBelowLabel", (long) m_branchBelowLabel);

  SaveFont("/TreeDisplay/NodeAboveFont", config, m_nodeAboveFont);
  SaveFont("/TreeDisplay/NodeBelowFont", config, m_nodeBelowFont);
  SaveFont("/TreeDisplay/NodeRightFont", config, m_nodeRightFont);
  SaveFont("/TreeDisplay/BranchAboveFont", config, m_branchAboveFont);
  SaveFont("/TreeDisplay/BranchBelowFont", config, m_branchBelowFont);

  SaveColor("/TreeDisplay/ChanceColor", config, m_chanceColor);
  SaveColor("/TreeDisplay/TerminalColor", config, m_terminalColor);
  for (int pl = 0; pl < 8; pl++) {
    SaveColor(wxString::Format("/TreeDisplay/Player%dColor", pl + 1),
	      config, m_playerColor[pl]);
  }

  config.Write("/TreeDisplay/NumDecimals", (long) m_numDecimals);

  config.Write("/NfgDisplay/OutcomeValues", (long) m_outcomeValues);

  SaveFont("/NfgDisplay/DataFont", config, m_dataFont);
  SaveFont("/NfgDisplay/LabelFont", config, m_labelFont);

}

void TreeDrawSettings::LoadOptions(void)
{
  wxConfig config("Gambit");
  config.Read("/TreeDisplay/NodeSize", &m_nodeSize, 20);
  config.Read("/TreeDisplay/TerminalSpacing", &m_terminalSpacing, 45);
  config.Read("/TreeDisplay/ChanceToken", &m_chanceToken, 2);
  config.Read("/TreeDisplay/PlayerToken", &m_playerToken, 2);
  config.Read("/TreeDisplay/TerminalToken", &m_terminalToken, 2);
  config.Read("/TreeDisplay/RootReachable", &m_rootReachable, 0);

  config.Read("/TreeDisplay/BranchLength", &m_branchLength, 60);
  config.Read("/TreeDisplay/TineLength", &m_tineLength, 20);
  config.Read("/TreeDisplay/BranchStyle", &m_branchStyle, 0);
  config.Read("/TreeDisplay/BranchLabels", &m_branchLabels, 0);

  config.Read("/TreeDisplay/InfosetConnect", &m_infosetConnect, 2);
  config.Read("/TreeDisplay/InfosetJoin", &m_infosetJoin, 1);

  config.Read("/TreeDisplay/SubgameStyle", &m_subgameStyle, 1);

  config.Read("/TreeDisplay/NodeAboveLabel", &m_nodeAboveLabel, 1);
  config.Read("/TreeDisplay/NodeBelowLabel", &m_nodeBelowLabel, 4);
  config.Read("/TreeDisplay/NodeRightLabel", &m_nodeRightLabel, 1);
  config.Read("/TreeDisplay/BranchAboveLabel", &m_branchAboveLabel, 1);
  config.Read("/TreeDisplay/BranchBelowLabel", &m_branchBelowLabel, 2);

  LoadFont("/TreeDisplay/NodeAboveFont", config, m_nodeAboveFont);
  LoadFont("/TreeDisplay/NodeBelowFont", config, m_nodeBelowFont);
  LoadFont("/TreeDisplay/NodeRightFont", config, m_nodeRightFont);
  LoadFont("/TreeDisplay/BranchAboveFont", config, m_branchAboveFont);
  LoadFont("/TreeDisplay/BranchBelowFont", config, m_branchBelowFont);

  LoadColor("/TreeDisplay/ChanceColor", config, m_chanceColor);
  LoadColor("/TreeDisplay/TerminalColor", config, m_terminalColor);
  for (int pl = 0; pl < 8; pl++) {
    LoadColor(wxString::Format("/TreeDisplay/Player%dColor", pl + 1),
	      config, m_playerColor[pl]);
  }

  config.Read("/TreeDisplay/NumDecimals", &m_numDecimals, 2);

  config.Read("/NfgDisplay/OutcomeValues", &m_outcomeValues, true);

  LoadFont("/NfgDisplay/DataFont", config, m_dataFont);
  LoadFont("/NfgDisplay/LabelFont", config, m_labelFont);
}



