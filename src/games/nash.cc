//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/nash.cc
// Framework for computing (sub)sets of Nash equilibria.
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

#include "nash.h"

namespace Gambit::Nash {

// A nested anonymous namespace to privatize these functions

namespace {

///
/// Returns a list of the root nodes of all the immediate proper subgames
/// in the subtree rooted at 'p_node'.
///
std::list<GameNode> ChildSubgames(const GameNode &p_node, const bool p_root = true)
{
  if (!p_root && p_node->IsSubgameRoot()) {
    return {p_node};
  }
  std::list<GameNode> ret;
  for (const auto &child : p_node->GetChildren()) {
    ret.splice(ret.end(), ChildSubgames(child, false));
  }
  return ret;
}

} // namespace

//
// Some general notes on the strategy for solving by subgames:
//
// * We work with a *copy* of the original game, which is destroyed
//   as we go.
// * Before solving, information set labels on the copy game are
//   set to unique IDs.  These are used to match up information
//   sets in the subgames (which are themselves copies) to the
//   original game.
//

template <class T> class SubgameSolution {
private:
  std::map<GameAction, T> profile;
  std::map<GameNode, GameOutcome> node_values;

public:
  SubgameSolution(const std::map<GameAction, T> &p_profile,
                  const std::map<GameNode, GameOutcome> &p_nodeValues)
    : profile(p_profile), node_values(p_nodeValues)
  {
  }

  const std::map<GameAction, T> &GetProfile() const { return profile; }
  const std::map<GameNode, GameOutcome> &GetNodeValues() const { return node_values; }

  SubgameSolution<T> Combine(const SubgameSolution<T> &other) const
  {
    auto combined = *this;
    combined.profile.insert(other.profile.cbegin(), other.profile.cend());
    combined.node_values.insert(other.node_values.cbegin(), other.node_values.cend());
    return combined;
  }

  SubgameSolution<T> Update(const GameNode &p_subroot, const MixedBehaviorProfile<T> &p_profile,
                            const std::map<std::string, GameInfoset> &p_infosetMap)
  {
    SubgameSolution<T> solution = {profile, {{p_subroot, p_subroot->GetGame()->NewOutcome()}}};

    for (const auto &subplayer : p_profile.GetGame()->GetPlayers()) {
      for (const auto &subinfoset : subplayer->GetInfosets()) {
        const GameInfoset infoset = p_infosetMap.at(subinfoset->GetLabel());
        const auto &subactions = subinfoset->GetActions();
        auto subaction = subactions.begin();
        for (const auto &action : infoset->GetActions()) {
          solution.profile[action] = p_profile[*subaction];
          ++subaction;
        }
      }
    }

    const GameOutcome outcome = p_subroot->GetOutcome();
    const auto &subplayers = p_profile.GetGame()->GetPlayers();
    auto subplayer = subplayers.begin();
    for (const auto &player : p_subroot->GetGame()->GetPlayers()) {
      T value = p_profile.GetPayoff(*subplayer);
      if (outcome) {
        value += outcome->GetPayoff<T>(*subplayer);
      }
      solution.node_values[p_subroot]->SetPayoff(player, Number(static_cast<Rational>(value)));
      ++subplayer;
    }
    return solution;
  }
};

template <class T>
std::list<SubgameSolution<T>> SolveSubgames(const GameNode &p_root,
                                            const std::map<std::string, GameInfoset> &p_infosetMap,
                                            BehaviorSolverType<T> p_solver)
{
  std::list<SubgameSolution<T>> subsolutions = {{{}, {}}};
  for (const auto &subroot : ChildSubgames(p_root)) {
    std::list<SubgameSolution<T>> combined_solutions;
    for (const auto &solution : SolveSubgames(subroot, p_infosetMap, p_solver)) {
      for (const auto &subsolution : subsolutions) {
        combined_solutions.push_back(subsolution.Combine(solution));
      }
    }
    if (combined_solutions.empty()) {
      return {};
    }
    subsolutions = combined_solutions;
  }

  std::list<SubgameSolution<T>> solutions;
  for (auto subsolution : subsolutions) {
    for (auto [subroot, outcome] : subsolution.GetNodeValues()) {
      subroot->GetGame()->SetOutcome(subroot, outcome);
    }
    // This prevents double-counting of outcomes at roots of subgames.
    // By convention, we will just put the payoffs in the parent subgame.
    const Game subgame = p_root->GetGame()->CopySubgame(p_root);
    subgame->SetOutcome(subgame->GetRoot(), nullptr);

    for (const auto &solution : p_solver(subgame)) {
      solutions.push_back(subsolution.Update(p_root, solution, p_infosetMap));
    }
  }

  p_root->GetGame()->DeleteTree(p_root);
  return solutions;
}

template <class T>
MixedBehaviorProfile<T> BuildProfile(const Game &p_game, const SubgameSolution<T> &p_solution)
{
  MixedBehaviorProfile<T> profile(p_game);
  for (const auto &player : p_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        profile[action] = p_solution.GetProfile().at(action);
      }
    }
  }
  return profile;
}

template <class T>
List<MixedBehaviorProfile<T>> SolveBySubgames(const Game &p_game, BehaviorSolverType<T> p_solver,
                                              BehaviorCallbackType<T> p_onEquilibrium)
{
  const Game efg = p_game->CopySubgame(p_game->GetRoot());

  int index = 1;
  std::map<std::string, GameInfoset> infoset_map;
  for (const auto &player : p_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      infoset_map[std::to_string(index++)] = infoset;
    }
  }
  index = 1;
  for (const auto &player : efg->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      infoset->SetLabel(std::to_string(index++));
    }
  }

  auto results = SolveSubgames(efg->GetRoot(), infoset_map, p_solver);
  List<MixedBehaviorProfile<T>> solutions;
  for (const auto &result : results) {
    solutions.push_back(BuildProfile(p_game, result));
    p_onEquilibrium(solutions.back(), "NE");
  }
  return solutions;
}

template List<MixedBehaviorProfile<double>>
SolveBySubgames(const Game &p_game, BehaviorSolverType<double> p_solver,
                BehaviorCallbackType<double> p_onEquilibrium);
template List<MixedBehaviorProfile<Rational>>
SolveBySubgames(const Game &p_game, BehaviorSolverType<Rational> p_solver,
                BehaviorCallbackType<Rational> p_onEquilibrium);

} // namespace Gambit::Nash
