//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Display configuration class for the extensive form
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
    m_branchBelowFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL))
{
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
  int size, family, style, weight;
  wxString face;
  p_config.Read(p_prefix + "Size", &size, 10);
  p_config.Read(p_prefix + "Family", &family, wxMODERN);
  p_config.Read(p_prefix + "Face", &face, "");
  p_config.Read(p_prefix + "Style", &style, wxNORMAL);
  p_config.Read(p_prefix + "Weight", &weight, wxNORMAL);

  p_font = *wxTheFontList->FindOrCreateFont(size, family, style, weight,
					    false, face);
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

  config.Write("/TreeDisplay/NumDecimals", (long) m_numDecimals);
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

  config.Read("/TreeDisplay/NumDecimals", &m_numDecimals, 2);
}



