// agg.h: header for the Action Graph Game class


#ifndef __AGG_H
#define __AGG_H

using namespace std;

/* #include "gnmgame.h" */

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
//alternatively: typedef vector<Number> ....
#ifdef USE_CVECTOR
  #include "../cmatrix.h"
  typedef cvector StrategyProfile;
  typedef cvector NumberVector;
#else
  //typedef  Number*  StrategyProfile;
  typedef double Number;
  typedef vector<Number> StrategyProfile;
  typedef vector<Number> NumberVector;
#endif

//data structure for payoff function:
//alternatively: typedef map<vector<int> , Number> aggpayoff;
typedef trie_map<Number> aggpayoff;


//data struct for prob distribution over configurations:
typedef trie_map<Number> aggdistrib;

//types of input formats for payoff func
typedef enum{COMPLETE,MAPPING,ADDITIVE} payofftype; 



class agg {

public:
  typedef vector<int> config;
  typedef vector<int> ActionSet;
  typedef vector<int> PlayerSet;

  static const char COMMENT_CHAR= '#';
  static const char LBRACKET='[';
  static const char RBRACKET=']';


#ifdef USE_CVECTOR
  friend class aggame;   //wrapper class for gametracer
#endif

  //read an AGG from a file
  static agg* makeAGG(char* filename);

  //read an AGG from input stream
  static agg* makeAGG(istream& in);
  
  //make AGG with random payoffs
  static agg* makeRandomAGG(int n, int* actions, int S, int P, 
vector<vector<int> >& ASets, vector<vector<int> >& neighb,
vector<projtype>& projTypes, int seed, bool int_payoffs=false, int int_factor=100);


  //constructor
  agg(int numPlayers, int * actions, int numANodes, int numPNodes,
   vector<vector<int> >& actionSets, vector<vector<int> >& neighbors,
   vector<projtype>& projTypes,
   vector<vector<aggdistrib > >& projS,
   vector<vector<vector<config> > >& proj,
   vector<vector<proj_func*> > & projF,
   vector<vector<vector<int> > >& Po,
   vector<aggdistrib> &P,
      vector<aggpayoff>& payoffs); 


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
  Number getMixedPayoff(int player, StrategyProfile &s);
  void getPayoffVector(NumberVector &dest, int player,const StrategyProfile &s);
  Number getV (int player, int action,const StrategyProfile &s);
  Number getJ(int player,int action, int player2,int action2,StrategyProfile &s);

#ifdef USE_CVECTOR
  //compute payoff jacobian
  void payoffMatrix(cmatrix &dest, cvector &s, Number fuzz);
#endif


  Number getPurePayoff(int player, int *s);
  inline void printPayoffs( ostream & s, int node){
    s << payoffs[node].size()<<endl;
    s << payoffs[node];
  }

  bool isSymmetric(){
    for (int i=0;i<numPlayers;++i){
      if (actions[i]<numActionNodes) return false;
    }
    return true;
  }
  Number getSymMixedPayoff( StrategyProfile &s);
  Number getSymMixedPayoff(int actnode, StrategyProfile &s);
  void getSymPayoffVector(NumberVector& dest, StrategyProfile &s);
  Number getKSymMixedPayoff( int playerClass,vector<StrategyProfile> &s);
  Number getKSymMixedPayoff( int playerClass,StrategyProfile &s);
  Number getKSymMixedPayoff(int playerClass, int act, vector<StrategyProfile> &s);
  Number getKSymMixedPayoff(const StrategyProfile &s,int pClass1,int act1,int pClass2=-1,int act2=-1);
  void getKSymPayoffVector(NumberVector& dest, int playerClass, StrategyProfile &s);

#ifdef USE_CVECTOR
  void SymPayoffMatrix(cmatrix &dest, cvector &s, Number fuzz);
  void KSymPayoffMatrix(cmatrix &dest, cvector &s, Number fuzz);
#endif

  //void KSymNormalizeStrategy(StrategyProfile &s);


  NumberVector getExpectedConfig(StrategyProfile &s){
	  NumberVector res(numActionNodes, 0);
	  for (int i=0;i<numPlayers;++i){
		  for(int j=0;j<actions[i];++j){
			  res[actionSets[i][j]] += s[firstAction(i)+j];
		  }
	  }
	  return res;
  }

  vector<proj_func*>& getProjFunctions(int node){return projFunctions.at(node);}
  const vector<int>& getPorder(int player, int action){return Porder.at(player).at(action);}
  const vector<vector<config> >& getProjection(int node){return projection.at(node);}
  const vector<int>& getActionSet(int player){return actionSets.at(player);}
  const aggpayoff& getPayoffMap(int node){return payoffs.at(node);}

  Number getMaxPayoff();
  Number getMinPayoff();



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
  vector<vector<int> > actionSets;

  //neighbor lists: for each node \in S&P
  vector<vector<int> > neighbors; 

  //Projection signature for each projected node \in P
  vector<projtype> projectionTypes;

  //payoff function for each action node \in S
  vector<aggpayoff> payoffs; 

  //auxillary data strucutres

  //originally:
  //foreach s \in S, foreach i \in N, foreach s_i \in S_i,
  //the 'contribution' of s_i to D^(s)
  vector< vector<vector<config> > > projection;

  //a more compact way:
  //foreach s \in S, foreach s' \in S, 
  // the contribution of s' to D^(s)
  //vector<vector<config> > projection;

  //foreach s \in S, foreach i \in N, the projected mixed strat
  //which is a prob distribution over the set of 'contributions'
  vector< vector<aggdistrib > > projectedStrat;

  // foreach s in S, i in N, the full set of projected actions.
  vector<vector<aggdistrib> >fullProjectedStrat;

  //foreach s in S, foreach neighbor of s, its projection function 
  vector<vector<proj_func*> > projFunctions;

  //foreach i \in N, foreach s_i in S_i, the order of agents o_1.. o_{n-1}
  // in which we apply the DP algorithm
  vector< vector< vector<int> > > Porder;

  //when computing the induced distribution via ComputeP():
  //foreach k<= n-1,  
  //prob. distrib P_k induced by the partial strat profile of agents o_1..o_k

  //when computing the partial distributions for the payoff jacobian:
  //  foreach  j \in N,
  // the partial distribution induced by all agents except j. 
  vector<aggdistrib>  Pr;

  //foreach s in S, whether s's neighbors are all action nodes
  vector<bool> isPure;

  //foreach s in S, j in N, the index of s in j's action set, or -1 if N/A
  vector<vector<int> > node2Action;

  //cache of jacobian entries.
  trie_map<Number> cache;

  //the unique action sets
  vector<ActionSet> uniqueActionSets;

  //equivalent classes of players
  vector<PlayerSet> playerClasses;

  //the class index for each player
  vector<int> player2Class;

  //sum of the sizes of uniqueActionSets
  int numKSymActions;

  //strategyOffset for kSymmetric strategy profile
  vector<int> kSymStrategyOffset;


  //input functor 
  struct input : public unary_function<aggpayoff::iterator , void>{
    input(istream& i): in(i) {}
    void operator() (aggpayoff::iterator p) {
	in >> (*p).second;
    }
    istream& in;
  };

  struct inputRand : public unary_function<aggpayoff::iterator, void>{
    inputRand(bool int_payoffs=false, int int_factor=100):int_payoffs(int_payoffs),int_factor(int_factor) {}
    void operator() (aggpayoff::iterator p){
      p->second = drand48();
      if(int_payoffs) p->second = floor(p->second * Number(int_factor) );
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

  static void setProjections(vector<vector<aggdistrib > >& projS, 
  vector<vector<vector<config> > >& proj, int N,int S,int P, vector<vector<int> >& AS, vector<vector<int> >& neighb, vector<projtype>& projTypes);

  static void getAn(multiset<int>& dest, vector<vector<int> >& neighb, vector<projtype>& projTypes,int S,int Node, vector<int>& path);

  static  void initPorder(vector<int>& Po,
		     int i,  int N,
		     vector<aggdistrib>& projS);


  //private methods:
  void computeP(int player, int act, int player2=-1,int act2=-1);
  void  doProjection(int Node,const StrategyProfile& s);
  void doProjection(int Node, int player, const StrategyProfile& s);

#ifdef USE_CVECTOR
  //helper functions for computing jacobian
  void computePartialP_PureNode(int player,int act,vector<int>& tasks);
  void computePartialP_bisect(int player,int act, vector<int>::iterator f,vector<int>::iterator l,aggdistrib& temp);
  void computePartialP(int player1, int act1, vector<int>& tasks,vector<int>& nontasks);
  void computePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,trie_map<Number>& cache);
  void savePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,Number result,
	trie_map<Number>& cache, bool partial=false );
  void computeUndisturbedPayoff(Number& undisturbedPayoff,bool& has,int player1,int act1,int player2);
#endif

  void getSymConfigProb(int plClass, StrategyProfile &s, int ownPlClass, int act, aggdistrib &dest,int plClass2=-1,int act2=-1);
};



#endif
