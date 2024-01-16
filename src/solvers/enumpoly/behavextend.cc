//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/behavextend.cc
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
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "ineqsolv.h"

void TerminalDescendants(const Gambit::GameNode &p_node, Gambit::List<Gambit::GameNode> &current)
{
  if (p_node->IsTerminal()) {
    current.push_back(p_node);
  }
  else {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      TerminalDescendants(p_node->GetChild(i), current);
    }
  }
}

Gambit::List<Gambit::GameNode> TerminalNodes(const Gambit::Game &p_efg)
{
  Gambit::List<Gambit::GameNode> ret;
  TerminalDescendants(p_efg->GetRoot(), ret);
  return ret;
}

//
// Design choice: the auxiliary functions here are made static
// rather than members to help hide the gPoly-related details of
// the implementation.  Some of these functions might be more
// generally useful, in which case they should be made visible
// somehow.  Also, a namespace would be preferable to using
// static, but static is used for portability.  -- TLT, 5/2001.
//

//=========================================================================
//                      class algExtendsToNash
//=========================================================================

static void DeviationInfosets(Gambit::List<Gambit::GameInfoset> &answer,
                              const Gambit::BehaviorSupportProfile &big_supp,
                              const Gambit::GamePlayer &pl, const Gambit::GameNode &node,
                              const Gambit::GameAction &act)
{
  Gambit::GameNode child = node->GetChild(act->GetNumber());
  if (!child->IsTerminal()) {
    Gambit::GameInfoset iset = child->GetInfoset();
    if (iset->GetPlayer() == pl) {
      int insert = 0;
      bool done = false;
      while (!done) {
        insert++;
        if (insert > answer.Length() || iset->Precedes(answer[insert]->GetMember(1))) {
          done = true;
        }
      }
      answer.Insert(iset, insert);
    }

    Gambit::List<Gambit::GameAction> action_list;
    for (int j = 1; j <= iset->NumActions(); j++) {
      action_list.push_back(iset->GetAction(j));
    }
    for (int j = 1; j <= action_list.Length(); j++) {
      DeviationInfosets(answer, big_supp, pl, child, action_list[j]);
    }
  }
}

static Gambit::List<Gambit::GameInfoset>
DeviationInfosets(const Gambit::BehaviorSupportProfile &big_supp, const Gambit::GamePlayer &pl,
                  const Gambit::GameInfoset &iset, const Gambit::GameAction &act)
{
  Gambit::List<Gambit::GameInfoset> answer;

  Gambit::List<Gambit::GameNode> node_list;
  for (int i = 1; i <= iset->NumMembers(); i++) {
    node_list.push_back(iset->GetMember(i));
  }

  for (int i = 1; i <= node_list.Length(); i++) {
    DeviationInfosets(answer, big_supp, pl, node_list[i], act);
  }

  return answer;
}

static gPolyList<double>
ActionProbsSumToOneIneqs(const Gambit::MixedBehaviorProfile<double> &p_solution,
                         const gSpace &BehavStratSpace, const term_order &Lex,
                         const Gambit::BehaviorSupportProfile &big_supp,
                         const Gambit::List<Gambit::List<int>> &var_index)
{
  gPolyList<double> answer(&BehavStratSpace, &Lex);

  for (int pl = 1; pl <= p_solution.GetGame()->NumPlayers(); pl++) {
    for (int i = 1; i <= p_solution.GetGame()->GetPlayer(pl)->NumInfosets(); i++) {
      Gambit::GameInfoset current_infoset = p_solution.GetGame()->GetPlayer(pl)->GetInfoset(i);
      if (!big_supp.HasAction(current_infoset)) {
        int index_base = var_index[pl][i];
        gPoly<double> factor(&BehavStratSpace, (double)1.0, &Lex);
        for (int k = 1; k < current_infoset->NumActions(); k++) {
          factor -= gPoly<double>(&BehavStratSpace, index_base + k, 1, &Lex);
        }
        answer += factor;
      }
    }
  }
  return answer;
}

static Gambit::List<Gambit::BehaviorSupportProfile>
DeviationSupports(const Gambit::BehaviorSupportProfile &big_supp,
                  const Gambit::List<Gambit::GameInfoset> &isetlist,
                  const Gambit::GamePlayer & /*pl*/, const Gambit::GameInfoset & /*iset*/,
                  const Gambit::GameAction & /*act*/)
{
  Gambit::List<Gambit::BehaviorSupportProfile> answer;

  Gambit::Array<int> active_act_no(isetlist.Length());

  for (int k = 1; k <= active_act_no.Length(); k++) {
    active_act_no[k] = 0;
  }

  Gambit::BehaviorSupportProfile new_supp(big_supp);

  for (int i = 1; i <= isetlist.Length(); i++) {
    for (int j = 1; j < isetlist[i]->NumActions(); j++) {
      new_supp.RemoveAction(isetlist[i]->GetAction(j));
    }
    new_supp.AddAction(isetlist[i]->GetAction(1));

    active_act_no[i] = 1;
    for (int k = 1; k < i; k++) {
      if (isetlist[k]->Precedes(isetlist[i]->GetMember(1))) {
        if (isetlist[k]->GetAction(1)->Precedes(isetlist[i]->GetMember(1))) {
          new_supp.RemoveAction(isetlist[i]->GetAction(1));
          active_act_no[i] = 0;
        }
      }
    }
  }
  answer.push_back(new_supp);

  int iset_cursor = isetlist.Length();
  while (iset_cursor > 0) {
    if (active_act_no[iset_cursor] == 0 ||
        active_act_no[iset_cursor] == isetlist[iset_cursor]->NumActions()) {
      iset_cursor--;
    }
    else {
      new_supp.RemoveAction(isetlist[iset_cursor]->GetAction(active_act_no[iset_cursor]));
      active_act_no[iset_cursor]++;
      new_supp.AddAction(isetlist[iset_cursor]->GetAction(active_act_no[iset_cursor]));
      for (int k = iset_cursor + 1; k <= isetlist.Length(); k++) {
        if (active_act_no[k] > 0) {
          new_supp.RemoveAction(isetlist[k]->GetAction(1));
        }
        int h = 1;
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

static bool NashNodeProbabilityPoly(const Gambit::MixedBehaviorProfile<double> &p_solution,
                                    gPoly<double> &node_prob, const gSpace &BehavStratSpace,
                                    const term_order &Lex,
                                    const Gambit::BehaviorSupportProfile &dsupp,
                                    const Gambit::List<Gambit::List<int>> &var_index,
                                    Gambit::GameNode tempnode, const Gambit::GamePlayer & /*pl*/,
                                    const Gambit::GameInfoset &iset, const Gambit::GameAction &act)
{
  while (tempnode != p_solution.GetGame()->GetRoot()) {

    Gambit::GameAction last_action = tempnode->GetPriorAction();
    Gambit::GameInfoset last_infoset = last_action->GetInfoset();

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
          node_prob *= (double)p_solution.GetActionProb(last_action);
        }
      }
      else {
        return false;
      }
    }
    else {
      int initial_var_no =
          var_index[last_infoset->GetPlayer()->GetNumber()][last_infoset->GetNumber()];
      if (last_action->GetNumber() < last_infoset->NumActions()) {
        int varno = initial_var_no + last_action->GetNumber();
        node_prob *= gPoly<double>(&BehavStratSpace, varno, 1, &Lex);
      }
      else {
        gPoly<double> factor(&BehavStratSpace, (double)1.0, &Lex);
        int k;
        for (k = 1; k < last_infoset->NumActions(); k++) {
          factor -= gPoly<double>(&BehavStratSpace, initial_var_no + k, 1, &Lex);
        }
        node_prob *= factor;
      }
    }
    tempnode = tempnode->GetParent();
  }
  return true;
}

static gPolyList<double>
NashExpectedPayoffDiffPolys(const Gambit::MixedBehaviorProfile<double> &p_solution,
                            const gSpace &BehavStratSpace, const term_order &Lex,
                            const Gambit::BehaviorSupportProfile &little_supp,
                            const Gambit::BehaviorSupportProfile &big_supp,
                            const Gambit::List<Gambit::List<int>> &var_index)
{
  gPolyList<double> answer(&BehavStratSpace, &Lex);

  Gambit::List<Gambit::GameNode> terminal_nodes = TerminalNodes(p_solution.GetGame());

  for (int pl = 1; pl <= p_solution.GetGame()->NumPlayers(); pl++) {
    Gambit::Array<Gambit::GameInfoset> isets_for_pl;
    for (int iset = 1; iset <= p_solution.GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      isets_for_pl.push_back(p_solution.GetGame()->GetPlayer(pl)->GetInfoset(iset));
    }

    for (int i = 1; i <= isets_for_pl.Length(); i++) {
      if (little_supp.IsReachable(isets_for_pl[i])) {
        Gambit::Array<Gambit::GameAction> acts_for_iset;
        for (int act = 1; act <= isets_for_pl[i]->NumActions(); act++) {
          acts_for_iset.push_back(isets_for_pl[i]->GetAction(act));
        }

        for (int j = 1; j <= acts_for_iset.Length(); j++) {
          if (!little_supp.Contains(acts_for_iset[j])) {
            Gambit::List<Gambit::GameInfoset> isetlist = DeviationInfosets(
                big_supp, p_solution.GetGame()->GetPlayer(pl), isets_for_pl[i], acts_for_iset[j]);
            Gambit::List<Gambit::BehaviorSupportProfile> dsupps =
                DeviationSupports(big_supp, isetlist, p_solution.GetGame()->GetPlayer(pl),
                                  isets_for_pl[i], acts_for_iset[j]);
            for (int k = 1; k <= dsupps.Length(); k++) {

              // This will be the utility difference between the
              // payoff resulting from the profile and deviation to
              // the strategy for pl specified by dsupp[k]

              gPoly<double> next_poly(&BehavStratSpace, &Lex);

              for (int n = 1; n <= terminal_nodes.Length(); n++) {
                gPoly<double> node_prob(&BehavStratSpace, (double)1.0, &Lex);
                if (NashNodeProbabilityPoly(p_solution, node_prob, BehavStratSpace, Lex, dsupps[k],
                                            var_index, terminal_nodes[n],
                                            p_solution.GetGame()->GetPlayer(pl), isets_for_pl[i],
                                            acts_for_iset[j])) {
                  if (terminal_nodes[n]->GetOutcome()) {
                    node_prob *=
                        static_cast<double>(terminal_nodes[n]->GetOutcome()->GetPayoff(pl));
                  }
                  next_poly += node_prob;
                }
              }
              answer += -next_poly + (double)p_solution.GetPayoff(pl);
            }
          }
        }
      }
    }
  }
  return answer;
}

static gPolyList<double> ExtendsToNashIneqs(const Gambit::MixedBehaviorProfile<double> &p_solution,
                                            const gSpace &BehavStratSpace, const term_order &Lex,
                                            const Gambit::BehaviorSupportProfile &little_supp,
                                            const Gambit::BehaviorSupportProfile &big_supp,
                                            const Gambit::List<Gambit::List<int>> &var_index)
{
  gPolyList<double> answer(&BehavStratSpace, &Lex);
  answer += ActionProbsSumToOneIneqs(p_solution, BehavStratSpace, Lex, big_supp, var_index);

  answer += NashExpectedPayoffDiffPolys(p_solution, BehavStratSpace, Lex, little_supp, big_supp,
                                        var_index);
  return answer;
}

bool algExtendsToNash::ExtendsToNash(const Gambit::MixedBehaviorProfile<double> &p_solution,
                                     const Gambit::BehaviorSupportProfile &little_supp,
                                     const Gambit::BehaviorSupportProfile &big_supp)
{
  // This asks whether there is a Nash extension of the Gambit::MixedBehaviorProfile<double> to
  // all information sets at which the behavioral probabilities are not
  // specified.  The assumption is that the support has active actions
  // at infosets at which the behavioral probabilities are defined, and
  // no others.  Also, the BehavSol is assumed to be already a Nash
  // equilibrium for the truncated game obtained by eliminating stuff
  // outside little_supp.

  // First we compute the number of variables, and indexing information
  int num_vars(0);
  Gambit::List<Gambit::List<int>> var_index;
  int pl;
  for (pl = 1; pl <= p_solution.GetGame()->NumPlayers(); pl++) {

    Gambit::List<int> list_for_pl;

    for (int i = 1; i <= p_solution.GetGame()->GetPlayer(pl)->NumInfosets(); i++) {
      list_for_pl.push_back(num_vars);
      if (!big_supp.HasAction(p_solution.GetGame()->GetPlayer(pl)->GetInfoset(i))) {
        num_vars += p_solution.GetGame()->GetPlayer(pl)->GetInfoset(i)->NumActions() - 1;
      }
    }
    var_index.push_back(list_for_pl);
  }

  // We establish the space
  gSpace BehavStratSpace(num_vars);
  ORD_PTR ptr = &lex;
  term_order Lex(&BehavStratSpace, ptr);

  num_vars = BehavStratSpace.Dmnsn();

  gPolyList<double> inequalities =
      ExtendsToNashIneqs(p_solution, BehavStratSpace, Lex, little_supp, big_supp, var_index);
  // set up the rectangle of search
  Gambit::Vector<double> bottoms(num_vars), tops(num_vars);
  bottoms = (double)0;
  tops = (double)1;
  gRectangle<double> Cube(bottoms, tops);

  // Set up the test and do it
  IneqSolv<double> extension_tester(inequalities);
  Gambit::Vector<double> sample(num_vars);
  bool answer = extension_tester.ASolutionExists(Cube, sample);

  //  assert (answer == m_profile->ExtendsToNash(little_supp, big_supp, m_status));

  return answer;
}

//=========================================================================
//                   class algExtendsToAgentNash
//=========================================================================

static bool ANFNodeProbabilityPoly(const Gambit::MixedBehaviorProfile<double> &p_solution,
                                   gPoly<double> &node_prob, const gSpace &BehavStratSpace,
                                   const term_order &Lex,
                                   const Gambit::BehaviorSupportProfile &big_supp,
                                   const Gambit::List<Gambit::List<int>> &var_index,
                                   Gambit::GameNode tempnode, const int &pl, const int &i,
                                   const int &j)
{
  while (tempnode != p_solution.GetGame()->GetRoot()) {
    Gambit::GameAction last_action = tempnode->GetPriorAction();
    Gambit::GameInfoset last_infoset = last_action->GetInfoset();

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
        node_prob *= (double)p_solution.GetActionProb(last_action);
      }
      else {
        return false;
      }
    }
    else {
      int initial_var_no =
          var_index[last_infoset->GetPlayer()->GetNumber()][last_infoset->GetNumber()];
      if (last_action->GetNumber() < last_infoset->NumActions()) {
        int varno = initial_var_no + last_action->GetNumber();
        node_prob *= gPoly<double>(&BehavStratSpace, varno, 1, &Lex);
      }
      else {
        gPoly<double> factor(&BehavStratSpace, (double)1.0, &Lex);
        int k;
        for (k = 1; k < last_infoset->NumActions(); k++) {
          factor -= gPoly<double>(&BehavStratSpace, initial_var_no + k, 1, &Lex);
        }
        node_prob *= factor;
      }
    }
    tempnode = tempnode->GetParent();
  }
  return true;
}

static gPolyList<double>
ANFExpectedPayoffDiffPolys(const Gambit::MixedBehaviorProfile<double> &p_solution,
                           const gSpace &BehavStratSpace, const term_order &Lex,
                           const Gambit::BehaviorSupportProfile &little_supp,
                           const Gambit::BehaviorSupportProfile &big_supp,
                           const Gambit::List<Gambit::List<int>> &var_index)
{
  gPolyList<double> answer(&BehavStratSpace, &Lex);

  Gambit::List<Gambit::GameNode> terminal_nodes = TerminalNodes(p_solution.GetGame());

  for (int pl = 1; pl <= p_solution.GetGame()->NumPlayers(); pl++) {
    for (int i = 1; i <= p_solution.GetGame()->GetPlayer(pl)->NumInfosets(); i++) {
      Gambit::GameInfoset infoset = p_solution.GetGame()->GetPlayer(pl)->GetInfoset(i);
      if (little_supp.IsReachable(infoset)) {
        for (int j = 1; j <= infoset->NumActions(); j++) {
          if (!little_supp.Contains(infoset->GetAction(j))) {

            // This will be the utility difference between the
            // payoff resulting from the profile and deviation to
            // action j
            gPoly<double> next_poly(&BehavStratSpace, &Lex);

            for (int n = 1; n <= terminal_nodes.Length(); n++) {
              gPoly<double> node_prob(&BehavStratSpace, (double)1.0, &Lex);
              if (ANFNodeProbabilityPoly(p_solution, node_prob, BehavStratSpace, Lex, big_supp,
                                         var_index, terminal_nodes[n], pl, i, j)) {
                if (terminal_nodes[n]->GetOutcome()) {
                  node_prob *= static_cast<double>(terminal_nodes[n]->GetOutcome()->GetPayoff(pl));
                }
                next_poly += node_prob;
              }
            }
            answer += -next_poly + (double)p_solution.GetPayoff(pl);
          }
        }
      }
    }
  }
  return answer;
}

static gPolyList<double>
ExtendsToANFNashIneqs(const Gambit::MixedBehaviorProfile<double> &p_solution,
                      const gSpace &BehavStratSpace, const term_order &Lex,
                      const Gambit::BehaviorSupportProfile &little_supp,
                      const Gambit::BehaviorSupportProfile &big_supp,
                      const Gambit::List<Gambit::List<int>> &var_index)
{
  gPolyList<double> answer(&BehavStratSpace, &Lex);
  answer += ActionProbsSumToOneIneqs(p_solution, BehavStratSpace, Lex, big_supp, var_index);
  answer += ANFExpectedPayoffDiffPolys(p_solution, BehavStratSpace, Lex, little_supp, big_supp,
                                       var_index);
  return answer;
}

bool algExtendsToAgentNash::ExtendsToAgentNash(
    const Gambit::MixedBehaviorProfile<double> &p_solution,
    const Gambit::BehaviorSupportProfile &little_supp,
    const Gambit::BehaviorSupportProfile &big_supp)
{
  // This asks whether there is an ANF Nash extension of the Gambit::MixedBehaviorProfile<double>
  // to all information sets at which the behavioral probabilities are not specified.  The
  // assumption is that the support has active actions at infosets at which the behavioral
  // probabilities are defined, and no others.

  // First we compute the number of variables, and indexing information
  int num_vars(0);
  Gambit::List<Gambit::List<int>> var_index;
  int pl;
  for (pl = 1; pl <= p_solution.GetGame()->NumPlayers(); pl++) {

    Gambit::List<int> list_for_pl;

    for (int i = 1; i <= p_solution.GetGame()->GetPlayer(pl)->NumInfosets(); i++) {
      list_for_pl.push_back(num_vars);
      if (!big_supp.HasAction(p_solution.GetGame()->GetPlayer(pl)->GetInfoset(i))) {
        num_vars += p_solution.GetGame()->GetPlayer(pl)->GetInfoset(i)->NumActions() - 1;
      }
    }
    var_index.push_back(list_for_pl);
  }

  // We establish the space
  gSpace BehavStratSpace(num_vars);
  ORD_PTR ptr = &lex;
  term_order Lex(&BehavStratSpace, ptr);

  num_vars = BehavStratSpace.Dmnsn();
  gPolyList<double> inequalities =
      ExtendsToANFNashIneqs(p_solution, BehavStratSpace, Lex, little_supp, big_supp, var_index);

  // set up the rectangle of search
  Gambit::Vector<double> bottoms(num_vars), tops(num_vars);
  bottoms = (double)0;
  tops = (double)1;
  gRectangle<double> Cube(bottoms, tops);

  // Set up the test and do it
  IneqSolv<double> extension_tester(inequalities);
  Gambit::Vector<double> sample(num_vars);

  // Temporarily, we check the old set up vs. the new
  bool ANFanswer = extension_tester.ASolutionExists(Cube, sample);
  //  assert (ANFanswer == m_profile->ExtendsToANFNash(little_supp,
  //						   big_supp,
  //						   m_status));

  /*
  bool NASHanswer = m_profile->ExtendsToNash(Support(),Support(),m_status);

  //DEBUG
  if (ANFanswer && !NASHanswer)
    gout <<
      "The following should be extendable to an ANF Nash, but not to a Nash:\n"
         << *m_profile << "\n\n";
  if (NASHanswer && !ANFanswer)
    gout <<
      "ERROR: said to be extendable to a Nash, but not to an ANF Nash:\n"
         << *m_profile << "\n\n";
          */
  return ANFanswer;
}
