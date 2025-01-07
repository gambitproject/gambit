//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solver/enumpoly/behavextend.cc
// Algorithms for extending behavior profiles to Nash equilibria
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

#include "behavextend.h"
#include "polysystem.h"
#include "polyfeasible.h"

namespace {

using namespace Gambit;

void TerminalDescendants(const GameNode &p_node, std::list<GameNode> &current)
{
  if (p_node->IsTerminal()) {
    current.push_back(p_node);
  }
  else {
    for (auto child : p_node->GetChildren()) {
      TerminalDescendants(child, current);
    }
  }
}

std::list<GameNode> TerminalNodes(const Game &p_efg)
{
  std::list<GameNode> ret;
  TerminalDescendants(p_efg->GetRoot(), ret);
  return ret;
}

void DeviationInfosets(List<GameInfoset> &answer, const BehaviorSupportProfile &big_supp,
                       const GamePlayer &p_player, const GameNode &p_node,
                       const GameAction &p_action)
{
  GameNode child = p_node->GetChild(p_action);
  if (child->IsTerminal()) {
    return;
  }
  GameInfoset iset = child->GetInfoset();
  if (iset->GetPlayer() == p_player) {
    size_t insert = 0;
    bool done = false;
    while (!done) {
      insert++;
      if (insert > answer.size() || iset->Precedes(answer[insert]->GetMember(1))) {
        done = true;
      }
    }
    answer.insert(std::next(answer.begin(), insert - 1), iset);
  }

  for (auto action : iset->GetActions()) {
    DeviationInfosets(answer, big_supp, p_player, child, action);
  }
}

List<GameInfoset> DeviationInfosets(const BehaviorSupportProfile &big_supp,
                                    const GamePlayer &p_player, const GameInfoset &p_infoset,
                                    const GameAction &p_action)
{
  List<GameInfoset> answer;
  for (auto member : p_infoset->GetMembers()) {
    DeviationInfosets(answer, big_supp, p_player, member, p_action);
  }
  return answer;
}

PolynomialSystem<double> ActionProbsSumToOneIneqs(const MixedBehaviorProfile<double> &p_solution,
                                                  std::shared_ptr<VariableSpace> BehavStratSpace,
                                                  const BehaviorSupportProfile &big_supp,
                                                  const std::map<GameInfoset, int> &var_index)
{
  PolynomialSystem<double> answer(BehavStratSpace);

  for (auto player : p_solution.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      if (!big_supp.HasAction(infoset)) {
        int index_base = var_index.at(infoset);
        Polynomial<double> factor(BehavStratSpace, 1.0);
        for (int k = 1; k < infoset->NumActions(); k++) {
          factor -= Polynomial<double>(BehavStratSpace, index_base + k, 1);
        }
        answer.push_back(factor);
      }
    }
  }
  return answer;
}

std::list<BehaviorSupportProfile> DeviationSupports(const BehaviorSupportProfile &big_supp,
                                                    const List<GameInfoset> &isetlist)
{
  std::list<BehaviorSupportProfile> answer;

  Array<int> active_act_no(isetlist.size());
  for (int k = 1; k <= active_act_no.size(); k++) {
    active_act_no[k] = 0;
  }

  BehaviorSupportProfile new_supp(big_supp);

  for (size_t i = 1; i <= isetlist.size(); i++) {
    for (int j = 1; j < isetlist[i]->NumActions(); j++) {
      new_supp.RemoveAction(isetlist[i]->GetAction(j));
    }
    new_supp.AddAction(isetlist[i]->GetAction(1));

    active_act_no[i] = 1;
    for (size_t k = 1; k < i; k++) {
      if (isetlist[k]->Precedes(isetlist[i]->GetMember(1))) {
        if (isetlist[k]->GetAction(1)->Precedes(isetlist[i]->GetMember(1))) {
          new_supp.RemoveAction(isetlist[i]->GetAction(1));
          active_act_no[i] = 0;
        }
      }
    }
  }
  answer.push_back(new_supp);

  int iset_cursor = isetlist.size();
  while (iset_cursor > 0) {
    if (active_act_no[iset_cursor] == 0 ||
        active_act_no[iset_cursor] == isetlist[iset_cursor]->NumActions()) {
      iset_cursor--;
    }
    else {
      new_supp.RemoveAction(isetlist[iset_cursor]->GetAction(active_act_no[iset_cursor]));
      active_act_no[iset_cursor]++;
      new_supp.AddAction(isetlist[iset_cursor]->GetAction(active_act_no[iset_cursor]));
      for (size_t k = iset_cursor + 1; k <= isetlist.size(); k++) {
        if (active_act_no[k] > 0) {
          new_supp.RemoveAction(isetlist[k]->GetAction(1));
        }
        size_t h = 1;
        bool active = true;
        while (active && h < k) {
          if (isetlist[h]->Precedes(isetlist[k]->GetMember(1))) {
            if (active_act_no[h] == 0 ||
                !isetlist[h]->GetAction(active_act_no[h])->Precedes(isetlist[k]->GetMember(1))) {
              active = false;
              if (active_act_no[k] > 0) {
                new_supp.RemoveAction(isetlist[k]->GetAction(active_act_no[k]));
                active_act_no[k] = 0;
              }
            }
          }
          h++;
        }
        if (active) {
          new_supp.AddAction(isetlist[k]->GetAction(1));
          active_act_no[k] = 1;
        }
      }
      answer.push_back(new_supp);
    }
  }
  return answer;
}

bool NashNodeProbabilityPoly(const MixedBehaviorProfile<double> &p_solution,
                             Polynomial<double> &node_prob,
                             std::shared_ptr<VariableSpace> BehavStratSpace,
                             const BehaviorSupportProfile &dsupp,
                             const std::map<GameInfoset, int> &var_index, GameNode tempnode,
                             const GameInfoset &iset, const GameAction &act)
{
  while (tempnode != p_solution.GetGame()->GetRoot()) {
    GameAction last_action = tempnode->GetPriorAction();
    GameInfoset last_infoset = last_action->GetInfoset();

    if (last_infoset->IsChanceInfoset()) {
      node_prob *= static_cast<double>(last_infoset->GetActionProb(last_action->GetNumber()));
    }
    else if (dsupp.HasAction(last_infoset)) {
      if (last_infoset == iset) {
        if (act != last_action) {
          return false;
        }
      }
      else if (dsupp.Contains(last_action)) {
        if (last_action->GetInfoset()->GetPlayer() != act->GetInfoset()->GetPlayer() ||
            !act->Precedes(tempnode)) {
          node_prob *= p_solution.GetActionProb(last_action);
        }
      }
      else {
        return false;
      }
    }
    else {
      int initial_var_no = var_index.at(last_infoset);
      if (last_action->GetNumber() < last_infoset->NumActions()) {
        int varno = initial_var_no + last_action->GetNumber();
        node_prob *= Polynomial<double>(BehavStratSpace, varno, 1);
      }
      else {
        Polynomial<double> factor(BehavStratSpace, 1.0);
        for (int k = 1; k < last_infoset->NumActions(); k++) {
          factor -= Polynomial<double>(BehavStratSpace, initial_var_no + k, 1);
        }
        node_prob *= factor;
      }
    }
    tempnode = tempnode->GetParent();
  }
  return true;
}

PolynomialSystem<double> NashExpectedPayoffDiffPolys(
    const MixedBehaviorProfile<double> &p_solution, std::shared_ptr<VariableSpace> BehavStratSpace,
    const BehaviorSupportProfile &little_supp, const BehaviorSupportProfile &big_supp,
    const std::map<GameInfoset, int> &var_index)
{
  PolynomialSystem<double> answer(BehavStratSpace);

  auto terminal_nodes = TerminalNodes(p_solution.GetGame());

  for (auto player : p_solution.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      if (!little_supp.IsReachable(infoset)) {
        continue;
      }
      for (auto action : infoset->GetActions()) {
        if (little_supp.Contains(action)) {
          continue;
        }
        auto isetlist = DeviationInfosets(big_supp, player, infoset, action);
        auto dsupps = DeviationSupports(big_supp, isetlist);
        for (auto support : dsupps) {
          // The utility difference between the
          // payoff resulting from the profile and deviation to
          // the strategy for pl specified by dsupp[k]
          Polynomial<double> next_poly(BehavStratSpace);

          for (auto node : terminal_nodes) {
            Polynomial<double> node_prob(BehavStratSpace, 1.0);
            if (NashNodeProbabilityPoly(p_solution, node_prob, BehavStratSpace, support, var_index,
                                        node, infoset, action)) {
              if (node->GetOutcome()) {
                node_prob *= static_cast<double>(node->GetOutcome()->GetPayoff(player));
              }
              next_poly += node_prob;
            }
          }
          answer.push_back(-next_poly + p_solution.GetPayoff(player));
        }
      }
    }
  }
  return answer;
}

PolynomialSystem<double> ExtendsToNashIneqs(const MixedBehaviorProfile<double> &p_solution,
                                            std::shared_ptr<VariableSpace> BehavStratSpace,
                                            const BehaviorSupportProfile &little_supp,
                                            const BehaviorSupportProfile &big_supp,
                                            const std::map<GameInfoset, int> &var_index)
{
  PolynomialSystem<double> answer(BehavStratSpace);
  answer.push_back(ActionProbsSumToOneIneqs(p_solution, BehavStratSpace, big_supp, var_index));
  answer.push_back(
      NashExpectedPayoffDiffPolys(p_solution, BehavStratSpace, little_supp, big_supp, var_index));
  return answer;
}

} // end anonymous namespace

namespace Gambit {
namespace Nash {

bool ExtendsToNash(const MixedBehaviorProfile<double> &p_solution,
                   const BehaviorSupportProfile &little_supp,
                   const BehaviorSupportProfile &big_supp)
{

  // First we compute the number of variables, and indexing information
  int num_vars = 0;
  std::map<GameInfoset, int> var_index;
  for (auto player : p_solution.GetGame()->GetPlayers()) {
    List<int> list_for_pl;
    for (auto infoset : player->GetInfosets()) {
      var_index[infoset] = num_vars;
      if (!big_supp.HasAction(infoset)) {
        num_vars += infoset->NumActions() - 1;
      }
    }
  }

  // We establish the space
  auto BehavStratSpace = std::make_shared<VariableSpace>(num_vars);

  PolynomialSystem<double> inequalities =
      ExtendsToNashIneqs(p_solution, BehavStratSpace, little_supp, big_supp, var_index);
  // set up the rectangle of search
  Vector<double> bottoms(num_vars), tops(num_vars);
  bottoms = 0;
  tops = 1;
  return PolynomialFeasibilitySolver(inequalities).HasSolution(Rectangle<double>(bottoms, tops));
}

} // namespace Nash
} // end namespace Gambit

namespace {

bool ANFNodeProbabilityPoly(const MixedBehaviorProfile<double> &p_solution,
                            Polynomial<double> &node_prob,
                            std::shared_ptr<VariableSpace> BehavStratSpace,
                            const BehaviorSupportProfile &big_supp,
                            const std::map<GameInfoset, int> &var_index, GameNode tempnode, int pl,
                            int i, int j)
{
  while (tempnode != p_solution.GetGame()->GetRoot()) {
    GameAction last_action = tempnode->GetPriorAction();
    GameInfoset last_infoset = last_action->GetInfoset();

    if (last_infoset->IsChanceInfoset()) {
      node_prob *= static_cast<double>(last_infoset->GetActionProb(last_action->GetNumber()));
    }
    else if (big_supp.HasAction(last_infoset)) {
      if (last_infoset == p_solution.GetGame()->GetPlayer(pl)->GetInfoset(i)) {
        if (j != last_action->GetNumber()) {
          return false;
        }
      }
      else if (big_supp.Contains(last_action)) {
        node_prob *= p_solution.GetActionProb(last_action);
      }
      else {
        return false;
      }
    }
    else {
      int initial_var_no = var_index.at(last_infoset);
      if (last_action->GetNumber() < last_infoset->NumActions()) {
        int varno = initial_var_no + last_action->GetNumber();
        node_prob *= Polynomial<double>(BehavStratSpace, varno, 1);
      }
      else {
        Polynomial<double> factor(BehavStratSpace, 1.0);
        for (int k = 1; k < last_infoset->NumActions(); k++) {
          factor -= Polynomial<double>(BehavStratSpace, initial_var_no + k, 1);
        }
        node_prob *= factor;
      }
    }
    tempnode = tempnode->GetParent();
  }
  return true;
}

PolynomialSystem<double> ANFExpectedPayoffDiffPolys(const MixedBehaviorProfile<double> &p_solution,
                                                    std::shared_ptr<VariableSpace> BehavStratSpace,
                                                    const BehaviorSupportProfile &little_supp,
                                                    const BehaviorSupportProfile &big_supp,
                                                    const std::map<GameInfoset, int> &var_index)
{
  PolynomialSystem<double> answer(BehavStratSpace);

  auto terminal_nodes = TerminalNodes(p_solution.GetGame());

  for (auto player : p_solution.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      if (!little_supp.IsReachable(infoset)) {
        continue;
      }
      for (auto action : infoset->GetActions()) {
        if (little_supp.Contains(action)) {
          continue;
        }
        // This will be the utility difference between the
        // payoff resulting from the profile and deviation to
        // action j
        Polynomial<double> next_poly(BehavStratSpace);
        for (auto terminal : terminal_nodes) {
          Polynomial<double> node_prob(BehavStratSpace, 1.0);
          if (ANFNodeProbabilityPoly(p_solution, node_prob, BehavStratSpace, big_supp, var_index,
                                     terminal, player->GetNumber(), infoset->GetNumber(),
                                     action->GetNumber())) {
            if (terminal->GetOutcome()) {
              node_prob *= static_cast<double>(terminal->GetOutcome()->GetPayoff(player));
            }
            next_poly += node_prob;
          }
        }
        answer.push_back(-next_poly + p_solution.GetPayoff(player));
      }
    }
  }
  return answer;
}

PolynomialSystem<double> ExtendsToANFNashIneqs(const MixedBehaviorProfile<double> &p_solution,
                                               std::shared_ptr<VariableSpace> BehavStratSpace,
                                               const BehaviorSupportProfile &little_supp,
                                               const BehaviorSupportProfile &big_supp,
                                               const std::map<GameInfoset, int> &var_index)
{
  PolynomialSystem<double> answer(BehavStratSpace);
  answer.push_back(ActionProbsSumToOneIneqs(p_solution, BehavStratSpace, big_supp, var_index));
  answer.push_back(
      ANFExpectedPayoffDiffPolys(p_solution, BehavStratSpace, little_supp, big_supp, var_index));
  return answer;
}

} // end anonymous namespace

namespace Gambit {
namespace Nash {

bool ExtendsToAgentNash(const MixedBehaviorProfile<double> &p_solution,
                        const BehaviorSupportProfile &little_supp,
                        const BehaviorSupportProfile &big_supp)
{
  // First we compute the number of variables, and indexing information
  int num_vars = 0;
  std::map<GameInfoset, int> var_index;
  for (auto player : p_solution.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      var_index[infoset] = num_vars;
      if (!big_supp.HasAction(infoset)) {
        num_vars += infoset->NumActions() - 1;
      }
    }
  }

  // We establish the space
  auto BehavStratSpace = std::make_shared<VariableSpace>(num_vars);
  PolynomialSystem<double> inequalities =
      ExtendsToANFNashIneqs(p_solution, BehavStratSpace, little_supp, big_supp, var_index);

  // set up the rectangle of search
  Vector<double> bottoms(num_vars), tops(num_vars);
  bottoms = 0;
  tops = 1;

  return PolynomialFeasibilitySolver(inequalities).HasSolution(Rectangle<double>(bottoms, tops));
}

} // namespace Nash
} // end namespace Gambit
