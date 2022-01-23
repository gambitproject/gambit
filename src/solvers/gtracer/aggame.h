//
// This file is part of Gambit
// Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: library/include/gambit/gtracer/aggame.h
// Interface to GNM-specific routines for action graph games
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

#ifndef GAMBIT_GTRACER_AGGAME_H
#define GAMBIT_GTRACER_AGGAME_H

#include "cmatrix.h"
#include "gnmgame.h"
#include "games/agg/agg.h"
#include "gambit.h"
#include "games/gameagg.h"

namespace Gambit {
namespace gametracer {

class aggame : public gnmgame {
  public:

    static aggame* makeAGGame(char* filename) {
      return new aggame(Gambit::agg::AGG::makeAGG(filename) );
    }


    static aggame* makeAGGame(std::istream& in) {
      return new aggame ( Gambit::agg::AGG::makeAGG(in) );
    }

    aggame ( Gambit::agg::AGG *_aggPtr)
      :gnmgame(_aggPtr->getNumPlayers(), _aggPtr->actions),
      aggPtr (_aggPtr)
    {
    }

    aggame(Gambit::GameAggRep& g)
      :gnmgame(g.aggPtr->getNumPlayers(), g.aggPtr->actions),
      aggPtr (g.aggPtr)
    {
    }

    virtual ~aggame () {
      //delete aggPtr;
    }

    int getNumActionNodes(){return aggPtr->getNumActionNodes();}


    double getMixedPayoff(int player, cvector &s){
      std::vector<double> sp (s.values(), s.values()+s.getm());
      return (double)aggPtr->getMixedPayoff(player,sp);
    }

    double getKSymMixedPayoff(int cls, cvector &s){
      std::vector<double> sp (s.values(), s.values()+s.getm());
      return (double) aggPtr->getKSymMixedPayoff(cls,sp);
    }

    void payoffMatrix(cmatrix &dest, cvector &s, double fuzz);

    void payoffMatrix(cmatrix &dest, cvector &s, double fuzz,bool ksym){
      if(ksym){
        KSymPayoffMatrix(dest, s, fuzz);
      }
      else{
        payoffMatrix(dest,s,fuzz);
      }
    }

    void SymPayoffMatrix(cmatrix &dest, cvector &s, double fuzz);
    void KSymPayoffMatrix(cmatrix &dest, cvector &s, double fuzz);

    void getPayoffVector(cvector &dest, int player,const cvector &s){
      cvector & ss = const_cast<cvector &>(s);
      std::vector<double> sp (ss.values(), ss.values()+ss.getm());
      std::vector<double> d(aggPtr->getNumActions(player));
      aggPtr->getPayoffVector(d,player,sp);
      std::copy(d.begin(),d.end(), dest.values());
    }
    void getSymPayoffVector(cvector& dest, cvector &s){
      std::vector<double> sp (s.values(), s.values()+s.getm());
      std::vector<double> d(aggPtr->getNumActionNodes());
      aggPtr->getSymPayoffVector(d,sp);
      std::copy(d.begin(),d.end(), dest.values());
    }
    void getKSymPayoffVector(cvector &dest, int playerClass, cvector &s){
      std::vector<double> sp (s.values(), s.values()+s.getm());
      std::vector<double> d (aggPtr->getNumKSymActions(playerClass));
      aggPtr->getKSymPayoffVector(d,playerClass,sp);
      std::copy(d.begin(),d.end(), dest.values());
    }
    double getPurePayoff(int player, std::vector<int> &s){
      return aggPtr->getPurePayoff(player,s);
    }

    inline void setPurePayoff(int player, std::vector<int> &s, double value){
    	throw Gambit::UndefinedException();
    }

    cvector getExpectedConfig(cvector &s){
      std::vector<double> sp (s.values(), s.values()+s.getm());
      std::vector<double> c ( aggPtr->getExpectedConfig(sp) );
      cvector res (&c[0], c.size(), false);
      return res;
    }

    int getNumPlayerClasses(){return aggPtr->getNumPlayerClasses();}

    const std::vector<int>& getPlayerClass(int cls){return aggPtr->getPlayerClass(cls);}
    int getPlayerClassSize(int cls){return aggPtr->getPlayerClass(cls).size();}
    int getNumKSymActions(){return aggPtr->getNumKSymActions();}
    int getNumKSymActions(int p){return aggPtr->getNumKSymActions(p);}
    int firstKSymAction(int p){return aggPtr->firstKSymAction(p);}
    int lastKSymAction(int p){return aggPtr->lastKSymAction(p);}
    bool isSymmetric () {return aggPtr->isSymmetric();}
    //void KSymRetractJac(cmatrix &dest, int *support){aggPtr->KSymRetractJac(dest,support);}
    //void KSymNormalizeStrategy(cvector& s){aggPtr->KSymNormalizeStrategy(s);}

//  private:

    Gambit::agg::AGG *aggPtr;

  private:

  //helper functions for computing jacobian
    void computePartialP_PureNode(int player,int act,std::vector<int>& tasks);
    void computePartialP_bisect(int player,int act, std::vector<int>::iterator f,std::vector<int>::iterator l,Gambit::agg::aggdistrib& temp);
    void computePartialP(int player1, int act1, std::vector<int>& tasks,std::vector<int>& nontasks);
    void computePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,Gambit::agg::trie_map<Gambit::agg::AggNumber>& cache);
    void savePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,Gambit::agg::AggNumber result,
		    Gambit::agg::trie_map<Gambit::agg::AggNumber>& cache, bool partial=false );
    void computeUndisturbedPayoff(Gambit::agg::AggNumber& undisturbedPayoff,bool& has,int player1,int act1,int player2);

};

}  // end namespace Gambit::gametracer
}  // end namespace Gambit
 
#endif  // GAMBIT_GTRACER_AGGAME_H
