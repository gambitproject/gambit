//#
//# FILE: sftab.cc -- Sequence form tableau
//#
//# $Id$
//#

#include "sftab.h"
#include "extform.h"
#include "infoset.h"
#include "player.h"
#include "rational.h"
#include "node.h"

template <class T> int NumSequences(int j, const ExtForm<T> &E)
{
  gArray<Infoset *> isets;
  isets = E.PlayerList()[j]->InfosetList();
  int num = 1;
  for(int i = isets.First();i<= isets.Last();i++)
    num+=(isets[i])->NumActions();
  return num;
}

template <class T> int NumInfosets(int j, const ExtForm<T> &E)
{
  return E.PlayerList()[j]->InfosetList().Length();
}

//---------------------------------------------------------------------------
//                        Sequence Form  Tableau: member functions
//---------------------------------------------------------------------------

template <class T> SFTableau<T>::SFTableau(SFTableau<T> &tab)
  : LTableau<T>((Tableau<T>)tab), E(tab.E),ns1(tab.ns1),ns2(tab.ns2),
    ni1(tab.ni1),ni2(tab.ni2),maxpay(0)
{ }

template <class T> SFTableau<T>::SFTableau(const ExtForm<T> &E)
  : LTableau<T>(1, NumSequences(1,E) + NumSequences(2,E)
		+ 2*NumInfosets(1,E) + 2*NumInfosets(2,E)+4,
		0, NumSequences(1,E) + NumSequences(2,E)
		+ 2*NumInfosets(1,E) + 2*NumInfosets(2,E)+4), E(E),
		ns1(NumSequences(1,E)),ns2(NumSequences(2,E)),
		ni1(NumInfosets(1,E)+1) ,ni2(NumInfosets(2,E)+1),
		maxpay(0)
{
  T newpay;
  int i,j;
  for(i=1;i<=E.NumOutcomes();i++)
    for(j=1;j<=E.NumPlayers();j++) {
      newpay = ( (OutcomeVector<T> &) *( E.OutcomeList()[i] ))[j];
      if(newpay>=maxpay) maxpay=newpay;
    }
  maxpay=maxpay+(T)1;
  T prob = (T)1;
  FillTableau(E.RootNode(),prob,1,1,0,0);
  for(i=MinRow();i<=MaxRow();i++)
    dtab->Set_A(i,0) = -(T)1;
  dtab->Set_A(1,ns1+ns2+1) = -(T)1;
  dtab->Set_A(1,ns1+ns2+ni1+1) = (T)1;
  dtab->Set_A(ns1+ns2+1,1) = (T)1;
  dtab->Set_A(ns1+ns2+ni1+1,1) = -(T)1;
  dtab->Set_A(ns1+1,ns1+ns2+ni1+ni1+1) = -(T)1;
  dtab->Set_A(ns1+1,ns1+ns2+ni1+ni1+ni2+1) = (T)1;
  dtab->Set_A(ns1+ns2+ni1+ni1+1,ns1+1) = (T)1;
  dtab->Set_A(ns1+ns2+ni1+ni1+ni2+1,ns1+1) = -(T)1;
  dtab->Set_b(ns1+ns2+1) = -(T)1;
  dtab->Set_b(ns1+ns2+ni1+1) = (T)1;
  dtab->Set_b(ns1+ns2+ni1+ni1+1) = -(T)1;
  dtab->Set_b(ns1+ns2+ni1+ni1+ni2+1) = (T)1;
//  gout.SetWidth(1).SetPrec(3);
//  gout << "\n";
//  dtab->Dump(gout);
  Refactor();
  Pivot(ns1+ns2+ni1+1,0);
}

template <class T> SFTableau<T>::~SFTableau(void)
{ }

template <class T> void SFTableau<T>
::FillTableau(const Node *n, T prob,int s1,int s2, int i1,int i2)
{
//  gout << "\ns1,s2,i1,i2: " << s1 << " " << s2  << " " << i1  << " " << i2;
//  gout << " prob = " << prob;
  int i,snew;
  if(n->GetOutcome()) {
    dtab->Set_A(s1,ns1+s2) = dtab->Get_A(s1,ns1+s2) +
       prob*(((OutcomeVector<T> &) *n->GetOutcome())[1] -maxpay);
    dtab->Set_A(ns1+s2,s1) = dtab->Get_A(ns1+s2,s1) +
       prob*(((OutcomeVector<T> &) *n->GetOutcome())[2] -maxpay);
  }
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	FillTableau(n->GetChild(i),
		    prob*((ChanceInfoset<T> *)n->GetInfoset())\
		    ->GetActionProb(i),s1,s2,i1,i2);
    }
    int pl = n->GetPlayer()->GetNumber();
    if(pl==1) {
      i1=n->GetInfoset()->GetNumber();
      snew=1;
      for(i=1;i<i1;i++)
	snew+=n->GetPlayer()->InfosetList()[i]->NumActions();
      dtab->Set_A(s1,ns1+ns2+i1+1) = (T)1;
      dtab->Set_A(s1,ns1+ns2+ni1+i1+1) = -(T)1;
      dtab->Set_A(ns1+ns2+i1+1,s1) = -(T)1;
      dtab->Set_A(ns1+ns2+ni1+i1+1,s1) = (T)1;
      for(i=1;i<=n->NumChildren();i++) {
	dtab->Set_A(snew+i,ns1+ns2+i1+1) = -(T)1;
	dtab->Set_A(snew+i,ns1+ns2+ni1+i1+1) = (T)1;
	dtab->Set_A(ns1+ns2+i1+1,snew+i) = (T)1;
	dtab->Set_A(ns1+ns2+ni1+i1+1,snew+i) = -(T)1;
	FillTableau(n->GetChild(i),prob,snew+i,s2,i1,i2);
      }
    }
    if(pl==2) {
      i2=n->GetInfoset()->GetNumber();
      snew=1;
      for(i=1;i<i2;i++)
	snew+=n->GetPlayer()->InfosetList()[i]->NumActions();
      dtab->Set_A(ns1+s2,ns1+ns2+ni1+ni1+i2+1) = (T)1;
      dtab->Set_A(ns1+s2,ns1+ns2+ni1+ni1+ni2+i2+1) = -(T)1;
      dtab->Set_A(ns1+ns2+ni1+ni1+i2+1,ns1+s2) = -(T)1;
      dtab->Set_A(ns1+ns2+ni1+ni1+ni2+i2+1,ns1+s2) = (T)1;
      for(i=1;i<=n->NumChildren();i++) {
	dtab->Set_A(ns1+snew+i,ns1+ns2+ni1+ni1+i2+1) = -(T)1;
	dtab->Set_A(ns1+snew+i,ns1+ns2+ni1+ni1+ni2+i2+1) = (T)1;
	dtab->Set_A(ns1+ns2+ni1+ni1+i2+1,ns1+snew+i) = (T)1;
	dtab->Set_A(ns1+ns2+ni1+ni1+ni2+i2+1,ns1+snew+i) = -(T)1;
	FillTableau(n->GetChild(i),prob,s1,snew+i,i1,i2);
      }
    }
    
  }
  
/*
  Refactor();
  */
}

template <class T> void SFTableau<T>
::GetProfile(gDPVector<T> &v, const gVector<T> &sol,
	       const Node *n, int s1,int s2)
{
  int i,pl,inf,snew,ind,ind2;
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	GetProfile(v,sol,n->GetChild(i),s1,s2);
    }
    pl = n->GetPlayer()->GetNumber();
    inf= n->GetInfoset()->GetNumber();
    if(pl==1) {
      snew=1;
      for(i=1;i<inf;i++)
	snew+=n->GetPlayer()->InfosetList()[i]->NumActions(); 
      for(i=1;i<=n->NumChildren();i++) {
	v(pl,inf,i) = (T)0;
	if(Member(s1)) {
	  ind = Find(s1);
	  if(sol[ind]!=(T)0) {
	    if(Member(snew+i)) {
	      ind2 = Find(snew+i);
	      v(pl,inf,i) = sol[ind2]/sol[ind];
	    }
	  } 
	} 
	GetProfile(v,sol,n->GetChild(i),snew+i,s2);
      }
    }
    if(pl==2) {
      snew=1;
      for(i=1;i<inf;i++)
	snew+=n->GetPlayer()->InfosetList()[i]->NumActions(); 
      for(i=1;i<=n->NumChildren();i++) {
	v(pl,inf,i) = (T)0;
	if(Member(ns1+s2)) {
	  ind = Find(ns1+s2);
	  if(sol[ind]!=(T)0) {
	    if(Member(ns1+snew+i)) {
	      ind2 = Find(ns1+snew+i);
	      v(pl,inf,i) = sol[ind2]/sol[ind];
	    }
	  } 
	} 
	GetProfile(v,sol,n->GetChild(i),s1,snew+i);
      }
    }
  }
}

template <class T> int SFTableau<T>::LCPPath()
{
//  if (!At_CBFS())  return 0;
  int enter, exit;
  enter = ns1+ns2+ni1+1;
//  if(params.plev >=2) {
//    (*params.output) << "\nbegin LCP path: enter = " << enter << "\n";
//    Dump(*params.output);
//  }
//  gout << "\nbegin LCP path: enter = " << enter << "\n";
//  Dump(gout);
  
//  enter = dup;
//  if (Member(dup))
//    enter = -dup;
      // Central loop - pivot until another CBFS is found
  do  {
    exit = PivotIn(enter);
//    if(params.plev >=2)
//      Dump(*params.output);
    
//    Dump(gout);
    
    enter = -exit;
  } while (exit != 0);
      // Quit when at a CBFS.
      //  if(params.plev >=2 ) (*params.output) << "\nend of path " << dup;
//  gout << "\nend of path ";
  return 1;
}



#ifdef __GNUG__
template int NumSequences(int, const ExtForm<double> &);
template int NumSequences(int, const ExtForm<gRational> &);
template int NumInfosets(int, const ExtForm<double> &);
template int NumInfosets(int, const ExtForm<gRational> &);
template class SFTableau<double>;
template class SFTableau<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
int NumSequences(int, const ExtForm<double> &);
int NumSequences(int, const ExtForm<gRational> &);
int NumInfosets(int, const ExtForm<double> &);
int NumInfosets(int, const ExtForm<gRational> &);
class SFTableau<double>;
class SFTableau<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__





