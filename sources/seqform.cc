//#
//# FILE: seqform.cc -- Sequence Form module
//#
//# $Id$ 
//#

#include "rational.h"
#include "gwatch.h"
#include "gdpvect.h"

#include "seqform.h"
#include "behav.h"
#include "player.h"
#include "infoset.h"

//---------------------------------------------------------------------------
//                        SeqFormParams: member functions
//---------------------------------------------------------------------------

SeqFormParams::SeqFormParams(void) 
  :  plev(0), nequilib(0), output(&gnull)
{ }

//
// Lemke is the most important routine.
// It implements the Lemke-Howson algorithm, as refined by Eaves.
// It is assumed that the starting point is a complementary basic
// feasible solution.  If not it returns 0 without doing anything.
//


template <class T> int SeqFormModule<T>::Lemke(int dup)
{
  BFS<T> cbfs((T) 0);
  int i;
  
  if (EF.NumPlayers() != 2 || !params.output)   return 0;
  
  gWatch watch;
  
  List = BFS_List();
  
  SFTableau<T> B(EF);
  B.LCPPath();
  Add_BFS(B);
  
  if (params.plev >= 2)  {
    for (i = 1; i <= List.Length(); i++)   {
      List[i].Dump(*params.output);
      (*params.output) << "\n";
    }
  }
  gVector<T> sol(B.MinRow(),B.MaxRow());
  BehavProfile<T> profile(EF);
  B.BasisVector(sol);
  B.GetProfile(profile,sol,EF.RootNode(),1,1);
//  gout << "\nprofile = " << profile << "\n";
  solutions.Flush();
  solutions.Append(profile);
    
//  if(params.plev >= 1)
//    (*params.output) << "\nN Pivots = " << npivots << "\n";
  
  time = watch.Elapsed();
  return List.Length();
}

template <class T> int SeqFormModule<T>::Add_BFS(const SFTableau<T> &B)
{
  BFS<T> cbfs((T) 0);
  gVector<T> v(B.MinRow(), B.MaxRow());
  B.BasisVector(v);

  for (int i = B.MinCol(); i <= B.MaxCol(); i++)
    if (B.Member(i)) {
      cbfs.Define(i, v[B.Find(i)]);
    }

  if (List.Contains(cbfs))  return 0;
//  if(params.plev >=2) (*params.output) << "\nFound CBFS";
//  (*params.output)  << "\nB = ";
//  B.Dump(*params.output);
//  (*params.output)  << "\ncbfs = ";
//  cbfs.Dump(*params.output );
  List.Append(cbfs);
  return 1;
}

//-------------------------------------------------------------------------
//                   SeqFormModule<T>: Returning solutions
//-------------------------------------------------------------------------

template <class T> 
const gList<BehavProfile<T> > &SeqFormModule<T>::GetSolutions(void) const
{
  return solutions;
}

template <class T> int SeqFormModule<T>::NumPivots(void) const
{
  return npivots;
}

//-------------------------------------------------------------------------
//                    SeqFormModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
SeqFormModule<T>::SeqFormModule(const ExtForm<T> &E, const SeqFormParams &p)
  : EF(E), params(p), npivots(0)
{ }

template <class T> SeqFormModule<T>::~SeqFormModule()
{ }

template <class T> double SeqFormModule<T>::Time(void) const
{
  return time;
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


//-------------------------------------------------------------------------
//                    Convenience functions for Sequence Form
//-------------------------------------------------------------------------

template <class T>
int SeqForm(const ExtForm<T> &E, const SeqFormParams &p,
	  gList<BehavProfile<T> > &solutions,
	  int &npivots, double &time)
{ 
  SeqFormModule<T> SM(E, p);
  int result = SM.Lemke();

  npivots = SM.NumPivots();
  time = SM.Time();
  
//  solutions = SM.GetSolutions();

  return result;
}

#ifdef __GNUG__
template int SeqForm(const ExtForm<double> &, const SeqFormParams &,
		   gList<BehavProfile<double> > &, int &, double &);
template int SeqForm(const ExtForm<gRational> &, const SeqFormParams &,
		   gList<BehavProfile<gRational> > &, int &, double &);
#elif defined __BORLANDC__
#pragma option -Jgd
int SeqForm(const ExtForm<double> &, const SeqFormParams &,
	  gList<BehavProfile<double> > &, int &, double &);
int SeqForm(const ExtForm<gRational> &, const SeqFormParams &,
	  gList<BehavProfile<gRational> > &, int &, double &);
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__




