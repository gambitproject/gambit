//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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

#ifndef __AGG_H
#define __AGG_H



#include <iostream>
#include <vector>
#include <iterator>
#include "proj_func.h"
#include "trie_map.h"

#ifdef WIN32
#ifndef drand48
#define srand48(x) srand(x)
#define drand48(x) ((double)rand(x)/RAND_MAX)
#endif
#endif


//data structure for mixed strategy profile:
//alternatively: typedef std::vector<Number> ....
#ifdef USE_CVECTOR
  #include "../cmatrix.h"
  typedef cvector StrategyProfile;
  typedef cvector NumberVector;
#else
  //typedef  Number*  StrategyProfile;
  typedef double AggNumber;
  typedef std::vector<AggNumber> StrategyProfile;
  typedef std::vector<AggNumber> AggNumberVector;
#endif

//data structure for payoff function:
//alternatively: typedef map<std::vector<int> , AggNumber> aggpayoff;
typedef trie_map<AggNumber> aggpayoff;


//data struct for prob distribution over configurations:
typedef trie_map<AggNumber> aggdistrib;

//types of input formats for payoff func
typedef enum{COMPLETE,MAPPING,ADDITIVE} payofftype; 



class agg {

public:
  typedef std::vector<int> config;
  typedef std::vector<int> ActionSet;
  typedef std::vector<int> PlayerSet;

  static const char COMMENT_CHAR= '#';
  static const char LBRACKET='[';
  static const char RBRACKET=']';



  friend class aggame;   //wrapper class for gametracer


  //read an AGG from a file
  static agg* makeAGG(char* filename);

  //read an AGG from input stream
  static agg* makeAGG(istream& in);
  
  //make AGG with random payoffs
  static agg* makeRandomAGG(int n, int* actions, int S, int P, 
std::vector<std::vector<int> >& ASets, std::vector<std::vector<int> >& neighb,
std::vector<projtype>& projTypes, int seed, bool int_payoffs=false, int int_factor=100);


  //constructor
  agg(int numPlayers, int * actions, int numANodes, int numPNodes,
   std::vector<std::vector<int> >& actionSets, std::vector<std::vector<int> >& neighbors,
   std::vector<projtype>& projTypes,
   std::vector<std::vector<aggdistrib > >& projS,
   std::vector<std::vector<std::vector<config> > >& proj,
   std::vector<std::vector<proj_func*> > & projF,
   std::vector<std::vector<std::vector<int> > >& Po,
   std::vector<aggdistrib> &P,
      std::vector<aggpayoff>& payoffs);


  //agg(const agg& other, bool completeGraph = false);


  //destructor
  virtual ~agg(){
      delete [] actions;
      delete [] strategyOffset;
      //free projFunctions
      /*
      for (size_t i=0; i<projFunctions.size(); ++i)
	  for (size_t j=0;j<projFunctions[i].size(); ++j)
	      delete projFunctions[i][j];
      */
      for (size_t i=0;i<projectionTypes.size();++i){
        delete projectionTypes[i];
      }
  }


  inline int getNumPlayers() {return numPlayers;}
  inline int getNumActions() {return totalActions;}
  inline int getNumActions(int i){return actions[i];}  
  inline int getMaxActions() {return maxActions;}
  inline int firstAction(int i) {return strategyOffset[i];}
  inline int lastAction(int i) {return strategyOffset[i+1];}

  inline int getNumActionNodes() {return numActionNodes;}
  inline int getNumFunctionNodes() {return numPNodes;}
  //inline int getNumUniqueActionSets(){return uniqueActionSets.size();}
  inline int getNumKSymActions(){return numKSymActions;}
  inline int getNumKSymActions(int i){return uniqueActionSets[i].size();}
  inline int getNumPlayerClasses(){return playerClasses.size();}
  inline const PlayerSet& getPlayerClass(int cls){return playerClasses.at(cls);}
  inline int firstKSymAction(int i){return kSymStrategyOffset[i];}
  inline int lastKSymAction(int i){return kSymStrategyOffset[i+1];}

  inline void printActionGraph(ostream& s) {
    for(size_t i=0;i< neighbors.size(); ++i){
      s<<neighbors[i].size()<<"\t";
      copy(neighbors[i].begin(),neighbors[i].end(), ostream_iterator<int>(s," ") );
      s<<endl;
    }
  }

  inline void printTypes(ostream& s) {
    for(size_t i=0;i<projectionTypes.size();i++ ){
      projectionTypes[i]->print(s);
    }
  }


  //exp. payoff under mixed strat profile
  AggNumber getMixedPayoff(int player, StrategyProfile &s);
  void getPayoffVector(AggNumberVector &dest, int player,const StrategyProfile &s);
  AggNumber getV (int player, int action,const StrategyProfile &s);
  AggNumber getJ(int player,int action, int player2,int action2,StrategyProfile &s);


  AggNumber getPurePayoff(int player, int *s);
  inline void printPayoffs( ostream & s, int node){
    s << payoffs.at(node).size()<<endl;
    s << payoffs[node];
  }

  bool isSymmetric(){
    for (int i=0;i<numPlayers;++i){
      if (actions[i]<numActionNodes) return false;
    }
    return true;
  }
  AggNumber getSymMixedPayoff( StrategyProfile &s);
  AggNumber getSymMixedPayoff(int actnode, StrategyProfile &s);
  void getSymPayoffVector(AggNumberVector& dest, StrategyProfile &s);
  AggNumber getKSymMixedPayoff( int playerClass,std::vector<StrategyProfile> &s);
  AggNumber getKSymMixedPayoff( int playerClass,StrategyProfile &s);
  AggNumber getKSymMixedPayoff(int playerClass, int act, std::vector<StrategyProfile> &s);
  AggNumber getKSymMixedPayoff(const StrategyProfile &s,int pClass1,int act1,int pClass2=-1,int act2=-1);
  void getKSymPayoffVector(AggNumberVector& dest, int playerClass, StrategyProfile &s);



  //void KSymNormalizeStrategy(StrategyProfile &s);


  AggNumberVector getExpectedConfig(StrategyProfile &s){
	  AggNumberVector res(numActionNodes, 0);
	  for (int i=0;i<numPlayers;++i){
		  for(int j=0;j<actions[i];++j){
			  res[actionSets[i][j]] += s[firstAction(i)+j];
		  }
	  }
	  return res;
  }

  std::vector<proj_func*>& getProjFunctions(int node){return projFunctions.at(node);}
  const std::vector<int>& getPorder(int player, int action){return Porder.at(player).at(action);}
  const std::vector<std::vector<config> >& getProjection(int node){return projection.at(node);}
  const std::vector<int>& getActionSet(int player){return actionSets.at(player);}
  const aggpayoff& getPayoffMap(int node){return payoffs.at(node);}

  AggNumber getMaxPayoff();
  AggNumber getMinPayoff();



  //Data structure

  int numPlayers;
  int *actions;
private:
  int *strategyOffset;
  int totalActions;
  int maxActions;


  int numActionNodes;   // |S|
  int numPNodes;        // |P|

  //action sets: for each player i \in N
  std::vector<std::vector<int> > actionSets;

  //neighbor lists: for each node \in S&P
  std::vector<std::vector<int> > neighbors;

  //Projection signature for each projected node \in P
  std::vector<projtype> projectionTypes;

  //payoff function for each action node \in S
  std::vector<aggpayoff> payoffs;

  //auxillary data strucutres

  //originally:
  //foreach s \in S, foreach i \in N, foreach s_i \in S_i,
  //the 'contribution' of s_i to D^(s)
  std::vector< std::vector<std::vector<config> > > projection;

  //a more compact way:
  //foreach s \in S, foreach s' \in S, 
  // the contribution of s' to D^(s)
  //std::vector<std::vector<config> > projection;

  //foreach s \in S, foreach i \in N, the projected mixed strat
  //which is a prob distribution over the set of 'contributions'
  std::vector< std::vector<aggdistrib > > projectedStrat;

  // foreach s in S, i in N, the full set of projected actions.
  std::vector<std::vector<aggdistrib> >fullProjectedStrat;

  //foreach s in S, foreach neighbor of s, its projection function 
  std::vector<std::vector<proj_func*> > projFunctions;

  //foreach i \in N, foreach s_i in S_i, the order of agents o_1.. o_{n-1}
  // in which we apply the DP algorithm
  std::vector< std::vector< std::vector<int> > > Porder;

  //when computing the induced distribution via ComputeP():
  //foreach k<= n-1,  
  //prob. distrib P_k induced by the partial strat profile of agents o_1..o_k

  //when computing the partial distributions for the payoff jacobian:
  //  foreach  j \in N,
  // the partial distribution induced by all agents except j. 
  std::vector<aggdistrib>  Pr;

  //foreach s in S, whether s's neighbors are all action nodes
  std::vector<bool> isPure;

  //foreach s in S, j in N, the index of s in j's action set, or -1 if N/A
  std::vector<std::vector<int> > node2Action;

  //cache of jacobian entries.
  trie_map<AggNumber> cache;

  //the unique action sets
  std::vector<ActionSet> uniqueActionSets;

  //equivalent classes of players
  std::vector<PlayerSet> playerClasses;

  //the class index for each player
  std::vector<int> player2Class;

  //sum of the sizes of uniqueActionSets
  int numKSymActions;

  //strategyOffset for kSymmetric strategy profile
  std::vector<int> kSymStrategyOffset;


  //input functor 
  struct input : public std::unary_function<aggpayoff::iterator , void>{
    input(istream& i): in(i) {}
    void operator() (aggpayoff::iterator p) {
	in >> (*p).second;
    }
    istream& in;
  };

  struct inputRand : public std::unary_function<aggpayoff::iterator, void>{
    inputRand(bool int_payoffs=false, int int_factor=100):int_payoffs(int_payoffs),int_factor(int_factor) {}
    void operator() (aggpayoff::iterator p){
      p->second = drand48();
      if(int_payoffs) p->second = floor(p->second * AggNumber(int_factor) );
    }  
    bool int_payoffs;
    int int_factor;
  };

  //private static methods:


  static void makeCOMPLETEpayoff( istream& in, aggpayoff& pay){
      pay.in_order(input(in));
  }
  static void makeMAPPINGpayoff(istream& in, aggpayoff& pay, int);

  static void stripComment(istream& in);

  static void setProjections(std::vector<std::vector<aggdistrib > >& projS,
  std::vector<std::vector<std::vector<config> > >& proj, int N,int S,int P, std::vector<std::vector<int> >& AS, std::vector<std::vector<int> >& neighb, std::vector<projtype>& projTypes);

  static void getAn(std::multiset<int>& dest, std::vector<std::vector<int> >& neighb, std::vector<projtype>& projTypes,int S,int Node, std::vector<int>& path);

  static  void initPorder(std::vector<int>& Po,
		     int i,  int N,
		     std::vector<aggdistrib>& projS);


  //private methods:
  void computeP(int player, int act, int player2=-1,int act2=-1);
  void doProjection(int Node,const StrategyProfile& s){
	  doProjection (Node, &(const_cast<StrategyProfile &>(s)[0]));
  }
  void doProjection(int Node, int player, const StrategyProfile& s){
	  doProjection(Node,player, &(const_cast<StrategyProfile &>(s)[firstAction(player)]));
  }
  void doProjection(int Node, AggNumber* s);
  void doProjection(int Node, int player, AggNumber* s);

  void getSymConfigProb(int plClass, StrategyProfile &s, int ownPlClass, int act, aggdistrib &dest,int plClass2=-1,int act2=-1);
};



#endif
