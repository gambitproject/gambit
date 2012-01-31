#include "agg.h"

#include <iostream>
#include <cstdlib>
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

  int m=aggPtr->getNumActions();
  StrategyProfile s(m);
  //Number* s = new Number[m];

  while(readstrat(s, m) ){


      	cout<<"Expected utility for each player"<<endl;
	for (int player=0;player<aggPtr->getNumPlayers();player++){
	  cout<<aggPtr->getMixedPayoff(player, s) <<" ";
	}
	cout<<endl;
        //cout<<endl<<"Expected configuration"<<endl;
        //cout <<aggPtr->getExpectedConfig(s) <<endl;

  }
  delete aggPtr;
  //delete [] s;
  return 0;
}




