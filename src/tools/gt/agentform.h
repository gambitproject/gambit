#ifndef __AGENTFORM_H
#define __AGENTFORM_H

//agent form wrapper for bayesian agg



#include "cmatrix.h"
#include "gnmgame.h"
#include "libagg/bagg.h"
#include "libgambit/gamebagg.h"
#include "libgambit/libgambit.h"


class agentform : public gnmgame {
public:

  //static agentform* makeAgentForm(char* filename){ return new agentform(bagg::makeBAGG(filename));}
  //static agentform* makeAgentForm(istream& in){ return new agentform(bagg::makeBAGG(in)); }

  agentform(bagg* ptr)
    :gnmgame(ptr->getNumTypes(), makeActions(ptr) ), baggPtr(ptr),agent2baggPlayer(ptr->getNumTypes()),
     numKSymActions(0),kSymStrategyOffset(1,0),
     playerClasses((ptr->isSymmetric())?ptr->getNumTypes(0):ptr->getNumTypes(), vector<int>())
  {
    int k=0;

    for (int i=0;i<ptr->getNumPlayers();i++){

      for (int j=0;j<ptr->getNumTypes(i);j++,k++){
        agent2baggPlayer[k] = i;
        if (ptr->isSymmetric()){
          playerClasses.at(j).push_back(k);
        } else {
          playerClasses.at(k).push_back(k);
        }
      }
    }
    int K=(baggPtr->isSymmetric())? 1:baggPtr->getNumPlayers();
    for (int bp = 0;bp<K;bp++){
      for (int i=0;i<baggPtr->getNumTypes(bp); ++i){
        numKSymActions+=baggPtr->typeActionSets[bp][i].size();
        kSymStrategyOffset.push_back(numKSymActions);
      }
    }
  }

  ~agentform()
  {
    //delete baggPtr;
  }

  int getNumActionNodes() {return baggPtr->getNumActionNodes();}

  int getNumPlayerClasses(){return (baggPtr->isSymmetric())? baggPtr->getNumTypes(0) : getNumPlayers(); }
  const std::vector<int>& getPlayerClass(int cls){return playerClasses.at(cls);}
  int getPlayerClassSize(int cls){return playerClasses.at(cls).size();}
  int getNumKSymActions(){return (baggPtr->isSymmetric())?numKSymActions :getNumActions();}
  int getNumKSymActions(int p){return (baggPtr->isSymmetric())?baggPtr->typeActionSets[0][p].size():getNumActions(p);}
  int firstKSymAction(int p){return (baggPtr->isSymmetric())?kSymStrategyOffset[p]:firstAction(p);}
  int lastKSymAction(int p){return (baggPtr->isSymmetric())?kSymStrategyOffset[p+1]:lastAction(p);}

  /*
  void KSymNormalizeStrategy(cvector& s){
    if(baggPtr->isSymmetric()){
      Number sum;
      int offs=0;
      for(int n = 0; n < (int)baggPtr->getNumTypes(0); n++) {
        sum = 0.0;
        for(int i = 0; i < (int)baggPtr->typeActionSets[0][n].size(); i++) {
          sum += s[offs+i];
        }
        for(int i = 0; i < (int)baggPtr->typeActionSets[0][n].size(); i++) {
          s[offs+i] /= sum;
        }
        offs+=baggPtr->typeActionSets[0][n].size();
      }
    }else{
      normalizeStrategy(s);
    }
  }
  */

  Number getMixedPayoff(int player, cvector &s){
      int p = agent2baggPlayer[player];
      int tp = player - baggPtr->typeOffset[p];
      return baggPtr->getMixedPayoff(p, tp, s);
  }
  Number getKSymMixedPayoff(int cls,cvector &s){
    if (baggPtr->isSymmetric())
      return baggPtr->getSymMixedPayoff(cls,s);
    else
      return getMixedPayoff(cls,s);
  }

  Number getPurePayoff(int player, int* s){
      int p = agent2baggPlayer[player];
      int tp= player - baggPtr->typeOffset[p];
      return baggPtr->getPurePayoff(p,tp,s);
  }

  void payoffMatrix(cmatrix &dest, cvector &s, Number fuzz){
    Number fuzzcount;
    vector<int> done(baggPtr->aggPtr->getNumActions(), -1);
    cmatrix d (baggPtr->aggPtr->getNumActions(), baggPtr->aggPtr->getNumActions());
    cvector as (baggPtr->aggPtr->getNumActions());
    baggPtr->getAGGStrat(as, s); //get the induced AGG strategies
    baggPtr->aggPtr->payoffMatrix(d,as,fuzz);

    //diagonals
    for (int rown=0; rown<getNumPlayers(); ++rown){
      fuzzcount=fuzz;
      for (int rowi=firstAction(rown); rowi<lastAction(rown);rowi++){
        for (int coli=firstAction(rown);coli<lastAction(rown);coli++){
  		  dest[rowi][coli]=fuzzcount;
  		  fuzzcount+=fuzz;
        }
      }
    }

    for (int rowp =0; rowp<getNumPlayers();rowp++){//row player of the agent form
      int rowbp=agent2baggPlayer[rowp];
      int rowtp = rowp - baggPtr->typeOffset[rowbp];
      for (int rowa = 0; rowa<getNumActions(rowp);rowa++){//row action of agent form
        //get corresponding action in the agg
        int rowaact = baggPtr->typeAction2ActionIndex[rowbp][rowtp][rowa]+baggPtr->aggPtr->firstAction(rowbp);
        


        if (done.at(rowaact) != -1){//this row have been done earlier, copy
            for(int colp=0;colp<getNumPlayers(); ++colp)if(colp!=rowp){
                int colbp=agent2baggPlayer[colp];
                if (colbp==rowbp){
                    //here the values being copied are idential;
                    //just need to avoid copying from the diagonal block of row done[rowaact].
                    for(int coli=firstAction(colp);coli<lastAction(colp);++coli)
                        dest[rowa+firstAction(rowp)][coli]=dest[done[rowaact]][firstAction(rowp)]; //since we know rowp!=colp, i.e. rhs not from a diagonal block
                }
                else{
                    for(int coli=firstAction(colp);coli<lastAction(colp);++coli)
                        dest[rowa+firstAction(rowp)][coli]=dest[done[rowaact]][coli]; //straight copy
                }
            }
            continue; //go do next row
        }

        done.at(rowaact)=rowa+firstAction(rowp);

        for (int colbp=0; colbp<baggPtr->getNumPlayers(); colbp++){//col player of Bayesian game
          
          if (colbp==rowbp){
            if (baggPtr->getNumTypes(colbp)>1) {//since otherwise the only corresponding block is the diagonal block

              //just need to calculate one value, the EU under deviation to rowaact of the AGG
              int colbp2=0; //a player of the BAGG that is not colbp (in order to avoid the diagonal block)
              for (;colbp2<baggPtr->getNumPlayers();colbp2++){
                if (colbp2!=colbp) break;
              }
              Number u=0;
              for (int aact=baggPtr->aggPtr->firstAction(colbp2); aact<baggPtr->aggPtr->lastAction(colbp2);++aact){
                  u+=d[rowaact][aact]*as[aact];
              }

              for (int colp=baggPtr->typeOffset[colbp];colp<baggPtr->typeOffset[colbp+1];colp++)if(colp!=rowp){
                  for (int coli=firstAction(colp);coli<lastAction(colp);++coli) dest[rowa+firstAction(rowp)][coli]=u;
              }
            }
          }
          else{
            for (int coltp=0;coltp<baggPtr->getNumTypes(colbp);++coltp){//col type of Bayesian game
              int colp = baggPtr->typeOffset[colbp] + coltp;
              //if (colp==rowp) continue;
              //get induced AGG strategy for colbp, except contribution from type coltp:
              for (size_t act=0; act< baggPtr->typeActionSets[colbp][coltp].size(); ++act){
                  int aact= baggPtr->typeAction2ActionIndex[colbp][coltp][act];
                  as[aact+baggPtr->aggPtr->firstAction(colbp)] -= baggPtr->indepTypeDist[colbp][coltp] * s[act+firstAction(colp)];
              }
              Number u=0;
              for (int aact=baggPtr->aggPtr->firstAction(colbp); aact<baggPtr->aggPtr->lastAction(colbp) ; ++aact){
                  u+=d[rowaact][aact]*as[aact];
              }
              for (int cola=0; cola<getNumActions(colp); cola++){//col action of agent form
                int colaact=baggPtr->typeAction2ActionIndex[colbp][coltp][cola]+baggPtr->aggPtr->firstAction(colbp);
                //as[colaact] += baggPtr->indepTypeDist[colbp][coltp];
                dest[rowa+firstAction(rowp)][cola+firstAction(colp)] =
                    u +
                    d[rowaact][colaact]*baggPtr->indepTypeDist[colbp][coltp];

                //for (int aact=baggPtr->aggPtr->firstAction(colbp); aact<baggPtr->aggPtr->lastAction(colbp) ; ++aact){
                //    dest[rowa+firstAction(rowp)][cola+firstAction(colp)] += d[rowaact][aact] * as[aact];
                //}
                //as[colaact] -= baggPtr->indepTypeDist[colbp][coltp];
              }
              //restore as to original values
              for (size_t act=0; act< baggPtr->typeActionSets[colbp][coltp].size(); ++act){
                  int aact= baggPtr->typeAction2ActionIndex[colbp][coltp][act];
                  as[aact+baggPtr->aggPtr->firstAction(colbp)] += baggPtr->indepTypeDist[colbp][coltp] * s[act+firstAction(colp)];
              }

            }//end for coltp
          }//end else
        }
      }
    }
  }



  void KSymPayoffMatrix(cmatrix &dest, cvector &s, Number fuzz){
    if(!baggPtr->isSymmetric()){
      assert(s.getm()==getNumActions());
      payoffMatrix(dest,s,fuzz);
      return;
    }

    Number fuzzcount;
    assert (s.getm()==getNumKSymActions());

    //vector<int> done(baggPtr->aggPtr->getNumKSymActions(), -1);

    cmatrix d (baggPtr->aggPtr->getNumKSymActions(), baggPtr->aggPtr->getNumKSymActions());
    cvector as (baggPtr->aggPtr->getNumKSymActions());
    baggPtr->getSymAGGStrat(as, s); //get the induced AGG strategies
    //baggPtr->aggPtr->KSymPayoffMatrix(d,as,0);
    baggPtr->aggPtr->SymPayoffMatrix(d,as,0);

    for (int rowp =0; rowp<getNumPlayerClasses();rowp++){//row player class of the agent form

      //int rowbp=0;
      int rowtp = rowp;
      for (int rowa = 0; rowa<getNumKSymActions(rowp);rowa++){//row action of agent form
        //get corresponding action in the agg
        int rowaact = baggPtr->typeAction2ActionIndex[0][rowtp][rowa];

        //for (int colbp=0; colbp<baggPtr->getNumPlayers(); colbp++){//col player of Bayesian game

        //  if (colbp==0){

        //contribution to the jacobian from the same col bayesian player as the row bayesian player
        //just need to calculate one value, the EU under deviation to rowaact of the AGG

        Number u=0;
        for (int aact=baggPtr->aggPtr->firstAction(0); aact<baggPtr->aggPtr->lastAction(0);++aact){
                  u+=d[rowaact][aact]*as[aact];
        }
        u/=baggPtr->getNumPlayers()-1.0;


        for (int colp=0;colp<baggPtr->getNumTypes(0);colp++){
                  for (int coli=firstKSymAction(colp);coli<lastKSymAction(colp);++coli)
                    dest[rowa+firstKSymAction(rowp)][coli]=(colp!=rowp)?u:(Number)0;
        }

          //}
          //else{

        //contribution to the jacobian from the col bayesian players that are different from row bp
        for (int coltp=0;coltp<baggPtr->getNumTypes(0);++coltp){//col type of Bayesian game
              int colp = coltp;

              //get induced AGG strategy for colbp, except contribution from type coltp:
              for (size_t act=0; act< baggPtr->typeActionSets[0][coltp].size(); ++act){
                  int aact= baggPtr->typeAction2ActionIndex[0][coltp][act];
                  as[aact] -= baggPtr->indepTypeDist[0][coltp] * s[act+firstKSymAction(colp)];
              }
              Number u=0;
              for (int aact=baggPtr->aggPtr->firstKSymAction(0); aact<baggPtr->aggPtr->lastKSymAction(0) ; ++aact){
                  u+=d[rowaact][aact]*as[aact];
              }
              for (int cola=0; cola<getNumKSymActions(colp); cola++){//col action of agent form
                int colaact=baggPtr->typeAction2ActionIndex[0][coltp][cola];
                //as[colaact] += baggPtr->indepTypeDist[colbp][coltp];
                dest[rowa+firstKSymAction(rowp)][cola+firstKSymAction(colp)] +=
                    (u + d[rowaact][colaact]*baggPtr->indepTypeDist[0][coltp]);

                //for (int aact=baggPtr->aggPtr->firstAction(colbp); aact<baggPtr->aggPtr->lastAction(colbp) ; ++aact){
                //    dest[rowa+firstAction(rowp)][cola+firstAction(colp)] += d[rowaact][aact] * as[aact];
                //}
                //as[colaact] -= baggPtr->indepTypeDist[colbp][coltp];
              }
              //restore as to original values
              for (size_t act=0; act< baggPtr->typeActionSets[0][coltp].size(); ++act){
                  int aact= baggPtr->typeAction2ActionIndex[0][coltp][act];
                  as[aact] += baggPtr->indepTypeDist[0][coltp] * s[act+firstKSymAction(colp)];
              }


        }//end for coltp
      }
    }



    //diagonals
    for (int rown=0; rown<getNumPlayerClasses(); ++rown){
      fuzzcount=fuzz;
      for (int rowi=firstKSymAction(rown); rowi<lastKSymAction(rown);rowi++){
        for (int coli=firstKSymAction(rown);coli<lastKSymAction(rown);coli++){
                  dest[rowi][coli]+=fuzzcount;
                  fuzzcount+=fuzz;
        }
      }
    }

  }

  void payoffMatrix(cmatrix &dest, cvector &s, Number fuzz,bool ksym){
    if(ksym){
      KSymPayoffMatrix(dest,s,fuzz);
    }
    else{
      payoffMatrix(dest,s,fuzz);
    }
  }
  void getPayoffVector(cvector &dest, int player,const cvector &s){
      int p = agent2baggPlayer.at(player);
      int tp = player - baggPtr->typeOffset[p];
      assert(s.getm()==getNumActions());
      baggPtr->getPayoffVector(dest, p, tp, s);
  }
  void getKSymPayoffVector(cvector &dest, int playerClass, cvector &s){
    if (baggPtr->isSymmetric()){
      for(size_t act=0;act<baggPtr->typeActionSets[0][playerClass].size();++act){
            dest[act] = baggPtr->getSymMixedPayoff(playerClass,act,s);
      }
    }else{
      getPayoffVector(dest,playerClass,s);
    }
  }





//private:
  bagg* baggPtr;

  std::vector<int> agent2baggPlayer;

  int numKSymActions;
  //strategyOffset for kSymmetric strategy profile
  std::vector<int> kSymStrategyOffset;
  std::vector<std::vector<int> > playerClasses;
private:

  static int* makeActions(bagg* baggPtr){
    int* actions=new int[baggPtr->getNumTypes()];
    int k=0;
    for (int i=0;i<baggPtr->getNumPlayers();i++){
      for(int j=0;j<baggPtr->getNumTypes(i);j++,k++){
        actions[k]=baggPtr->getNumActions(i,j);
      }
    }
    return actions;
  }

};


#endif

