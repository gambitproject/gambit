//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of analysis storage classesy
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "tinyxml.h"    // for XML parser for Load()


#include "libgambit/libgambit.h"
#include "analysis.h"
#include "gamedoc.h"

using namespace Gambit;



//=========================================================================
//                     class gbtAnalysisProfileList
//=========================================================================

void 
gbtAnalysisProfileList::Append(const MixedBehavProfile<double> &p_profile)
{
  m_behavProfiles.Append(p_profile);
  m_mixedProfiles.Append(MixedStrategyProfile<double>(p_profile));
}

void
gbtAnalysisProfileList::Append(const MixedStrategyProfile<double> &p_profile)
{
  m_mixedProfiles.Append(p_profile);
  if (m_doc->IsTree()) {
    m_behavProfiles.Append(MixedBehavProfile<double>(p_profile));
  }
}

void gbtAnalysisProfileList::BuildNfg(void)
{
  for (int i = 1; i <= m_behavProfiles.Length(); i++) {
    m_mixedProfiles.Append(MixedStrategyProfile<double>(m_behavProfiles[i]));
  }
}

int gbtAnalysisProfileList::NumProfiles(void) const
{
  if (m_doc->IsTree()) {
    return m_behavProfiles.Length();
  }
  else {
    return m_mixedProfiles.Length();
  }
}

void gbtAnalysisProfileList::Clear(void)
{
  m_behavProfiles = List<MixedBehavProfile<double> >();
  m_mixedProfiles = List<MixedStrategyProfile<double> >();
  m_current = 0;
}

//-------------------------------------------------------------------------
//         gbtAnalysisProfileList: Saving and loading profile lists 
//-------------------------------------------------------------------------

#include <wx/tokenzr.h>

static MixedStrategyProfile<double> 
TextToMixedProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  MixedStrategyProfile<double> profile(p_doc->GetGame());

  wxStringTokenizer tok(p_text, wxT(","));

  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = ToNumber(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  return profile;
}

static MixedBehavProfile<double> 
TextToBehavProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  MixedBehavProfile<double> profile(p_doc->GetGame());

  wxStringTokenizer tok(p_text, wxT(","));
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = ToNumber(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  return profile;
}

//
// Load a profile list from XML.  Pass a node pointing to an
// <analysis> entry in the workbook file
//
void gbtAnalysisProfileList::Load(TiXmlNode *p_analysis)
{
  Clear();

  TiXmlNode *description = p_analysis->FirstChild("description");
  if (description) {
    m_description = wxString(description->FirstChild()->Value(),
			     *wxConvCurrent);
  }

  if (m_doc->IsTree()) {
    for (TiXmlNode *node = p_analysis->FirstChild("profile"); node;
	 node = node->NextSiblingElement()) {
      Append(TextToBehavProfile(m_doc,
				wxString(node->FirstChild()->Value(),
					 *wxConvCurrent)));
    }
  }
  else {
    for (TiXmlNode *node = p_analysis->FirstChild("profile"); node;
	 node = node->NextSiblingElement()) {
      Append(TextToMixedProfile(m_doc,
				wxString(node->FirstChild()->Value(),
					 *wxConvCurrent)));
    }
  }
}

std::string 
gbtAnalysisProfileList::GetRealizProb(const GameNode &p_node,
				      int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  return ToText(m_behavProfiles[index].GetRealizProb(p_node),
			m_doc->GetStyle().NumDecimals());
}

std::string 
gbtAnalysisProfileList::GetBeliefProb(const GameNode &p_node,
				      int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer()) return "";

  if (m_behavProfiles[index].GetIsetProb(p_node->GetInfoset()) > Rational(0)) {
    return ToText(m_behavProfiles[index].GetBeliefProb(p_node),
		  m_doc->GetStyle().NumDecimals());
  }
  else {
    // We don't compute assessments yet!
    return "*";
  }
}

std::string 
gbtAnalysisProfileList::GetNodeValue(const GameNode &p_node,
				     int p_player, int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  return ToText(m_behavProfiles[index].GetNodeValue(p_node)[p_player], 
		m_doc->GetStyle().NumDecimals());
}

std::string 
gbtAnalysisProfileList::GetInfosetProb(const Gambit::GameNode &p_node,
				       int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer()) return "";

  return ToText(m_behavProfiles[index].GetIsetProb(p_node->GetInfoset()),
		m_doc->GetStyle().NumDecimals());
}

std::string 
gbtAnalysisProfileList::GetInfosetValue(const Gambit::GameNode &p_node,
					int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer() || p_node->GetPlayer()->IsChance())  return "";

  if (m_behavProfiles[index].GetIsetProb(p_node->GetInfoset()) > Rational(0)) {
    return ToText(m_behavProfiles[index].GetIsetValue(p_node->GetInfoset()),
		  m_doc->GetStyle().NumDecimals());
  }
  else {
    // In the absence of beliefs, this is not well-defined in general
    return "*";
  }
}

std::string 
gbtAnalysisProfileList::GetActionProb(const GameNode &p_node, int p_act,
				      int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    GameInfoset infoset = p_node->GetInfoset();
    return infoset->GetActionProb<std::string>(p_act);
  }

  if (!p_node->GetPlayer())  return "";
  
  const MixedBehavProfile<double> &profile = m_behavProfiles[index];

  if (!profile.IsDefinedAt(p_node->GetInfoset())) {
    return "*";
  }
  
  return ToText(profile.GetActionProb(p_node->GetInfoset()->GetAction(p_act)),
		m_doc->GetStyle().NumDecimals());
}

std::string 
gbtAnalysisProfileList::GetActionValue(const GameNode &p_node, int p_act,
				       int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer() || p_node->GetPlayer()->IsChance()) return "";

  if (m_behavProfiles[index].GetIsetProb(p_node->GetInfoset()) > Rational(0)) {
    return ToText(m_behavProfiles[index].GetActionValue(p_node->GetInfoset()->GetAction(p_act)),
		  m_doc->GetStyle().NumDecimals());
  }
  else  {
    // In the absence of beliefs, this is not well-defined
    return "*";
  }
}








