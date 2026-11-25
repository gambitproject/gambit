//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/analysis.cc
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

#include <sstream>

#include "games.h"
#include "games/gametree.h"
#include "games/workspace.h"

#include "analysis.h"
#include "gameworkspace.h"

namespace Gambit::GUI {
GameAction ActionAt(const Game &p_game, int p_index)
{
  int index = 1;
  for (const auto &player : p_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        if (index++ == p_index) {
          return action;
        }
      }
    }
  }
  throw std::out_of_range("Action index out of range");
}

//=========================================================================
//                     class AnalysisProfileList
//=========================================================================

template <class T>
void AnalysisProfileList<T>::AddProfile(const MixedStrategyProfile<T> &p_profile)
{
  m_mixedProfiles.push_back(std::make_shared<MixedStrategyProfile<T>>(p_profile));
  if (As<GameTreeRep>(m_workspace->GetGame())) {
    m_behavProfiles.push_back(std::make_shared<MixedBehaviorProfile<T>>(p_profile));
  }
  m_current = m_mixedProfiles.size();
}

template <class T>
void AnalysisProfileList<T>::AddProfile(const MixedBehaviorProfile<T> &p_profile)
{
  m_behavProfiles.push_back(std::make_shared<MixedBehaviorProfile<T>>(p_profile));
  m_current = m_behavProfiles.size();
}

template <class T>
const MixedStrategyProfile<T> &AnalysisProfileList<T>::GetMixedProfile(int p_index) const
{
  while (static_cast<int>(m_mixedProfiles.size()) < p_index &&
         m_mixedProfiles.size() < m_behavProfiles.size()) {
    m_mixedProfiles.push_back(std::make_shared<MixedStrategyProfile<T>>(
        m_behavProfiles[m_mixedProfiles.size() + 1]->ToMixedProfile()));
  }
  return *m_mixedProfiles[p_index];
}

template <class T> int AnalysisProfileList<T>::NumProfiles() const
{
  return As<GameTreeRep>(m_workspace->GetGame()) ? m_behavProfiles.size() : m_mixedProfiles.size();
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

// The next comma-separated entry of p_text, as a value of type T
template <class T> T NextEntry(std::istringstream &p_text)
{
  std::string entry;
  std::getline(p_text, entry, ',');
  return static_cast<T>(lexical_cast<Rational>(entry));
}

template <class T>
MixedStrategyProfile<T> TextToMixedProfile(const Game &p_game, const std::string &p_text)
{
  MixedStrategyProfile<T> profile(p_game->NewMixedStrategyProfile(static_cast<T>(0)));
  std::istringstream text(p_text);
  for (size_t i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = NextEntry<T>(text);
  }
  return profile;
}

template <class T>
MixedBehaviorProfile<T> TextToBehavProfile(const Game &p_game, const std::string &p_text)
{
  MixedBehaviorProfile<T> profile(p_game);
  std::istringstream text(p_text);
  for (size_t i = 1; i <= profile.BehaviorProfileLength(); i++) {
    profile[i] = NextEntry<T>(text);
  }
  return profile;
}

} // end anonymous namespace

// Load a profile list from the neutral workspace representation.
template <class T> void AnalysisProfileList<T>::Load(const WorkspaceFile::Analysis &p_analysis)
{
  Clear();
  m_description = p_analysis.description;
  for (const auto &saved_profile : p_analysis.profiles) {
    if (saved_profile.type == "behav") {
      const MixedBehaviorProfile<T> profile =
          TextToBehavProfile<T>(m_workspace->GetGame(), saved_profile.probabilities);
      m_behavProfiles.push_back(std::make_shared<MixedBehaviorProfile<T>>(profile));
      m_isBehav = true;
      m_current = m_behavProfiles.size();
    }
    else {
      const MixedStrategyProfile<T> profile =
          TextToMixedProfile<T>(m_workspace->GetGame(), saved_profile.probabilities);
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
    if (As<GameTreeRep>(m_workspace->GetGame())) {
      return lexical_cast<std::string>(
          m_behavProfiles[index]->GetPayoff(m_workspace->GetGame()->GetPlayer(pl)),
          m_workspace->GetStyle().NumDecimals());
    }
    return lexical_cast<std::string>(
        GetMixedProfile(index).GetPayoff(m_workspace->GetGame()->GetPlayer(pl)),
        m_workspace->GetStyle().NumDecimals());
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
                                     m_workspace->GetStyle().NumDecimals());
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
    auto belief = m_behavProfiles[index]->GetBeliefProb(p_node);
    if (belief.has_value()) {
      return lexical_cast<std::string>(belief.value(), m_workspace->GetStyle().NumDecimals());
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
                                     m_workspace->GetStyle().NumDecimals());
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
                                     m_workspace->GetStyle().NumDecimals());
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
    auto payoff = m_behavProfiles[index]->GetPayoff(p_node->GetInfoset());
    if (payoff.has_value()) {
      return lexical_cast<std::string>(payoff.value(), m_workspace->GetStyle().NumDecimals());
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
                                     m_workspace->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetActionProb(int p_action, int p_index) const
{
  try {
    return GetActionProb(ActionAt(m_workspace->GetGame(), p_action), p_index);
  }
  catch (std::out_of_range &) {
    return "";
  }
}

template <class T>
std::string AnalysisProfileList<T>::GetActionProb(const GameAction &p_action, int p_index) const
{
  const int index = (p_index == -1) ? m_current : p_index;

  try {
    const MixedBehaviorProfile<T> &profile = *m_behavProfiles[index];

    if (!profile.IsDefinedAt(p_action->GetInfoset())) {
      return "*";
    }

    return lexical_cast<std::string>(profile[p_action], m_workspace->GetStyle().NumDecimals());
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
    std::optional<T> actionValue =
        m_behavProfiles[index]->GetPayoff(p_node->GetInfoset()->GetAction(p_act));
    if (actionValue.has_value()) {
      return lexical_cast<std::string>(actionValue.value(), m_workspace->GetStyle().NumDecimals());
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
    const MixedStrategyProfile<T> &profile = GetMixedProfile(index);
    return lexical_cast<std::string>(profile[p_strategy], m_workspace->GetStyle().NumDecimals());
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
    const MixedStrategyProfile<T> &profile = GetMixedProfile(index);
    const GameStrategy strategy = profile.GetGame()->GetStrategy(p_strategy);
    return lexical_cast<std::string>(profile.GetPayoff(strategy),
                                     m_workspace->GetStyle().NumDecimals());
  }
  catch (std::out_of_range &) {
    return "";
  }
}

namespace {

/// Order two entries, either of which may be undefined; undefined entries
/// order after defined ones.  Only operator< is used, so entries are ordered
/// exactly in whatever type the profiles are stored in.
template <class T>
int CompareEntries(const std::optional<T> &p_left, const std::optional<T> &p_right)
{
  if (!p_left.has_value()) {
    return p_right.has_value() ? 1 : 0;
  }
  if (!p_right.has_value()) {
    return -1;
  }
  if (p_left.value() < p_right.value()) {
    return -1;
  }
  if (p_right.value() < p_left.value()) {
    return 1;
  }
  return 0;
}

} // end anonymous namespace

template <class T>
std::optional<T> AnalysisProfileList<T>::GetActionProbEntry(int p_action, int p_index) const
{
  try {
    const MixedBehaviorProfile<T> &profile = *m_behavProfiles[p_index];

    if (!profile.IsDefinedAt(ActionAt(m_workspace->GetGame(), p_action)->GetInfoset())) {
      return {};
    }

    return profile[p_action];
  }
  catch (std::out_of_range &) {
    return {};
  }
}

template <class T>
std::optional<T> AnalysisProfileList<T>::GetStrategyProbEntry(int p_strategy, int p_index) const
{
  try {
    return GetMixedProfile(p_index)[p_strategy];
  }
  catch (std::out_of_range &) {
    return {};
  }
}

template <class T>
int AnalysisProfileList<T>::CompareActionProb(int p_action, int p_left, int p_right) const
{
  return CompareEntries(GetActionProbEntry(p_action, p_left),
                        GetActionProbEntry(p_action, p_right));
}

template <class T>
int AnalysisProfileList<T>::CompareStrategyProb(int p_strategy, int p_left, int p_right) const
{
  return CompareEntries(GetStrategyProbEntry(p_strategy, p_left),
                        GetStrategyProbEntry(p_strategy, p_right));
}

template <class T> WorkspaceFile::Analysis AnalysisProfileList<T>::Save() const
{
  WorkspaceFile::Analysis result;
  result.description = m_description;
  if (As<GameTreeRep>(m_workspace->GetGame())) {
    for (int j = 1; j <= NumProfiles(); j++) {
      const MixedBehaviorProfile<T> &behav = *m_behavProfiles[j];
      std::ostringstream probabilities;
      for (size_t k = 1; k <= behav.BehaviorProfileLength(); k++) {
        if (k > 1) {
          probabilities << ',';
        }
        probabilities << behav[k];
      }
      result.profiles.push_back({.type = "behav", .probabilities = probabilities.str()});
    }
  }
  else {
    for (int j = 1; j <= NumProfiles(); j++) {
      const MixedStrategyProfile<T> &mixed = GetMixedProfile(j);
      std::ostringstream probabilities;
      for (size_t k = 1; k <= mixed.MixedProfileLength(); k++) {
        if (k > 1) {
          probabilities << ',';
        }
        probabilities << mixed[k];
      }
      result.profiles.push_back({.type = "mixed", .probabilities = probabilities.str()});
    }
  }
  return result;
}

// Explicit instantiations
template class AnalysisProfileList<double>;
template class AnalysisProfileList<Rational>;

//=========================================================================
//                   class StrategyDominanceStack
//=========================================================================

StrategyDominanceStack::StrategyDominanceStack(const GameWorkspace *p_workspace, bool p_strict)
  : m_workspace(p_workspace), m_strict(p_strict)
{
  Reset();
}

void StrategyDominanceStack::SetStrict(bool p_strict)
{
  if (m_strict != p_strict) {
    Reset();
  }
  m_strict = p_strict;
}

void StrategyDominanceStack::Reset()
{
  m_supports.clear();
  m_current = 1;
  m_noFurther = false;
}

void StrategyDominanceStack::EnsureFullSupport() const
{
  if (m_supports.empty()) {
    m_supports.push_back(std::make_shared<StrategySupportProfile>(m_workspace->GetGame()));
  }
}

bool StrategyDominanceStack::NextLevel()
{
  EnsureFullSupport();
  if (m_current < m_supports.size()) {
    m_current++;
    return true;
  }

  if (m_noFurther) {
    return false;
  }

  const StrategySupportProfile newSupport = m_supports[m_current]->Undominated(m_strict);

  if (newSupport != *m_supports[m_current]) {
    m_supports.push_back(std::make_shared<StrategySupportProfile>(newSupport));
    m_current++;
    return true;
  }
  else {
    m_noFurther = true;
    return false;
  }
}

bool StrategyDominanceStack::PreviousLevel()
{
  if (m_current > 1) {
    m_current--;
    return true;
  }
  else {
    return false;
  }
}

//=========================================================================
//                          class AnalysisWorkspace
//=========================================================================

AnalysisWorkspace::AnalysisWorkspace(const GameWorkspace *p_workspace)
  : m_workspace(p_workspace), m_stratSupports(p_workspace, true)
{
}

void AnalysisWorkspace::ResetForGameChange()
{
  m_stratSupports.Reset();

  // Even though modifications only to payoffs doesn't make the
  // computed profiles invalid for the edited game, it does mean
  // that, in general, they won't be Nash.  For now, to avoid confusion,
  // we will wipe them out.
  m_profiles.clear();
  m_currentProfileList = 0;
}

void AnalysisWorkspace::BuildNfg() { m_stratSupports.Reset(); }

void AnalysisWorkspace::AddEquilibriumOutput(const std::shared_ptr<AnalysisOutput> &p_profs)
{
  m_profiles.push_back(p_profs);
  m_currentProfileList = m_profiles.size();
}

void AnalysisWorkspace::SelectEquilibriumOutput(int p_index) { m_currentProfileList = p_index; }

void AnalysisWorkspace::SelectProfile(int p_profile)
{
  m_profiles[m_currentProfileList]->SetCurrent(p_profile);
}

void AnalysisWorkspace::SetDominanceStrictness(bool p_strict)
{
  m_stratSupports.SetStrict(p_strict);
}

bool AnalysisWorkspace::GetStrategyElimStrength() const { return m_stratSupports.GetStrict(); }

bool AnalysisWorkspace::NextDominanceLevel() { return m_stratSupports.NextLevel(); }

void AnalysisWorkspace::PreviousDominanceLevel() { m_stratSupports.PreviousLevel(); }

void AnalysisWorkspace::TopDominanceLevel() { m_stratSupports.TopLevel(); }

bool AnalysisWorkspace::CanStrategyElim() const { return m_stratSupports.CanEliminate(); }

int AnalysisWorkspace::GetStrategyElimLevel() const { return m_stratSupports.GetLevel(); }

std::vector<WorkspaceFile::Analysis> AnalysisWorkspace::Save() const
{
  std::vector<WorkspaceFile::Analysis> result;
  for (const auto &analysis : m_profiles) {
    result.push_back(analysis->Save());
  }
  return result;
}

bool AnalysisWorkspace::Load(const std::vector<WorkspaceFile::Analysis> &p_analyses)
{
  m_stratSupports.Reset();

  m_profiles.clear();

  for (const auto &analysis : p_analyses) {
    // We need to try to guess whether the profiles are float or rational
    bool isFloat = false;
    for (const auto &profile : analysis.profiles) {
      if (profile.probabilities.find('.') != std::string::npos ||
          profile.probabilities.find('e') != std::string::npos) {
        isFloat = true;
        break;
      }
    }
    if (isFloat) {
      const auto plist = std::make_shared<AnalysisProfileList<double>>(m_workspace, false);
      plist->Load(analysis);
      m_profiles.push_back(plist);
    }
    else {
      const auto plist = std::make_shared<AnalysisProfileList<Rational>>(m_workspace, false);
      plist->Load(analysis);
      m_profiles.push_back(plist);
    }
  }

  m_currentProfileList = m_profiles.size();

  return true;
}

} // namespace Gambit::GUI
