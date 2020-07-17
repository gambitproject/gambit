//
// This file is part of Gambit
// Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/libagg/agg.cc
// Implementation of Action Graph Game representation
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


#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include "gambit/agg/gray.h"
#include "gambit/agg/agg.h"

using namespace std;

namespace Gambit {

namespace agg {

inline int select2nd(const pair <int,int> &x) { return x.second; } 

AGG::AGG(int numPlayers,int* _actions, int numANodes, int _numPNodes, 
 vector<vector<int> >& _actionSets, vector<vector<int> >& neighb,
 vector<projtype>& projTypes,
 vector<vector<aggdistrib > >& projS,
 vector<vector<vector<config> > >& proj,
 vector<vector<proj_func*> > & projF,
 vector<vector<vector<int> > >& Po,
 vector<aggdistrib> &P,
 vector<aggpayoff>& _payoffs) :
numPlayers(numPlayers),
numActionNodes(numANodes),
numPNodes(_numPNodes),
actionSets(_actionSets),
neighbors(neighb),
projectionTypes(projTypes),
payoffs(_payoffs),
projection(proj),
projectedStrat(projS),
fullProjectedStrat(projS),
projFunctions(projF),
Porder(Po),
Pr(P),
isPure(numANodes,true),
node2Action(numANodes,vector<int>(numPlayers)),
cache(numPlayers+1),
player2Class(numPlayers),
kSymStrategyOffset(1,0)
{

  //actions
  actions=new int[numPlayers];
  strategyOffset= new int[numPlayers+1];
  strategyOffset[0]=0;
  maxActions=0;
  totalActions=0;
  
  for (int i=0;i<numPlayers;i++){
    actions[i]=_actions[i];
    totalActions+=actions[i];
    strategyOffset[i+1]=strategyOffset[i]+actions[i];
    if(actions[i]>maxActions) maxActions=actions[i];
  }



  //construct uniqueActionSets,playerClasses and player2Class
  vector<pair<vector<int> ,int> >t;
  vector<vector<int> > sortedActionSets (actionSets);
  //sort the actions in each action set in ascending order
  for (int i=0;i<numPlayers;i++){
    sort(sortedActionSets[i].begin(),sortedActionSets[i].end());
    if (sortedActionSets[i] != actionSets[i]){
	cerr<< "WARNING: action set for player "<<i<<" is not in ascending order:"<<endl;
	copy(actionSets[i].begin(),actionSets[i].end(), ostream_iterator<int>(cerr," "));
	cerr<<endl<<"This potentially affects computations with symmetric (or k-symmetric) strategies"<<endl;
    }
    t.push_back(make_pair(sortedActionSets[i],i));
  }
  sort(t.begin(),t.end());
  //vector<pair<vector<int>,int> >::iterator new_end = unique(t.begin(),t.end());
  vector<pair<vector<int>,int> >::iterator p=t.begin();
  uniqueActionSets.push_back(p->first);
  playerClasses.push_back(vector<int> (1,p->second));
  player2Class[p->second]=0;
  numKSymActions=p->first.size();
  kSymStrategyOffset.push_back(numKSymActions);

  while(++p!=t.end()){
      if(p->first == uniqueActionSets.at(uniqueActionSets.size()-1)){
	  playerClasses[playerClasses.size()-1].push_back(p->second);
      }
      else{
	  uniqueActionSets.push_back(p->first);
	  playerClasses.push_back(vector<int> (1, p->second));
	  numKSymActions += p->first.size();
	  kSymStrategyOffset.push_back(numKSymActions);
      }
      player2Class[p->second]=playerClasses.size()-1;
  }


  //set isPure
  for (int i=0; i<numANodes; i++)if (neighb.at(i).size()>0){
    int maxNode=*( max_element(neighb.at(i).begin(),neighb.at(i).end()));
    isPure[i]=(maxNode <numANodes );
  }

  //set node2Action
  for (int i=0;i<numANodes;i++)
    for(int j=0;j<numPlayers;j++)
	node2Action[i][j]=-1;
  for(int i=0;i<numPlayers;i++)
    for(int j=0;j<actions[i];j++)
	node2Action[actionSets[i][j]][i]=j;

}

/*
AGG::AGG(const agg& other, bool completeGraph)
:
    numPlayers(other.numPlayers)
    //numActionNodes(other.numActionNodes),
    //numPNodes(other.numPNodes)
    //actionSets(actionSets),
    //neighbors(neighb),
    //projectionTypes(projTypes),
    //payoffs(_payoffs),
    //projection(proj),
    //projectedStrat(projS),
    //fullProjectedStrat(projS),
    //projFunctions(projF),
    //Porder(Po),
    //Pr(P),
    //isPure(numANodes,true),
    //node2Action(numANodes,vector<int>(numPlayers)),
    //cache(numPlayers+1),
    //player2Class(numPlayers)
{
  //actions
  actions=new int[numPlayers];
  strategyOffset= new int[numPlayers+1];
  strategyOffset[0]=0;
  maxActions=0;
  totalActions=0;

  for (int i=0;i<numPlayers;i++){
    actions[i]=other.actions[i];
    totalActions+=actions[i];
    strategyOffset[i+1]=strategyOffset[i]+actions[i];
    if(actions[i]>maxActions) maxActions=actions[i];
  }

  if (completeGraph){

    numActionNodes=totalActions;
    numPNodes=0;
    for (int i=0;i<numPlayers;i++){
      for (int j=0;j<actions[i];j++)
        actionSets[i][j] = firstAction(i)+j;
    }
  }
  else{
b    numActionNodes=other.numActionNodes;

    numPNodes=other.numPNodes;
    actionSets=other.actionSets;
    neighbors=other.neighbors;
    projectionTypes=other.projectionTypes;
    payoffs=other.payoffs;
    projection=other.projection;
    projectedStrat=other.projectedStrat;
    fullProjectedStrat=other.fullProjectedStrat;
    //projFunctions=other.projFunctions;
    Porder=other.Porder;
    Pr=other.Pr;
    isPure=other.isPure;
    node2Action=other.node2Action;
    cache=other.cache;
    uniqueActionSets=other.uniqueActionSets;
    playerClasses=other.playerClasses;
    player2Class=other.player2Class;

    projFunctions=vector<vector<proj_func*> > (numActionNodes);
    for (int i=0;i<numActionNodes;i++){
         int neighb_size=neighbors[i].size();
         for(int j=0;j<neighb_size; j++){
           projtype t=(neighbors[i][j]<numActionNodes)?P_SUM:projectionTypes[neighbors[i][j]-numActionNodes];
           projFunctions[i].push_back(make_proj_func(t, numActionNodes+numPNodes) );
         }
     }

  }
}
*/

void AGG::stripComment(istream& in){
  in>>ws;
  char c =in.peek();
  stringbuf discard(ios_base::out);
  if(c== AGG::COMMENT_CHAR){
    in.get (discard);
#ifdef AGGDEBUG
    cerr<<discard.str()<<endl;
#endif
    stripComment(in);
  }
}


AGG *AGG::makeAGG(char* filename){
  ifstream in(filename);
  return AGG::makeAGG(in);
}
AGG *AGG::makeAGG(istream &in){
  int i,j,n,S,P;
  int neighb_size;
  
  if(in.good() && !in.eof()) {
    stripComment(in);
    in>>n;
    if(!in.good()) {
      cout<<"Error reading the number of players"<<endl;
      return 0;
    }
    stripComment(in);
    in>>S;
    if(!in.good()) {
      cout<<"Error reading the number of action nodes"<<endl;
      return 0;
    }
    stripComment(in);
    in>>P;
    if(!in.good()) {
      cout<<"Error reading the number of function nodes"<<endl;
      return 0;
    }
    stripComment(in);

    //enter sizes of action sets:
    int* size = new int[n];
    for (i=0;i<n;i++){
      in >> size[i];
      if(in.eof()||in.fail()) {
              cout << "Error in game file while trying to read the size of action set of player "<<i<<".\n";
              return 0;
      }
      //sumActionSets+=size[i];
    }

    stripComment(in);
    vector<vector<int> > ASets(n); //action sets
    for (i=0;i<n;i++){
      stripComment(in);
      for (j=0;j<size[i];j++){
	int aindex;
	in>>aindex;
	if(in.eof()||in.fail()) {
	        cout<< "Error in game file while trying to read the node index of action "<<j<< " of player "
	            <<i<<".\n";
	        return 0;
	}
	ASets[i].push_back(aindex);
      }
    }

    stripComment(in);
    vector<vector<int> > neighb(S+P); //neighbor lists
    for(i=0;i<S+P;i++){

      stripComment(in);
      in>>neighb_size;
      if(in.eof()||in.fail()) {
        cout << "Error in game file while trying to read the size of the neighbor list of node "<<i<<".\n";
        return 0;
      }
      for(j=0;j<neighb_size;j++){
	int nindex;
	in>>nindex;
	if(!in.good()){
	  cout<<"Error while reading neighor #"<<j<<" of node "<<i<<".\n";
	  return 0;
	}
	neighb[i].push_back(nindex);
      }
    }

    stripComment(in);
    //enter the projection types:
    vector<projtype> projTypes(P);
    for (i=0;i<P;++i) {
	int pt;
	stripComment(in);
	in >> pt;
        if (in.eof()||in.fail()){
          cout<<"Error in game file: expected integer for type of function node #"<<i<<endl;
          return 0;
        }
	projTypes[i] = make_proj_func((TypeEnum)pt,in,S,P);
    }

    vector<vector<aggdistrib > > projS;
    vector<vector<vector<config> > > proj;
    setProjections(projS,proj,n,S,P, ASets, neighb,projTypes);

    vector<vector<proj_func*> > projF(S);
    for (i=0;i<S;i++){
	neighb_size=neighb[i].size();
	for(j=0;j<neighb_size; j++){
	  projtype t=(neighb[i][j]<S)?(new proj_func_SUM):projTypes[neighb[i][j]-S];
	  projF[i].push_back(t );
	}
    }

    vector<vector<vector<int> > > Po(n);
    vector<aggdistrib>  Pr(n);
    vector<aggpayoff> pays(S); //payoffs


    set<vector<int> > doneASets;
    for (i=0;i<n;i++){
      for(j=0;j<size[i] ; j++){
	Po[i].push_back(vector<int>(n) );
	initPorder (Po[i][j], i,n,projS[ASets[i][j]]);
      }
      vector<int> as = ASets[i];
      sort(as.begin(),as.end());
      if (doneASets.count(as)==0){
        for(j=0;j<size[i];j++){
          // apply i's strategy j
          Pr[0].reset();
          Pr[0].insert (make_pair(proj[ASets[i][j]][i][j], 1));

          // apply the rest of players strats
          for (int k=1; k<n;k++){
            Pr[k].multiply (Pr[k-1], projS[ASets[i][j]][Po[i][j][k]],proj[ASets[i][j]][i][j].size()  ,projF[ASets[i][j]] );
          }
	  pays[ASets[i][j]].insert(Pr[n-1].begin(), Pr[n-1].end());
        }
        doneASets.insert(as);
      }
    }

    stripComment(in);
    //read in payoffs
    for(i=0;i<S;i++){
      if(in.eof()||in.bad()) {
	cout << "Error in game file: not enough payoffs.\n";
	return 0;
      }
      stripComment(in);
      int t;
      in>>t;
      if(!in.good()){
        cout<< "Error reading the integer type of the utility function for action node "<<i<<endl;
        return 0;
      }
      switch (t){
        case COMPLETE:
	    AGG::makeCOMPLETEpayoff(in,pays[i]);
	    break;
	case MAPPING:
	    AGG::makeMAPPINGpayoff(in,pays[i],neighb[i].size());
	    break;
	case ADDITIVE:
        default:
	    cerr<<"Unknown payoff type "<<t 
			<<endl;
	    exit(1);
      }
      
    }
    AGG *r=NULL;
    r=new AGG(n,size,S,P,ASets,neighb,projTypes,projS,proj,projF,Po,Pr,pays);
    if (!r)cout<<"Failed to allocate memory for new AGG";
    delete [] size;
    return r;
  } else {
    cout << "Bad game file.\n";
    exit(1);
  }
}


AGG *AGG::makeRandomAGG(int n, int* actions, int S, int P, 
vector<vector<int> >& ASets, vector<vector<int> >& neighb,
vector<projtype>& projTypes, int seed, bool int_payoffs, int int_factor){
    int i,j;
#if HAVE_SRAND48
    srand48(seed);
#else
    srand(seed);
#endif  // HAVE_SRAND48
    vector<vector<aggdistrib > > projS;
    vector<vector<vector<config> > > proj;
    setProjections(projS,proj,n,S,P, ASets, neighb,projTypes);

    vector<vector<proj_func*> > projF(S);
    for (i=0;i<S;i++){
	int neighb_size=neighb[i].size();
	for(j=0;j<neighb_size; j++){
	  projtype t=(neighb[i][j]<S)?(new proj_func_SUM):projTypes[neighb[i][j]-S];
	  projF[i].push_back(t );
	}
    }

    vector<vector<vector<int> > > Po(n);
    vector<aggdistrib>  Pr(n);
    vector<aggpayoff> pays(S); //payoffs



    set<vector<int> > doneASets;
    for (i=0;i<n;i++){
      for(j=0;j<actions[i]; j++){
        Po[i].push_back(vector<int>(n) );
        initPorder (Po[i][j], i,n,projS[ASets[i][j]]);
      }
      vector<int> as = ASets[i];
      sort(as.begin(),as.end());
      if (doneASets.count(as)==0){
        for(j=0;j<actions[i];j++){
          // apply i's strategy j
          Pr[0].reset();
          Pr[0].insert (make_pair(proj[ASets[i][j]][i][j], 1));

          // apply the rest of players strats
          for (int k=1; k<n;k++){
            Pr[k].multiply (Pr[k-1], projS[ASets[i][j]][Po[i][j][k]],proj[ASets[i][j]][i][j].size()  ,projF[ASets[i][j]] );
          }
          pays[ASets[i][j]].insert(Pr[n-1].begin(), Pr[n-1].end());
        }
        doneASets.insert(as);
      }
    }


    //read in payoffs
    int numPayoffs=0;
    for(i=0;i<S;i++){
	    pays[i].in_order( inputRand(int_payoffs,int_factor) );
	    numPayoffs += pays[i].size();
    }
    cout << "Creating an AGG with "<<numPayoffs <<" payoff values"<<endl;
    AGG *r= new AGG(n,actions,S,P,ASets,neighb,projTypes,projS,proj,projF,Po,Pr,pays);
    
    return r;
 
}



void
AGG::setProjections(vector<vector<aggdistrib > >& projS, 
  vector<vector<vector<config> > >& proj, int N,int S,int P,
  vector<vector<int> >& AS, vector<vector<int> >& neighb, vector<projtype>& projTypes)
{

  int Node, i,j,k,numNei,actions;

  vector<multiset<int> > an(P); //set of ancestors for P nodes
  vector<int> path;
  for (i=0;i<P;i++){
    path.clear();
    getAn(an[i],neighb,projTypes,S,S+i,path);
  } 
    

  projS.clear();
  proj.clear();

  for (Node=0;Node<S;++Node){//for each action node
    projS.push_back(vector<aggdistrib >(N));
    proj.push_back(vector<vector<config> >(N));
    numNei=neighb[Node].size();

    for (i=0;i<N;i++){//for each player

      actions =AS[i].size();
      for (j=0;j<actions;j++){  // for each action in S_i
	proj[Node][i].push_back( config(numNei) );
	for(k=0;k<numNei;k++){  //foreach neighbor of Node
	  //get i's action j's contribution to the count of node k
	  proj[Node][i][j][k]=0;
	  if (AS[i][j]==neighb[Node][k]){
		proj[Node][i][j][k]=1;
		//break;
	  }
	  else if (neighb[Node][k]>=S ){
		proj_func *f=projTypes[neighb[Node][k]-S];
		assert(f);
		pair<multiset<int>::iterator,multiset<int>::iterator> \
		  p=an[neighb[Node][k]-S].equal_range(AS[i][j]); 
		multiset<int> blah(p.first, p.second);
		proj[Node][i][j][k] = (*f) (blah);

	  }
	} //end for(k..

	//insert player i's action j's contribution to projS
	projS[Node][i].insert(make_pair(proj[Node][i][j], 1));
      }//end for(j..
    }//end for(i..
  }//end for(Node..
}

void
AGG::getAn(multiset<int>& dest, vector<vector<int> >& neighb, vector<projtype>& projTypes,
	int S,int Node, vector<int>& path)
{
  //get ancestors
  if (Node<S) {
	dest.insert(Node);
	return;
  }
  //cycle check
  for (vector<int>::iterator p=path.begin();p!=path.end();++p){
    if (Node == (*p)) {
	cout<<"ERROR: cycle of projected nodes at "<<Node  <<endl;
	cout <<"Path: (Size "<<path.size() <<")"<<endl;
	copy(path.begin(),path.end(),ostream_iterator<int>(cout, " "));
	cout<<endl;
	exit(1);
    }
  }

  int numNei=neighb[Node].size();
  path.push_back(Node);
  for (int i=0; i<numNei;++i){
    //check consistency of proj. signatures
    if(neighb[Node][i]>=S && *(projTypes[neighb[Node][i]-S])!= *(projTypes[Node-S])){
	cout<<"ERROR: projection type mismatch: Node "<< Node 
	   <<" and its neighbor "<<neighb[Node][i]<<endl;
	exit(1);
    }
    getAn(dest, neighb,projTypes, S, neighb[Node][i],path);
  }
  path.pop_back();

}

void
AGG::initPorder(vector<int>& Po,
//vector<aggdistrib>& P,
    int i,  int N, vector<aggdistrib>& projS)
//config & proj,
//vector<proj_func*>& projF
{
  vector<pair<int,int> > order;
  int k;
  for (k=0;k<N;k++) if (k!=i){
    order.push_back (make_pair( projS[k].size() , k) );
  }

  sort (order.begin(),order.end() );
  vector<int>::iterator p = Po.begin();
  (*p) = i;
  ++p;
  transform(order.begin(),order.end(), p, select2nd );

}



//compute the induced distribution 
void
AGG::computeP(int player, int act, int player2,int act2)
{
  //apply player's strat
  Pr[0].reset();
  Pr[0].insert(make_pair(projection[actionSets[player][act]][player][act], 1.0) );

  int numNei = neighbors[actionSets[player][act]].size();
  //apply others' strat
  for (int k=1; k<numPlayers;k++){
    Pr[k].reset();
    if (Porder[player][act][k]==player2){ 
      if (act2==-1){
	//Pr[k].swap(Pr[k-1]);
	Pr[k]=Pr[k-1];
      } else {
	//apply player2's pure strat
	aggdistrib temp;
	temp.insert(make_pair(projection[actionSets[player][act]][player2][act2],1.0));
	Pr[k].multiply(Pr[k-1],temp ,numNei, projFunctions[actionSets[player][act]]);
      }
    } else {
      Pr[k].multiply (Pr[k-1], 
	projectedStrat[actionSets[player][act]][Porder[player][act][k]],
	numNei  ,projFunctions[actionSets[player][act]] ); 
    }
  }
    
}

void AGG::doProjection(int Node, AggNumber* s)
{
  for (int i=0;i<numPlayers;i++){
    doProjection(Node,i, &(s[firstAction(i)]));
  }
}

void AGG::doProjection(int Node, int i, AggNumber* s)
{
  projectedStrat[Node][i].reset();
  for (int j=0;j<actions[i];j++)if(s[j]>(AggNumber)0.0){
    projectedStrat[Node][i]+= make_pair(projection[Node][i][j],
              s[j]);
  }
}

AggNumber AGG::getPurePayoff(int player, int *s){
  assert(player>=0 && player < numPlayers);
  int Node = actionSets[player][s[player]]; 
  int keylen = neighbors[Node].size();
  config pureprofile (projection[Node][0][s[0]]);
  for (int i=1; i<numPlayers; i++){
    for (int j=0; j<keylen; j++){
      pureprofile[j]= 
        (*projFunctions[Node][j]) (pureprofile[j],projection[Node][i][s[i]][j] );
    }
  }
  aggpayoff::iterator p= payoffs[Node].find(pureprofile);
  if ( p == payoffs[Node].end() ){
    cout<<"AGG::getPurePayoff ERROR: unable to find the following configuration"
        <<endl;
    cout <<"[";
    copy(pureprofile.begin(),pureprofile.end(),ostream_iterator<int>(cout, " "));
    cout<<"]" <<endl;
    cout<< "\tin payoffs of action node #"<<Node<<endl;
    exit(1);
  }
  return p->second;
}

AggNumber AGG::getMixedPayoff(int player, StrategyProfile &s){
  AggNumber result=0.0;
  assert(player>=0 && player < numPlayers);
  for (int act=0;act <actions[player];++act)if (s[act+firstAction(player)]>(AggNumber)0.0){
	result+= s[act+firstAction(player)]* getV(player, act, s);
  }
  return result;
}

void AGG::getPayoffVector(AggNumberVector &dest, int player,const StrategyProfile &s){
    assert(player>=0 && player < numPlayers);
    for (int act=0;act<actions[player]; ++act){
	dest[act]=getV(player,act,s);
    }
}

AggNumber AGG::getV(int player, int act,const StrategyProfile &s){
    //project s to the projectedStrat
    doProjection(actionSets.at(player).at(act), s);
    computeP(player, act);
    return Pr[numPlayers-1].inner_prod(payoffs[actionSets[player][act]]);
}

AggNumber AGG::getJ(int player1, int act1, int player2,int act2,StrategyProfile &s)
{
    doProjection(actionSets[player1][act1],s);
    computeP(player1,act1,player2,act2);
    return Pr[numPlayers-1].inner_prod(payoffs[actionSets[player1][act1]]);
}

//getSymMixedPayoff: compute expected payoff under a symmetric mixed strat,
//  for a symmetric game.
// parameter: s is the mixed strategy of one player. It is a vector of 
// probabilities, indexed by the action node.

AggNumber AGG::getSymMixedPayoff( StrategyProfile &s){
  AggNumber result=0;
  if (! isSymmetric() ) {
    cerr<< "AGG::getSymMixedPayoff: the game is not symmetric!"<<endl;
    exit(1);
  }


  for (int node=0; node<numActionNodes; ++node)if(s[node]>(AggNumber)0.0){
    result+= s[node]* getSymMixedPayoff(node,s);
  }
  return result;
}
void AGG::getSymPayoffVector(AggNumberVector& dest, StrategyProfile &s){
  if (! isSymmetric() ) {
    cerr<< "AGG::getSymMixedPayoff: the game is not symmetric!"<<endl;
    exit(1);
  }

  //check the pureness
  //bool pure=true;
  //for (int i=0;i<numActionNodes;++i){
  //  if(!(s[ i ] == (AggNumber)0.0 || isPure[i])){
  //    pure=false;
  //    break;
  //  }
  //}
  //if(!pure){
  //  StrategyProfile fulls(getNumActions());
  //  for(int i=0;i<getNumPlayers();++i){
  //    for(int j=0;j<numActionNodes;++j)
  //      fulls[j+firstAction(i)]=s[j];
  //  }
  //  getPayoffVector(dest, 0, fulls);
  //  return;
  //}
  for (int act=0;act<numActionNodes; ++act){
          dest[act]=getSymMixedPayoff(act,s);
  }
}
AggNumber AGG::getSymMixedPayoff(int node, StrategyProfile &s)
{
    int numNei = neighbors[node].size();

    if(!isPure[node]){ // then compute EU using trie_map::power()
      doProjection(node,0,s);
      assert(numPlayers>1);
      //aggdistrib *dest;
      //projectedStrat[node][0].power(numPlayers-1, dest, Pr, numNei,projFunctions[node]);
      aggdistrib &dest = Pr[numPlayers-1];
      projectedStrat[node][0].power(numPlayers-1, dest, Pr[numPlayers-2],numNei,projFunctions[node]);
      return dest.inner_prod(projection[node][0][node], numNei, projFunctions[node], payoffs[node]);
    }

    AggNumber V = 0.0;
    vector<int> support;
    AggNumber null_prob=1;
    //do projection  & get support
    int self = -1;
    for (int i=0; i<numNei; ++i){
	if (neighbors[node][i] == node) self=i;
	if (s[neighbors[node][i]]>(AggNumber)0) {
	  support.push_back(i);
	  null_prob -= s[neighbors[node][i]];
	}
    }
    if (numNei < numActionNodes && null_prob>(AggNumber)0) 
	support.push_back(-1);


    //gray code
    GrayComposition gc (numPlayers-1, support.size() );

    AggNumber prob = pow((support.at(0)>=0)?s[neighbors[node][support[0]]]:null_prob,
		numPlayers-1);

    while (1){
      const vector<int>& comp = gc.get();
      config c(numNei, 0);
      for (size_t j=0;j<support.size(); ++j) { 
	if(support[j]!=-1)
	  c[support[j]] = comp[j];
      }
      //add current player's action
      if (self!=-1) c[self]++;
      V+= prob *  payoffs[node].find(c)->second ;

      //get next composition
      gc.incr();
      if (gc.eof() ) break;
      //update prob
      AggNumber i_prob = (support.at(gc.i)!=-1)?s[neighbors[node][support[gc.i]]]:null_prob;
      AggNumber d_prob =(support.at(gc.d)!=-1)?s[neighbors[node][support[gc.d]]]:null_prob;
      assert(i_prob>(AggNumber)0  && d_prob>(AggNumber)0 );
      prob *= ((AggNumber)(gc.get().at(gc.d)+1)) * i_prob /(AggNumber)(gc.get().at(gc.i)) /d_prob; 
      
    }//end while
    
    return V;
}

//get the prob distribution over configurations of neighbourhood of node.
//plClass: the index for the player class
//s: mixed strat for that player class

void AGG::getSymConfigProb(int plClass, StrategyProfile &s, int ownPlClass, int act, aggdistrib &dest,int plClass2,int act2){
    int node = uniqueActionSets.at(ownPlClass).at(act);
    int numPl = playerClasses.at(plClass).size();
    assert(numPl>0);
    //int numA = uniqueActionSets[plClass].size();


    if (plClass==ownPlClass) numPl--;
    if (plClass==plClass2) numPl--;
    dest.reset();
    int numNei = neighbors.at(node).size();


    if(!isPure[node]){
      int player = playerClasses[plClass].at(0);
      projectedStrat[node][player].reset();
      if(numPl>0){
        for (int j=0;j<actions[player];j++)if(s[j]>(AggNumber)0.0){
          projectedStrat[node][player]+= make_pair(projection[node][player][j], s[j]);
        }
        projectedStrat[node][player].power(numPl, dest,Pr[0],numNei, projFunctions[node]);
      }
      if(plClass==ownPlClass){
        aggdistrib temp;
        temp.insert(make_pair(projection[node][player].at(act),1.0));
        if(dest.size()>0){
          dest.multiply(temp, numNei, projFunctions[node]);
        }else{
          //dest.swap(temp);
          dest=temp;
        }
      }
      if(plClass==plClass2){
        aggdistrib temp;
        temp.insert(make_pair(projection[node][player].at(act2),1.0));
        if(dest.size()>0){
          dest.multiply(temp, numNei, projFunctions[node]);
        }else{
          //dest.swap(temp);
          dest=temp;
        }
      }
      return;
    }





    //AggNumber V = 0.0;
    vector<int> support;
    AggNumber null_prob=1;
    //do projection  & get support
    int self = -1;   //index of self in the neighbor list
    int ind2=-1;     //index of act2 in the neighbor list
    int p=playerClasses[plClass][0];
    for (int i=0; i<numNei; ++i){
	if (neighbors[node][i] == node) self=i;
	if (plClass2>=0 && neighbors[node][i] == uniqueActionSets.at(plClass2).at(act2)) ind2=i;

	int a=node2Action.at(neighbors[node][i]).at(p);
	if (a>=0&&s[a]>(AggNumber)0) {
	  support.push_back(i);
	  null_prob -= s[a];
	}
    }
    if (null_prob>(AggNumber)0)
	support.push_back(-1);


    //gray code
    GrayComposition gc (numPl, support.size() );

    AggNumber prob0=(support.at(0)>=0)?s[node2Action[neighbors[node].at(support[0])][p]]:null_prob;
    AggNumber prob = pow(prob0,numPl);

    while (1){
      const vector<int>& comp = gc.get();
      config c(numNei, 0);
      for (size_t j=0;j<support.size(); ++j) { 
	if(support[j]!=-1)
	  c[support[j]] = comp[j];
      }
      //add current player's action
      if (plClass == ownPlClass && self!=-1) c[self]++;
      
      if(plClass==plClass2 && ind2!=-1)c[ind2]++;

      //V+= prob *  payoffs[node].find(c)->second ;
      dest.insert(make_pair(c, prob));

      //get next composition
      gc.incr();
      if (gc.eof() ) break;
      //update prob
      AggNumber i_prob = (support.at(gc.i)!=-1)?s[node2Action[neighbors[node][support[gc.i]]][p]]:null_prob;
      AggNumber d_prob =(support.at(gc.d)!=-1)?s[node2Action[neighbors[node][support[gc.d]]][p]]:null_prob;
      assert(i_prob>(AggNumber)0  && d_prob>(AggNumber)0 );
      prob *= ((AggNumber)(gc.get().at(gc.d)+1)) * i_prob /(AggNumber)(gc.get().at(gc.i)) /d_prob; 
      
    }//end while

  
}

AggNumber AGG::getKSymMixedPayoff( int playerClass,vector<StrategyProfile> &s){
  AggNumber result=0.0;

  for(int act=0;act<(int)uniqueActionSets[playerClass].size();act++)if(s[playerClass][act]>(AggNumber)0.0){

      result += s[playerClass][act] *getKSymMixedPayoff(playerClass, act,s);
  }
  return result;
}
AggNumber AGG::getKSymMixedPayoff( int playerClass,StrategyProfile &s){
  AggNumber result=0.0;

  for(int act=0;act<(int)uniqueActionSets[playerClass].size();act++)if(s[firstKSymAction(playerClass)+act]>(AggNumber)0.0){

      result += s[firstKSymAction(playerClass)+act] *getKSymMixedPayoff(s,playerClass, act);
  }
  return result;
}
void AGG::getKSymPayoffVector(AggNumberVector& dest,int playerClass, StrategyProfile &s){
  for (size_t act=0;act<uniqueActionSets[playerClass].size();++act){
    dest[act]=getKSymMixedPayoff(s,playerClass,act);
  }
}
AggNumber AGG::getKSymMixedPayoff(int playerClass, int act, vector<StrategyProfile> &s){
      
      int numPC = playerClasses.size();
      
      int numNei = neighbors[uniqueActionSets[playerClass][act]].size();

      static aggdistrib d,temp;
      d.reset();
      temp.reset();
      getSymConfigProb(0, s[0], playerClass, act, d);
      for(int pc=1;pc<numPC;pc++){
	  getSymConfigProb(pc, s[pc], playerClass, act, temp);
	  d.multiply(temp, numNei, projFunctions[uniqueActionSets[playerClass][act]]);
      }
      return d.inner_prod(payoffs[uniqueActionSets[playerClass][act]]);
}

AggNumber AGG::getKSymMixedPayoff(const StrategyProfile &s,int pClass1,int act1,int pClass2,int act2){
  int numPC=playerClasses.size();
  int numNei=neighbors[uniqueActionSets[pClass1][act1]].size();
  static aggdistrib d,temp;
  if (pClass2>=0 && pClass1==pClass2 && playerClasses.at(pClass1).size()<=1){
    return 0;
  }
  d.reset();
  temp.reset();
  StrategyProfile s0(getNumKSymActions(0), 0.0);
  //if (0==pClass2) s0[act2]=1;
  //else
  for (int a=firstKSymAction(0);a<lastKSymAction(0);++a)s0[a]=s[a];
  getSymConfigProb(0,s0,pClass1,act1,d,pClass2,act2);
  for (int pc=1;pc<numPC;pc++){
    StrategyProfile ss(getNumKSymActions(pc), 0.0);
    //if (pc==pClass2)ss[act2]=1;
    //else
    for (int a=0;a<getNumKSymActions(pc);++a)ss[a]=s[a+firstKSymAction(pc)];
    getSymConfigProb(pc,ss,pClass1,act1,temp,pClass2,act2);
    d.multiply(temp,numNei,projFunctions[uniqueActionSets[pClass1][act1]]);
  }
  return d.inner_prod(payoffs[uniqueActionSets[pClass1][act1]]);
}



void AGG::makeMAPPINGpayoff(std::istream& in, aggpayoff& pay, int numNei){
    int num;
    char c;
    AggNumber u;
    aggpayoff temp;
    //temp.swap(pay);
    temp=pay;
    pay.clear();

    stripComment(in);
    in>>num;
    if(!in.good()){
      cerr<<"Error reading the integer number of configuration-value pairs"<<endl;
      exit(1);
    }

    while (num--){
        in>>ws;
        if(in.eof()||in.bad()){
          cerr<<"ERROR: bad input"<<endl;
          exit(1);
        }

	c = in.get();
	assert(in.good());
	if (c != AGG::LBRACKET){
	    cerr<< "ERROR: "<<AGG::LBRACKET <<" expected. Instead, got "<<c <<endl;
	    cerr<<"The rest of the line is: ";
	    stringbuf discard(ios_base::out);
	    in.get(discard);
	    cerr<<discard.str()<<endl;
	    exit(1);
	}
	vector<int> key;

	for(int j=0;j<numNei; ++j){
	    int cnt;
	    in>>cnt;
	    if(!in.good()){
	      cerr<<"ERROR trying to read element #"<<j<<" of the configuraiton"<<endl;
	      exit(1);
	    }
	    key.push_back(cnt);
	}


	in>>ws;
	c=in.get(); //get right bracket
	assert (in.good());

	if (c!=AGG::RBRACKET){
	    cerr<< "ERROR: "<<AGG::RBRACKET <<" expected. Instead, got "<<c<<endl;
	    cerr<< "current configuration: ";
	    copy(key.begin(),key.end(), ostream_iterator<int>(cerr, " "));
	    cerr<<endl;
	    exit(1);
	}
	
	in>>u;  //get payoff
	if(!in.good()){
	  cerr<<"Error trying to read the utility value for configuration ";
	  copy(key.begin(),key.end(), ostream_iterator<int>(cerr, " "));
	  cerr<<endl;
	  exit(1);
	}


	//insert
	pair<trie_map<AggNumber>::iterator, bool> r = pay.insert(make_pair(key,u));
	if (!r.second){
	    cerr<<"WARNING: overwriting utility at [";
	    copy(key.begin(),key.end(), ostream_iterator<int>(cerr, " "));
	    cerr<<"]"<<endl;
	    cerr<<"previous value: "<<r.first->second<<" new value: "<<u<<endl;

	    r.first->second = u;
	}

    }
    //check
    for(trie_map<AggNumber>::iterator it = temp.begin(); it!=temp.end(); ++it){

	//pair<trie_map<AggNumber>::iterator,bool> res = pay.insert( *it);
	
	if (pay.count(it->first)==0){
	    cerr<<"ERROR: utility at [";
	    copy(it->first.begin(),it->first.end(), ostream_iterator<int>(cerr, " "));
	    cerr<<"] not specified."<<endl;
	    cerr<<"The set of configurations (requiring utility to be specified) are: "<<endl;
	    cerr<<temp;
	    cerr<<endl;
	    cerr<<"The set of configurations to which utility has been specified are: "<<endl;
	    cerr<<pay;
	    cerr<<endl;
	    exit(1);
	}
    }


}

AggNumber AGG::getMaxPayoff(){
  static bool done=false;
  static AggNumber result=0;
  if (done)return result;
  assert(numActionNodes>0);
  result=payoffs[0].begin()->second;
  for (int i=1;i<numActionNodes;i++)
    for (aggpayoff::iterator it=payoffs[i].begin();it!=payoffs[i].end();++it)
      result=max(result, it->second);

  done=true;
  return result;
}
AggNumber AGG::getMinPayoff(){
  static bool done=false;
  static AggNumber result=0;
  if (done)return result;
  assert(numActionNodes>0);
  result=payoffs[0].begin()->second;
  for (int i=1;i<numActionNodes;i++)
      for (aggpayoff::iterator it=payoffs[i].begin();it!=payoffs[i].end();++it)
        result=min(result, it->second);
  done=true;
  return result;
}


}  // end namespace Gambit::agg

}  // end namespace Gambit
