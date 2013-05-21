
#ifndef __AGGAME_H
#define __AGGAME_H

/*
** AGG for gametracer. This is a wrapper class of agg
** Written by Albert Xin Jiang
*/

#include "cmatrix.h"
#include "libagg/agg.h"
#include "libgambit/libgambit.h"
#include "libgambit/gameagg.h"
#include "gnmgame.h"


class aggame : public gnmgame {
  public:

    static aggame* makeAGGame(char* filename) {
      return new aggame( agg::makeAGG(filename) );
    }


    static aggame* makeAGGame(istream& in) {
      return new aggame ( agg::makeAGG(in) );
    }

    aggame ( agg* _aggPtr)
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
    double getPurePayoff(int player, int* s){
      return aggPtr->getPurePayoff(player,s);
    }

    inline void setPurePayoff(int player, int *s, double value){
    	throw Gambit::UndefinedException();
    }

    cvector getExpectedConfig(cvector &s){
      std::vector<double> sp (s.values(), s.values()+s.getm());
      std::vector<double> c ( aggPtr->getExpectedConfig(sp) );
      cvector res (&c[0], c.size(), false);
      return res;
    }

    int getNumPlayerClasses(){return aggPtr->getNumPlayerClasses();}

    const vector<int>& getPlayerClass(int cls){return aggPtr->getPlayerClass(cls);}
    int getPlayerClassSize(int cls){return aggPtr->getPlayerClass(cls).size();}
    int getNumKSymActions(){return aggPtr->getNumKSymActions();}
    int getNumKSymActions(int p){return aggPtr->getNumKSymActions(p);}
    int firstKSymAction(int p){return aggPtr->firstKSymAction(p);}
    int lastKSymAction(int p){return aggPtr->lastKSymAction(p);}
    bool isSymmetric () {return aggPtr->isSymmetric();}
    //void KSymRetractJac(cmatrix &dest, int *support){aggPtr->KSymRetractJac(dest,support);}
    //void KSymNormalizeStrategy(cvector& s){aggPtr->KSymNormalizeStrategy(s);}

//  private:

    agg* aggPtr;

  private:

  //helper functions for computing jacobian
    void computePartialP_PureNode(int player,int act,std::vector<int>& tasks);
    void computePartialP_bisect(int player,int act, std::vector<int>::iterator f,std::vector<int>::iterator l,aggdistrib& temp);
    void computePartialP(int player1, int act1, std::vector<int>& tasks,std::vector<int>& nontasks);
    void computePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,trie_map<AggNumber>& cache);
    void savePayoff(cmatrix& dest,int player1,int act1,int player2,int act2,AggNumber result,
	  trie_map<AggNumber>& cache, bool partial=false );
    void computeUndisturbedPayoff(AggNumber& undisturbedPayoff,bool& has,int player1,int act1,int player2);

};

#endif
