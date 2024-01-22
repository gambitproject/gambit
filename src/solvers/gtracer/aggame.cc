//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: library/src/gtracer/aggame.cc
// Implement GNM-specific routines for action graph games
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

#include "aggame.h"

namespace Gambit {
namespace gametracer {

void aggame::computePartialP_PureNode(int player1, int act1, std::vector<int> &tasks) const
{
  int i, j, Node = aggPtr->actionSets[player1][act1];
  int numNei = aggPtr->neighbors[Node].size();

  // assert(aggPtr->isPure[Node]||tasks.size()==0);
  std::vector<agg::AggNumber> strat(numNei);
  agg::AGG::config a(numNei, 0);
  // compute the full distrib
  aggPtr->computeP(player1, act1);

  // store the full distrib in Pr[player1]
  aggPtr->Pr[player1].swap(aggPtr->Pr[numPlayers - 1]);
  for (i = 0; i < (int)tasks.size(); i++) {
    // assert(tasks[i]!=player1);
    agg::aggdistrib &P = aggPtr->Pr[tasks[i]];
    // P.clear();  // to get ready for division, we need clear()
    P = aggPtr->Pr[player1];

    bool NullOnly = true;
    for (j = 0; j < numNei; ++j) {
      a[j]++;
      auto pp = aggPtr->projectedStrat[Node][tasks[i]].find(a);
      if (pp == aggPtr->projectedStrat[Node][tasks[i]].end()) {
        strat[j] = 0;
      }
      else {
        strat[j] = pp->second;
        if (strat[j] > (agg::AggNumber)0) {
          NullOnly = false;
        }
      }

      a[j]--;
    }
#ifdef AGGDEBUG
    cout << "dividing " << endl;
    P.print_in_order();
    cout << endl << "by [";
    copy(strat.begin(), strat.end(), ostream_iterator<agg::AggNumber>(cout, " "));
    cout << "]\n";
#endif
    if (!NullOnly) {
      P /= strat;
    }
#ifdef AGGDEBUG
    cout << "result is: " << endl;
    P.print_in_order();
#endif
  } // end for(i
}

void aggame::computePartialP_bisect(int player1, int act1, std::vector<int>::iterator start,
                                    std::vector<int>::iterator endp, agg::aggdistrib &temp) const
{
#ifdef AGGDEBUG
  cout << "calling computePartialP_bisect with player1=" << player1 << ", act1=" << act1
       << " *start=" << *start << " *(endp-1)=" << *(endp - 1) << ", (endp-start)=" << endp - start
       << endl;
#endif
  if (endp - start == 1) {
    aggPtr->Pr[*start].reset();
    return;
  }
  int Node = aggPtr->actionSets[player1][act1];
  int numNei = aggPtr->neighbors[Node].size();

  int player2;
  std::vector<int>::iterator ptr, mid = start + (endp - start) / 2;
#ifdef AGGDEBUG
  cout << "*mid=" << *mid << " mid-start=" << mid - start << " endp-mid=" << endp - mid << endl;
#endif
  computePartialP_bisect(player1, act1, start, mid, temp);
  computePartialP_bisect(player1, act1, mid, endp, temp);

  temp.reset();
  temp = aggPtr->projectedStrat[Node][*start];
  if (mid - start > 1) {
    temp.multiply(aggPtr->Pr[*start], numNei, aggPtr->projFunctions[Node]);
  }

  if (mid - start == 1) {
    // assert(aggPtr->Pr[*start].empty());
    aggPtr->Pr[*start] = aggPtr->projectedStrat[Node][*mid];
    if (endp - mid > 1) {
      aggPtr->Pr[*start].multiply(aggPtr->Pr[*mid], numNei, aggPtr->projFunctions[Node]);
    }
  }
  else {
    for (ptr = start; ptr != mid; ++ptr) {
      player2 = *ptr;
      aggPtr->Pr[player2].multiply(aggPtr->projectedStrat[Node][*mid], numNei,
                                   aggPtr->projFunctions[Node]);
      if (endp - mid > 1) {
        aggPtr->Pr[player2].multiply(aggPtr->Pr[*mid], numNei, aggPtr->projFunctions[Node]);
      }
    }
  }

  if (endp - mid == 1) {
    // assert(aggPtr->Pr[*mid].empty());
    aggPtr->Pr[*mid] = temp;
  }
  else {
    for (ptr = mid; ptr != endp; ++ptr) {
      player2 = *ptr;
      aggPtr->Pr[player2].multiply(temp, numNei, aggPtr->projFunctions[Node]);
    }
  }
}

void aggame::payoffMatrix(cmatrix &dest, const cvector &s, agg::AggNumber fuzz) const
{
  // compute jacobian
  // s: mixed strat

#ifdef AGGDEBUG
  cout << "calling payoffMatrix with stratety s=" << endl << s << endl;
#endif
  agg::AggNumber fuzzcount;
  int rown, coln, rowi, coli, act1, act2, currNode, numNei;
  static std::vector<int>::iterator p;
  static std::vector<int> tasks, spares, nontasks;
  tasks.reserve(aggPtr->numPlayers);
  spares.reserve(aggPtr->numPlayers);
  nontasks.reserve(aggPtr->numPlayers);
  aggPtr->cache.reset();

  // do projection
  for (int Node = 0; Node < aggPtr->numActionNodes; Node++) {
    aggPtr->doProjection(Node, s.values());
  }

  // deal with the diagonal
  for (rown = 0; rown < aggPtr->numPlayers; ++rown) {
    fuzzcount = fuzz;
    for (rowi = firstAction(rown); rowi < lastAction(rown); rowi++) {
      for (coli = firstAction(rown); coli < lastAction(rown); coli++) {
        dest(rowi, coli) = fuzzcount;
        fuzzcount += fuzz;
      }
    }
  }
  for (rown = 0; rown < aggPtr->numPlayers; ++rown) {      // rown: the row player
    for (act1 = 0; act1 < aggPtr->actions[rown]; act1++) { // act1: player rown's action

      currNode = aggPtr->actionSets[rown][act1];
      numNei = aggPtr->neighbors[currNode].size();
#ifdef AGGDEBUG
      cout << "for player " << rown << ", action " << act1 << ", action node " << currNode << endl;
      cout << "cache is: " << endl << aggPtr->cache << endl;
#endif
      tasks.clear(); // for these col players, we need to compute the distribution induced by their
                     // complements. input of the bisection alg
      spares.clear();   // these col players have only one projected action
      nontasks.clear(); // complement of tasks. includes spares, and completely cached col players

      std::vector<int> key(numNei + 3);
      key[numNei] = rown;
      key[numNei + 1] = act1;

      // first, populate tasks, spares and nontasks
      for (coln = 0; coln < aggPtr->numPlayers; ++coln) {
        if (rown != coln) { // coln: col player

          key[numNei + 2] = coln;
          bool allCached = true;
          for (act2 = 0; act2 < aggPtr->actions[coln]; ++act2) {

            copy(aggPtr->projection[currNode][coln][act2].begin(),
                 aggPtr->projection[currNode][coln][act2].end(), key.begin());
#ifdef AGGDEBUG
            cout << "for player2=" << coln << " act2=" << act2 << endl;
            cout << "checking cache for: [";
            copy(key.begin(), key.end(), ostream_iterator<int>(cout, " "));
            cout << "]\n";
#endif
            auto r = aggPtr->cache.findExact(key);
            if (r != aggPtr->cache.end()) {
              dest(act1 + firstAction(rown), act2 + firstAction(coln)) = r->second;
            }
            else {
              allCached = false;
            }
          }
          if (allCached) { // if all coln's actions are already cached:
            nontasks.push_back(coln);
          }
          else {
            if (aggPtr->fullProjectedStrat[currNode][coln].size() ==
                1) { // if coln has only one projected action
              spares.push_back(coln);
              nontasks.push_back(coln);
            }
            else {
              tasks.push_back(coln);
            }
          }
        }
      }
#ifdef AGGDEBUG
      cout << "spares are: [";
      copy(spares.begin(), spares.end(), ostream_iterator<int>(cout, " "));
      cout << "]\ntasks are :[";
      copy(tasks.begin(), tasks.end(), ostream_iterator<int>(cout, " "));
      cout << "]\n";
#endif

      // compute partial prob distributions
      if (tasks.empty() && spares.empty()) {
        continue;
      } // nothing to be done for this row

      if (aggPtr->isPure[currNode] || tasks.empty()) {
        computePartialP_PureNode(rown, act1, tasks);
      }
      else { // do bisection
        computePartialP_bisect(rown, act1, tasks.begin(), tasks.end(), aggPtr->Pr[rown]);
#ifdef AGGDEBUG
        cout << "after calling computePartialP_bisect:" << endl;
        for (int tt = 0; tt < tasks.size(); tt++) {
          cout << "for player " << tasks[tt] << endl;
          cout << aggPtr->Pr[tasks[tt]] << endl;
        }
#endif
        // now apply rown's action (act1), and the strategies of
        // players in nontasks
        aggPtr->Pr[rown].reset();
        aggPtr->Pr[rown].insert(make_pair(aggPtr->projection[currNode][rown][act1], 1.0));
        for (p = nontasks.begin(); p != nontasks.end(); ++p) {
          aggPtr->Pr[rown].multiply(aggPtr->projectedStrat[currNode][*p], numNei,
                                    aggPtr->projFunctions[currNode]);
        }
#ifdef AGGDEBUG
        cout << "the polynomial product of strats of player " << rown
             << " and players in the vector nontasks is:" << endl;
        cout << aggPtr->Pr[rown] << endl;
#endif
        if (tasks.size() == 1) {
          aggPtr->Pr[tasks[0]] = aggPtr->Pr[rown];
        }
        else {
          for (p = tasks.begin(); p != tasks.end(); ++p) {
            if (aggPtr->Pr[*p].empty()) {
              std::cerr << "AGG::payoffMatrix() ERROR for rown=" << rown << " act1=" << act1
                        << " *p=" << *p << ": the distribution should not be empty!" << std::endl;
#ifdef AGGDEBUG
              std::cerr << "strategy profile is: " << s << std::endl;
#endif
            }
            aggPtr->Pr[*p].multiply(aggPtr->Pr[rown], numNei, aggPtr->projFunctions[currNode]);
          } // end for(p=tasks.begin...
        }

        // if spares not empty, we need to compute nondisturbed payoffs
        // which requires the distrib induced by everyone (except rown).
        // we store this distrib in Pr[rown][act1][rown]
        if (!spares.empty()) {
          // assert(tasks.size()>0);
          aggPtr->Pr[rown].reset();
          aggPtr->Pr[rown].multiply(aggPtr->Pr[tasks[0]],
                                    aggPtr->projectedStrat[currNode][tasks[0]], numNei,
                                    aggPtr->projFunctions[currNode]);
        }
      } // end else
#ifdef AGGDEBUG
      cout << "after computing parital distributions, the distributions are" << endl;
      for (int tt = 0; tt < numPlayers; tt++) {
        cout << "for player " << tt << endl;
        cout << aggPtr->Pr[tt];
        cout << endl;
      }
#endif
      // compute entries
      agg::AggNumber undisturbedPayoff;
      bool hasUndisturbed = false;

      if (!spares.empty()) { // for players in spares, we compute one undisturbed payoff
        computeUndisturbedPayoff(undisturbedPayoff, hasUndisturbed, rown, act1, rown);
        for (p = spares.begin(); p != spares.end(); ++p) {
          for (act2 = 0; act2 < aggPtr->actions[*p]; act2++) {
            savePayoff(dest, rown, act1, *p, act2, undisturbedPayoff, aggPtr->cache);
          }
        }
      }
      for (p = tasks.begin(); p != tasks.end(); ++p) {
        for (act2 = 0; act2 < aggPtr->actions[*p]; act2++) { // act2: col action

          if (aggPtr->projectedStrat[currNode][*p].size() == 1 &&
              aggPtr->projectedStrat[currNode][*p].begin()->first ==
                  aggPtr->projection[currNode][*p][act2]) {
            computeUndisturbedPayoff(undisturbedPayoff, hasUndisturbed, rown, act1, *p);
            savePayoff(dest, rown, act1, *p, act2, undisturbedPayoff, aggPtr->cache);
          }
          computePayoff(dest, rown, act1, *p, act2, aggPtr->cache);
        } // end for(act2
      }   // end for(p
    }     // end for(act1
  }       // end for(rown
}

void aggame::computeUndisturbedPayoff(agg::AggNumber &undisturbedPayoff, bool &has, int player1,
                                      int act1, int player2) const
{
  if (has) {
    return;
  }
  int Node = aggPtr->actionSets[player1][act1];
  int numNei = aggPtr->neighbors[Node].size();
  if (player2 == player1) {
    undisturbedPayoff = aggPtr->Pr[player2].inner_prod(aggPtr->payoffs[Node]);
  }
  else {
    // assert(aggPtr->projectedStrat[Node][player2].size()==1);
    undisturbedPayoff =
        aggPtr->Pr[player2].inner_prod(aggPtr->projectedStrat[Node][player2].begin()->first,
                                       numNei, aggPtr->projFunctions[Node], aggPtr->payoffs[Node]);
  }
  has = true;
}

void aggame::savePayoff(cmatrix &dest, int player1, int act1, int player2, int act2,
                        agg::AggNumber result, agg::trie_map<agg::AggNumber> &cache,
                        bool partial) const
{

  int Node = aggPtr->actionSets[player1][act1];
  int numNei = aggPtr->neighbors[Node].size();

  if (!partial) {
    std::pair<std::vector<int>, agg::AggNumber> pair1(aggPtr->projection[Node][player2][act2],
                                                      result);
    pair1.first.reserve(numNei + 3);
    pair1.first.push_back(player1);
    pair1.first.push_back(act1);
    pair1.first.push_back(player2);
    cache.insert(pair1);
  }

  if (aggPtr->node2Action[Node][player2] != -1 &&
      aggPtr->fullProjectedStrat[Node][player1].count(aggPtr->projection[Node][player2][act2])) {
    std::pair<std::vector<int>, agg::AggNumber> pair2(aggPtr->projection[Node][player2][act2],
                                                      result);
    pair2.first.reserve(numNei + 3);
    pair2.first.push_back(player2);
    pair2.first.push_back(aggPtr->node2Action[Node][player2]);
    pair2.first.push_back(player1);
    cache.insert(pair2);
  }
  dest(act1 + firstAction(player1), act2 + firstAction(player2)) = result;
}

void aggame::computePayoff(cmatrix &dest, int player1, int act1, int player2, int act2,
                           agg::trie_map<agg::AggNumber> &cache) const
{
  int Node = aggPtr->actionSets[player1][act1];
  int numNei = aggPtr->neighbors[Node].size();

  std::pair<std::vector<int>, agg::AggNumber> insPair(aggPtr->projection[Node][player2][act2], 0);
  insPair.first.reserve(numNei + 3);
  insPair.first.push_back(player1);
  insPair.first.push_back(act1);
  insPair.first.push_back(player2);
  std::pair<agg::trie_map<agg::AggNumber>::iterator, bool> r = cache.insert(insPair);
  if (!r.second) {
    dest(act1 + firstAction(player1), act2 + firstAction(player2)) = r.first->second;
  }
  else {
    r.first->second =
        aggPtr->Pr[player2].inner_prod(aggPtr->projection[Node][player2][act2], numNei,
                                       aggPtr->projFunctions[Node], aggPtr->payoffs[Node]);
    savePayoff(dest, player1, act1, player2, act2, r.first->second, cache, r.second);
  }
}

void aggame::KSymPayoffMatrix(cmatrix &dest, const cvector &s, agg::AggNumber fuzz) const
{
  std::vector<double> sp(s.values(), s.values() + s.getm());
  // simple implementation using expected payoffs:
  for (int rowcls = 0; rowcls < getNumPlayerClasses(); ++rowcls) {
    for (int rowa = 0; rowa < getNumKSymActions(rowcls); ++rowa) {
      for (int colcls = 0; colcls < getNumPlayerClasses(); ++colcls) {
        int multiplier = aggPtr->playerClasses[colcls].size();
        if (rowcls == colcls) {
          multiplier -= 1;
        }
        for (int cola = 0; cola < getNumKSymActions(colcls); ++cola) {

          dest(rowa + firstKSymAction(rowcls), cola + firstKSymAction(colcls)) =
              (agg::AggNumber)multiplier *
              aggPtr->getKSymMixedPayoff(sp, rowcls, rowa, colcls, cola);
        }
      }
    }
  }

  agg::AggNumber fuzzcount;
  for (int rown = 0; rown < getNumPlayerClasses(); ++rown) {
    fuzzcount = fuzz;
    for (int rowi = firstKSymAction(rown); rowi < lastKSymAction(rown); rowi++) {
      if (getNumPlayerClasses() == 1) { // symmetric case
        dest(rowi, rowi) += fuzzcount;
        fuzzcount += fuzz;
      }
      else {
        for (int coli = firstKSymAction(rown); coli < lastKSymAction(rown); coli++) {
          dest(rowi, coli) += fuzzcount;
          fuzzcount += fuzz;
        }
      }
    }
  }
}

} // namespace gametracer
} // end namespace Gambit
