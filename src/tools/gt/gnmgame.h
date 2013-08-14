/* Copyright 2002 Ben Blum, Christian Shelton
 *
 * This file is part of GameTracer.
 *
 * GameTracer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GameTracer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GameTracer; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GNMGAME_H
#define __GNMGAME_H

#include "cmatrix.h"
#define BIGFLOAT 3.0e+28F

#ifdef WIN32
#ifndef drand48
#define srand48(x) srand(x)
#define drand48(x) ((double)rand(x)/RAND_MAX)
#endif
#endif

class gnmgame {
 public:
  
  //actions[i] = number of actions player i has
  gnmgame(int numPlayers, int *actions);
  virtual ~gnmgame();
  
  
  
  // Input: s[i] has integer index of player i's pure strategy
  // s is of length numPlayers
  virtual double getPurePayoff(int player, int *s) = 0; 

  virtual void setPurePayoff(int player, int *s, double value) = 0;

  // The actions of all players are combined in one linear array of length
  // numActions; this gives the index of a player's first action in the array.
  inline int firstAction(int player) {
    return strategyOffset[player];
  }

  inline int lastAction(int player) {
    return strategyOffset[player+1];
  }

  // s is the mixed strategy profile.  It is of length numActions, and
  // s[i] is the probability that the appropriate player takes action i.
  virtual double getMixedPayoff(int player, cvector &s) = 0;

  virtual double getSymMixedPayoff(cvector &s){
    cvector fulls(getNumActions());
    int nact=getNumActions(0);
    for(int i=0;i<getNumPlayers();++i){
      //assert(nact==getNumActions(i));
      for(int j=0;j<nact;++j)
        fulls[j+firstAction(i)]=s[j];
    }
    return getMixedPayoff(0,fulls);
  }

  virtual double getKSymMixedPayoff(int cls, cvector &s){
    //assert(s.getm()==getNumKSymActions());
    return getMixedPayoff(cls,s);
  }

  // s is the mixed strategy profile, as above.  This function stores
  // the Jacobian of the payoff function G, where G(i) is the payoff to
  // the owner of action i if he deviates from s by choosing i instead.
  virtual void payoffMatrix(cmatrix &dest, cvector &s, double fuzz) = 0;

  virtual void payoffMatrix(cmatrix &dest, cvector &s, double fuzz, bool ksym){
    if(ksym && s.getm()!=getNumKSymActions()){
      cerr<<"payoffMatrix() error: k-symmetric version of Jacobian not implemented for this class"<<endl;
      exit(1);
    }
    payoffMatrix(dest, s, fuzz);
  }

  // store in dest the the payoff function G, where G(i) is the payoff to
  // the owner of action i if he deviates from s by choosing i instead.
  virtual void getPayoffVector(cvector &dest, int player,const cvector &s) = 0; 

  //get payoff vector for a symmetric game under a symmetric strategy. only one player's strategy is given in s
  virtual void getSymPayoffVector(cvector &dest, cvector &s){
    cvector fulls(getNumActions());
    int nact=getNumActions(0);
    for(int i=0;i<getNumPlayers();++i){
      //assert(nact==getNumActions(i));
      for(int j=0;j<nact;++j)
        fulls[j+firstAction(i)]=s[j];
    }
    getPayoffVector(dest, 0, fulls);
  }
  virtual void getKSymPayoffVector(cvector &dest, int playerClass, cvector &s){
    //assert(s.getm()==getNumKSymActions());
    getPayoffVector(dest,playerClass,s);
  }

  // return the regret for a player under strategy profile s
  double getRegret(int player, cvector &s)
  {
    double p=0;
    cvector payoffs(actions[player]);
    getPayoffVector(payoffs, player, s);
    for (int i=0;i<actions[player];i++){
      p+=payoffs[i] * s[firstAction(player)+i];
    }
    return payoffs.max()-p;
  }

  //return the max regret under strategy profile s
  double getRegret(cvector &s)
  {
    cvector regrets(numPlayers);
    for (int i=0;i<numPlayers;i++) regrets[i]= getRegret(i,s);
    return regrets.max();
  }

  double getKSymRegret(int playerClass,cvector &s){
     double p=0;
     cvector payoffs(getNumKSymActions(playerClass));
     getKSymPayoffVector(payoffs, playerClass, s);
     for (int i=0;i<getNumKSymActions(playerClass);i++){
       p+=payoffs[i] * s[firstKSymAction(playerClass)+i];
     }
     return payoffs.max()-p;
  }
  double getKSymRegret(cvector &s){
    cvector regrets(getNumPlayerClasses());
    for (int i=0;i<getNumPlayerClasses();i++) regrets[i]= getKSymRegret(i,s);
    return regrets.max();
  }


  // this stores the Jacobian of the retraction function in dest.  
  void retractJac(cmatrix &dest, std::vector<int> &support);

  // This retracts z onto the nearest normalized strategy profile, according
  // to the Euclidean metric
  void retract(cvector &dest, cvector &z);
  void retract(cvector &dest, cvector &z, bool ksym);

  // LNM runs the local Newton method on z to attempt to bring it closer to
  // the image of the graph of the equilibrium correspondence above the ray,
  // under the homeomorphism.  In order to prevent costly memory allocation,
  // a number of scratch vectors are passed in.

  double LNM(cvector &z, const cvector &g, double det, cmatrix &J, cmatrix &DG,  cvector &s, int MaxLNM, double fuzz, cvector &del, cvector &scratch, cvector &backup, bool ksym=false);

  // This normalizes a strategy profile by scaling appropriately.
  void normalizeStrategy(cvector &s);

  void LemkeHowson(cvector &dest, cmatrix &T, std::vector<int> &Im);


  inline int getNumPlayers() { return numPlayers; }
  inline int getNumActions() { return numActions; }
  inline int getNumActions(int p) { return actions[p]; }
  inline int getMaxActions() { return maxActions; }

  virtual int getNumPlayerClasses(){return numPlayers;}
  virtual int getPlayerClassSize(int cls){return 1;}
  virtual int getNumKSymActions(){return numActions;}
  virtual int getNumKSymActions(int p){return actions[p];}
  virtual int firstKSymAction(int p){return strategyOffset[p];}
  virtual int lastKSymAction(int p){return strategyOffset[p+1];}

  void KSymRetractJac(cmatrix &dest, int *support);
  void KSymNormalizeStrategy(cvector& s);



  // generate random strategy profile, with full support
  void randomFullStrategy(cvector& dest, int player){
      double normconst;
      normconst=0;
      for (int j=firstAction(player); j<lastAction(player); j++){
            dest[j] = drand48();
            normconst+= dest[j];
      }
      for (int j= firstAction(player);j<lastAction(player); j++)
            dest[j] /= normconst;
            
  }
  void randomFullStrategy (cvector& dest){
    for (int i=0; i<numPlayers; ++i){
      randomFullStrategy(dest, i);
    }
  }
  // generate random strategy profile, with random support size.
  // returns the volume of the support profile.
  unsigned long long randomSupportStrategy(cvector& dest,int player, double posprob){
      double normconst;
      unsigned long long currsupp;
      
      do{
        currsupp=0;
        normconst=0;
        for (int j=firstAction(player);j<lastAction(player); j++){
          if ( drand48() < posprob) {
            dest[j] = drand48();
            normconst+=dest[j];
            currsupp++;
          }
          else {
            dest[j]=0;
          }
        }
      } while (currsupp<=0 || normconst==0.0);
      
      // normalize
      for (int j=firstAction(player);j<lastAction(player); j++)
        dest[j] /= normconst;
        
      return currsupp;
  }

  unsigned long long randomSupportStrategy(cvector& dest , double posprob){
    unsigned long long suppsize=1;
    for (int i=0;i<numPlayers; ++i){
      suppsize *= randomSupportStrategy(dest, i, posprob);
    }
    return suppsize;
  }

 protected:
  
  int Pivot(cmatrix &T, int pr, int pc, std::vector<int> &row, std::vector<int> &col, 
	    double &D);

  int *strategyOffset;
  int numPlayers, numStrategies, numActions;
  int *actions;
  int maxActions;
};

#endif
