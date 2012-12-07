//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/analysis.cc
// Declaration of analysis storage classes
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
#include <wx/tokenzr.h>

#include "tinyxml.h"    // for XML parser for Load()


#include "libgambit/libgambit.h"
#include "analysis.h"
#include "gamedoc.h"

using namespace Gambit;



//=========================================================================
//                     class gbtAnalysisProfileList
//=========================================================================

// Use anonymous namespace to make these helpers private
namespace {

class gbtNotNashException : public Exception {
public:
  virtual ~gbtNotNashException() throw() { }
  const char *what(void) const throw() 
  { return "Output line does not contain a Nash equilibrium"; }
};

template <class T> MixedStrategyProfile<T> 
OutputToMixedProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  MixedStrategyProfile<T> profile(p_doc->GetGame()->NewMixedStrategyProfile((T) 0.0));

  wxStringTokenizer tok(p_text, wxT(","));

  if (tok.GetNextToken() == wxT("NE")) {
    if (tok.CountTokens() == (unsigned int) profile.MixedProfileLength()) {
      for (int i = 1; i <= profile.MixedProfileLength(); i++) {
	profile[i] = lexical_cast<Rational>(std::string((const char *) tok.GetNextToken().mb_str()));
      }
      return profile;
    }
  }

  throw gbtNotNashException();
}

template <class T> MixedBehavProfile<T>
OutputToBehavProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  MixedBehavProfile<T> profile(p_doc->GetGame());

  wxStringTokenizer tok(p_text, wxT(","));

  if (tok.GetNextToken() == wxT("NE")) {
    if (tok.CountTokens() == (unsigned int) profile.Length()) {
      for (int i = 1; i <= profile.Length(); i++) {
	profile[i] = lexical_cast<Rational>(std::string((const char *) tok.GetNextToken().mb_str()));
      }
      return profile;
    }
  }

  throw gbtNotNashException();
}

}  // end anonymous namespace


template <class T> void
gbtAnalysisProfileList<T>::AddOutput(const wxString &p_output)
{
  try {
    if (m_isBehav) {
      MixedBehavProfile<T> profile(OutputToBehavProfile<T>(m_doc, p_output));
      m_behavProfiles.Append(profile);
      m_mixedProfiles.Append(profile.ToMixedProfile());
      m_current = m_behavProfiles.Length();
    }
    else {
      MixedStrategyProfile<T> profile(OutputToMixedProfile<T>(m_doc,
							      p_output));
      m_mixedProfiles.Append(profile);
      if (m_doc->IsTree()) {
	m_behavProfiles.Append(MixedBehavProfile<T>(profile));
      }
      m_current = m_mixedProfiles.Length();
    }
  }
  catch (gbtNotNashException &) { }
}

template <class T>
void gbtAnalysisProfileList<T>::BuildNfg(void)
{
  for (int i = 1; i <= m_behavProfiles.Length(); i++) {
    m_mixedProfiles.Append(m_behavProfiles[i].ToMixedProfile());
  }
}

template <class T>
int gbtAnalysisProfileList<T>::NumProfiles(void) const
{
  if (m_doc->IsTree()) {
    return m_behavProfiles.Length();
  }
  else {
    return m_mixedProfiles.Length();
  }
}

template <class T>
void gbtAnalysisProfileList<T>::Clear(void)
{
  m_behavProfiles = List<MixedBehavProfile<T> >();
  m_mixedProfiles = List<MixedStrategyProfile<T> >();
  m_current = 0;
}

//-------------------------------------------------------------------------
//         gbtAnalysisProfileList: Saving and loading profile lists 
//-------------------------------------------------------------------------

// Use anonymous namespace to make these helpers private
namespace {

template <class T> MixedStrategyProfile<T> 
TextToMixedProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  MixedStrategyProfile<T> profile(p_doc->GetGame()->NewMixedStrategyProfile((T) 0));

  wxStringTokenizer tok(p_text, wxT(","));

  for (int i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = lexical_cast<Rational>(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  return profile;
}

template <class T> MixedBehavProfile<T> 
TextToBehavProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  MixedBehavProfile<T> profile(p_doc->GetGame());

  wxStringTokenizer tok(p_text, wxT(","));
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = lexical_cast<Rational>(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  return profile;
}

} // end anonymous namespace

//
// Load a profile list from XML.  Pass a node pointing to an
// <analysis> entry in the workbook file
//
template <class T>
void gbtAnalysisProfileList<T>::Load(TiXmlNode *p_analysis)
{
  Clear();

  TiXmlNode *description = p_analysis->FirstChild("description");
  if (description) {
    m_description = wxString(description->FirstChild()->Value(),
			     *wxConvCurrent);
  }

  for (TiXmlNode *node = p_analysis->FirstChild("profile"); node;
       node = node->NextSiblingElement()) {
    const char *type = node->ToElement()->Attribute("type");
    if (!strcmp(type, "behav")) {
      MixedBehavProfile<T> profile = 
	TextToBehavProfile<T>(m_doc,
			      wxString(node->FirstChild()->Value(),
				       *wxConvCurrent));
      m_behavProfiles.Append(profile);
      m_isBehav = true;
      m_current = m_behavProfiles.Length();
    }
    else {
      MixedStrategyProfile<T> profile =
	TextToMixedProfile<T>(m_doc,
			      wxString(node->FirstChild()->Value(),
				       *wxConvCurrent));
      m_mixedProfiles.Append(profile);
      m_isBehav = false;
      m_current = m_mixedProfiles.Length();
    }
  }
}

template <class T> std::string
gbtAnalysisProfileList<T>::GetPayoff(int pl, int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  try {
    if (m_doc->IsTree()) {
      return lexical_cast<std::string>(m_behavProfiles[index].GetPayoff(pl),
		    m_doc->GetStyle().NumDecimals());
    }
    else {
      return lexical_cast<std::string>(m_mixedProfiles[index].GetPayoff(pl),
		    m_doc->GetStyle().NumDecimals());
    }
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string 
gbtAnalysisProfileList<T>::GetRealizProb(const GameNode &p_node,
					 int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  try {
    return lexical_cast<std::string>(m_behavProfiles[index].GetRealizProb(p_node),
		  m_doc->GetStyle().NumDecimals());
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string 
gbtAnalysisProfileList<T>::GetBeliefProb(const GameNode &p_node,
					 int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer()) return "";

  try {
    if (m_behavProfiles[index].GetInfosetProb(p_node->GetInfoset()) > Rational(0)) {
      return lexical_cast<std::string>(m_behavProfiles[index].GetBeliefProb(p_node),
		    m_doc->GetStyle().NumDecimals());
    }
    else {
      // We don't compute assessments yet!
      return "*";
    }
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string 
gbtAnalysisProfileList<T>::GetNodeValue(const GameNode &p_node,
					int p_player, int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  try {
    return lexical_cast<std::string>(m_behavProfiles[index].GetNodeValue(p_node)[p_player], 
		  m_doc->GetStyle().NumDecimals());
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string 
gbtAnalysisProfileList<T>::GetInfosetProb(const GameNode &p_node,
					  int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer()) return "";

  try {
    return lexical_cast<std::string>(m_behavProfiles[index].GetInfosetProb(p_node->GetInfoset()),
		  m_doc->GetStyle().NumDecimals());
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string 
gbtAnalysisProfileList<T>::GetInfosetValue(const GameNode &p_node,
					   int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer() || p_node->GetPlayer()->IsChance())  return "";

  try {
    if (m_behavProfiles[index].GetInfosetProb(p_node->GetInfoset()) > Rational(0)) {
      return lexical_cast<std::string>(m_behavProfiles[index].GetInfosetValue(p_node->GetInfoset()),
		    m_doc->GetStyle().NumDecimals());
    }
    else {
      // In the absence of beliefs, this is not well-defined in general
      return "*";
    }
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string 
gbtAnalysisProfileList<T>::GetActionProb(const GameNode &p_node, int p_act,
					 int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    GameInfoset infoset = p_node->GetInfoset();
    return infoset->GetActionProb(p_act, "");
  }

  if (!p_node->GetPlayer())  return "";
  
  try {
    const MixedBehavProfile<T> &profile = m_behavProfiles[index];

    if (!profile.IsDefinedAt(p_node->GetInfoset())) {
      return "*";
    }
  
    return lexical_cast<std::string>(profile.GetActionProb(p_node->GetInfoset()->GetAction(p_act)),
		  m_doc->GetStyle().NumDecimals());
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string
gbtAnalysisProfileList<T>::GetActionProb(int p_action, int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  try {
    const MixedBehavProfile<T> &profile = m_behavProfiles[index];

    if (!profile.IsDefinedAt(profile.GetGame()->GetAction(p_action)->GetInfoset())) {
      return "*";
    }
    
    return lexical_cast<std::string>(profile[p_action], m_doc->GetStyle().NumDecimals());
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string 
gbtAnalysisProfileList<T>::GetActionValue(const GameNode &p_node, int p_act,
					  int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer() || p_node->GetPlayer()->IsChance()) return "";
  
  try {
    if (m_behavProfiles[index].GetInfosetProb(p_node->GetInfoset()) > Rational(0)) {
      return lexical_cast<std::string>(m_behavProfiles[index].GetActionValue(p_node->GetInfoset()->GetAction(p_act)),
		    m_doc->GetStyle().NumDecimals());
    }
    else  {
      // In the absence of beliefs, this is not well-defined
      return "*";
    }
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string
gbtAnalysisProfileList<T>::GetStrategyProb(int p_strategy, int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  try {
    const MixedStrategyProfile<T> &profile = m_mixedProfiles[index];
    return lexical_cast<std::string>(profile[p_strategy], m_doc->GetStyle().NumDecimals());
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> std::string
gbtAnalysisProfileList<T>::GetStrategyValue(int p_strategy, int p_index) const
{
  int index = (p_index == -1) ? m_current : p_index;

  try {
    const MixedStrategyProfile<T> &profile = m_mixedProfiles[index];
    GameStrategy strategy = profile.GetGame()->GetStrategy(p_strategy);
    return lexical_cast<std::string>(profile.GetStrategyValue(strategy),
		  m_doc->GetStyle().NumDecimals());
  }
  catch (IndexException &) {
    return "";
  }
}

template <class T> void 
gbtAnalysisProfileList<T>::Save(std::ostream &p_file) const
{
  p_file << "<analysis type=\"list\">\n";

  p_file << "<description>\n";
  p_file << (const char *) m_description.mb_str() << "\n";
  p_file << "</description>\n";

  if (m_doc->IsTree()) {
    for (int j = 1; j <= NumProfiles(); j++) {
      const MixedBehavProfile<T> &behav = m_behavProfiles[j];
      p_file << "<profile type=\"behav\">\n";
      for (int k = 1; k <= behav.Length(); k++) {
	p_file << behav[k];
	if (k < behav.Length()) {
	  p_file << ",";
	}
	else {
	  p_file << "\n";
	}
      }
      p_file << "</profile>\n";
    }
  }
  else {
    for (int j = 1; j <= NumProfiles(); j++) {
      const MixedStrategyProfile<T> &mixed = m_mixedProfiles[j];
      p_file << "<profile type=\"mixed\">\n";
      for (int k = 1; k <= mixed.MixedProfileLength(); k++) {
	p_file << mixed[k];
	if (k < mixed.MixedProfileLength()) {
	  p_file << ",";
	}
	else {
	  p_file << "\n";
	}
      }
      p_file << "</profile>\n";
    }
  }
  p_file << "</analysis>\n";
}



// Explicit instantiations
template class gbtAnalysisProfileList<double>;
template class gbtAnalysisProfileList<Rational>;
