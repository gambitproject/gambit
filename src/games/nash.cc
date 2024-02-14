//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
void ChildSubgames(const GameNode &p_node, List<GameNode> &p_list)
{
  if (p_node->IsSubgameRoot()) {
    p_list.push_back(p_node);
  }
  else {
    for (int i = 1; i <= p_node->NumChildren(); ChildSubgames(p_node->GetChild(i++), p_list))
      ;
  }
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
// * We only carry around DVectors instead of full MixedBehaviorProfiles,
//   because MixedBehaviorProfiles allocate space several times the
//   size of the tree to carry around useful quantities.  These
//   quantities are irrelevant for this calculation, so we only
//   store the probabilities, and convert to MixedBehaviorProfiles
//   at the end of the computation
//

template <class T>
void SubgameBehavSolver<T>::SolveSubgames(const Game &p_game, const DVector<T> &p_templateSolution,
                                          const GameNode &n, List<DVector<T>> &solns,
                                          List<GameOutcome> &values) const
{
  List<DVector<T>> thissolns;
  thissolns.push_back(p_templateSolution);
  ((Vector<T> &)thissolns[1]).operator=(T(0));

  List<GameNode> subroots;
  for (int i = 1; i <= n->NumChildren(); i++) {
    ChildSubgames(n->GetChild(i), subroots);
  }

  List<Array<GameOutcome>> subrootvalues;
  subrootvalues.push_back(Array<GameOutcome>(subroots.Length()));

  for (int i = 1; i <= subroots.Length(); i++) {
    // printf("Looking at subgame %d of %d\n", i, subroots.Length());
    List<DVector<T>> subsolns;
    List<GameOutcome> subvalues;

    SolveSubgames(p_game, p_templateSolution, subroots[i], subsolns, subvalues);

    if (subsolns.empty()) {
      solns = List<DVector<T>>();
      return;
    }

    List<DVector<T>> newsolns;
    List<Array<GameOutcome>> newsubrootvalues;

    for (int soln = 1; soln <= thissolns.Length(); soln++) {
      for (int subsoln = 1; subsoln <= subsolns.Length(); subsoln++) {
        DVector<T> bp(thissolns[soln]);
        DVector<T> tmp(subsolns[subsoln]);
        for (int j = 1; j <= bp.Length(); j++) {
          bp[j] += tmp[j];
        }
        newsolns.push_back(bp);

        newsubrootvalues.push_back(subrootvalues[soln]);
        newsubrootvalues[newsubrootvalues.Length()][i] = subvalues[subsoln];
      }
    }

    thissolns = newsolns;
    subrootvalues = newsubrootvalues;
    // printf("Finished solving subgame %d\n", i);
  }

  for (int soln = 1; soln <= thissolns.Length(); soln++) {
    // printf("Analyzing scenario %d of %d\n", soln, thissolns.Length());
    for (int i = 1; i <= subroots.Length(); i++) {
      subroots[i]->SetOutcome(subrootvalues[soln][i]);
    }

    Game subgame = n->CopySubgame();
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    subgame->GetRoot()->SetOutcome(nullptr);

    BehaviorSupportProfile subsupport(subgame);
    List<MixedBehaviorProfile<T>> sol = m_solver->Solve(p_game);

    if (sol.empty()) {
      solns = List<DVector<T>>();
      return;
    }

    // Put behavior profile in "total" solution here...
    for (int solno = 1; solno <= sol.Length(); solno++) {
      solns.push_back(thissolns[soln]);

      for (int pl = 1; pl <= subgame->NumPlayers(); pl++) {
        GamePlayer subplayer = subgame->GetPlayer(pl);
        GamePlayer player = p_game->GetPlayer(pl);

        for (int iset = 1; iset <= subplayer->NumInfosets(); iset++) {
          GameInfoset subinfoset = subplayer->GetInfoset(iset);

          for (int j = 1; j <= player->NumInfosets(); j++) {
            if (subinfoset->GetLabel() == player->GetInfoset(j)->GetLabel()) {
              int id = atoi(subinfoset->GetLabel().c_str());
              for (int act = 1; act <= subsupport.NumActions(pl, iset); act++) {
                int actno = subsupport.GetAction(pl, iset, act)->GetNumber();
                solns[solns.Length()](pl, id, actno) = sol[solno][subinfoset->GetAction(act)];
              }
              break;
            }
          }
        }
      }

      Vector<T> subval(subgame->NumPlayers());
      GameOutcome outcome = n->GetOutcome();
      for (int pl = 1; pl <= subgame->NumPlayers(); pl++) {
        subval[pl] = sol[solno].GetPayoff(pl);
        if (outcome) {
          subval[pl] += static_cast<T>(outcome->GetPayoff(pl));
        }
      }

      GameOutcome ov = p_game->NewOutcome();
      for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
        ov->SetPayoff(pl, Number(static_cast<Rational>(subval[pl])));
      }

      values.push_back(ov);
    }
    // printf("Finished with scenario %d of %d; total solutions so far = %d\n",
    // soln, thissolns.Length(), solns.Length());
  }

  n->DeleteTree();
}

template <class T>
List<MixedBehaviorProfile<T>> SubgameBehavSolver<T>::Solve(const Game &p_game) const
{
  Game efg = p_game->GetRoot()->CopySubgame();

  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    for (int iset = 1; iset <= efg->GetPlayer(pl)->NumInfosets(); iset++) {
      efg->GetPlayer(pl)->GetInfoset(iset)->SetLabel(lexical_cast<std::string>(iset));
    }
  }

  List<DVector<T>> vectors;
  List<GameOutcome> values;
  SolveSubgames(efg, DVector<T>(efg->NumActions()), efg->GetRoot(), vectors, values);

  List<MixedBehaviorProfile<T>> solutions;
  for (int i = 1; i <= vectors.Length(); i++) {
    solutions.push_back(MixedBehaviorProfile<T>(p_game));
    for (int j = 1; j <= vectors[i].Length(); j++) {
      solutions[i][j] = vectors[i][j];
    }
  }
  for (int i = 1; i <= solutions.Length(); i++) {
    this->m_onEquilibrium->Render(solutions[i]);
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
