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

template <class T>
void MixedStrategyCSVRenderer<T>::Render(const MixedStrategyProfile<T> &p_profile,
                                         const std::string &p_label) const
{
  m_stream << p_label;
  for (size_t i = 1; i <= p_profile.MixedProfileLength(); i++) {
    m_stream << "," << lexical_cast<std::string>(p_profile[i], m_numDecimals);
  }
  m_stream << std::endl;
}

template <class T>
void MixedStrategyDetailRenderer<T>::Render(const MixedStrategyProfile<T> &p_profile,
                                            const std::string &p_label) const
{
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    m_stream << "Strategy profile for player " << player->GetNumber() << ":\n";

    m_stream << "Strategy   Prob          Value\n";
    m_stream << "--------   -----------   -----------\n";

    for (auto strategy : player->GetStrategies()) {
      if (!strategy->GetLabel().empty()) {
        m_stream << std::setw(8) << strategy->GetLabel() << "    ";
      }
      else {
        m_stream << std::setw(8) << strategy->GetNumber() << "    ";
      }
      m_stream << std::setw(10);
      m_stream << lexical_cast<std::string>(p_profile[strategy], m_numDecimals);
      m_stream << "   ";
      m_stream << std::setw(11);
      m_stream << lexical_cast<std::string>(p_profile.GetPayoff(strategy), m_numDecimals);
      m_stream << std::endl;
    }
  }
}

template <class T>
void BehavStrategyCSVRenderer<T>::Render(const MixedBehaviorProfile<T> &p_profile,
                                         const std::string &p_label) const
{
  m_stream << p_label;
  for (size_t i = 1; i <= p_profile.BehaviorProfileLength(); i++) {
    m_stream << "," << lexical_cast<std::string>(p_profile[i], m_numDecimals);
  }
  m_stream << std::endl;
}

template <class T>
void BehavStrategyDetailRenderer<T>::Render(const MixedBehaviorProfile<T> &p_profile,
                                            const std::string &p_label) const
{
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    m_stream << "Behavior profile for player " << player->GetNumber() << ":\n";

    m_stream << "Infoset    Action     Prob          Value\n";
    m_stream << "-------    -------    -----------   -----------\n";

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);

      for (int act = 1; act <= infoset->NumActions(); act++) {
        GameAction action = infoset->GetAction(act);

        if (!infoset->GetLabel().empty()) {
          m_stream << std::setw(7) << infoset->GetLabel() << "    ";
        }
        else {
          m_stream << std::setw(7) << infoset->GetNumber() << "    ";
        }
        if (!action->GetLabel().empty()) {
          m_stream << std::setw(7) << action->GetLabel() << "   ";
        }
        else {
          m_stream << std::setw(7) << action->GetNumber() << "   ";
        }
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile[action], m_numDecimals);
        m_stream << "   ";
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile.GetPayoff(infoset->GetAction(act)),
                                              m_numDecimals);
        m_stream << std::endl;
      }
    }

    m_stream << std::endl;
    m_stream << "Infoset    Node       Belief        Prob\n";
    m_stream << "-------    -------    -----------   -----------\n";

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);

      for (int n = 1; n <= infoset->NumMembers(); n++) {
        GameNode node = infoset->GetMember(n);
        if (!infoset->GetLabel().empty()) {
          m_stream << std::setw(7) << infoset->GetLabel() << "    ";
        }
        else {
          m_stream << std::setw(7) << infoset->GetNumber() << "    ";
        }
        if (!node->GetLabel().empty()) {
          m_stream << std::setw(7) << node->GetLabel() << "   ";
        }
        else {
          m_stream << std::setw(7) << node->GetNumber() << "   ";
        }
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile.GetBeliefProb(infoset->GetMember(n)),
                                              m_numDecimals);
        m_stream << "   ";
        m_stream << std::setw(11);
        m_stream << lexical_cast<std::string>(p_profile.GetRealizProb(infoset->GetMember(n)),
                                              m_numDecimals);
        m_stream << std::endl;
      }
    }
    m_stream << std::endl;
  }
}

template class MixedStrategyRenderer<double>;
template class MixedStrategyRenderer<Rational>;

template class MixedStrategyNullRenderer<double>;
template class MixedStrategyNullRenderer<Rational>;

template class MixedStrategyCSVRenderer<double>;
template class MixedStrategyCSVRenderer<Rational>;

template class MixedStrategyDetailRenderer<double>;
template class MixedStrategyDetailRenderer<Rational>;

template class StrategyProfileRenderer<double>;
template class StrategyProfileRenderer<Rational>;

template class BehavStrategyNullRenderer<double>;
template class BehavStrategyNullRenderer<Rational>;

template class BehavStrategyCSVRenderer<double>;
template class BehavStrategyCSVRenderer<Rational>;

template class BehavStrategyDetailRenderer<double>;
template class BehavStrategyDetailRenderer<Rational>;

template <class T>
StrategySolver<T>::StrategySolver(
    std::shared_ptr<StrategyProfileRenderer<T>> p_onEquilibrium /* = 0 */)
  : m_onEquilibrium(p_onEquilibrium)
{
  if (m_onEquilibrium.get() == nullptr) {
    m_onEquilibrium.reset(new MixedStrategyNullRenderer<T>());
  }
}

template <class T>
BehavSolver<T>::BehavSolver(std::shared_ptr<StrategyProfileRenderer<T>> p_onEquilibrium /* = 0 */)
  : m_onEquilibrium(p_onEquilibrium)
{
  if (m_onEquilibrium.get() == nullptr) {
    m_onEquilibrium.reset(new BehavStrategyNullRenderer<T>());
  }
}

template <class T>
BehavViaStrategySolver<T>::BehavViaStrategySolver(
    std::shared_ptr<StrategySolver<T>> p_solver,
    std::shared_ptr<StrategyProfileRenderer<T>> p_onEquilibrium /* = 0 */)
  : BehavSolver<T>(p_onEquilibrium), m_solver(p_solver)
{
}

template <class T>
List<MixedBehaviorProfile<T>> BehavViaStrategySolver<T>::Solve(const Game &p_game) const
{
  List<MixedStrategyProfile<T>> output = m_solver->Solve(p_game);
  List<MixedBehaviorProfile<T>> solutions;
  for (const auto &profile : output) {
    solutions.push_back(MixedBehaviorProfile<T>(profile));
  }
  return solutions;
}

template <class T>
SubgameBehavSolver<T>::SubgameBehavSolver(
    std::shared_ptr<BehavSolver<T>> p_solver,
    std::shared_ptr<StrategyProfileRenderer<T>> p_onEquilibrium /* = 0 */)
  : BehavSolver<T>(p_onEquilibrium), m_solver(p_solver)
{
}

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
        GameInfoset infoset = p_infosetMap.at(subinfoset->GetLabel());
        const auto &subactions = subinfoset->GetActions();
        auto subaction = subactions.begin();
        for (const auto &action : infoset->GetActions()) {
          solution.profile[action] = p_profile[*subaction];
          ++subaction;
        }
      }
    }

    GameOutcome outcome = p_subroot->GetOutcome();
    const auto &subplayers = p_profile.GetGame()->GetPlayers();
    auto subplayer = subplayers.begin();
    for (const auto &player : p_subroot->GetGame()->GetPlayers()) {
      T value = p_profile.GetPayoff(*subplayer);
      if (outcome) {
        value += static_cast<T>(outcome->GetPayoff(*subplayer));
      }
      solution.node_values[p_subroot]->SetPayoff(player, Number(static_cast<Rational>(value)));
      ++subplayer;
    }
    return solution;
  }
};

template <class T>
std::list<SubgameSolution<T>>
SubgameBehavSolver<T>::SolveSubgames(const GameNode &p_root,
                                     const std::map<std::string, GameInfoset> &p_infosetMap) const
{
  std::list<SubgameSolution<T>> subsolutions = {{{}, {}}};
  for (const auto &subroot : ChildSubgames(p_root)) {
    std::list<SubgameSolution<T>> combined_solutions;
    for (const auto &solution : SolveSubgames(subroot, p_infosetMap)) {
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
      subroot->SetOutcome(outcome);
    }
    // This prevents double-counting of outcomes at roots of subgames.
    // By convention, we will just put the payoffs in the parent subgame.
    Game subgame = p_root->CopySubgame();
    subgame->GetRoot()->SetOutcome(nullptr);

    for (const auto &solution : m_solver->Solve(subgame)) {
      solutions.push_back(subsolution.Update(p_root, solution, p_infosetMap));
    }
  }

  p_root->DeleteTree();
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
List<MixedBehaviorProfile<T>> SubgameBehavSolver<T>::Solve(const Game &p_game) const
{
  Game efg = p_game->GetRoot()->CopySubgame();

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

  auto results = SolveSubgames(efg->GetRoot(), infoset_map);
  List<MixedBehaviorProfile<T>> solutions;
  for (const auto &result : results) {
    solutions.push_back(BuildProfile(p_game, result));
    this->m_onEquilibrium->Render(solutions.back());
  }
  return solutions;
}

template class StrategySolver<double>;
template class StrategySolver<Rational>;

template class BehavSolver<double>;
template class BehavSolver<Rational>;

template class BehavViaStrategySolver<double>;
template class BehavViaStrategySolver<Rational>;

template class SubgameBehavSolver<double>;
template class SubgameBehavSolver<Rational>;

} // namespace Gambit::Nash
