//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/libagg/getpayoffs.cc
// Implementation of computation of expected payoffs given strategy profile
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

#include "agg.h"
#include "bagg.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
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

  //agg *aggPtr=NULL;
  //aggPtr=agg::makeAGG(argv[1]);

  std::ifstream file_s;
  file_s.open(argv[1]);
  if (!file_s.is_open()) {
      cerr<<"Failed to read "<<argv[1]<<endl;
      exit(1);
  }
  //bool useGambit=false;
  //if (strstr(argv[0],"gambit"))useGambit=true;

  Gambit::Game g=  Gambit::ReadGame(file_s);
  Gambit::MixedStrategyProfile<double> p=g->NewMixedStrategyProfile(0);
  int m=p.MixedProfileLength();
  StrategyProfile s(m);


  while(readstrat(s, m) ){
    //if(useGambit){
    	p = g->NewMixedStrategyProfile(0);
    	for (int i=1,offs=0;i<=g->NumPlayers();offs+=p.GetSupport().NumStrategies(i),++i){
    		for (int j=1;j<=p.GetSupport().NumStrategies(i);++j)
    			p[p.GetSupport().GetStrategy(i,j)]=s[offs+j-1];
    	}
    //}

    cout<<"Expected utility for each player"<<endl;
	for (int player=0;player<g->NumPlayers();player++){
	  //if(useGambit){
		  cout<<p.GetPayoff(player+1)<<" ";
	  //}
	}
	cout<<endl;
        //cout<<endl<<"Expected configuration"<<endl;
        //cout <<aggPtr->getExpectedConfig(s) <<endl;

  }

  return 0;
}




