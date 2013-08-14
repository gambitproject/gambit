
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include "bagg.h"

using namespace std;


bagg::bagg(int N, int S, 
vector<int>& numTypes,
vector<ProbDist>& TDist,
vector<vector<vector<int > > > &typeActionSets,
vector<vector<vector<int > > > &ta2a,
agg* aggPtr):
numPlayers(N),
numActionNodes(S),
numTypes(numTypes),
indepTypeDist(TDist),
typeActionSets(typeActionSets),
typeAction2ActionIndex(ta2a),
aggPtr(aggPtr)
{
  typeOffset=new int[numPlayers+1];
  typeOffset[0]=0;
  for(int i=0;i<numPlayers;i++){
      typeOffset[i+1]=typeOffset[i]+numTypes[i];
  }
  strategyOffset=new int[typeOffset[numPlayers]+1];
  strategyOffset[0]=0;
  for(int i=0;i<numPlayers;++i){
      for(int j=0;j<numTypes[i];++j){
	  int idx=typeOffset[i]+j;
          strategyOffset[idx+1]=strategyOffset[idx]+typeActionSets[i][j].size();
      }
  }
  symmetric = true;
  for(int i=1;i<numPlayers;++i){
     if (numTypes[i]!=numTypes[0]) {symmetric= false;break;}
     if (indepTypeDist[i]!=indepTypeDist[0]) {symmetric= false;break;}
     if (typeActionSets[i]!=typeActionSets[0]) {symmetric=false;break;}
  }
}


void bagg::stripComment(istream& in){
  in>>ws;
  char c =in.peek();
  stringbuf discard(ios_base::out);
  if(c== bagg::COMMENT_CHAR){
    in.get (discard);
    stripComment(in);
  }
}

bagg* bagg::makeBAGG(char* filename)
{
  ifstream in(filename);
  return bagg::makeBAGG(in);
}

bagg* bagg::makeBAGG(istream& in){
  int N,S,P;

  stripComment(in);
  in>> N;
  stripComment(in);
  in>>S;
  stripComment(in);
  in>>P;

  stripComment(in);
  vector<int> numTypes(N);

  //input number of types for each player
  for(int i=0;i<N;++i){
    if(in.eof()||in.bad()){
      cerr<<"Error in game file: integer expected for the number of types for player "<<i<<endl;
      exit(1);
    }
    in>>numTypes[i];
  }

  //input the type distributions
  stripComment(in);
  vector<ProbDist> TDist;
  for(int i=0;i<N;++i){
    TDist.push_back(ProbDist(numTypes[i]) );
    for(int j=0;j<numTypes[i];++j){
      if(in.eof()||in.bad()){
            cerr<<"Error in game file: number expected for type distribution"<<endl;
            exit(1);
      }
      in>>TDist[i][j];
    }
  }

  stripComment(in);
  //sizes of type action sets
  vector<vector<vector<int> > > typeActionSets(N);
  for (int i=0;i<N;++i){
    for(int j=0;j<numTypes[i];++j){
      if(in.eof()||in.bad()){
            cerr<<"Error in game file: integer expected for size of type action set"<<endl;
            exit(1);
      }
      int temp;
      in>>temp;
      typeActionSets[i].push_back(vector<int>(temp));
    
    }
  }

    
  //type action sets
  stripComment(in);
  for(int i=0;i<N;++i){
    for(int j=0;j<numTypes[i];++j){
      for(size_t k=0;k<typeActionSets[i][j].size();++k){
        if(in.eof()||in.bad()){
              cerr<<"Error in game file: integer expected for type action set"<<endl;
              exit(1);
        }
        in>> typeActionSets[i][j][k];
      }
    }
  }
  stripComment(in);

  //action sets
  vector<vector<int> > aggActionSets;
  for (int i=0;i<N;++i){
    set<int> aSet;
    for (int j=0;j<numTypes[i];++j){
      aSet.insert(typeActionSets[i][j].begin(),typeActionSets[i][j].end());
    }
    aggActionSets.push_back( vector<int>(aSet.begin(),aSet.end()) );
  }

  vector<vector<vector<int > > > typeAction2ActionIndex(typeActionSets);
  for(int i=0;i<N;++i){
    for(int j=0;j<numTypes[i];++j){
      for(size_t k=0;k<typeActionSets[i][j].size();++k){
        typeAction2ActionIndex[i][j][k]=
          find(aggActionSets[i].begin(),aggActionSets[i].end(),typeActionSets[i][j][k]) 
          - aggActionSets[i].begin();
      }
    }
  }

  //stream for constructing agg
  stringstream aggss(stringstream::in | stringstream::out);
  aggss<<N<<endl<<S<<endl<<P<<endl;
  for (int i=0;i<N;i++) aggss<<aggActionSets[i].size()<<endl;
  for (int i=0;i<N;i++){
    for(size_t j=0;j<aggActionSets[i].size();j++){
      aggss<<aggActionSets[i][j]<<" ";
    }
    aggss<<endl;
  }

  //copy rest of input into aggss
  while(in){
    string ln;
    getline(in,ln);
    aggss<<ln<<endl;
  }

  agg* aggPtr = agg::makeAGG(aggss);
  if(!aggPtr){
	  cerr<<"Error in BAGG file when reading the AGG part of the input."<<endl;
	  exit(1);
  }
  return new bagg(N,S,numTypes,TDist,typeActionSets,typeAction2ActionIndex,aggPtr);
}

bagg* bagg::makeRandomBAGG(int N,vector<int> &numTypes,vector<ProbDist> &TDist,int S,int P,
        vector<vector<vector<int> > > &typeActionSets,
        vector<vector<int> > &neighb,
        vector<projtype> &projTypes,
        int seed, bool int_payoffs, int int_factor)
{
  //action sets
  vector<vector<int> > aggActionSets;
  for (int i=0;i<N;++i){
    set<int> aSet;
    for (int j=0;j<numTypes[i];++j){
      aSet.insert(typeActionSets[i][j].begin(),typeActionSets[i][j].end());
    }
    aggActionSets.push_back( vector<int>(aSet.begin(),aSet.end()) );
  }


  int actions[N];
  for(int i=0;i<N;++i){
    actions[i]=aggActionSets[i].size();
  }

  vector<vector<vector<int > > > typeAction2ActionIndex(typeActionSets);
  for(int i=0;i<N;++i){
    for(int j=0;j<numTypes[i];++j){
      for(size_t k=0;k<typeActionSets[i][j].size();++k){
        typeAction2ActionIndex[i][j][k]=
          find(aggActionSets[i].begin(),aggActionSets[i].end(),typeActionSets[i][j][k]) 
          - aggActionSets[i].begin();
      }
    }
  }


  agg* aggPtr = agg::makeRandomAGG(N,actions,S,P, aggActionSets, neighb, projTypes, seed,int_payoffs,int_factor);
  if(!aggPtr){
	  std::cerr<<"MakeRandomBAGG(): failed to make underlying AGG."<<endl;
	  return NULL;
  }
  return new bagg(N,S,numTypes,TDist,typeActionSets, typeAction2ActionIndex, aggPtr);
}

AggNumber bagg::getMixedPayoff(int player, StrategyProfile &s){
  AggNumber res(0);
  for (int tp=0;tp<numTypes[player];++tp){
    res+=indepTypeDist[player][tp] * getMixedPayoff(player,tp,s);
  }
  return res;
}

AggNumber bagg::getMixedPayoff(int player,int tp, StrategyProfile &s){
    AggNumber res(0);
    for (size_t act=0;act<typeActionSets[player][tp].size(); ++act)
	if (s[act+firstAction(player,tp)]>AggNumber(0.0))
	    res+= s[act+firstAction(player,tp)] * getV(player,tp,act,s);
    return res;
}

void bagg::getPayoffVector(AggNumberVector &dest, int player,int tp, const StrategyProfile &s){
    assert(player>=0&&player < getNumPlayers() && tp>=0 && tp<getNumTypes(player));
    for(size_t act=0;act<typeActionSets[player][tp].size(); ++act){
      dest[act] = getV(player,tp,act,s);
    }
}

void bagg::getAGGStrat(StrategyProfile &as, const StrategyProfile &s, int player, int tp, int action){
    for (int i=0;i<aggPtr->getNumActions();++i) as[i]=AggNumber(0.0);

    for (int pl=0;pl<numPlayers;++pl){
	if (pl!=player){
	    for(int t=0;t<numTypes[pl];++t){
		for (size_t act=0; act< typeActionSets[pl][t].size(); ++act){
		    int aact= typeAction2ActionIndex[pl][t][act];
		    as[aact+aggPtr->firstAction(pl)] += indepTypeDist[pl][t] * s[act+firstAction(pl,t)];
		}
	    }
	}
	else{
	    int aact = typeAction2ActionIndex[player][tp][action];
	    as[aact+aggPtr->firstAction(player)]=1;
	}
    }

}
AggNumber bagg::getV (int player, int tp, int action,const StrategyProfile &s){
    StrategyProfile as(aggPtr->getNumActions());
    getAGGStrat(as, s, player,tp,action);
    return aggPtr->getV(player, typeAction2ActionIndex[player][tp][action], as);
}

AggNumber bagg::getPurePayoff(int player, int tp, int *ps)
{
  StrategyProfile st(strategyOffset[typeOffset[numPlayers]]);
  for (int i=0;i<strategyOffset[typeOffset[numPlayers]];i++) st[i]=(AggNumber) 0.0;
  for(int i=0;i<typeOffset[numPlayers];i++){
    st[ strategyOffset[i]+ps[i] ]=(AggNumber)1.0;
  }
  return getMixedPayoff(player,tp,st);
}

void bagg::getSymAGGStrat(StrategyProfile &as, const StrategyProfile &s)
{
  for (int i=0;i<aggPtr->getNumActionNodes();++i) as[i]=AggNumber(0.0);
  for(int t=0;t<numTypes[0];++t){
        for (size_t act=0; act< typeActionSets[0][t].size(); ++act){
            int aact= typeAction2ActionIndex[0][t][act];
            as[aact] += indepTypeDist[0][t] * s[act+firstAction(0,t)];
        }
  }
}


AggNumber bagg::getSymMixedPayoff(StrategyProfile &s)
{
  AggNumber res(0);
  for (int tp=0;tp<numTypes[0];++tp){
    res+=indepTypeDist[0][tp] * getSymMixedPayoff(tp,s);
  }
  return res;

}

AggNumber bagg::getSymMixedPayoff(int tp, StrategyProfile &s)
{
  AggNumber res(0);
  for (size_t act=0;act<typeActionSets[0][tp].size(); ++act)
      if (s[act+firstAction(0,tp)]>AggNumber(0.0))
          res+= s[act+firstAction(0,tp)] * getSymMixedPayoff(tp,act,s);
  return res;
}

AggNumber bagg::getSymMixedPayoff(int tp, int act, StrategyProfile &s)
{
  StrategyProfile as(aggPtr->getNumActionNodes());
  getSymAGGStrat(as, s);
  return aggPtr->getSymMixedPayoff(typeActionSets[0][tp][act], as);
}





ostream& operator<< (ostream& s, const bagg& g)
{
  //BAGG identifier for Gambit ReadGame
  s<<"#BAGG"<<endl;

  //num players

  s<<g.getNumPlayers()<<endl;
  
  //num action nodes

  s<<g.getNumActionNodes()<<endl;

  //num func nodes
  s<<g.getNumFunctionNodes()<<endl;


  //num types
  for (int i=0;i<g.getNumPlayers();++i) {
    s<<g.getNumTypes(i)<<" ";
  }
  s<<endl;

  //type distrib
  for (int i=0;i<g.getNumPlayers();++i){
    for(int j=0;j<g.getNumTypes(i);++j){
      s<<g.indepTypeDist[i][j]<<" ";
    }
    s<<endl;
  }

  //sizes of type action sets
  for(int i=0;i<g.getNumPlayers();++i){
    for(int j=0;j<g.getNumTypes(i);++j){
      s<< g.typeActionSets[i][j].size()<<" ";
    }
    s<<endl;
  }

  //type action sets
  for(int i=0;i<g.getNumPlayers();++i){
    for(int j=0;j<g.getNumTypes(i);++j){
      copy(g.typeActionSets[i][j].begin(), g.typeActionSets[i][j].end(), ostream_iterator<int>(s," ") );
      s<<endl;
    }
  }

  //action graph
  g.aggPtr->printActionGraph(s);
  s<<endl;

  //types of func nodes
  //copy(g.aggPtr->projectionTypes.begin(),g.aggPtr->projectionTypes.end(), ostream_iterator<int>(s," ") );
  g.aggPtr->printTypes(s);
  s<<endl;

  //payoffs
  for (int i=0;i<g.getNumActionNodes();++i){
    s<<"1"<<endl; //type of payoff output
    g.aggPtr->printPayoffs(s,i);
    s<<endl;
  }

  return s;
}
