//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/libagg/agg.h
// Interface of Action Graph Game representation
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

#ifndef GAMBIT_AGG_AGG_H
#define GAMBIT_AGG_AGG_H

#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include <type_traits>
#include "proj_func.h"
#include "trie_map.h"
#include "games/number.h"

namespace Gambit::agg {

// data structure for a mixed strategy profile: one probability per action, using numeric type V
// (double for the floating-point engine, Rational for the exact-arithmetic counterpart).
template <class V> using StrategyProfile = std::vector<V>;

// the local payoff function for one action node: maps a neighbor configuration to the payoff of
// playing that node under it. Populated at parse time; consumed by the floating-point
// convolution engine (getMixedPayoff/getV/getJ) via trie_map's own inner_prod, so it has to stay
// a trie_map<double> -- unlike StrategyProfile/ConfigDistribution above, this is deliberately not
// a template (see ExactPayoffTable below for why a Rational instantiation would be unsound).
using PayoffTable = trie_map<double>;

// exact (arbitrary-precision) counterpart of PayoffTable, populated alongside it at parse time
// from the same source text. Deliberately not a trie_map<Rational>, for two independent reasons:
// (1) trie_map's own inner_prod has an epsilon-tolerance discard/throw policy
// (trie_map<V>::THRESH = 1e-12) that is meaningful for double but unsound for exact arithmetic --
// there is no notion of "too small to matter" once payoffs are exact; and (2) this stores Number,
// not Rational, to preserve the original entered text for exact serialization (printPayoffs),
// which a bare Rational cannot do. As a result this is only ever used for storage/lookup
// (find/insert/iterate) and a hand-written inner product (AGG::exactInnerProd), never trie_map's
// own arithmetic.
using ExactPayoffTable = std::map<std::vector<int>, Number>;

// data struct for a probability distribution over configurations, using numeric type V (double
// for the floating-point engine, Rational for the exact-arithmetic counterpart). Unlike
// PayoffTable/ExactPayoffTable above, this participates in the convolution algorithm
// (multiply/inner_prod), so V must support arithmetic -- which Rational, unlike Number,
// provides -- letting both instantiations reuse trie_map directly.
template <class V> using ConfigDistribution = trie_map<V>;

// AGG's mutable working state for the convolution algorithm, at numeric type V. AGG holds one
// instance per V (see AGG::state()) rather than a single instance of one or the other, since both
// may be in use simultaneously -- e.g. from different MixedStrategyProfile instantiations over
// the same AGG -- and each is rebuilt fresh per call regardless of its previous contents.
template <class V> struct ConvolutionState {
  // foreach s \in S, foreach i \in N, the projected mixed strat: a prob distribution over the
  // set of 'contributions' (see AGG::projection).
  std::vector<std::vector<ConfigDistribution<V>>> projectedStrat;

  // when computing the induced distribution via AGG::computeP(): foreach k <= n-1, the prob.
  // distribution P_k induced by the partial strat profile of agents o_1..o_k. Also reused, when
  // computing the partial distributions for the payoff jacobian, as the partial distribution
  // induced by all agents except j.
  std::vector<ConfigDistribution<V>> Pr;

  ConvolutionState(int numActionNodes, int numPlayers)
    : projectedStrat(numActionNodes, std::vector<ConfigDistribution<V>>(numPlayers)),
      Pr(numPlayers)
  {
  }

  // reset projectedStrat[Node][player] to the weighted sum, over actions j with s[j] > 0, of
  // contributions[j] weighted by s[j].
  void project(int Node, int player, const std::vector<std::vector<int>> &contributions,
               int numActions, const V *s)
  {
    projectedStrat[Node][player].reset();
    for (int j = 0; j < numActions; j++) {
      if (s[j] > V(0)) {
        projectedStrat[Node][player] += std::make_pair(contributions[j], s[j]);
      }
    }
  }
};

// types of input formats for payoff func
using PayoffType = enum { COMPLETE, MAPPING, ADDITIVE };

class AGG {

public:
  using Config = std::vector<int>;
  using ActionSet = std::vector<int>;
  using PlayerSet = std::vector<int>;

  static const char COMMENT_CHAR = '#';
  static const char LBRACKET = '[';
  static const char RBRACKET = ']';

  // read an AGG from input stream
  static std::shared_ptr<AGG> makeAGG(std::istream &in);

  // constructor. The convolution engine's own working state (ConvolutionState) is sized from
  // numANodes/numPlayers directly, not seeded from a caller-supplied array.
  AGG(int numPlayers, std::vector<int> &actions, int numANodes, int numPNodes,
      std::vector<std::vector<int>> &actionSets, std::vector<std::vector<int>> &neighbors,
      std::vector<projtype> &projTypes, std::vector<std::vector<std::vector<Config>>> &proj,
      std::vector<std::vector<projtype>> &projF, std::vector<std::vector<std::vector<int>>> &Po,
      std::vector<PayoffTable> &payoffs, std::vector<ExactPayoffTable> &exactPayoffs);

  ~AGG() = default;

  int getNumPlayers() const { return numPlayers; }
  int getNumActions() const { return totalActions; }
  int getNumActions(int i) const { return actions[i]; }
  // per-player action counts, e.g. for constructing a gnmgame from an AGG.
  const std::vector<int> &getActionCounts() const { return actions; }
  int getMaxActions() const { return maxActions; }
  int firstAction(int i) const { return strategyOffset[i]; }
  int lastAction(int i) const { return strategyOffset[i + 1]; }

  int getNumActionNodes() const { return numActionNodes; }
  int getNumFunctionNodes() const { return numPNodes; }
  int getNumKSymActions() const { return numKSymActions; }
  int getNumKSymActions(int i) { return uniqueActionSets[i].size(); }
  int getNumPlayerClasses() { return playerClasses.size(); }
  const PlayerSet &getPlayerClass(int cls) const { return playerClasses.at(cls); }
  int firstKSymAction(int i) const { return kSymStrategyOffset[i]; }
  int lastKSymAction(int i) const { return kSymStrategyOffset[i + 1]; }

  // exp. payoff under mixed strat profile, via the convolution algorithm (doProjection/computeP,
  // templated on V; inner_prod is trie_map's own method for V=double, exactInnerProd for
  // V=Rational -- see exactInnerProd's comment for why that one isn't merged too).
  template <class V> V getMixedPayoff(int player, const StrategyProfile<V> &s);
  void getPayoffVector(std::vector<double> &dest, int player, const StrategyProfile<double> &s);
  template <class V> V getV(int player, int action, const StrategyProfile<V> &s);
  template <class V>
  V getJ(int player, int action, int player2, int action2, const StrategyProfile<V> &s);

  // payoff of the pure profile s: a direct lookup, no floating-point computation involved.
  // V=double looks up PayoffTable; V=Rational looks up ExactPayoffTable and casts the stored
  // Number to Rational (see ExactPayoffTable's comment for why the table itself stays
  // Number-valued). V is never deducible from s (a plain vector<int> of action indices, not
  // parametrized by V), so callers must specify it explicitly, e.g. getPurePayoff<Rational>(...).
  template <class V> V getPurePayoff(int player, const std::vector<int> &s) const;

  bool isSymmetric() const
  {
    for (int i = 0; i < numPlayers; ++i) {
      if (actions[i] < numActionNodes) {
        return false;
      }
    }
    return true;
  }
  double getSymMixedPayoff(StrategyProfile<double> &s);
  double getSymMixedPayoff(int actnode, StrategyProfile<double> &s);
  void getSymPayoffVector(std::vector<double> &dest, StrategyProfile<double> &s);
  double getKSymMixedPayoff(int playerClass, std::vector<StrategyProfile<double>> &s);
  double getKSymMixedPayoff(int playerClass, StrategyProfile<double> &s);
  double getKSymMixedPayoff(int playerClass, int act, std::vector<StrategyProfile<double>> &s);
  double getKSymMixedPayoff(const StrategyProfile<double> &s, int pClass1, int act1,
                            int pClass2 = -1, int act2 = -1);
  void getKSymPayoffVector(std::vector<double> &dest, int playerClass, StrategyProfile<double> &s);

  std::vector<double> getExpectedConfig(StrategyProfile<double> &s)
  {
    std::vector<double> res(numActionNodes, 0);
    for (int i = 0; i < numPlayers; ++i) {
      for (int j = 0; j < actions[i]; ++j) {
        res[actionSets[i][j]] += s[firstAction(i) + j];
      }
    }
    return res;
  }

  std::vector<projtype> &getProjFunctions(int node) { return projFunctions.at(node); }
  const std::vector<int> &getPorder(int player, int action)
  {
    return Porder.at(player).at(action);
  }
  const std::vector<std::vector<Config>> &getProjection(int node) { return projection.at(node); }
  const std::vector<int> &getActionSet(int player) { return actionSets.at(player); }
  const PayoffTable &getPayoffMap(int node) { return payoffs.at(node); }

  // largest/smallest payoff in any pure profile. Same V dispatch as getPurePayoff above; likewise
  // never deducible (no arguments at all), so callers write getMaxPayoff<Rational>(), etc.
  template <class V> V getMaxPayoff() const;
  template <class V> V getMinPayoff() const;

  void printPayoffs(std::ostream &s, int node) const
  {
    s << exactPayoffs.at(node).size() << std::endl;
    for (const auto &entry : exactPayoffs.at(node)) {
      s << "[ ";
      std::copy(entry.first.begin(), entry.first.end(), std::ostream_iterator<int>(s, " "));
      s << "] " << entry.second << std::endl;
    }
  }

  void printActionGraph(std::ostream &s) const
  {
    for (const auto &neighbor : neighbors) {
      s << neighbor.size() << "\t";
      copy(neighbor.begin(), neighbor.end(), std::ostream_iterator<int>(s, " "));
      s << std::endl;
    }
  }

  void printTypes(std::ostream &s) const
  {
    for (const auto &projectionType : projectionTypes) {
      projectionType->print(s);
    }
  }

private:
  int numPlayers;
  std::vector<int> actions;
  std::vector<int> strategyOffset;
  int totalActions{0};
  int maxActions{0};

  int numActionNodes; // |S|
  int numPNodes;      // |P|

  // action sets: for each player i \in N
  std::vector<std::vector<int>> actionSets;

  // neighbor lists: for each node \in S&P
  std::vector<std::vector<int>> neighbors;

  // Projection signature for each projected node \in P
  std::vector<projtype> projectionTypes;

  // payoff function for each action node \in S
  std::vector<PayoffTable> payoffs;

  // exact (arbitrary-precision) counterpart of payoffs, for each action node \in S
  std::vector<ExactPayoffTable> exactPayoffs;

  // auxiliary data structures

  // originally:
  // foreach s \in S, foreach i \in N, foreach s_i \in S_i,
  // the 'contribution' of s_i to D^(s)
  std::vector<std::vector<std::vector<Config>>> projection;

  // working state for the convolution algorithm (see ConvolutionState above), one instance per
  // numeric type V, since both may be in use simultaneously.
  ConvolutionState<double> m_state;
  ConvolutionState<Rational> m_exactState;

  // foreach s in S, foreach neighbor of s, its projection function
  std::vector<std::vector<projtype>> projFunctions;

  // foreach i \in N, foreach s_i in S_i, the order of agents o_1.. o_{n-1}
  //  in which we apply the DP algorithm
  std::vector<std::vector<std::vector<int>>> Porder;

  // foreach s in S, whether s's neighbors are all action nodes
  std::vector<bool> isPure;

  // foreach s in S, j in N, the index of s in j's action set, or -1 if N/A
  std::vector<std::vector<int>> node2Action;

  // the unique action sets
  std::vector<ActionSet> uniqueActionSets;

  // equivalent classes of players
  std::vector<PlayerSet> playerClasses;

  // the class index for each player
  std::vector<int> player2Class;

  // sum of the sizes of uniqueActionSets
  int numKSymActions;

  // strategyOffset for kSymmetric strategy profile
  std::vector<int> kSymStrategyOffset;

  // input functor
  struct input {
    input(std::istream &i, ExactPayoffTable &e) : in(i), exact(e) {}
    void operator()(PayoffTable::iterator p)
    {
      std::string word;
      in >> word;
      const Number num(word);
      p->second = static_cast<double>(num);
      exact.insert(std::make_pair(p->first, num));
    }
    std::istream &in;
    ExactPayoffTable &exact;
  };

  // private static methods:

  static void makeCOMPLETEpayoff(std::istream &in, PayoffTable &pay, ExactPayoffTable &exactPay)
  {
    pay.in_order(input(in, exactPay));
  }
  static void makeMAPPINGpayoff(std::istream &in, PayoffTable &pay, ExactPayoffTable &exactPay,
                                int);

  static void setProjections(std::vector<std::vector<ConfigDistribution<double>>> &projS,
                             std::vector<std::vector<std::vector<Config>>> &proj, int N, int S,
                             int P, std::vector<std::vector<int>> &AS,
                             std::vector<std::vector<int>> &neighb,
                             std::vector<projtype> &projTypes);

  static void getAn(std::multiset<int> &dest, std::vector<std::vector<int>> &neighb,
                    std::vector<projtype> &projTypes, int S, int Node, std::vector<int> &path);

  static void initPorder(std::vector<int> &Po, int i, int N,
                         std::vector<ConfigDistribution<double>> &projS);

  // private methods. computeP/doProjection are the same algorithm for both V; they're made
  // non-branching by going through state<V>() for the V-specific working arrays (m_state vs
  // m_exactState), rather than an if constexpr per call.
  template <class V> ConvolutionState<V> &state()
  {
    if constexpr (std::is_same_v<V, Rational>) {
      return m_exactState;
    }
    else {
      return m_state;
    }
  }
  template <class V> void computeP(int player, int act, int player2 = -1, int act2 = -1);
  template <class V> void doProjection(int Node, const StrategyProfile<V> &s)
  {
    doProjection(Node, &(const_cast<StrategyProfile<V> &>(s)[0]));
  }
  template <class V> void doProjection(int Node, int player, const StrategyProfile<V> &s)
  {
    doProjection(Node, player, &(const_cast<StrategyProfile<V> &>(s)[firstAction(player)]));
  }
  template <class V> void doProjection(int Node, V *s);
  template <class V> void doProjection(int Node, int player, V *s);

  // exactPayoffs is a std::map<..., Number>, not a trie_map<Rational>, so trie_map's own
  // inner_prod can't be used directly against it (unlike the double path, which calls
  // dist.inner_prod(payoffs[node]) directly) -- this does the equivalent sum manually.
  Rational exactInnerProd(int node, const ConfigDistribution<Rational> &dist) const;

  // dispatch point used by getV/getJ: V=double calls trie_map's own inner_prod against
  // PayoffTable, V=Rational calls exactInnerProd against ExactPayoffTable. Not merged further
  // than this one function -- same reason PayoffTable/ExactPayoffTable themselves aren't a
  // template pair (see their comments): the two sides genuinely differ in container and value
  // type, not just in V.
  template <class V> V payoffInnerProd(int node, const ConfigDistribution<V> &dist);

  void getSymConfigProb(int plClass, StrategyProfile<double> &s, int ownPlClass, int act,
                        ConfigDistribution<double> &dest, int plClass2 = -1, int act2 = -1);
};

} // namespace Gambit::agg

#endif // GAMBIT_AGG_AGG_H
