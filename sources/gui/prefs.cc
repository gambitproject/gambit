//
// $Source$
// $Revision$
// $Date$
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/spinctrl.h"

#include "prefs.h"

//===========================================================================
//                 class gbtPreferences: Implementation
//===========================================================================

gbtPreferences::gbtPreferences(void)
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

void gbtPreferences::SaveFont(const wxString &p_prefix, 
				wxConfig &p_config, const wxFont &p_font)
{
  p_config.Write(p_prefix + wxT("Size"), (long) p_font.GetPointSize());
  p_config.Write(p_prefix + wxT("Family"), (long) p_font.GetFamily());
  p_config.Write(p_prefix + wxT("Face"), p_font.GetFaceName());
  p_config.Write(p_prefix + wxT("Style"), (long) p_font.GetStyle());
  p_config.Write(p_prefix + wxT("Weight"), (long) p_font.GetWeight());
}

void gbtPreferences::LoadFont(const wxString &p_prefix,
				const wxConfig &p_config, wxFont &p_font)
{
  long size, family, style, weight;
  wxString face;
  p_config.Read(p_prefix + wxT("Size"), &size, 10);
  p_config.Read(p_prefix + wxT("Family"), &family, wxMODERN);
  p_config.Read(p_prefix + wxT("Face"), &face, wxT(""));
  p_config.Read(p_prefix + wxT("Style"), &style, wxNORMAL);
  p_config.Read(p_prefix + wxT("Weight"), &weight, wxNORMAL);

  p_font = *wxTheFontList->FindOrCreateFont(size, family, style, weight,
					    false, face);
}

void gbtPreferences::SaveColor(const wxString &p_prefix,
				 wxConfig &p_config, const wxColour &p_color)
{
  p_config.Write(p_prefix + wxT("Red"), (long) p_color.Red());
  p_config.Write(p_prefix + wxT("Green"), (long) p_color.Green());
  p_config.Write(p_prefix + wxT("Blue"), (long) p_color.Blue());
}

void gbtPreferences::LoadColor(const wxString &p_prefix,
				 const wxConfig &p_config, wxColour &p_color)
{
  long red, green, blue;
  p_config.Read(p_prefix + wxT("Red"), &red, p_color.Red());
  p_config.Read(p_prefix + wxT("Green"), &green, p_color.Green());
  p_config.Read(p_prefix + wxT("Blue"), &blue, p_color.Blue());
  p_color = wxColour(red, green, blue);
}

void gbtPreferences::SaveOptions(void) const
{
  wxConfig config(wxT("Gambit"));

  config.Write(wxT("/TreeDisplay/NodeSize"), (long) m_nodeSize);
  config.Write(wxT("/TreeDisplay/TerminalSpacing"), (long) m_terminalSpacing);
  config.Write(wxT("/TreeDisplay/ChanceToken"), (long) m_chanceToken);
  config.Write(wxT("/TreeDisplay/PlayerToken"), (long) m_playerToken);
  config.Write(wxT("/TreeDisplay/TerminalToken"), (long) m_terminalToken);
  config.Write(wxT("/TreeDisplay/RootReachable"), (long) m_rootReachable);

  config.Write(wxT("/TreeDisplay/BranchLength"), (long) m_branchLength);
  config.Write(wxT("/TreeDisplay/TineLength"), (long) m_tineLength);
  config.Write(wxT("/TreeDisplay/BranchStyle"), (long) m_branchStyle);
  config.Write(wxT("/TreeDisplay/BranchLabels"), (long) m_branchLabels);

  config.Write(wxT("/TreeDisplay/InfosetConnect"), (long) m_infosetConnect);
  config.Write(wxT("/TreeDisplay/InfosetJoin"), (long) m_infosetJoin);

  config.Write(wxT("/TreeDisplay/SubgameStyle"), (long) m_subgameStyle);

  config.Write(wxT("/TreeDisplay/ProfileStyle"), (long) m_profileStyle);

  config.Write(wxT("/TreeDisplay/NodeAboveLabel"), (long) m_nodeAboveLabel);
  config.Write(wxT("/TreeDisplay/NodeBelowLabel"), (long) m_nodeBelowLabel);
  config.Write(wxT("/TreeDisplay/OutcomeLabel"), (long) m_outcomeLabel);
  config.Write(wxT("/TreeDisplay/BranchAboveLabel"),
	       (long) m_branchAboveLabel);
  config.Write(wxT("/TreeDisplay/BranchBelowLabel"),
	       (long) m_branchBelowLabel);

  SaveFont(wxT("/TreeDisplay/NodeAboveFont"), config, m_nodeAboveFont);
  SaveFont(wxT("/TreeDisplay/NodeBelowFont"), config, m_nodeBelowFont);
  SaveFont(wxT("/TreeDisplay/NodeRightFont"), config, m_nodeRightFont);
  SaveFont(wxT("/TreeDisplay/BranchAboveFont"), config, m_branchAboveFont);
  SaveFont(wxT("/TreeDisplay/BranchBelowFont"), config, m_branchBelowFont);

  SaveColor(wxT("/TreeDisplay/ChanceColor"), config, m_chanceColor);
  SaveColor(wxT("/TreeDisplay/TerminalColor"), config, m_terminalColor);
  for (int pl = 0; pl < 8; pl++) {
    SaveColor(wxString::Format(wxT("/TreeDisplay/Player%dColor"), pl + 1),
	      config, m_playerColor[pl]);
  }

  config.Write(wxT("/TreeDisplay/NumDecimals"), (long) m_numDecimals);

  SaveFont(wxT("/NfgDisplay/DataFont"), config, m_dataFont);
  SaveFont(wxT("/NfgDisplay/LabelFont"), config, m_labelFont);

}

void gbtPreferences::LoadOptions(void)
{
  wxConfig config(wxT("Gambit"));
  config.Read(wxT("/TreeDisplay/NodeSize"), &m_nodeSize, 20);
  config.Read(wxT("/TreeDisplay/TerminalSpacing"), &m_terminalSpacing, 45);
  config.Read(wxT("/TreeDisplay/ChanceToken"), &m_chanceToken, 2);
  config.Read(wxT("/TreeDisplay/PlayerToken"), &m_playerToken, 2);
  config.Read(wxT("/TreeDisplay/TerminalToken"), &m_terminalToken, 2);
  config.Read(wxT("/TreeDisplay/RootReachable"), &m_rootReachable, 0);

  config.Read(wxT("/TreeDisplay/BranchLength"), &m_branchLength, 60);
  config.Read(wxT("/TreeDisplay/TineLength"), &m_tineLength, 20);
  config.Read(wxT("/TreeDisplay/BranchStyle"), &m_branchStyle, 0);
  config.Read(wxT("/TreeDisplay/BranchLabels"), &m_branchLabels, 0);

  config.Read(wxT("/TreeDisplay/InfosetConnect"), &m_infosetConnect, 2);
  config.Read(wxT("/TreeDisplay/InfosetJoin"), &m_infosetJoin, 1);

  config.Read(wxT("/TreeDisplay/SubgameStyle"), &m_subgameStyle, 0);

  config.Read(wxT("/TreeDisplay/ProfileStyle"), &m_profileStyle, 
	      GBT_PROFILES_MYERSON);

  config.Read(wxT("/TreeDisplay/NodeAboveLabel"), &m_nodeAboveLabel, 1);
  config.Read(wxT("/TreeDisplay/NodeBelowLabel"), &m_nodeBelowLabel, 4);
  config.Read(wxT("/TreeDisplay/OutcomeLabel"), &m_outcomeLabel, 0);
  config.Read(wxT("/TreeDisplay/BranchAboveLabel"), &m_branchAboveLabel, 1);
  config.Read(wxT("/TreeDisplay/BranchBelowLabel"), &m_branchBelowLabel, 2);

  LoadFont(wxT("/TreeDisplay/NodeAboveFont"), config, m_nodeAboveFont);
  LoadFont(wxT("/TreeDisplay/NodeBelowFont"), config, m_nodeBelowFont);
  LoadFont(wxT("/TreeDisplay/NodeRightFont"), config, m_nodeRightFont);
  LoadFont(wxT("/TreeDisplay/BranchAboveFont"), config, m_branchAboveFont);
  LoadFont(wxT("/TreeDisplay/BranchBelowFont"), config, m_branchBelowFont);

  LoadColor(wxT("/TreeDisplay/ChanceColor"), config, m_chanceColor);
  LoadColor(wxT("/TreeDisplay/TerminalColor"), config, m_terminalColor);
  for (int pl = 0; pl < 8; pl++) {
    LoadColor(wxString::Format(wxT("/TreeDisplay/Player%dColor"), pl + 1),
	      config, m_playerColor[pl]);
  }

  config.Read(wxT("/TreeDisplay/NumDecimals"), &m_numDecimals, 2);

  LoadFont(wxT("/NfgDisplay/DataFont"), config, m_dataFont);
  LoadFont(wxT("/NfgDisplay/LabelFont"), config, m_labelFont);
}



