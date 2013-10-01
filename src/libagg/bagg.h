// bagg.h: header for the Bayesian Action Graph Game class

#ifndef __BAGG_H
#define __BAGG_H



#include <iostream>
#include "agg.h"



#ifdef USE_CVECTOR
#include "../cmatrix.h"
typedef cvector ProbDist;
//typedef cvector NumberVector;
#else
typedef  std::vector<AggNumber>  ProbDist;
#endif


namespace Gambit{
  class BagentPureStrategyProfileRep;
  template <class T> class BagentMixedStrategyProfileRep;
}

class bagg {

public:
  static const char COMMENT_CHAR= '#';

  friend class agentform;
  friend class Gambit::BagentPureStrategyProfileRep;
  template <class T> friend class Gambit::BagentMixedStrategyProfileRep;

  friend std::ostream& operator<< (std::ostream& s, const bagg& g);

  static bagg* makeBAGG(char *filename);
  static bagg* makeBAGG(istream& in);
  static bagg* makeRandomBAGG(int n,std::vector<int> &numTypes,std::vector<ProbDist> &TDist,int S,int P,
	std::vector<std::vector<std::vector<int> > > &TASets,
	std::vector<std::vector<int> > &neighb,
	std::vector<projtype> &projTypes,
	int seed, bool int_payoffs=false, int int_factor=100);

  bagg(int N, int S, 
    std::vector<int>& numTypes,
    std::vector<ProbDist>& TDist,
    std::vector<std::vector<std::vector<int > > > &typeActionSets,
    std::vector<std::vector<std::vector<int > > > &ta2a,
    agg* aggPtr);

  ~bagg(){
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

  AggNumber getPurePayoff(int player, int tp, int *s);
  AggNumber getPurePayoff(int player, int *s){
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
  agg* aggPtr;

  bool symmetric;

  static void stripComment(istream& in);

  void getAGGStrat(StrategyProfile &as, const StrategyProfile &s, int player=-1, int tp=-1, int action=-1);
  void getSymAGGStrat(StrategyProfile &as, const StrategyProfile &s);

};

std::ostream& operator<< (std::ostream& s, const bagg& g);


#endif
