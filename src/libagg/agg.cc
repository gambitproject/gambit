

using namespace std;

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <ext/functional>
#include "GrayComposition.h"
#include "agg.h"


using __gnu_cxx::select2nd;


agg::agg(int numPlayers,int* _actions, int numANodes, int _numPNodes, 
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
  //use swap instead of copy; faster but destroys the input parameters.
  //payoffs.swap(_payoffs);
  //Pr.swap(P);

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
agg::agg(const agg& other, bool completeGraph)
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
    numActionNodes=other.numActionNodes;

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

void agg::stripComment(istream& in){
  in>>ws;
  char c =in.peek();
  stringbuf discard(ios_base::out);
  if(c== agg::COMMENT_CHAR){
    in.get (discard);
#ifdef AGGDEBUG
    cerr<<discard.str()<<endl;
#endif
    stripComment(in);
  }
}


agg* agg::makeAGG(char* filename){
  ifstream in(filename);
  return agg::makeAGG(in);
}
agg* agg::makeAGG(istream &in){
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
	    agg::makeCOMPLETEpayoff(in,pays[i]);
	    break;
	case MAPPING:
	    agg::makeMAPPINGpayoff(in,pays[i],neighb[i].size());
	    break;
	case ADDITIVE:
        default:
	    cerr<<"Unknown payoff type "<<t 
			<<endl;
	    exit(1);
      }
      
    }
    agg* r=NULL;
    r=new agg(n,size,S,P,ASets,neighb,projTypes,projS,proj,projF,Po,Pr,pays);
    if (!r)cout<<"Failed to allocate memory for new AGG";
    delete [] size;
    return r;
  } else {
    cout << "Bad game file.\n";
    exit(1);
  }
}


agg* agg::makeRandomAGG(int n, int* actions, int S, int P, 
vector<vector<int> >& ASets, vector<vector<int> >& neighb,
vector<projtype>& projTypes, int seed, bool int_payoffs, int int_factor){
    int i,j;
    srand48(seed);
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
    agg* r= new agg(n,actions,S,P,ASets,neighb,projTypes,projS,proj,projF,Po,Pr,pays);
    
    return r;
 
}



void
agg::setProjections(vector<vector<aggdistrib > >& projS, 
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
agg::getAn(multiset<int>& dest, vector<vector<int> >& neighb, vector<projtype>& projTypes,
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
agg::initPorder(vector<int>& Po,
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
  transform(order.begin(),order.end(), p, select2nd<pair<int,int> >() );

}



//compute the induced distribution 
void
agg::computeP(int player, int act, int player2,int act2)
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
	Pr[k].swap(Pr[k-1]);
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

#ifdef USE_CVECTOR
void agg:: computePartialP_PureNode(int player1,int act1, vector<int>& tasks){
    int i,j,Node = actionSets[player1][act1];
    int numNei = neighbors[Node].size();
  
    assert(isPure[Node]||tasks.size()==0); 
    vector<Number> strat (numNei);
    config    a(numNei,0);
    //compute the full distrib
    computeP (player1,act1);

    //store the full distrib in Pr[player1]
    Pr[player1].swap(Pr[numPlayers-1]);
    for(i=0;i<(int)tasks.size();i++){
      assert(tasks[i]!=player1);
      aggdistrib& P = Pr[tasks[i]];
      //P.clear();  // to get ready for division, we need clear()
      P=Pr[player1];

      bool NullOnly =true;
      for(j=0;j<numNei;++j){
	a[j]++;
	aggdistrib::iterator pp =projectedStrat[Node][tasks[i]].find(a);
	if (pp== projectedStrat[Node][tasks[i]].end()) {
	    strat[j]=0;
	}
	else {
	    strat[j]= pp->second;
	    if (strat[j] > (Number)0) NullOnly=false;
	}

	a[j]--;
      }
#ifdef AGGDEBUG
      cout<<"dividing "<<endl;
      P.print_in_order();
      cout<<endl<<"by [";
      copy(strat.begin(),strat.end(),ostream_iterator<Number>(cout," ") );
      cout<<"]\n";
#endif
      if (!NullOnly) P/= strat;
#ifdef AGGDEBUG
      cout<<"result is: "<<endl;
      P.print_in_order();
#endif
    }//end for(i

}
#endif

#ifdef USE_CVECTOR
void agg::computePartialP(int player1, int act1, vector<int>& tasks,vector<int>& nontasks){
//TODO
}
#endif

#ifdef USE_CVECTOR
void agg::computePartialP_bisect(int player1,int act1,
    vector<int>::iterator start,vector<int>::iterator endp,
    aggdistrib& temp){
  assert (endp-start>0);
#ifdef AGGDEBUG
  cout<<"calling computePartialP_bisect with player1="<<player1
    <<", act1="<<act1<<" *start="<<*start<<" *(endp-1)="<<*(endp-1)
    <<", (endp-start)="<< endp-start <<endl;
#endif
  if(endp-start==1){Pr[*start].reset();return;}
  int Node = actionSets[player1][act1];
  int numNei=neighbors[Node].size();

  int player2;
  vector<int>::iterator ptr, mid = start + (endp-start)/2;
#ifdef AGGDEBUG
  cout<< "*mid="<<*mid<<" mid-start="<<mid-start<<" endp-mid="
    <<endp-mid <<endl;
#endif
  computePartialP_bisect(player1,act1,start,mid,temp);
  computePartialP_bisect(player1,act1,mid,endp,temp);
  
  
  temp.reset();
  temp = projectedStrat[Node][*start];
  if (mid-start>1) temp.multiply(Pr[*start],numNei,projFunctions[Node]);
  
  if (mid-start==1) {
    assert(Pr[*start].empty());
    Pr[*start]= projectedStrat[Node][*mid];
    if(endp-mid>1)Pr[*start].multiply(Pr[*mid],numNei,projFunctions[Node]);
  }
  else for (ptr=start; ptr!=mid; ++ptr){
    player2= *ptr;
    Pr[player2].multiply(projectedStrat[Node][*mid],numNei,projFunctions[Node] );
    if(endp-mid>1)Pr[player2].multiply(Pr[*mid],numNei,projFunctions[Node]);
  }

  if(endp-mid==1){
    assert(Pr[*mid].empty());
    Pr[*mid]=temp;
  }
  else for (ptr=mid;ptr!=endp;++ptr){
    player2=*ptr;
    Pr[player2].multiply(temp,numNei, projFunctions[Node]);
    
  }
  
}
#endif


void agg:: doProjection(int Node,const StrategyProfile& s)
{
  for (int i=0;i<numPlayers;i++){
    doProjection(Node,i,s);
  }
}

inline void agg:: doProjection(int Node, int i, const StrategyProfile& s)
{
  projectedStrat[Node][i].reset();
  for (int j=0;j<actions[i];j++)if(s[j+firstAction(i)]>(Number)0.0){
    projectedStrat[Node][i]+= make_pair(projection[Node][i][j],
              s[j+firstAction(i)]);
  }
}
Number agg::getPurePayoff(int player, int *s){
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
    cout<<"agg::getPurePayoff ERROR: unable to find the following configuration"
        <<endl;
    cout <<"[";
    copy(pureprofile.begin(),pureprofile.end(),ostream_iterator<int>(cout, " "));
    cout<<"]" <<endl;
    cout<< "\tin payoffs of action node #"<<Node<<endl;
    exit(1);
  }
  return p->second;
}

Number agg::getMixedPayoff(int player, StrategyProfile &s){
  Number result=0.0;
  assert(player>=0 && player < numPlayers);
  for (int act=0;act <actions[player];++act)if (s[act+firstAction(player)]>(Number)0.0){
	result+= s[act+firstAction(player)]* getV(player, act, s);
  }
  return result;
}

void agg::getPayoffVector(NumberVector &dest, int player,const StrategyProfile &s){
    assert(player>=0 && player < numPlayers);
    for (int act=0;act<actions[player]; ++act){
	dest[act]=getV(player,act,s);
    }
}

Number agg::getV(int player, int act,const StrategyProfile &s){
    //project s to the projectedStrat
    doProjection(actionSets.at(player).at(act), s);
    computeP(player, act);
    return Pr[numPlayers-1].inner_prod(payoffs[actionSets[player][act]]);
}

Number agg::getJ(int player1, int act1, int player2,int act2,StrategyProfile &s)
{
    doProjection(actionSets[player1][act1],s);
    computeP(player1,act1,player2,act2);
    return Pr[numPlayers-1].inner_prod(payoffs[actionSets[player1][act1]]);
}

#ifdef USE_CVECTOR
void agg::payoffMatrix(cmatrix &dest, cvector &s, Number fuzz){
  //compute jacobian
  //s: mixed strat

#ifdef AGGDEBUG
  cout<<"calling payoffMatrix with stratety s="<<endl
      <<s<<endl;
#endif
  Number fuzzcount;
  int rown, coln, rowi, coli,act1,act2,currNode,numNei;
  static vector<int>::iterator p;
  static vector<int> tasks,spares,nontasks;
  tasks.reserve(numPlayers); 
  spares.reserve(numPlayers); 
  nontasks.reserve(numPlayers);
  cache.reset();

  //do projection
  for(int Node=0; Node< numActionNodes; Node++)
    doProjection(Node,s);

  //deal with the diagonal
  for (rown=0; rown<numPlayers; ++rown){
	  fuzzcount=fuzz;
	  for (rowi=firstAction(rown); rowi<lastAction(rown);rowi++){
	    for (coli=firstAction(rown);coli<lastAction(rown);coli++){
		dest[rowi][coli]=fuzzcount;
		fuzzcount+=fuzz;
	    }
	  }
  }
  for(rown=0;rown<numPlayers; ++rown){   //rown: the row player
	for(act1=0;act1<actions[rown];act1++){  //act1: player rown's action

	    currNode =actionSets[rown][act1];
	    numNei= neighbors[currNode].size();
#ifdef AGGDEBUG
            cout<<"for player "<<rown<<", action "<<act1
                <<", action node "<<currNode<<endl;
	    cout<< "cache is: "<<endl<<cache<<endl; 
#endif
	    tasks.clear();  //for these col players, we need to compute the distribution induced by their complements. input of the bisection alg
	    spares.clear(); //these col players have only one projected action
	    nontasks.clear(); //complement of tasks. includes spares, and completely cached col players

	    vector<int> key (numNei+3);
	    key[numNei]=rown;
	    key[numNei+1]=act1;
	    
	    //first, populate tasks, spares and nontasks
	    for(coln=0;coln<numPlayers;++coln)if(rown!=coln){//coln: col player
              
              key[numNei+2]=coln;
              bool allCached=true;
	      for (act2=0;act2<actions[coln];++act2){
       
	        copy(projection[currNode][coln][act2].begin(),projection[currNode][coln][act2].end(), key.begin());
#ifdef AGGDEBUG
                cout<<"for player2="<<coln<<" act2="<<act2<<endl;
                cout<<"checking cache for: [";
                copy(key.begin(),key.end(),ostream_iterator<int>(cout," ") );
                cout<<"]\n";
#endif
	        aggdistrib::iterator r= cache.findExact(key);
	        if (r!=cache.end()){
	          dest[act1+firstAction(rown)][act2+firstAction(coln)]=r->second;
	        }
	        else{
	          allCached=false;
	        }
	      }
	      if ( allCached){//if all coln's actions are already cached:
	        nontasks.push_back(coln);
       	      }  
              else {
	        if(fullProjectedStrat[currNode][coln].size()==1){//if coln has only one projected action
		  spares.push_back (coln);
		  nontasks.push_back(coln);
		}  
	        else
		  tasks.push_back(coln);
	      }	
	    }
#ifdef AGGDEBUG
            cout<<"spares are: [";
            copy(spares.begin(),spares.end(),ostream_iterator<int>(cout," "));
            cout<<"]\ntasks are :[";
            copy(tasks.begin(),tasks.end(),ostream_iterator<int>(cout," "));
            cout<<"]\n";
#endif


	    //compute partial prob distributions
	    if (tasks.size()==0 && spares.size()==0) continue; //nothing to be done for this row

	    if(isPure[currNode]||tasks.size()==0){
	      computePartialP_PureNode(rown, act1,tasks);
	    }else{//do bisection 
	      computePartialP_bisect(rown,act1,tasks.begin(),tasks.end(),Pr[rown]);
#ifdef AGGDEBUG
              cout<<"after calling computePartialP_bisect:"<<endl;
              for (int tt=0;tt<tasks.size();tt++){
                cout<<"for player "<<tasks[tt]<<endl;
                cout<<Pr[tasks[tt]]<<endl;
              }
#endif
	      //now apply rown's action (act1), and the strategies of 
	      //players in nontasks
	      Pr[rown].reset();
	      Pr[rown].insert( 
		make_pair(projection[currNode][rown][act1],1.0));
	      for(p=nontasks.begin();p!=nontasks.end();++p)
		Pr[rown].multiply(projectedStrat[currNode][*p],numNei, projFunctions[currNode]);
#ifdef AGGDEBUG
              cout<<"the polynomial product of strats of player "
                  <<rown<< " and players in the vector nontasks is:"
                  <<endl;
              cout<<Pr[rown]<<endl;
#endif
	      if (tasks.size()==1){
		Pr[tasks[0]]=Pr[rown];
	      }
	      else {
                for(p=tasks.begin();p!=tasks.end();++p){
		  if(Pr[*p].size()==0){
		    cerr<<"AGG::payoffMatrix() ERROR for rown="
		        <<rown<<" act1="<<act1<<" *p=" <<*p
		        <<": the distribution should not be empty!"<<endl;
#ifdef AGGDEBUG
      		    cerr<<"strategy profile is: "<<s<<endl;    
#endif
      		          
		  }
		  Pr[*p].multiply(
		    Pr[rown],numNei,projFunctions[currNode]);
	        }//end for(p=tasks.begin...
	      } 
 
	      //if spares not empty, we need to compute nondisturbed payoffs
	      //which requires the distrib induced by everyone (except rown).
	      //we store this distrib in Pr[rown][act1][rown]
	      if (spares.size()>0){
		//assert(tasks.size()>0);
		Pr[rown].reset();
		Pr[rown].multiply(
		  Pr[tasks[0]],
		  projectedStrat[currNode][tasks[0]],numNei,projFunctions[currNode]);
	      }
	    } //end else
#ifdef AGGDEBUG
            cout<< "after computing parital distributions, the distributions are"
                <<endl;
            for (int tt = 0;tt<numPlayers;tt++){
              cout<<"for player "<<tt<<endl;
              cout<<Pr[tt];
              cout<<endl;
            }
#endif                
	    //compute entries
	    Number undisturbedPayoff;
	    bool hasUndisturbed=false;

	    if(spares.size()>0){//for players in spares, we compute one undisturbed payoff
	      computeUndisturbedPayoff(undisturbedPayoff,hasUndisturbed,rown,act1, rown);
	      for(p=spares.begin();p!=spares.end();++p)
		for(act2=0;act2<actions[*p];act2++)
		  savePayoff(dest,rown,act1,*p,act2, undisturbedPayoff,cache);

	    }
	    for(p=tasks.begin();p!=tasks.end();++p){
	      for(act2=0;act2<actions[*p];act2++){//act2: col action

		if (projectedStrat[currNode][*p].size()==1  &&
		  projectedStrat[currNode][*p].begin()->first==projection[currNode][*p][act2])
		{
		  computeUndisturbedPayoff(undisturbedPayoff,hasUndisturbed,rown,act1,*p);
		  savePayoff(dest,rown,act1,*p,act2,undisturbedPayoff,cache);
		}
		computePayoff(dest,rown,act1,*p,act2,cache);
	      }//end for(act2
	    }//end for(p
	}//end for(act1
  }//end for(rown
}
#endif

#ifdef USE_CVECTOR
void agg::computeUndisturbedPayoff(Number& undisturbedPayoff,bool& has,int player1,int act1,int player2)
{
  if (has) return;
  int    Node =actionSets[player1][act1];
  int    numNei= neighbors[Node].size();
  if (player2==player1){
    undisturbedPayoff=Pr[player2].inner_prod(payoffs[Node]);
  }else{
    assert(projectedStrat[Node][player2].size()==1);
    undisturbedPayoff=Pr[player2].inner_prod(
			projectedStrat[Node][player2].begin()->first,numNei,projFunctions[Node],payoffs[Node]);
  }
  has=true;
}
void agg::savePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,Number result,
	trie_map<Number>& cache, bool partial ){

  int    Node =actionSets[player1][act1];
  int    numNei= neighbors[Node].size();

  if (!partial){
    pair< vector<int>, Number> pair1(projection[Node][player2][act2],result);
    pair1.first.reserve(numNei+3);
    pair1.first.push_back(player1);
    pair1.first.push_back(act1);
    pair1.first.push_back(player2);
    cache.insert(pair1); 
  }

  if (node2Action[Node][player2]!=-1 &&
     fullProjectedStrat[Node][player1].count(projection[Node][player2][act2]))
  {
    pair<vector<int>,Number> pair2(projection[Node][player2][act2],result);
    pair2.first.reserve(numNei+3);
    pair2.first.push_back(player2);
    pair2.first.push_back(node2Action[Node][player2]);
    pair2.first.push_back(player1);
    cache.insert(pair2);
  }
  dest[act1+firstAction(player1)][act2+firstAction(player2)]=result;
  
}
void agg::computePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,trie_map<Number>& cache){
  int    Node =actionSets[player1][act1];
  int    numNei= neighbors[Node].size();

  pair<vector<int>,Number> insPair( projection[Node][player2][act2],0);
  insPair.first.reserve(numNei+3);
  insPair.first.push_back(player1);
  insPair.first.push_back(act1);
  insPair.first.push_back(player2);
  pair<trie_map<Number>::iterator,bool> r =cache.insert(insPair);
  if (! r.second) {
    dest[act1+firstAction(player1)][act2+firstAction(player2)]=r.first->second;
  }else{
    r.first->second=Pr[player2].inner_prod(
		projection[Node][player2][act2],numNei,projFunctions[Node],payoffs[Node]);
    savePayoff(dest,player1,act1,player2,act2,r.first->second,cache,r.second);
  }
}
#endif

//getSymMixedPayoff: compute expected payoff under a symmetric mixed strat,
//  for a symmetric game.
// parameter: s is the mixed strategy of one player. It is a vector of 
// probabilities, indexed by the action node.

Number agg::getSymMixedPayoff( StrategyProfile &s){
  Number result=0;
  if (! isSymmetric() ) {
    cerr<< "agg::getSymMixedPayoff: the game is not symmetric!"<<endl;
    exit(1);
  }


  for (int node=0; node<numActionNodes; ++node)if(s[node]>(Number)0.0){
    result+= s[node]* getSymMixedPayoff(node,s);
  }
  return result;
}
void agg::getSymPayoffVector(NumberVector& dest, StrategyProfile &s){
  if (! isSymmetric() ) {
    cerr<< "agg::getSymMixedPayoff: the game is not symmetric!"<<endl;
    exit(1);
  }

  //check the pureness
  //bool pure=true;
  //for (int i=0;i<numActionNodes;++i){
  //  if(!(s[ i ] == (Number)0.0 || isPure[i])){
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
Number agg::getSymMixedPayoff(int node, StrategyProfile &s)
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

    Number V = 0.0;
    vector<int> support;
    Number null_prob=1;
    //do projection  & get support
    int self = -1;
    for (int i=0; i<numNei; ++i){
	if (neighbors[node][i] == node) self=i;
	if (s[neighbors[node][i]]>(Number)0) {
	  support.push_back(i);
	  null_prob -= s[neighbors[node][i]];
	}
    }
    if (numNei < numActionNodes && null_prob>(Number)0) 
	support.push_back(-1);


    //gray code
    GrayComposition gc (numPlayers-1, support.size() );

    Number prob = pow((support.at(0)>=0)?s[neighbors[node][support[0]]]:null_prob,
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
      Number i_prob = (support.at(gc.i)!=-1)?s[neighbors[node][support[gc.i]]]:null_prob;
      Number d_prob =(support.at(gc.d)!=-1)?s[neighbors[node][support[gc.d]]]:null_prob;
      assert(i_prob>(Number)0  && d_prob>(Number)0 );
      prob *= ((Number)(gc.get().at(gc.d)+1)) * i_prob /(Number)(gc.get().at(gc.i)) /d_prob; 
      
    }//end while
    
    return V;
}

//get the prob distribution over configurations of neighbourhood of node.
//plClass: the index for the player class
//s: mixed strat for that player class

void agg::getSymConfigProb(int plClass, StrategyProfile &s, int ownPlClass, int act, aggdistrib &dest,int plClass2,int act2){
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
        for (int j=0;j<actions[player];j++)if(s[j]>(Number)0.0){
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
          dest.swap(temp);
        }
      }
      if(plClass==plClass2){
        aggdistrib temp;
        temp.insert(make_pair(projection[node][player].at(act2),1.0));
        if(dest.size()>0){
          dest.multiply(temp, numNei, projFunctions[node]);
        }else{
          dest.swap(temp);
        }
      }
      return;
    }





    //Number V = 0.0;
    vector<int> support;
    Number null_prob=1;
    //do projection  & get support
    int self = -1;   //index of self in the neighbor list
    int ind2=-1;     //index of act2 in the neighbor list
    int p=playerClasses[plClass][0];
    for (int i=0; i<numNei; ++i){
	if (neighbors[node][i] == node) self=i;
	if (plClass2>=0 && neighbors[node][i] == uniqueActionSets.at(plClass2).at(act2)) ind2=i;

	int a=node2Action.at(neighbors[node][i]).at(p);
	if (a>=0&&s[a]>(Number)0) {
	  support.push_back(i);
	  null_prob -= s[a];
	}
    }
    if (null_prob>(Number)0)
	support.push_back(-1);


    //gray code
    GrayComposition gc (numPl, support.size() );

    Number prob0=(support.at(0)>=0)?s[node2Action[neighbors[node].at(support[0])][p]]:null_prob;
    Number prob = pow(prob0,numPl);

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
      Number i_prob = (support.at(gc.i)!=-1)?s[node2Action[neighbors[node][support[gc.i]]][p]]:null_prob;
      Number d_prob =(support.at(gc.d)!=-1)?s[node2Action[neighbors[node][support[gc.d]]][p]]:null_prob;
      assert(i_prob>(Number)0  && d_prob>(Number)0 );
      prob *= ((Number)(gc.get().at(gc.d)+1)) * i_prob /(Number)(gc.get().at(gc.i)) /d_prob; 
      
    }//end while

  
}

Number agg::getKSymMixedPayoff( int playerClass,vector<StrategyProfile> &s){
  Number result=0.0;

  for(int act=0;act<(int)uniqueActionSets[playerClass].size();act++)if(s[playerClass][act]>(Number)0.0){

      result += s[playerClass][act] *getKSymMixedPayoff(playerClass, act,s);
  }
  return result;
}
Number agg::getKSymMixedPayoff( int playerClass,StrategyProfile &s){
  Number result=0.0;

  for(int act=0;act<(int)uniqueActionSets[playerClass].size();act++)if(s[firstKSymAction(playerClass)+act]>(Number)0.0){

      result += s[firstKSymAction(playerClass)+act] *getKSymMixedPayoff(s,playerClass, act);
  }
  return result;
}
void agg::getKSymPayoffVector(NumberVector& dest,int playerClass, StrategyProfile &s){
  for (size_t act=0;act<uniqueActionSets[playerClass].size();++act){
    dest[act]=getKSymMixedPayoff(s,playerClass,act);
  }
}
Number agg::getKSymMixedPayoff(int playerClass, int act, vector<StrategyProfile> &s){
      
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

Number agg::getKSymMixedPayoff(const StrategyProfile &s,int pClass1,int act1,int pClass2,int act2){
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


#ifdef USE_CVECTOR
void agg::SymPayoffMatrix(cmatrix &dest, cvector &s, Number fuzz){
  if (getNumPlayerClasses()>1){
    cerr<<"SymPayoffMatrix() Error: game is not symmetric"<<endl;
    exit(1);
  }
  assert(numPlayers>1);

  cache.clear();

  Number fuzzcount;

  int currNode,numNei;
  for (int rowa=0;rowa<getNumActions(0);++rowa){
    currNode =actionSets[0][rowa];
    numNei= neighbors[currNode].size();
    //vector<int> key (numNei+1);
    //key[numNei]=currNode;
    doProjection(currNode,0,s);
    aggdistrib &Pdest = Pr[numPlayers-1];
    projectedStrat[currNode][0].power(numPlayers-2, Pdest, Pr[numPlayers-2],numNei,projFunctions[currNode]);
    aggdistrib &temp=Pr[numPlayers-2];
    temp.reset();
    temp.insert(make_pair(projection[currNode][0][rowa],1));
    Pdest.multiply(temp,numNei,projFunctions[currNode]);
    for (int cola=0;cola<getNumActions(0);++cola){
      pair<vector<int>,Number> insPair( projection[currNode][0][cola],0);

      //insPair.first.reserve(numNei+3);
      insPair.first.push_back(currNode);
      pair<trie_map<Number>::iterator,bool> r =cache.insert(insPair);

      if (! r.second) {
          dest[rowa][cola]=r.first->second;
      }else{
          r.first->second=Number(numPlayers-1)
              * Pdest.inner_prod(projection[currNode][0][cola], numNei, projFunctions[currNode], payoffs[currNode]);
          dest[rowa][cola]=r.first->second;
      }
    }
  }

  fuzzcount=fuzz;

  for (int rowi=firstKSymAction(0); rowi<lastKSymAction(0);rowi++){

              dest[rowi][rowi]+=fuzzcount;
              fuzzcount+=fuzz;

  }

}

void agg::KSymPayoffMatrix(cmatrix &dest, cvector &s, Number fuzz){
  //cerr<<"error: k-symmetric Jacobian not yet implemented";
  //exit(1);

  //simple implementation using expected payoffs:
  for(int rowcls=0;rowcls<getNumPlayerClasses();++rowcls){
    for(int rowa = 0; rowa<getNumKSymActions(rowcls);++rowa){
      for(int colcls=0;colcls<getNumPlayerClasses();++colcls){
        int multiplier=playerClasses[colcls].size();
        if (rowcls==colcls) multiplier-=1;
        for(int cola=0;cola<getNumKSymActions(colcls);++cola){

          dest[rowa+firstKSymAction(rowcls)][cola+firstKSymAction(colcls)]=
              (Number)multiplier *
              getKSymMixedPayoff(s,rowcls,rowa,colcls,cola);
        }
      }
    }
  }

  Number fuzzcount;
  for (int rown=0; rown<getNumPlayerClasses(); ++rown){
          fuzzcount=fuzz;
          for (int rowi=firstKSymAction(rown); rowi<lastKSymAction(rown);rowi++){
            if (getNumPlayerClasses()==1){ //symmetric case
              dest[rowi][rowi]+=fuzzcount;
              fuzzcount+=fuzz;
            }
            else for (int coli=firstKSymAction(rown);coli<lastKSymAction(rown);coli++){
                dest[rowi][coli]+=fuzzcount;
                fuzzcount+=fuzz;
            }
          }
  }
}
#endif

void agg::makeMAPPINGpayoff(std::istream& in, aggpayoff& pay, int numNei){
    int num;
    char c;
    Number u;
    aggpayoff temp;
    temp.swap(pay);

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
	if (c != agg::LBRACKET){
	    cerr<< "ERROR: "<<agg::LBRACKET <<" expected. Instead, got "<<c <<endl;
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

	if (c!=agg::RBRACKET){
	    cerr<< "ERROR: "<<agg::RBRACKET <<" expected. Instead, got "<<c<<endl;
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
	pair<trie_map<Number>::iterator, bool> r = pay.insert(make_pair(key,u));
	if (!r.second){
	    cerr<<"WARNING0: overwriting utility at [";
	    copy(key.begin(),key.end(), ostream_iterator<int>(cerr, " "));
	    cerr<<"]"<<endl;

	    r.first->second = u;
	}

    }
    //check
    for(trie_map<Number>::iterator it = temp.begin(); it!=temp.end(); ++it){

	//pair<trie_map<Number>::iterator,bool> res = pay.insert( *it);
	
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

Number agg::getMaxPayoff(){
  static bool done=false;
  static Number result=0;
  if (done)return result;
  assert(numActionNodes>0);
  result=payoffs[0].begin()->second;
  for (int i=1;i<numActionNodes;i++)
    for (aggpayoff::iterator it=payoffs[i].begin();it!=payoffs[i].end();++it)
      result=max(result, it->second);

  done=true;
  return result;
}
Number agg::getMinPayoff(){
  static bool done=false;
  static Number result=0;
  if (done)return result;
  assert(numActionNodes>0);
  result=payoffs[0].begin()->second;
  for (int i=1;i<numActionNodes;i++)
      for (aggpayoff::iterator it=payoffs[i].begin();it!=payoffs[i].end();++it)
        result=min(result, it->second);
  done=true;
  return result;
}
