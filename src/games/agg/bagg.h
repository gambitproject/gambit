//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/libagg/bagg.h
// Interface of Bayesian Action Graph Game representation
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

#ifndef GAMBIT_AGG_BAGG_H
#define GAMBIT_AGG_BAGG_H

#include <iostream>
#include "agg.h"

namespace Gambit  {

class BagentPureStrategyProfileRep;
template <class T> class BagentMixedStrategyProfileRep;

namespace agg {

#ifdef USE_CVECTOR
#include "../cmatrix.h"
typedef cvector ProbDist;
//typedef cvector NumberVector;
#else
typedef  std::vector<AggNumber>  ProbDist;
#endif

class BAGG {
public:
  static const char COMMENT_CHAR= '#';

  friend class agentform;
  friend class Gambit::BagentPureStrategyProfileRep;
  template <class T> friend class Gambit::BagentMixedStrategyProfileRep;

  friend std::ostream& operator<< (std::ostream& s, const BAGG& g);

  static BAGG* makeBAGG(char *filename);
  static BAGG* makeBAGG(std::istream &in);
  static BAGG* makeRandomBAGG(int n,std::vector<int> &numTypes,std::vector<ProbDist> &TDist,int S,int P,
	std::vector<std::vector<std::vector<int> > > &TASets,
	std::vector<std::vector<int> > &neighb,
	std::vector<projtype> &projTypes,
	int seed, bool int_payoffs=false, int int_factor=100);

  BAGG(int N, int S, 
    std::vector<int>& numTypes,
    std::vector<ProbDist>& TDist,
    std::vector<std::vector<std::vector<int > > > &typeActionSets,
    std::vector<std::vector<std::vector<int > > > &ta2a,
    AGG* aggPtr);

  ~BAGG(){
    delete[] typeOffset;
    delete[] strategyOffset;
    delete aggPtr;
  }

  inline int getNumPlayers() const {return numPlayers;}
  inline int getNumTypes() const {return typeOffset[numPlayers];}
  inline int getNumTypes(int i) const {return numTypes[i];}
  inline int getNumActions(int pl, int t) const {return typeActionSets[pl][t].size();}

  inline int getNumActionNodes() const {return numActionNodes;}
  inline int getNumFunctionNodes() const {return aggPtr->getNumFunctionNodes();}

  inline int firstAction(int pl,int t) {return strategyOffset[typeOffset[pl]+t];}
  inline int lastAction(int pl, int t) {return strategyOffset[typeOffset[pl]+t+1];}

  AggNumber getMaxPayoff(){
	  return aggPtr->getMaxPayoff();
  }
  AggNumber getMinPayoff(){
	  return aggPtr->getMinPayoff();
  }

  //exp. payoff under mixed strat profile
  AggNumber getMixedPayoff(int player, StrategyProfile &s);
  //exp payoff for player, conditioned on her receiving type tp.
  AggNumber getMixedPayoff(int player,int tp, StrategyProfile &s);

  void getPayoffVector(AggNumberVector &dest, int player,int tp, const StrategyProfile &s);
  AggNumber getV (int player, int tp, int action,const StrategyProfile &s);

  AggNumber getPurePayoff(int player, int tp, std::vector<int> &s);
  AggNumber getPurePayoff(int player, std::vector<int> &s){
    AggNumber r=0;
    for(int i=0;i<numTypes[player];++i){
      r+= indepTypeDist[player][i] * getPurePayoff(player, i, s);
    }
    return r;
  }

  AggNumber getSymMixedPayoff(StrategyProfile &s);

  AggNumber getSymMixedPayoff(int tp, StrategyProfile &s);

  AggNumber getSymMixedPayoff(int tp, int act, StrategyProfile &s);

  bool isSymmetric(){
    return symmetric;
  }

private:

  int numPlayers;
  int numActionNodes;
  std::vector<int> numTypes;
  std::vector<ProbDist> indepTypeDist;


  //for each player, each of her types, the set of actions
  std::vector<std::vector<std::vector<int> > > typeActionSets;


  std::vector<std::vector<std::vector<int > > > typeAction2ActionIndex;

  int *typeOffset;
  int *strategyOffset;
  

  //AGG that stores the action graph and utility functions
  AGG* aggPtr;

  bool symmetric;

  static void stripComment(std::istream &in);

  void getAGGStrat(StrategyProfile &as, const StrategyProfile &s, int player=-1, int tp=-1, int action=-1);
  void getSymAGGStrat(StrategyProfile &as, const StrategyProfile &s);

};

std::ostream& operator<< (std::ostream& s, const BAGG& g);

}  // end namespace Gambit::agg

}  // end namespace Gambit

 
#endif  // GAMBIT_AGG_BAGG_H
