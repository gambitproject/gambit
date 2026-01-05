//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#endif // WX_PRECOMP
#include <wx/tokenzr.h>

#include "gambit.h"
#include "core/tinyxml.h" // for XML parser for Load()

#include "analysis.h"
#include "gamedoc.h"

namespace Gambit::GUI {
//=========================================================================
//                     class AnalysisProfileList
//=========================================================================

// Use anonymous namespace to make these helpers private
namespace {

class NotNashException final : public std::runtime_error {
public:
  NotNashException() : std::runtime_error("Output line does not contain a Nash equilibrium") {}
  ~NotNashException() noexcept override = default;
};

template <class T>
MixedStrategyProfile<T> OutputToMixedProfile(GameDocument *p_doc, const wxString &p_text)
{
  MixedStrategyProfile<T> profile(p_doc->GetGame()->NewMixedStrategyProfile(static_cast<T>(0.0)));

  if (wxStringTokenizer tok(p_text, wxT(",")); tok.GetNextToken() == wxT("NE")) {
    if (tok.CountTokens() == static_cast<unsigned int>(profile.MixedProfileLength())) {
      for (size_t i = 1; i <= profile.MixedProfileLength(); i++) {
        profile[i] =
            lexical_cast<Rational>(std::string((const char *)tok.GetNextToken().mb_str()));
      }
      return profile;
    }
  }

  throw NotNashException();
}

template <class T>
MixedBehaviorProfile<T> OutputToBehavProfile(GameDocument *p_doc, const wxString &p_text)
{
  MixedBehaviorProfile<T> profile(p_doc->GetGame());

  wxStringTokenizer tok(p_text, wxT(","));

  if (tok.GetNextToken() == wxT("NE")) {
    if (tok.CountTokens() == static_cast<unsigned int>(profile.BehaviorProfileLength())) {
      for (size_t i = 1; i <= profile.BehaviorProfileLength(); i++) {
        profile[i] = lexical_cast<Rational>(std::string(tok.GetNextToken().mb_str()));
      }
      return profile;
    }
  }

  throw NotNashException();
}

} // end anonymous namespace

template <class T> void AnalysisProfileList<T>::AddOutput(const wxString &p_output)
{
  try {
    if (m_isBehav) {
      auto profile =
          std::make_shared<MixedBehaviorProfile<T>>(OutputToBehavProfile<T>(m_doc, p_output));
      m_behavProfiles.push_back(profile);
      m_mixedProfiles.push_back(
          std::make_shared<MixedStrategyProfile<T>>(profile->ToMixedProfile()));
      m_current = m_behavProfiles.size();
    }
    else {
      auto profile =
          std::make_shared<MixedStrategyProfile<T>>(OutputToMixedProfile<T>(m_doc, p_output));
      m_mixedProfiles.push_back(profile);
      if (m_doc->IsTree()) {
        m_behavProfiles.push_back(std::make_shared<MixedBehaviorProfile<T>>(*profile));
      }
      m_current = m_mixedProfiles.size();
    }
  }
  catch (NotNashException &) {
  }
}

template <class T> void AnalysisProfileList<T>::BuildNfg()
{
  for (auto profile : m_behavProfiles) {
    m_mixedProfiles.push_back(
        std::make_shared<MixedStrategyProfile<T>>(profile->ToMixedProfile()));
  }
}

template <class T> int AnalysisProfileList<T>::NumProfiles() const
{
  return (m_doc->IsTree()) ? m_behavProfiles.size() : m_mixedProfiles.size();
}

template <class T> void AnalysisProfileList<T>::Clear()
{
  m_behavProfiles.clear();
  m_mixedProfiles.clear();
  m_current = 0;
}

//-------------------------------------------------------------------------
//         AnalysisProfileList: Saving and loading profile lists
//-------------------------------------------------------------------------

// Use anonymous namespace to make these helpers private
namespace {

template <class T>
MixedStrategyProfile<T> TextToMixedProfile(GameDocument *p_doc, const wxString &p_text)
{
  MixedStrategyProfile<T> profile(p_doc->GetGame()->NewMixedStrategyProfile(static_cast<T>(0)));

  wxStringTokenizer tok(p_text, wxT(","));

  for (size_t i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = lexical_cast<Rational>(std::string(tok.GetNextToken().mb_str()));
  }

  return profile;
}

template <class T>
MixedBehaviorProfile<T> TextToBehavProfile(GameDocument *p_doc, const wxString &p_text)
{
  MixedBehaviorProfile<T> profile(p_doc->GetGame());

  wxStringTokenizer tok(p_text, wxT(","));
  for (size_t i = 1; i <= profile.BehaviorProfileLength(); i++) {
    profile[i] = lexical_cast<Rational>(std::string(tok.GetNextToken().mb_str()));
  }

  return profile;
}

} // end anonymous namespace

//
// Load a profile list from XML.  Pass a node pointing to an
// <analysis> entry in the workbook file
//
template <class T> void AnalysisProfileList<T>::Load(TiXmlNode *p_analysis)
{
  Clear();

  if (TiXmlNode *description = p_analysis->FirstChild("description")) {
    m_description = wxString(description->FirstChild()->Value(), *wxConvCurrent);
  }

  for (TiXmlNode *node = p_analysis->FirstChild("profile"); node;
       node = node->NextSiblingElement()) {
    if (const char *type = node->ToElement()->Attribute("type"); !strcmp(type, "behav")) {
      const MixedBehaviorProfile<T> profile =
          TextToBehavProfile<T>(m_doc, wxString(node->FirstChild()->Value(), *wxConvCurrent));
      m_behavProfiles.push_back(std::make_shared<MixedBehaviorProfile<T>>(profile));
      m_isBehav = true;
      m_current = m_behavProfiles.size();
    }
    else {
      const MixedStrategyProfile<T> profile =
          TextToMixedProfile<T>(m_doc, wxString(node->FirstChild()->Value(), *wxConvCurrent));
      m_mixedProfiles.push_back(std::make_shared<MixedStrategyProfile<T>>(profile));
      m_isBehav = false;
      m_current = m_mixedProfiles.size();
    }
  }
}

template <class T> std::string AnalysisProfileList<T>::GetPayoff(int pl, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  try {
    if (m_doc->IsTree()) {
      return lexical_cast<std::string>(
          m_behavProfiles[index]->GetPayoff(m_doc->GetGame()->GetPlayer(pl)),
          m_doc->GetStyle().NumDecimals());
    }
    return lexical_cast<std::string>(
        m_mixedProfiles[index]->GetPayoff(m_doc->GetGame()->GetPlayer(pl)),
        m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetRealizProb(const GameNode &p_node, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  try {
    return lexical_cast<std::string>(m_behavProfiles[index]->GetRealizProb(p_node),
                                     m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetBeliefProb(const GameNode &p_node, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer()) {
    return "";
  }

  try {
    if (m_behavProfiles[index]->GetInfosetProb(p_node->GetInfoset()) > Rational(0)) {
      return lexical_cast<std::string>(m_behavProfiles[index]->GetBeliefProb(p_node),
                                       m_doc->GetStyle().NumDecimals());
    }
    // We don't compute assessments yet!
    return "*";
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetNodeValue(const GameNode &p_node, int p_player,
                                                 int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  try {
    return lexical_cast<std::string>(m_behavProfiles[index]->GetPayoff(p_node)[p_player],
                                     m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetInfosetProb(const GameNode &p_node, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer()) {
    return "";
  }

  try {
    return lexical_cast<std::string>(m_behavProfiles[index]->GetInfosetProb(p_node->GetInfoset()),
                                     m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetInfosetValue(const GameNode &p_node, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer() || p_node->GetPlayer()->IsChance()) {
    return "";
  }

  try {
    if (m_behavProfiles[index]->GetInfosetProb(p_node->GetInfoset()) > Rational(0)) {
      return lexical_cast<std::string>(m_behavProfiles[index]->GetPayoff(p_node->GetInfoset()),
                                       m_doc->GetStyle().NumDecimals());
    }
    // In the absence of beliefs, this is not well-defined in general
    return "*";
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetActionProb(const GameNode &p_node, int p_act,
                                                  int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    const GameInfoset infoset = p_node->GetInfoset();
    return static_cast<std::string>(infoset->GetActionProb(infoset->GetAction(p_act)));
  }

  if (!p_node->GetPlayer()) {
    return "";
  }

  try {
    const MixedBehaviorProfile<T> &profile = *m_behavProfiles[index];

    if (!profile.IsDefinedAt(p_node->GetInfoset())) {
      return "*";
    }

    return lexical_cast<std::string>(profile.GetActionProb(p_node->GetInfoset()->GetAction(p_act)),
                                     m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetActionProb(int p_action, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  try {
    const MixedBehaviorProfile<T> &profile = *m_behavProfiles[index];

    if (!profile.IsDefinedAt(m_doc->GetAction(p_action)->GetInfoset())) {
      return "*";
    }

    return lexical_cast<std::string>(profile[p_action], m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetActionValue(const GameNode &p_node, int p_act,
                                                   int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  if (!p_node->GetPlayer() || p_node->GetPlayer()->IsChance()) {
    return "";
  }

  try {
    if (m_behavProfiles[index]->GetInfosetProb(p_node->GetInfoset()) > Rational(0)) {
      return lexical_cast<std::string>(
          m_behavProfiles[index]->GetPayoff(p_node->GetInfoset()->GetAction(p_act)),
          m_doc->GetStyle().NumDecimals());
    }
    // In the absence of beliefs, this is not well-defined
    return "*";
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetStrategyProb(int p_strategy, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  try {
    const MixedStrategyProfile<T> &profile = *m_mixedProfiles[index];
    return lexical_cast<std::string>(profile[p_strategy], m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetStrategyValue(int p_strategy, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  try {
    const MixedStrategyProfile<T> &profile = *m_mixedProfiles[index];
    const GameStrategy strategy = profile.GetGame()->GetStrategy(p_strategy);
    return lexical_cast<std::string>(profile.GetPayoff(strategy), m_doc->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T> void AnalysisProfileList<T>::Save(std::ostream &p_file) const
{
  p_file << "<analysis type=\"list\">\n";

  p_file << "<description>\n";
  p_file << static_cast<const char *>(m_description.mb_str()) << "\n";
  p_file << "</description>\n";

  if (m_doc->IsTree()) {
    for (int j = 1; j <= NumProfiles(); j++) {
      const MixedBehaviorProfile<T> &behav = *m_behavProfiles[j];
      p_file << "<profile type=\"behav\">\n";
      for (size_t k = 1; k <= behav.BehaviorProfileLength(); k++) {
        p_file << behav[k];
        if (k < behav.BehaviorProfileLength()) {
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
      const MixedStrategyProfile<T> &mixed = *m_mixedProfiles[j];
      p_file << "<profile type=\"mixed\">\n";
      for (size_t k = 1; k <= mixed.MixedProfileLength(); k++) {
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
template class AnalysisProfileList<double>;
template class AnalysisProfileList<Rational>;

} // namespace Gambit::GUI
