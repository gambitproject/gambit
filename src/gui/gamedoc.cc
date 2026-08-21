//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/gamedoc.cc
// Implementation of game document class
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
#include <fstream>
#include <set>

#include "games.h"
#include "games/workspace.h"

#include "app.h" // for wxGetApp()
#include "gamedoc.h"

namespace Gambit::GUI {

//=========================================================================
//                   class StrategyDominanceStack
//=========================================================================

StrategyDominanceStack::StrategyDominanceStack(GameDocument *p_doc, bool p_strict)
  : m_doc(p_doc), m_strict(p_strict), m_noFurther(false)
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
  m_supports.push_back(std::make_shared<StrategySupportProfile>(m_doc->GetGame()));
  m_current = 1;
  m_noFurther = false;
}

bool StrategyDominanceStack::NextLevel()
{
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

AnalysisWorkspace::AnalysisWorkspace(GameDocument *p_doc)
  : m_doc(p_doc), m_stratSupports(p_doc, true), m_currentProfileList(0)
{
}

void AnalysisWorkspace::Clear()
{
  m_stratSupports.Reset();
  m_profiles.clear();
  m_currentProfileList = 0;
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

void AnalysisWorkspace::BuildNfg()
{
  m_stratSupports.Reset();
  std::for_each(m_profiles.begin(), m_profiles.end(), std::mem_fn(&AnalysisOutput::BuildNfg));
}

void AnalysisWorkspace::AddEquilibriumOutput(std::shared_ptr<AnalysisOutput> p_profs)
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

std::vector<LegacyWorkspaceFile::Analysis> AnalysisWorkspace::Save() const
{
  std::vector<LegacyWorkspaceFile::Analysis> result;
  for (const auto &analysis : m_profiles) {
    result.push_back(analysis->Save());
  }
  return result;
}

bool AnalysisWorkspace::Load(const std::vector<LegacyWorkspaceFile::Analysis> &p_analyses)
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
      auto plist = std::make_shared<AnalysisProfileList<double>>(m_doc, false);
      plist->Load(analysis);
      m_profiles.push_back(plist);
    }
    else {
      auto plist = std::make_shared<AnalysisProfileList<Rational>>(m_doc, false);
      plist->Load(analysis);
      m_profiles.push_back(plist);
    }
  }

  m_currentProfileList = m_profiles.size();

  return true;
}

//=========================================================================
//                          class GameDocument
//=========================================================================

GameDocument::GameDocument(Game p_game)
  : m_game(p_game), m_gameModified(false), m_workspaceModified(false), m_workspace(this)
{
  wxGetApp().AddDocument(this);
  ResetUndoHistory();
}

GameDocument::~GameDocument() { wxGetApp().RemoveDocument(this); }

GameDocument::LoadOutcome GameDocument::Load(const wxString &p_filename)
{
  std::ifstream input(p_filename.mb_str());
  if (!input) {
    return {LoadResult::OpenFailed, nullptr};
  }

  // First, see whether this is a Gambit workspace (.gbt) file.
  try {
    const LegacyWorkspaceFile workspace = ReadLegacyWorkspace(input);
    std::istringstream game_text(workspace.game);
    auto doc = std::make_shared<GameDocument>(ReadGame(game_text));
    if (doc->m_workspace.Load(workspace.analyses)) {
      doc->m_style.Load(workspace);
      doc->SetFilename(p_filename);
      doc->ResetUndoHistory();
      return {LoadResult::Success, doc};
    }
  }
  catch (const std::exception &) {
    // Not a recognized (or not fully valid) workspace file; fall through to
    // try it as a bare game file instead.
  }

  // Not a (valid) workspace -- try reading it as a bare .efg/.nfg file.
  input.clear();
  input.seekg(0);
  try {
    const Game game = ReadGame(input);
    if (game->IsAgg()) {
      return {LoadResult::UnsupportedRepresentation, nullptr};
    }
    auto doc = std::make_shared<GameDocument>(game);
    doc->SetFilename(p_filename);
    return {LoadResult::Success, doc};
  }
  catch (const InvalidFileException &) {
    return {LoadResult::ParseFailed, nullptr};
  }
}

void GameDocument::SaveWorkspace(std::ostream &p_file) const
{
  LegacyWorkspaceFile workspace;
  m_style.Save(workspace);
  workspace.game_format = m_game->IsTree() ? "efg" : "nfg";
  if (!m_game->IsTree()) {
    workspace.layout.reset();
    workspace.labels.reset();
  }
  std::ostringstream game_text;
  m_game->Write(game_text, workspace.game_format);
  workspace.game = game_text.str();
  if (!workspace.game.empty() && workspace.game.back() == '\n') {
    workspace.game.pop_back();
  }
  workspace.analyses = m_workspace.Save();
  Gambit::WriteLegacyWorkspace(p_file, workspace);
}

void GameDocument::NotifyChanged(GameModificationType p_modifications)
{
  m_gameModified |= HasModification(p_modifications, GameModificationType::GameForm |
                                                         GameModificationType::GamePayoffs |
                                                         GameModificationType::GameLabels);
  m_workspaceModified |= HasModification(p_modifications, GameModificationType::Workspace);
  if (HasModification(p_modifications,
                      GameModificationType::GameForm | GameModificationType::GamePayoffs)) {
    m_workspace.ResetForGameChange();
  }
  if (p_modifications != GameModificationType::None) {
    m_redoList.clear();
    std::ostringstream s;
    SaveWorkspace(s);
    m_undoList.push_back(s.str());
  }
  UpdateViews();
}

void GameDocument::UpdateViews()
{
  std::for_each(m_views.begin(), m_views.end(), std::mem_fn(&GameView::OnUpdate));
}

void GameDocument::ResetUndoHistory()
{
  m_undoList.clear();
  m_redoList.clear();
  std::ostringstream s;
  SaveWorkspace(s);
  m_undoList.push_back(s.str());
}

void GameDocument::RestoreSnapshot(const std::string &p_snapshot)
{
  std::istringstream input(p_snapshot);
  const LegacyWorkspaceFile workspace = ReadLegacyWorkspace(input);
  std::istringstream game_text(workspace.game);
  m_game = ReadGame(game_text);
  m_workspace.Load(workspace.analyses);
  m_style.Load(workspace);
}

void GameDocument::Undo()
{
  if (!CanUndo()) {
    return;
  }
  m_redoList.push_back(m_undoList.back());
  m_undoList.pop_back();
  RestoreSnapshot(m_undoList.back());
  m_gameModified = m_workspaceModified = true;
  UpdateViews();
}

void GameDocument::Redo()
{
  if (!CanRedo()) {
    return;
  }
  m_undoList.push_back(m_redoList.back());
  m_redoList.pop_back();
  RestoreSnapshot(m_undoList.back());
  m_gameModified = m_workspaceModified = true;
  UpdateViews();
}

void GameDocument::PostPendingChanges()
{
  std::for_each(m_views.begin(), m_views.end(), std::mem_fn(&GameView::PostPendingChanges));
}

void GameDocument::BuildNfg()
{
  if (m_game->IsTree()) {
    m_workspace.BuildNfg();
  }
}

GameAction GameDocument::GetAction(int p_index) const
{
  int index = 1;
  for (auto player : m_game->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        if (index++ == p_index) {
          return action;
        }
      }
    }
  }
  throw std::out_of_range("Action index out of range");
}

void GameDocument::SetStyle(const TreeRenderConfig &p_style)
{
  m_style = p_style;
  NotifyChanged(GameModificationType::Presentation);
}

void GameDocument::DoSelectProfile(int p_profile)
{
  m_workspace.SelectProfile(p_profile);
  UpdateViews();
}

void GameDocument::DoAddEquilibriumOutput(std::shared_ptr<AnalysisOutput> p_profs)
{
  m_workspace.AddEquilibriumOutput(p_profs);
  NotifyChanged(GameModificationType::Workspace);
}

void GameDocument::DoAnalysisOutputChanged() { NotifyChanged(GameModificationType::Workspace); }

void GameDocument::DoSelectEquilibriumOutput(int p_index)
{
  m_workspace.SelectEquilibriumOutput(p_index);
  UpdateViews();
}

void GameDocument::DoSetDominanceStrictness(bool p_strict)
{
  m_workspace.SetDominanceStrictness(p_strict);
  UpdateViews();
}

bool GameDocument::DoNextDominanceLevel()
{
  const bool ret = m_workspace.NextDominanceLevel();
  UpdateViews();
  return ret;
}

void GameDocument::DoPreviousDominanceLevel()
{
  m_workspace.PreviousDominanceLevel();
  UpdateViews();
}

void GameDocument::DoTopDominanceLevel()
{
  m_workspace.TopDominanceLevel();
  UpdateViews();
}

//======================================================================
// Commands for model part of MVC architecture start here.
//======================================================================

void GameDocument::DoSave(const wxString &p_filename, GameSaveFormat p_format)
{
  std::ofstream file(p_filename.mb_str());
  if (!file) {
    throw std::runtime_error(std::string("Unable to open file for writing: ") +
                             static_cast<const char *>(p_filename.mb_str()));
  }
  switch (p_format) {
  case GameSaveFormat::Workspace:
    SaveWorkspace(file);
    m_filename = p_filename;
    m_gameModified = false;
    m_workspaceModified = false;
    break;

  case GameSaveFormat::Efg:
    m_game->Write(file, "efg");
    m_gameModified = false;
    break;

  case GameSaveFormat::Nfg:
    BuildNfg();
    m_game->Write(file, "nfg");
    m_gameModified = false;
    break;
  }
  UpdateViews();
}

void GameDocument::DoSetTitle(const wxString &p_title, const wxString &p_comment)
{
  m_game->SetTitle(static_cast<const char *>(p_title.mb_str()));
  m_game->SetDescription(static_cast<const char *>(p_comment.mb_str()));
  NotifyChanged(GameModificationType::GameLabels);
}

GamePlayer GameDocument::DoNewPlayer()
{
  std::set<std::string> playerLabels;

  for (const auto &player : m_game->GetPlayers()) {
    playerLabels.insert(player->GetLabel());
  }

  int number = m_game->NumPlayers() + 1;
  while (playerLabels.contains("Player " + lexical_cast<std::string>(number))) {
    number++;
  }
  const GamePlayer player = m_game->NewPlayer("Player " + lexical_cast<std::string>(number));
  NotifyChanged(GameModificationType::GameForm);
  return player;
}

void GameDocument::DoRelabelPlayers(const std::map<std::string, std::string> &p_labels)
{
  m_game->RelabelPlayers(p_labels);
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoSetStrategies(GamePlayer p_player,
                                   const std::vector<std::string> &p_stableLabels,
                                   const std::vector<std::string> &p_labels)
{
  // Phase 1: structure (which strategies exist, and in what order), resolved purely from
  // p_stableLabels -- untouched by any pending rename in p_labels.
  m_game->SetStrategies(p_player, p_stableLabels);

  // Phase 2: relabeling, applied once the structure has settled, so a label freed up by
  // a deletion in phase 1 is available for reuse here.
  std::map<std::string, std::string> relabels;
  for (size_t i = 0; i < p_stableLabels.size(); i++) {
    if (p_stableLabels[i] != p_labels[i]) {
      relabels[p_stableLabels[i]] = p_labels[i];
    }
  }
  if (!relabels.empty()) {
    m_game->RelabelStrategies(p_player, relabels);
  }
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetInfosetLabel(GameInfoset p_infoset, const wxString &p_label)
{
  p_infoset->SetLabel(p_label.ToStdString(wxConvUTF8));
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoRelabelActions(GameInfoset p_infoset,
                                    const std::map<std::string, std::string> &p_labels)
{
  m_game->RelabelActions(p_infoset, p_labels);
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoSetActions(GameInfoset p_infoset,
                                const std::vector<std::string> &p_stableLabels,
                                const std::vector<std::string> &p_labels,
                                const std::vector<Number> &p_probs)
{
  // Phase 1: structure (which actions exist, and in what order), resolved purely from
  // p_stableLabels -- untouched by any pending rename in p_labels.
  if (p_infoset->IsChanceInfoset()) {
    m_game->SetEventActions(p_infoset, p_stableLabels, p_probs);
  }
  else {
    m_game->SetMoveActions(p_infoset, p_stableLabels);
  }

  // Phase 2: relabeling, applied once the structure has settled, so a label freed up by
  // a deletion in phase 1 is available for reuse here.
  std::map<std::string, std::string> relabels;
  for (size_t i = 0; i < p_stableLabels.size(); i++) {
    if (p_stableLabels[i] != p_labels[i]) {
      relabels[p_stableLabels[i]] = p_labels[i];
    }
  }
  if (!relabels.empty()) {
    m_game->RelabelActions(p_infoset, relabels);
  }
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetInfoset(GameNode p_node, GameInfoset p_infoset)
{
  if (p_node->GetInfoset() == p_infoset) {
    return;
  }
  std::vector<GameNode> nodes(p_infoset->GetMembers().begin(), p_infoset->GetMembers().end());
  nodes.push_back(p_node);
  m_game->MakeInfoset(nodes, p_infoset->GetPlayer(), p_infoset->GetLabel());
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoLeaveInfoset(GameNode p_node)
{
  const GameInfoset infoset = p_node->GetInfoset();
  if (!infoset) {
    return;
  }
  std::vector<GameNode> members(infoset->GetMembers().begin(), infoset->GetMembers().end());
  if (members.size() == 1) {
    // Already a singleton: a no-op that keeps the infoset's identity and label.
    return;
  }
  m_game->MakeInfoset({p_node}, infoset->GetPlayer(), "");
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoRevealAction(GameInfoset p_infoset, GamePlayer p_player)
{
  m_game->Reveal(p_infoset, p_player);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetNodeLabel(GameNode p_node, const wxString &p_label)
{
  p_node->SetLabel(p_label.ToStdString(wxConvUTF8));
  NotifyChanged(GameModificationType::GameLabels);
}

void GameDocument::DoAppendMove(GameNode p_node, GameInfoset p_infoset)
{
  m_game->AppendMove(p_node, p_infoset);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoInsertMove(GameNode p_node, GamePlayer p_player, unsigned int p_actions)
{
  if (p_player->IsChance()) {
    // A newly-inserted chance move defaults to a uniform distribution over its actions;
    // the UX for specifying a distribution at creation time is a separate piece of work.
    std::vector<std::string> actions;
    for (unsigned int act = 1; act <= p_actions; act++) {
      actions.push_back(std::to_string(act));
    }
    m_game->InsertEvent(p_node, actions,
                        std::vector<Number>(p_actions, Number(Rational(1, p_actions))));
  }
  else {
    m_game->InsertMove(p_node, p_player, p_actions);
  }
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoInsertMove(GameNode p_node, GameInfoset p_infoset)
{
  m_game->InsertMove(p_node, p_infoset);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoCopyTree(GameNode p_destNode, GameNode p_srcNode)
{
  m_game->CopyTree(p_destNode, p_srcNode);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoMoveTree(GameNode p_destNode, GameNode p_srcNode)
{
  m_game->MoveTree(p_destNode, p_srcNode);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoDeleteParent(GameNode p_node)
{
  if (!p_node || !p_node->GetParent()) {
    return;
  }
  m_game->DeleteParent(p_node);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoDeleteTree(GameNode p_node)
{
  m_game->DeleteTree(p_node);
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetPlayer(GameInfoset p_infoset, GamePlayer p_player)
{
  if (p_player->IsChance() || p_infoset->GetPlayer()->IsChance()) {
    // Currently don't support switching nodes to/from chance player
    return;
  }
  if (p_infoset->GetPlayer() == p_player) {
    return;
  }
  std::vector<GameNode> members(p_infoset->GetMembers().begin(), p_infoset->GetMembers().end());
  m_game->MakeInfoset(members, p_player, p_infoset->GetLabel());
  NotifyChanged(GameModificationType::GameForm);
}

void GameDocument::DoSetPlayer(GameNode p_node, GamePlayer p_player)
{
  DoSetPlayer(p_node->GetInfoset(), p_player);
}

namespace {

std::string GenerateOutcomeLabel(const Game &p_game)
{
  std::set<std::string> outcomeLabels;
  for (const auto &outcome : p_game->GetOutcomes()) {
    outcomeLabels.insert(outcome->GetLabel());
  }
  int outc = p_game->GetOutcomes().size() + 1;
  while (outcomeLabels.contains("Outcome " + std::to_string(outc))) {
    outc++;
  }
  return "Outcome " + std::to_string(outc);
}

} // namespace

void GameDocument::DoNewOutcome(GameNode p_node)
{
  std::set<std::string> outcomeLabels;
  for (const auto &outcome : m_game->GetOutcomes()) {
    outcomeLabels.insert(outcome->GetLabel());
  }
  int outc = m_game->GetOutcomes().size() + 1;
  while (outcomeLabels.contains("Outcome " + std::to_string(outc))) {
    outc++;
  }
  m_game->SetOutcome(p_node, m_game->NewOutcome(GenerateOutcomeLabel(m_game)));
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoNewOutcome(const PureStrategyProfile &p_profile)
{
  p_profile->SetOutcome(m_game->NewOutcome(GenerateOutcomeLabel(m_game)));
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoSetOutcome(GameNode p_node, GameOutcome p_outcome)
{
  m_game->SetOutcome(p_node, p_outcome);
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoSetOutcomeData(const GameNode &p_node, const wxString &p_label,
                                    const std::vector<wxString> &p_payoffs)
{
  if (!p_node) {
    return;
  }

  if (p_payoffs.size() != GetGame()->NumPlayers()) {
    throw std::invalid_argument("Incorrect number of payoff values");
  }

  std::vector<Rational> parsedPayoffs;
  parsedPayoffs.reserve(p_payoffs.size());

  for (const auto &value : p_payoffs) {
    parsedPayoffs.push_back(lexical_cast<Rational>(value.ToStdString()));
  }

  const std::string label = p_label.ToStdString(wxConvUTF8);
  GameOutcome outcome = p_node->GetOutcome();

  bool changed = !outcome;

  if (outcome) {
    changed = outcome->GetLabel() != label;

    if (!changed) {
      for (size_t player = 1; player <= GetGame()->NumPlayers(); ++player) {
        if (outcome->GetPayoff<Rational>(GetGame()->GetPlayer(player)) !=
            parsedPayoffs[player - 1]) {
          changed = true;
          break;
        }
      }
    }
  }

  if (!changed) {
    return;
  }

  if (!outcome) {
    outcome = m_game->NewOutcome(p_label.ToStdString(wxConvUTF8));
    m_game->SetOutcome(p_node, outcome);
  }
  else {
    outcome->SetLabel(label);
  }

  for (size_t player = 1; player <= GetGame()->NumPlayers(); ++player) {
    outcome->SetPayoff(GetGame()->GetPlayer(player), Number(p_payoffs[player - 1].ToStdString()));
  }

  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoRemoveOutcome(GameNode p_node)
{
  if (!p_node || !p_node->GetOutcome()) {
    return;
  }
  m_game->SetOutcome(p_node, nullptr);
  NotifyChanged(GameModificationType::GamePayoffs);
}

void GameDocument::DoSetPayoff(GameOutcome p_outcome, int p_player, const wxString &p_value)
{
  p_outcome->SetPayoff(m_game->GetPlayer(p_player), Number(p_value.ToStdString()));
  NotifyChanged(GameModificationType::GamePayoffs);
}

} // namespace Gambit::GUI
