//#
//# FILE: seqform.cc -- Sequence Form module
//#
//# $Id$ 
//#

#include "seqform.h"
#include "rational.h"
#include "gwatch.h"
#include "gdpvect.h"

#include "behav.h"
#include "efplayer.h"
#include "infoset.h"


//---------------------------------------------------------------------------
//                        SeqFormParams: member functions
//---------------------------------------------------------------------------

SeqFormParams::SeqFormParams(gStatus &status_) 
  : trace(0), stopAfter(0), tracefile(&gnull), status(status_)
{ }


//---------------------------------------------------------------------------
//                        SeqFormModule: member functions
//---------------------------------------------------------------------------

template <class T>
SeqFormModule<T>::SeqFormModule(const Efg<T> &E, const SeqFormParams &p, 
			      const EFSupport &S)
  : EF(E), support(S), params(p), A(0), b(0), maxpay((T) 0), npivots(0)
{ 
  int ntot;
  ns1=S.NumSequences(1);
  ns2=S.NumSequences(2);
  ni1=E.PlayerList()[1]->NumInfosets()+1;
  ni2=E.PlayerList()[2]->NumInfosets()+1;

  ntot = ns1+ns2+(ni1+ni2)+2;
  A = new gMatrix<T>(1,ntot,0,ntot);
  b = new gVector<T>(1,ntot);
  T newpay;
  int i,j;
  for(i=1;i<=EF.NumOutcomes();i++)
    for(j=1;j<=EF.NumPlayers();j++) {
      newpay = ( (OutcomeVector<T> &) *( EF.OutcomeList()[i] ))[j];
      if(newpay>=maxpay) maxpay=newpay;
    }
  maxpay=maxpay+(T)1;
  T prob = (T)1;
  for(i=A->MinRow();i<=A->MaxRow();i++) {
    (*b)[i] = (T)0;
    for(j=A->MinCol();j<=A->MaxCol();j++)
      (*A)(i,j) = (T)0;
  }

  FillTableau(EF.RootNode(),prob,1,1,0,0);
  for(i=A->MinRow();i<=A->MaxRow();i++) 
    (*A)(i,0) = -(T)1;
  (*A)(1,ns1+ns2+1) = (T)1;
  (*A)(ns1+ns2+1,1) = -(T)1;
  (*A)(ns1+1,ns1+ns2+ni1+1) = (T)1;
  (*A)(ns1+ns2+ni1+1,ns1+1) = -(T)1;
  (*b)[ns1+ns2+1] = (T)1;
  (*b)[ns1+ns2+ni1+1] = (T)1;
//  gout.SetWidth(1).SetPrec(1);
//  gout << "\n";
//  A->Dump(gout);
//  b->Dump(gout);

  tab = new LTableau<T>(*A,*b);
  eps = tab->Epsilon();
//  tab->Refactor();  // not necessary?
  tab->Pivot(ns1+ns2+ni1+1,0);
//  tab->Dump(gout);
}

template <class T> SeqFormModule<T>::~SeqFormModule()
{ if(A) delete A; if(b) delete b; if(tab) delete tab;}

//
// Lemke is the most important routine.
// It implements the Lemke-Howson algorithm, as refined by Eaves.
// It is assumed that the starting point is a complementary basic
// feasible solution.  If not it returns 0 without doing anything.
//


template <class T> int SeqFormModule<T>::Lemke(int /*dup*/)
{
  BFS<T> cbfs((T) 0);
  int i;
  
  if (EF.NumPlayers() != 2 || !params.tracefile)   return 0;
  
  gWatch watch;
  
  List = BFS_List();
  LCPPath();
  Add_BFS(*tab);
  
  if (params.trace >= 2)  {
    for (i = 1; i <= List.Length(); i++)   {
      List[i].Dump(*params.tracefile);
      (*params.tracefile) << "\n";
    }
  }
  gVector<T> sol(tab->MinRow(),tab->MaxRow());
  BehavProfile<T> profile(EF);
//  gout << "\nsol = " << sol;
  tab->BasisVector(sol);
//  gout << "\nsol = " << sol;
  GetProfile(profile,sol,EF.RootNode(),1,1);
//  gout << "\nprofile = " << profile << "\n";
  solutions.Flush();
  solutions.Append(BehavSolution<T>(profile, id_SEQFORM));
  
//  if(params.trace >= 1)
//    (*params.tracefile) << "\nN Pivots = " << npivots << "\n";

  npivots=tab->NumPivots();
  time = watch.Elapsed();
  return List.Length();
}

template <class T> int SeqFormModule<T>::Add_BFS(const LTableau<T> &tab)
{
  BFS<T> cbfs((T) 0);
  gVector<T> v(tab.MinRow(), tab.MaxRow());
  tab.BasisVector(v);

  for (int i = tab.MinCol(); i <= tab.MaxCol(); i++)
    if (tab.Member(i)) {
      cbfs.Define(i, v[tab.Find(i)]);
    }

  if (List.Contains(cbfs))  return 0;
//  if(params.trace >=2) (*params.tracefile) << "\nFound CBFS";
//  (*params.tracefile)  << "\nB = ";
//  tab.Dump(*params.tracefile);
//  (*params.tracefile)  << "\ncbfs = ";
//  cbfs.Dump(*params.tracefile );
  List.Append(cbfs);
  return 1;
}

//-------------------------------------------------------------------------
//                   SeqFormModule<T>: Returning solutions
//-------------------------------------------------------------------------

template <class T> 
const gList<BehavSolution<T> > &SeqFormModule<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T> long SeqFormModule<T>::NumPivots(void) const
{
  return npivots;
}

template <class T> double SeqFormModule<T>::Time(void) const
{
  return time;
}



template <class T> void SeqFormModule<T>
::FillTableau(const Node *n, T prob,int s1,int s2, int i1,int i2)
{
//  gout << "\ns1,s2,i1,i2: " << s1 << " " << s2  << " " << i1  << " " << i2;
//  gout << " prob = " << prob;
  int i,snew;
  if(n->GetOutcome()) {
    (*A)(s1,ns1+s2) = (*A)(s1,ns1+s2) +
       prob*(((OutcomeVector<T> &) *n->GetOutcome())[1] -maxpay);
    (*A)(ns1+s2,s1) = (*A)(ns1+s2,s1) +
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

      (*A)(s1,ns1+ns2+i1+1) = -(T)1;
      (*A)(ns1+ns2+i1+1,s1) = (T)1;
      for(i=1;i<=n->NumChildren();i++) {
	(*A)(snew+i,ns1+ns2+i1+1) = (T)1;
	(*A)(ns1+ns2+i1+1,snew+i) = -(T)1;
	FillTableau(n->GetChild(i),prob,snew+i,s2,i1,i2);
      }
    }
    if(pl==2) {
      i2=n->GetInfoset()->GetNumber();
      snew=1;
      for(i=1;i<i2;i++)
	snew+=n->GetPlayer()->InfosetList()[i]->NumActions();
      (*A)(ns1+s2,ns1+ns2+ni1+i2+1) = -(T)1;
      (*A)(ns1+ns2+ni1+i2+1,ns1+s2) = (T)1;
      for(i=1;i<=n->NumChildren();i++) {
	(*A)(ns1+snew+i,ns1+ns2+ni1+i2+1) = (T)1;
	(*A)(ns1+ns2+ni1+i2+1,ns1+snew+i) = -(T)1;
	FillTableau(n->GetChild(i),prob,s1,snew+i,i1,i2);
      }
    }
    
  }
}

template <class T> void SeqFormModule<T>
::GetProfile(gDPVector<T> &v, const gVector<T> &sol,
	       const Node *n, int s1,int s2)
{
  int i,pl,inf,snew,ind,ind2;
//  T eps;
//  eps = tab->Epsilon();
//  gout << "\nv = " << v;
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	GetProfile(v,sol,n->GetChild(i),s1,s2);
    }
    pl = n->GetPlayer()->GetNumber();
    inf= n->GetInfoset()->GetNumber();
//    gout << "\niset: (" << pl << "," << inf << ")"; 
    if(pl==1) {
      snew=1;
      for(i=1;i<inf;i++)
	snew+=n->GetPlayer()->InfosetList()[i]->NumActions(); 
      for(i=1;i<=n->NumChildren();i++) {
	v(pl,inf,i) = (T)0;
//	gout << "\npl1 ";
//	gout << "\n  v = " << v;
	if(tab->Member(s1)) {
	  ind = tab->Find(s1);
//	  if(sol[ind]!=(T)0) {
	  if(sol[ind]<-eps) {
	    if(tab->Member(snew+i)) {
	      ind2 = tab->Find(snew+i);
	      if(sol[ind2]<-eps)
		v(pl,inf,i) = sol[ind2]/sol[ind];
//	      gout << "\nind: " << ind << " " << sol[ind] << " ";
//	      gout << "\nind2: " << ind2 << " " << sol[ind2] << " ";
//	      gout << "\n  v = " << v;
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
//	gout << "\npl2 ";
//	gout << "\n  v = " << v;
	if(tab->Member(ns1+s2)) {
	  ind = tab->Find(ns1+s2);
//	  if(sol[ind]!=(T)0) {
	  if(sol[ind]<-eps) {
	    if(tab->Member(ns1+snew+i)) {
	      ind2 = tab->Find(ns1+snew+i);
	      if(sol[ind2]<-eps)
		v(pl,inf,i) = sol[ind2]/sol[ind];
//	      gout << "\nind: " << ind << " " << sol[ind] << " ";
//	      gout << "\nind2: " << ind2 << " " << sol[ind2] << " ";
//	      gout << "\n  v = " << v;
	    }
	  } 
	} 
	GetProfile(v,sol,n->GetChild(i),s1,snew+i);
      }
    }
  }
}

template <class T> int SeqFormModule<T>::LCPPath()
{
//  if (!At_CBFS())  return 0;
  int enter, exit;
  enter = ns1+ns2+ni1+1;
//  if(params.trace >=2) {
//    (*params.tracefile) << "\nbegin LCP path: enter = " << enter << "\n";
//    Dump(*params.tracefile);
//  }
//  gout << "\nbegin LCP path: enter = " << enter << "\n";
//  tab->Dump(gout);
  
//  enter = dup;
//  if (Member(dup))
//    enter = -dup;
      // Central loop - pivot until another CBFS is found
  long nits = 0;
  do  {
    // Talk about optimism! This is dumb, but better than nothing (I guess):
    params.status.SetProgress((double)nits/(double)(nits+1)); 
    nits++;
    exit = tab->PivotIn(enter);
//    if(params.trace >=2)
//      Dump(*params.tracefile);
    
//    tab->Dump(gout);
    
    enter = -exit;
  } while (exit != 0 && !params.status.Get());
      // Quit when at a CBFS.
      //  if(params.trace >=2 ) (*params.tracefile) << "\nend of path " << dup;
//  gout << "\nend of path ";
  return 1;
}

#ifdef __GNUG__
template class SeqFormModule<double>;
template class SeqFormModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class SeqFormModule<double>;
class SeqFormModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__



//-----------------------------------
// Interfacing to solve-by-subgame
//-----------------------------------

template <class T>
int SeqFormBySubgame<T>::SolveSubgame(const Efg<T> &E,
				      gList<BehavSolution<T> > &solns)
{
  BehavProfile<T> bp(E);

  SeqFormModule<T> M(E, params, bp.GetEFSupport());
  
  M.Lemke();

  npivots += M.NumPivots();

  solns = M.GetSolutions();
  return params.status.Get();
}

template <class T>
SeqFormBySubgame<T>::SeqFormBySubgame(const Efg<T> &E, const SeqFormParams &p,
				      int max)
  : SubgameSolver<T>(E, max), npivots(0), params(p)
{ }

template <class T>  SeqFormBySubgame<T>::~SeqFormBySubgame()   { }
  


#ifdef __GNUG__
template class SeqFormBySubgame<double>;
template class SeqFormBySubgame<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class SeqFormBySubgame<double>;
class SeqFormBySubgame<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__













