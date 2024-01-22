//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
#include <vector>
#include <iterator>
#include "proj_func.h"
#include "trie_map.h"

namespace Gambit {

namespace gametracer {
class aggame;
}

namespace agg {

// data structure for mixed strategy profile
using AggNumber = double;
using StrategyProfile = std::vector<AggNumber>;
using AggNumberVector = std::vector<AggNumber>;

// data structure for payoff function:
using aggpayoff = trie_map<AggNumber>;

// data struct for prob distribution over configurations:
using aggdistrib = trie_map<AggNumber>;

// types of input formats for payoff func
using payofftype = enum { COMPLETE, MAPPING, ADDITIVE };

class AGG {

public:
  typedef std::vector<int> config;
  typedef std::vector<int> ActionSet;
  typedef std::vector<int> PlayerSet;

  static const char COMMENT_CHAR = '#';
  static const char LBRACKET = '[';
  static const char RBRACKET = ']';

  friend class gametracer::aggame; // wrapper class for gametracer

  // read an AGG from input stream
  static std::shared_ptr<AGG> makeAGG(std::istream &in);

  // constructor
  AGG(int numPlayers, std::vector<int> &actions, int numANodes, int numPNodes,
      std::vector<std::vector<int>> &actionSets, std::vector<std::vector<int>> &neighbors,
      std::vector<projtype> &projTypes, std::vector<std::vector<aggdistrib>> &projS,
      std::vector<std::vector<std::vector<config>>> &proj,
      std::vector<std::vector<projtype>> &projF, std::vector<std::vector<std::vector<int>>> &Po,
      std::vector<aggdistrib> &P, std::vector<aggpayoff> &payoffs);

  ~AGG() = default;

  int getNumPlayers() const { return numPlayers; }
  int getNumActions() const { return totalActions; }
  int getNumActions(int i) const { return actions[i]; }
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

  // exp. payoff under mixed strat profile
  AggNumber getMixedPayoff(int player, StrategyProfile &s);
  void getPayoffVector(AggNumberVector &dest, int player, const StrategyProfile &s);
  AggNumber getV(int player, int action, const StrategyProfile &s);
  AggNumber getJ(int player, int action, int player2, int action2, StrategyProfile &s);

  AggNumber getPurePayoff(int player, const std::vector<int> &s);

  bool isSymmetric() const
  {
    for (int i = 0; i < numPlayers; ++i) {
      if (actions[i] < numActionNodes) {
        return false;
      }
    }
    return true;
  }
  AggNumber getSymMixedPayoff(StrategyProfile &s);
  AggNumber getSymMixedPayoff(int actnode, StrategyProfile &s);
  void getSymPayoffVector(AggNumberVector &dest, StrategyProfile &s);
  AggNumber getKSymMixedPayoff(int playerClass, std::vector<StrategyProfile> &s);
  AggNumber getKSymMixedPayoff(int playerClass, StrategyProfile &s);
  AggNumber getKSymMixedPayoff(int playerClass, int act, std::vector<StrategyProfile> &s);
  AggNumber getKSymMixedPayoff(const StrategyProfile &s, int pClass1, int act1, int pClass2 = -1,
                               int act2 = -1);
  void getKSymPayoffVector(AggNumberVector &dest, int playerClass, StrategyProfile &s);

  AggNumberVector getExpectedConfig(StrategyProfile &s)
  {
    AggNumberVector res(numActionNodes, 0);
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
  const std::vector<std::vector<config>> &getProjection(int node) { return projection.at(node); }
  const std::vector<int> &getActionSet(int player) { return actionSets.at(player); }
  const aggpayoff &getPayoffMap(int node) { return payoffs.at(node); }

  AggNumber getMaxPayoff() const;
  AggNumber getMinPayoff() const;

  void printPayoffs(std::ostream &s, int node) const
  {
    s << payoffs.at(node).size() << std::endl;
    s << payoffs[node];
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
  int totalActions;
  int maxActions;

  int numActionNodes; // |S|
  int numPNodes;      // |P|

  // action sets: for each player i \in N
  std::vector<std::vector<int>> actionSets;

  // neighbor lists: for each node \in S&P
  std::vector<std::vector<int>> neighbors;

  // Projection signature for each projected node \in P
  std::vector<projtype> projectionTypes;

  // payoff function for each action node \in S
  std::vector<aggpayoff> payoffs;

  // auxiliary data structures

  // originally:
  // foreach s \in S, foreach i \in N, foreach s_i \in S_i,
  // the 'contribution' of s_i to D^(s)
  std::vector<std::vector<std::vector<config>>> projection;

  // foreach s \in S, foreach i \in N, the projected mixed strat
  // which is a prob distribution over the set of 'contributions'
  std::vector<std::vector<aggdistrib>> projectedStrat;

  // foreach s in S, i in N, the full set of projected actions.
  std::vector<std::vector<aggdistrib>> fullProjectedStrat;

  // foreach s in S, foreach neighbor of s, its projection function
  std::vector<std::vector<projtype>> projFunctions;

  // foreach i \in N, foreach s_i in S_i, the order of agents o_1.. o_{n-1}
  //  in which we apply the DP algorithm
  std::vector<std::vector<std::vector<int>>> Porder;

  // when computing the induced distribution via ComputeP():
  // foreach k<= n-1,
  // prob. distrib P_k induced by the partial strat profile of agents o_1..o_k

  // when computing the partial distributions for the payoff jacobian:
  //   foreach  j \in N,
  //  the partial distribution induced by all agents except j.
  std::vector<aggdistrib> Pr;

  // foreach s in S, whether s's neighbors are all action nodes
  std::vector<bool> isPure;

  // foreach s in S, j in N, the index of s in j's action set, or -1 if N/A
  std::vector<std::vector<int>> node2Action;

  // cache of jacobian entries.
  trie_map<AggNumber> cache;

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
    explicit input(std::istream &i) : in(i) {}
    void operator()(aggpayoff::iterator p) { in >> (*p).second; }
    std::istream &in;
  };

  // private static methods:

  static void makeCOMPLETEpayoff(std::istream &in, aggpayoff &pay) { pay.in_order(input(in)); }
  static void makeMAPPINGpayoff(std::istream &in, aggpayoff &pay, int);

  static void setProjections(std::vector<std::vector<aggdistrib>> &projS,
                             std::vector<std::vector<std::vector<config>>> &proj, int N, int S,
                             int P, std::vector<std::vector<int>> &AS,
                             std::vector<std::vector<int>> &neighb,
                             std::vector<projtype> &projTypes);

  static void getAn(std::multiset<int> &dest, std::vector<std::vector<int>> &neighb,
                    std::vector<projtype> &projTypes, int S, int Node, std::vector<int> &path);

  static void initPorder(std::vector<int> &Po, int i, int N, std::vector<aggdistrib> &projS);

  // private methods:
  void computeP(int player, int act, int player2 = -1, int act2 = -1);
  void doProjection(int Node, const StrategyProfile &s)
  {
    doProjection(Node, &(const_cast<StrategyProfile &>(s)[0]));
  }
  void doProjection(int Node, int player, const StrategyProfile &s)
  {
    doProjection(Node, player, &(const_cast<StrategyProfile &>(s)[firstAction(player)]));
  }
  void doProjection(int Node, AggNumber *s);
  void doProjection(int Node, int player, AggNumber *s);

  void getSymConfigProb(int plClass, StrategyProfile &s, int ownPlClass, int act, aggdistrib &dest,
                        int plClass2 = -1, int act2 = -1);
};

} // namespace agg

} // end namespace Gambit

#endif // GAMBIT_AGG_AGG_H
