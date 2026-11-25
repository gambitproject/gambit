//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/gameedit.cc
// Edits to the game made by the graphical interface
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

#include <algorithm>
#include <set>

#include "games/gametree.h"
#include "gameedit.h"

namespace Gambit::GUI {

namespace {

using enum GameModificationType;
using Reason = std::optional<std::string>;

// The relabelling from p_stableLabels to p_labels, by position, of the items that change label
std::map<std::string, std::string> Relabelling(const std::vector<std::string> &p_stableLabels,
                                               const std::vector<std::string> &p_labels)
{
  std::map<std::string, std::string> relabels;
  for (size_t i = 0; i < p_stableLabels.size(); i++) {
    if (p_stableLabels[i] != p_labels[i]) {
      relabels[p_stableLabels[i]] = p_labels[i];
    }
  }
  return relabels;
}

template <class R> std::vector<std::string> LabelsOf(const R &p_items)
{
  std::vector<std::string> labels;
  for (const auto &item : p_items) {
    labels.push_back(item->GetLabel());
  }
  return labels;
}

const std::string &Text(const Number &p_number)
{
  return static_cast<const std::string &>(p_number);
}

bool HasOutcome(const GameNode &p_node) { return !p_node->GetOutcome()->IsNull(); }

std::vector<GameStrategy> StrategiesOf(const Game &p_game, const PureStrategyProfile &p_profile)
{
  std::vector<GameStrategy> strategies;
  strategies.reserve(p_game->NumPlayers());
  for (const auto &player : p_game->GetPlayers()) {
    strategies.push_back(p_profile->GetStrategy(player));
  }
  return strategies;
}

// Whether p_outcome already has label p_label and payoffs whose text is p_payoffs; text is
// compared, rather than value, since the text is what is saved
bool OutcomeMatches(const Game &p_game, const GameOutcome &p_outcome, const std::string &p_label,
                    const std::vector<Number> &p_payoffs)
{
  if (p_outcome->GetLabel() != p_label) {
    return false;
  }
  size_t i = 0;
  for (const auto &player : p_game->GetPlayers()) {
    if (p_outcome->GetPayoff<std::string>(player) != Text(p_payoffs[i++])) {
      return false;
    }
  }
  return true;
}

Reason CheckLabelLists(const std::vector<std::string> &p_stableLabels,
                       const std::vector<std::string> &p_labels)
{
  if (p_stableLabels.size() != p_labels.size()) {
    return "The lists of labels before and after the edit differ in length";
  }
  return {};
}

Reason CheckPayoffCount(const Game &p_game, const std::vector<Number> &p_payoffs)
{
  if (p_payoffs.size() != p_game->NumPlayers()) {
    return "The number of payoffs differs from the number of players";
  }
  return {};
}

GameModificationType EditPlayers(const Game &p_game,
                                 const std::vector<std::string> &p_stableLabels,
                                 const std::vector<std::string> &p_labels)
{
  const auto relabels = Relabelling(p_stableLabels, p_labels);
  if (p_stableLabels == LabelsOf(p_game->GetPlayers())) {
    // Relabelling all players in one call lets two players' labels be swapped without
    // tripping the duplicate-label check; and it works for a game whose `SetPlayers` is
    // unsupported, e.g. one read from an action graph game file.
    if (relabels.empty()) {
      return None;
    }
    p_game->RelabelPlayers(relabels);
    return GameLabels;
  }
  p_game->SetPlayers(p_stableLabels);
  if (!relabels.empty()) {
    p_game->RelabelPlayers(relabels);
  }
  return GameForm;
}

GameModificationType EditActions(const Game &p_game, const GameInfoset &p_infoset,
                                 const std::vector<std::string> &p_stableLabels,
                                 const std::vector<std::string> &p_labels,
                                 const std::vector<Number> &p_probs)
{
  const bool isChance = p_infoset->IsChanceInfoset();
  bool restructured = p_stableLabels != LabelsOf(p_infoset->GetActions());
  if (isChance && !restructured) {
    // A probability counts as changed if its text does, e.g. from "0.5" to "1/2"
    size_t i = 0;
    for (const auto &action : p_infoset->GetActions()) {
      if (Text(p_infoset->GetActionProb(action)) != Text(p_probs.at(i++))) {
        restructured = true;
        break;
      }
    }
  }
  const auto relabels = Relabelling(p_stableLabels, p_labels);
  if (!restructured) {
    if (relabels.empty()) {
      return None;
    }
    p_game->RelabelActions(p_infoset, relabels);
    return GameLabels;
  }
  if (isChance) {
    p_game->SetEventActions(p_infoset, p_stableLabels, p_probs);
  }
  else {
    p_game->SetMoveActions(p_infoset, p_stableLabels);
  }
  if (!relabels.empty()) {
    p_game->RelabelActions(p_infoset, relabels);
  }
  return GameForm;
}

GameModificationType JoinNodeToInfoset(const Game &p_game, const GameNode &p_node,
                                       const GameInfoset &p_infoset)
{
  if (p_node->GetInfoset() == p_infoset) {
    return None;
  }
  std::vector<GameNode> nodes(p_infoset->GetMembers().begin(), p_infoset->GetMembers().end());
  nodes.push_back(p_node);
  p_game->MakeInfoset(nodes, p_infoset->GetPlayer(), p_infoset->GetLabel());
  return GameForm;
}

GameModificationType LeaveInfoset(const Game &p_game, const GameNode &p_node)
{
  const GameInfoset infoset = p_node->GetInfoset();
  if (infoset->GetMembers().size() == 1) {
    // Already a singleton; leaving it alone keeps the information set's identity and label
    return None;
  }
  p_game->MakeInfoset({p_node}, infoset->GetPlayer(), "");
  return GameForm;
}

GameModificationType AttachOutcome(const Game &p_game, const GameNode &p_node,
                                   const GameOutcome &p_outcome)
{
  if (p_node->GetOutcome() == p_outcome) {
    return None;
  }
  std::vector<GameNode> members{p_node};
  for (const auto &node : p_game->GetNodes()) {
    if (node != p_node && node->GetOutcome() == p_outcome) {
      members.push_back(node);
    }
  }
  std::vector<Number> payoffs;
  payoffs.reserve(p_game->NumPlayers());
  for (const auto &player : p_game->GetPlayers()) {
    payoffs.emplace_back(p_outcome->GetPayoff<std::string>(player));
  }
  p_game->MakeOutcome(members, payoffs, p_outcome->GetLabel());
  return GamePayoffs;
}

//
// Preconditions, one per edit
//

Reason Check(const Game &, const SetTreeStyle &) { return {}; }

Reason Check(const Game &, const EditGameProperties &p_edit)
{
  return CheckLabelLists(p_edit.stableLabels, p_edit.labels);
}

Reason Check(const Game &, const SetStrategies &p_edit)
{
  if (!p_edit.player) {
    return "No player given";
  }
  return CheckLabelLists(p_edit.stableLabels, p_edit.labels);
}

Reason Check(const Game &, const EditMove &p_edit)
{
  if (!p_edit.infoset) {
    return "No information set given";
  }
  if (p_edit.infoset->IsChanceInfoset() && p_edit.probs.size() != p_edit.labels.size()) {
    return "The number of probabilities differs from the number of actions";
  }
  return CheckLabelLists(p_edit.stableLabels, p_edit.labels);
}

Reason Check(const Game &, const EditNode &p_edit)
{
  if (!p_edit.node) {
    return "No node given";
  }
  return {};
}

Reason Check(const Game &p_game, const RefineInfosets &p_edit)
{
  if (!p_edit.infoset) {
    return "No information set given";
  }
  if (p_game->IsAbsentMinded(p_edit.infoset)) {
    return "Information sets cannot be refined by a move whose information set is absent-minded";
  }
  return {};
}

Reason Check(const Game &, const JoinInfoset &p_edit)
{
  if (!p_edit.node || p_edit.node->IsTerminal()) {
    return "Only a non-terminal node can join an information set";
  }
  if (!p_edit.infoset) {
    return "No information set given";
  }
  return {};
}

Reason Check(const Game &, const AppendMoveInInfoset &p_edit)
{
  if (!p_edit.node || !p_edit.node->IsTerminal()) {
    return "A move can be appended only at a terminal node";
  }
  if (!p_edit.infoset) {
    return "No information set given";
  }
  return {};
}

Reason Check(const Game &, const AppendMove &p_edit)
{
  if (!p_edit.node || !p_edit.node->IsTerminal()) {
    return "A move can be appended only at a terminal node";
  }
  if (!p_edit.player) {
    return "No player given";
  }
  return {};
}

Reason Check(const Game &, const InsertMove &p_edit)
{
  if (!p_edit.node) {
    return "No node given";
  }
  if (!p_edit.player) {
    return "No player given";
  }
  return {};
}

Reason Check(const Game &, const CopyTree &p_edit)
{
  if (!p_edit.target || !p_edit.source) {
    return "No node given";
  }
  return {};
}

Reason Check(const Game &, const MoveTree &p_edit)
{
  if (!p_edit.target || !p_edit.source) {
    return "No node given";
  }
  return {};
}

Reason Check(const Game &, const DeleteParent &p_edit)
{
  if (!p_edit.node || !p_edit.node->GetParent()) {
    return "The root node has no parent to delete";
  }
  return {};
}

Reason Check(const Game &, const DeleteTree &p_edit)
{
  if (!p_edit.node || p_edit.node->IsTerminal()) {
    return "A terminal node has no subtree to delete";
  }
  return {};
}

Reason Check(const Game &, const SetMovePlayer &p_edit)
{
  if (!p_edit.node || p_edit.node->IsTerminal()) {
    return "A terminal node has no move";
  }
  if (!p_edit.player) {
    return "No player given";
  }
  if (p_edit.player->IsChance() || p_edit.node->GetPlayer()->IsChance()) {
    return "A move cannot be changed between chance and a personal player";
  }
  return {};
}

Reason Check(const Game &, const SetNodeOutcome &p_edit)
{
  if (!p_edit.node) {
    return "No node given";
  }
  if (!p_edit.outcome || p_edit.outcome->IsNull()) {
    return "No outcome given";
  }
  return {};
}

Reason Check(const Game &, const RemoveNodeOutcome &p_edit)
{
  if (!p_edit.node || !HasOutcome(p_edit.node)) {
    return "The node has no outcome";
  }
  return {};
}

Reason Check(const Game &, const MoveOutcome &p_edit)
{
  if (!p_edit.target || !p_edit.source || p_edit.target == p_edit.source) {
    return "An outcome is moved between two different nodes";
  }
  if (!HasOutcome(p_edit.source)) {
    return "The node has no outcome";
  }
  return {};
}

Reason Check(const Game &p_game, const SetNodeOutcomeData &p_edit)
{
  if (!p_edit.node) {
    return "No node given";
  }
  return CheckPayoffCount(p_game, p_edit.payoffs);
}

Reason Check(const Game &p_game, const SetOutcomeData &p_edit)
{
  if (!p_edit.outcome || p_edit.outcome->IsNull()) {
    return "No outcome given";
  }
  return CheckPayoffCount(p_game, p_edit.payoffs);
}

Reason Check(const Game &p_game, const SetContingencyOutcome &p_edit)
{
  if (p_edit.profiles.empty()) {
    return "No contingency given";
  }
  return CheckPayoffCount(p_game, p_edit.payoffs);
}

Reason Check(const Game &, const RemoveContingencyOutcome &p_edit)
{
  if (std::ranges::none_of(p_edit.profiles, [](const PureStrategyProfile &p_profile) {
        return !p_profile->GetOutcome()->IsNull();
      })) {
    return "None of the contingencies has an outcome";
  }
  return {};
}

Reason Check(const Game &, const SetContingencyPayoff &p_edit)
{
  if (!p_edit.player || p_edit.player->IsChance()) {
    return "No player given";
  }
  return {};
}

//
// Changes, one per edit
//

GameModificationType Perform(const Game &p_game, TreeStyle &p_style,
                             const EditGameProperties &p_edit)
{
  GameModificationType modifications = None;
  if (p_game->GetTitle() != p_edit.title || p_game->GetDescription() != p_edit.description) {
    p_game->SetTitle(p_edit.title);
    p_game->SetDescription(p_edit.description);
    modifications |= GameLabels;
  }
  modifications |= EditPlayers(p_game, p_edit.stableLabels, p_edit.labels);

  TreeStyle style = p_style;
  if (As<GameTreeRep>(p_game)) {
    style.SetChanceColor(p_edit.chanceColor);
  }
  // Colours are by position among the players after the edit, which is their number.
  // A colour is recorded only where it differs from the one the player is drawn in already.
  for (size_t i = 0; i < p_edit.playerColors.size(); i++) {
    const int player = static_cast<int>(i) + 1;
    if (style.GetPlayerColor(player) != p_edit.playerColors[i]) {
      style.SetPlayerColor(player, p_edit.playerColors[i]);
    }
  }
  if (!(style == p_style)) {
    p_style = std::move(style);
    modifications |= Presentation;
  }
  return modifications;
}

GameModificationType Perform(const Game &, TreeStyle &p_style, const SetTreeStyle &p_edit)
{
  if (p_edit.style == p_style) {
    return None;
  }
  p_style = p_edit.style;
  return Presentation;
}

GameModificationType Perform(const Game &p_game, const SetStrategies &p_edit)
{
  const auto relabels = Relabelling(p_edit.stableLabels, p_edit.labels);
  if (relabels.empty() && p_edit.stableLabels == LabelsOf(p_edit.player->GetStrategies())) {
    return None;
  }
  p_game->SetStrategies(p_edit.player, p_edit.stableLabels);
  if (!relabels.empty()) {
    p_game->RelabelStrategies(p_edit.player, relabels);
  }
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const EditMove &p_edit)
{
  GameModificationType modifications = None;
  GameInfoset infoset = p_edit.infoset;
  if (infoset->GetLabel() != p_edit.infosetLabel) {
    infoset->SetLabel(p_edit.infosetLabel);
    modifications |= GameLabels;
  }
  if (!infoset->IsChanceInfoset() && p_edit.player && !p_edit.player->IsChance() &&
      p_edit.player != infoset->GetPlayer()) {
    const std::vector<GameNode> members(infoset->GetMembers().begin(),
                                        infoset->GetMembers().end());
    p_game->MakeInfoset(members, p_edit.player, infoset->GetLabel());
    // MakeInfoset forms a new information set, invalidating the handle; the members remain.
    infoset = members.front()->GetInfoset();
    modifications |= GameForm;
  }
  return modifications |
         EditActions(p_game, infoset, p_edit.stableLabels, p_edit.labels, p_edit.probs);
}

GameModificationType Perform(const Game &p_game, const EditNode &p_edit)
{
  GameModificationType modifications = None;
  if (p_edit.node->GetLabel() != p_edit.label) {
    p_edit.node->SetLabel(p_edit.label);
    modifications |= GameLabels;
  }
  if (!p_edit.outcome || p_edit.outcome->IsNull()) {
    if (HasOutcome(p_edit.node)) {
      p_game->MakeOutcomeNull({p_edit.node});
      modifications |= GamePayoffs;
    }
  }
  else {
    modifications |= AttachOutcome(p_game, p_edit.node, p_edit.outcome);
  }
  if (!p_edit.node->IsTerminal()) {
    modifications |= p_edit.infoset ? JoinNodeToInfoset(p_game, p_edit.node, p_edit.infoset)
                                    : LeaveInfoset(p_game, p_edit.node);
  }
  return modifications;
}

GameModificationType Perform(const Game &p_game, const RefineInfosets &p_edit)
{
  for (const auto &player : p_edit.players) {
    p_game->Reveal(p_edit.infoset, player);
  }
  return p_edit.players.empty() ? None : GameForm;
}

GameModificationType Perform(const Game &p_game, const JoinInfoset &p_edit)
{
  return JoinNodeToInfoset(p_game, p_edit.node, p_edit.infoset);
}

GameModificationType Perform(const Game &p_game, const AppendMoveInInfoset &p_edit)
{
  p_game->AppendMove(p_edit.node, p_edit.infoset);
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const AppendMove &p_edit)
{
  if (p_edit.player->IsChance()) {
    p_game->AppendEvent(p_edit.node, p_edit.labels, p_edit.probs);
  }
  else {
    p_game->AppendMove(p_edit.node, p_edit.player, p_edit.labels);
  }
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const InsertMove &p_edit)
{
  if (p_edit.player->IsChance()) {
    p_game->InsertEvent(p_edit.node, p_edit.labels, p_edit.probs);
  }
  else {
    p_game->InsertMove(p_edit.node, p_edit.player, p_edit.labels);
  }
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const CopyTree &p_edit)
{
  p_game->CopyTree(p_edit.target, p_edit.source);
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const MoveTree &p_edit)
{
  p_game->MoveTree(p_edit.target, p_edit.source);
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const DeleteParent &p_edit)
{
  p_game->DeleteParent(p_edit.node);
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const DeleteTree &p_edit)
{
  p_game->DeleteTree(p_edit.node);
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const SetMovePlayer &p_edit)
{
  const GameInfoset infoset = p_edit.node->GetInfoset();
  if (infoset->GetPlayer() == p_edit.player) {
    return None;
  }
  const std::vector<GameNode> members(infoset->GetMembers().begin(), infoset->GetMembers().end());
  p_game->MakeInfoset(members, p_edit.player, infoset->GetLabel());
  return GameForm;
}

GameModificationType Perform(const Game &p_game, const SetNodeOutcome &p_edit)
{
  return AttachOutcome(p_game, p_edit.node, p_edit.outcome);
}

GameModificationType Perform(const Game &p_game, const RemoveNodeOutcome &p_edit)
{
  p_game->MakeOutcomeNull({p_edit.node});
  return GamePayoffs;
}

GameModificationType Perform(const Game &p_game, const MoveOutcome &p_edit)
{
  AttachOutcome(p_game, p_edit.target, p_edit.source->GetOutcome());
  p_game->MakeOutcomeNull({p_edit.source});
  return GamePayoffs;
}

GameModificationType Perform(const Game &p_game, const SetNodeOutcomeData &p_edit)
{
  const GameOutcome outcome = p_edit.node->GetOutcome();
  if (outcome->IsNull()) {
    p_game->MakeOutcome({p_edit.node}, p_edit.payoffs, p_edit.label);
    return GamePayoffs;
  }
  if (OutcomeMatches(p_game, outcome, p_edit.label, p_edit.payoffs)) {
    return None;
  }
  outcome->SetLabel(p_edit.label);
  size_t i = 0;
  for (const auto &player : p_game->GetPlayers()) {
    outcome->SetPayoff(player, p_edit.payoffs[i++]);
  }
  return GamePayoffs;
}

GameModificationType Perform(const Game &p_game, const SetOutcomeData &p_edit)
{
  if (OutcomeMatches(p_game, p_edit.outcome, p_edit.label, p_edit.payoffs)) {
    return None;
  }
  p_edit.outcome->SetLabel(p_edit.label);
  size_t i = 0;
  for (const auto &player : p_game->GetPlayers()) {
    p_edit.outcome->SetPayoff(player, p_edit.payoffs[i++]);
  }
  return GamePayoffs;
}

GameModificationType Perform(const Game &p_game, const SetContingencyOutcome &p_edit)
{
  std::vector<std::vector<GameStrategy>> contingencies;
  contingencies.reserve(p_edit.profiles.size());
  for (const auto &profile : p_edit.profiles) {
    contingencies.push_back(StrategiesOf(p_game, profile));
  }
  p_game->MakeOutcome(contingencies, p_edit.payoffs, p_edit.label);
  return GamePayoffs;
}

GameModificationType Perform(const Game &p_game, const RemoveContingencyOutcome &p_edit)
{
  std::vector<std::vector<GameStrategy>> contingencies;
  for (const auto &profile : p_edit.profiles) {
    if (!profile->GetOutcome()->IsNull()) {
      contingencies.push_back(StrategiesOf(p_game, profile));
    }
  }
  p_game->MakeOutcomeNull(contingencies);
  return GamePayoffs;
}

GameModificationType Perform(const Game &p_game, const SetContingencyPayoff &p_edit)
{
  const GameOutcome outcome = p_edit.profile->GetOutcome();
  if (outcome->IsNull()) {
    std::vector<Number> payoffs;
    payoffs.reserve(p_game->NumPlayers());
    for (const auto &player : p_game->GetPlayers()) {
      payoffs.push_back(player == p_edit.player ? p_edit.value : Number());
    }
    p_game->MakeOutcome({StrategiesOf(p_game, p_edit.profile)}, payoffs,
                        GenerateOutcomeLabel(p_game));
    return GamePayoffs;
  }
  if (outcome->GetPayoff<std::string>(p_edit.player) == Text(p_edit.value)) {
    return None;
  }
  outcome->SetPayoff(p_edit.player, p_edit.value);
  return GamePayoffs;
}

} // namespace

std::optional<std::string> CheckEdit(const Game &p_game, const GameEdit &p_edit)
{
  return std::visit([&p_game](const auto &p_variant) { return Check(p_game, p_variant); }, p_edit);
}

GameModificationType PerformEdit(const Game &p_game, TreeStyle &p_style, const GameEdit &p_edit)
{
  return std::visit(
      [&p_game, &p_style](const auto &p_variant) {
        if constexpr (requires { Perform(p_game, p_style, p_variant); }) {
          return Perform(p_game, p_style, p_variant);
        }
        else {
          return Perform(p_game, p_variant);
        }
      },
      p_edit);
}

std::string GenerateOutcomeLabel(const Game &p_game)
{
  std::set<std::string> outcomeLabels;
  for (const auto &outcome : p_game->GetOutcomes()) {
    outcomeLabels.insert(outcome->GetLabel());
  }
  auto number = p_game->GetOutcomes().size() + 1;
  while (outcomeLabels.contains("Outcome " + std::to_string(number))) {
    number++;
  }
  return "Outcome " + std::to_string(number);
}

} // namespace Gambit::GUI
