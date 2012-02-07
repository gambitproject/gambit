#include "agg.h"

#include <iostream>
#include <cstdlib>
#include "libgambit/libgambit.h"
using namespace std;

void usage(char *name) {

    cout<<"usage:\n"<< name 
	<<" file"<<endl<<endl
	<<"Takes mixed strategy profiles from standard input, output expected payoffs"<<endl;
}


bool readstrat(StrategyProfile& s, int m){
    cin>>ws;
    if(cin.eof()) return false;
    for (int i=0;i<m;i++){
	if(! (cin>> s[i])) {
	    cerr<<"Error: more numbers expected."<<endl;
	    return false;
	}
    }
    return true;
}

int main(int argc, char **argv) {

  if (argc <2){
    usage(argv[0]);
    return -1;
  }

  agg *aggPtr=NULL;
  aggPtr=agg::makeAGG(argv[1]);
  if (!aggPtr) {
      cerr<<"Failed to read AGG"<<endl;
      exit(1);
  }
  bool useGambit=false;
  if (argv[0]=="gampayoffs")useGambit=true;

  Gambit::Game g= new Gambit::GameAggRep(aggPtr);

  int m=aggPtr->getNumActions();
  StrategyProfile s(m);
  //Number* s = new Number[m];

  Gambit::MixedStrategyProfile<double> p=g->NewMixedStrategyProfile(0);
  while(readstrat(s, m) ){
    if(useGambit){
    	p = g->NewMixedStrategyProfile(0);
    	for (int i=1;i<=g->NumPlayers();++i){
    		for (int j=1;j<=p.GetSupport().NumStrategies(i);++j)
    			p[p.GetSupport().GetStrategy(i,j)]=s[aggPtr->firstAction(i-1)+j-1];
    	}
    }

    cout<<"Expected utility for each player"<<endl;
	for (int player=0;player<aggPtr->getNumPlayers();player++){
	  if(useGambit){
		  cout<<p.GetPayoff(player+1)<<" ";
	  }
	  else{
		  cout<<aggPtr->getMixedPayoff(player, s) <<" ";
	  }
	}
	cout<<endl;
        //cout<<endl<<"Expected configuration"<<endl;
        //cout <<aggPtr->getExpectedConfig(s) <<endl;

  }
  delete aggPtr;
  //delete [] s;
  return 0;
}




