//
// FILE: lhtab.cc -- Lemke-Howson tableau
//
// $Id$
//

#include "lhtab.h"
#include "nfg.h"
#include "nfgiter.h"
#include "nfstrat.h"

//---------------------------------------------------------------------------
//                        LemkeHowson Tableau: member functions
//---------------------------------------------------------------------------

template <class T> gMatrix<T> Make_A1(const Nfg &N, const NFSupport &S,
                                      const T &)
{
  int n1, n2, i,j;
  n1=S.NumStrats(1);
  n2=S.NumStrats(2);
  gMatrix<T> A1(1,n1,n1+1,n1+n2);
  NfgIter iter(S); 
  T min; 

  min = MinPayoff(N) - gNumber(1);

  for (i = 1; i <= n1; i++)  {
    for (j = 1; j <= n2; j++)  {
      A1(i, n1 + j) = N.Payoff(iter.GetOutcome(), 1) - gNumber(min);
      iter.Next(2);
    }
    iter.Next(1);
  }
  return A1;
}

template <class T> gMatrix<T> Make_A2(const Nfg &N, const NFSupport &S,
                                      const T &)
{
  int n1, n2, i,j;
  n1=S.NumStrats(1);
  n2=S.NumStrats(2);
  gMatrix<T> A2(n1+1,n1+n2,1,n1);
  NfgIter iter(S); 
  T min; 

  min = MinPayoff(N) - gNumber(1);

  for (i = 1; i <= n1; i++)  {
    for (j = 1; j <= n2; j++)  {
      A2(n1 + j, i) = N.Payoff(iter.GetOutcome(), 2) - gNumber(min);
      iter.Next(2);
    }
    iter.Next(1);
  }
  return A2;
}

template <class T> gVector<T> Make_b1(const Nfg &, const NFSupport &S,
				      const T &)
{
  int n1, n2, i;
  n1=S.NumStrats(1);
  n2=S.NumStrats(2);
  gVector<T> b1(1,n1);

  for (i = 1; i <= n1; i++) 
    b1[i]=-(T)1;
  return b1;
}

template <class T> gVector<T> Make_b2(const Nfg &, const NFSupport &S, const T &)
{
  int n1, n2, i;
  n1=S.NumStrats(1);
  n2=S.NumStrats(2);
  gVector<T> b2(n1+1,n1+n2);

  for (i = n1+1; i <= n1+n2; i++) 
    b2[i]=-(T)1;
  return b2;
}

template <class T>
LHTableau<T>::LHTableau(const gMatrix<T> &A1, const gMatrix<T> &A2, 
			const gVector<T> &b1, const gVector<T> &b2)
  : T1(A1,b1), T2(A2,b2), 
//    tmpcol(b1.First(),b2.Last()), 
    tmp1(b1.First(),b1.Last()),tmp2(b2.First(),b2.Last()),
    solution(b1.First(), b2.Last())
{ }

template <class T> LHTableau<T>
::LHTableau(const LHTableau<T> &orig) 
  : T1(orig.T1), T2(orig.T2), 
//    tmpcol(orig.tmpcol), 
    tmp1(orig.tmp1),
    tmp2(orig.tmp2), solution(orig.solution)
{ }

template <class T> LHTableau<T>::~LHTableau(void) 
{ }

template <class T>
LHTableau<T>& LHTableau<T>::operator=(const LHTableau<T> &orig)
{
//  assert( A==orig.A );
  if(this!= &orig) {
    T1 = orig.T1;
    T2 = orig.T2;
//    tmpcol = orig.tmpcol;
    tmp1 = orig.tmp1;
    tmp2 = orig.tmp2;
    solution = orig.solution;
  }
  return *this;
}

template <class T>
int LHTableau<T>::MinRow() const { return T1.MinRow(); }

template <class T>
int LHTableau<T>::MaxRow() const { return T2.MaxRow(); }

template <class T>
int LHTableau<T>::MinCol() const { return T2.MinCol(); }

template <class T>
int LHTableau<T>::MaxCol() const { return T1.MaxCol(); }

template <class T>
T LHTableau<T>::Epsilon() const { return T1.Epsilon(); }


template <class T>
bool LHTableau<T>::Member(int i) const
{return (T1.Member(i) || T2.Member(i));}

template <class T>
int LHTableau<T>::Label(int i) const
{ 
  if(T1.RowIndex(i)) return T1.Label(i);
  if(T2.RowIndex(i)) return T2.Label(i);
  return 0;
}

template <class T>
int LHTableau<T>::Find(int i) const
{ 
  if(T1.ValidIndex(i)) return T1.Find(i);
  if(T2.ValidIndex(i)) return T2.Find(i);
  return 0;
}

//
// pivoting operations
//

template <class T>
int LHTableau<T>::CanPivot(int outlabel, int inlabel)
{
  if(T1.ValidIndex(outlabel)) {
    if(T1.CanPivot(outlabel,inlabel)) return 1;}
  else if(T2.ValidIndex(outlabel)) { 
    if(T2.CanPivot(outlabel,inlabel)) return 1;}
  return 0;
}

template <class T>
void LHTableau<T>::Pivot(int outrow,int inlabel)
{
  assert( RowIndex(outrow) );
  if(T1.RowIndex(outrow)) T1.Pivot(outrow,inlabel);
  if(T2.RowIndex(outrow)) T2.Pivot(outrow,inlabel);
}

template <class T> long LHTableau<T>::NumPivots() const
{ return T1.NumPivots() + T2.NumPivots(); }

//
// raw Tableau functions
//

template <class T> void LHTableau<T>::Refactor()
{
  T1.Refactor();
  T2.Refactor();
}

/*
    // solve A x = b 
template <class T>
void LHTableau<T>::Solve(const gVector<T> &b, gVector<T> &x) const
{
  B.solve(b,x);
}

    // solve y A = c 
template <class T>
void LHTableau<T>::SolveT(const gVector<T> &c, gVector<T> &y) const
{
  B.solveT(c,y);
}

template <class T>
void LHTableau<T>::BasisVector(gVector<T> &out) const
{
  T1.BasisVector(tmp1);
  T2.BasisVector(tmp2);
  for(i=tmp1.First;i<=tmp1.Last;i++)
    out[i] = tmp1[i];
  for(i=tmp2.First;i<=tmp2.Last;i++)
    out[i] = tmp2[i];
//  out= solution;
}

template <class T>
void LHTableau<T>::SolveColumn(int col, gVector<T> &out)
{
  gVector<T> tmpcol2(MinRow(),MaxRow());
  if( col<0 ) {
    tmpcol2= (T)0;
    tmpcol2[-col]=1;
  }
  else
    A->GetColumn(col,tmpcol2);
  B.solve(tmpcol2,out);
}


template <class T>
void LHTableau<T>::SetBasis(const Basis<T> &in)
{
  basis= in;
  B.refactor(template->A, code.basis);
  B.solve(template->b, solution);
  if( costdefined )
    SolveDual();
}


template <class T>
void LHTableau<T>::GetBasis(Basis<T> &out) const
{
  out= basis;
}
*/

// miscellaneous functions

template <class T>
BFS<T> LHTableau<T>::GetBFS()
{
  int i;
  T1.BasisVector(tmp1);
  T2.BasisVector(tmp2);
  for(i=tmp1.First();i<=tmp1.Last();i++)
    solution[i] = tmp1[i];
  for(i=tmp2.First();i<=tmp2.Last();i++)
    solution[i] = tmp2[i];
  BFS<T> cbfs((T) 0);
  for(i=MinCol();i<=MaxCol();i++) {
    if(Member(i)) 
      cbfs.Define(i,solution[Find(i)]);
  }
  return cbfs;
}

template <class T> gOutput &operator<<(gOutput &to, const LHTableau<T> &v)
{
  v.Dump(to); return to;
}

template <class T>
void LHTableau<T>::Dump(gOutput &to) const
{ T1.Dump(to);gout << "\n"; T2.Dump(to); }


template <class T> int LHTableau<T>::PivotIn(int inlabel)
{ 
//  gout << "\n inlabel = " << inlabel;
  int outindex = ExitIndex(inlabel);
  int outlabel = Label(outindex);
  if(outlabel==0)return 0;
//  gout << "\n outlabel = " << outlabel;
//  gout << " outindex = " << outindex << "\n\n";
  Pivot(outindex,inlabel);
  return outlabel;
}

//
// ExitIndex determines, for the current tableau and variable to
// to be added to the basis, which element should leave the basis.
// The choice is the one specified by Eaves, which is guaranteed
// to not cycle, even if the problem is degenerate.
//


template <class T> int LHTableau<T>::ExitIndex(int inlabel)
{
  if(T1.ValidIndex(inlabel)) return T1.ExitIndex(inlabel);
  if(T2.ValidIndex(inlabel)) return T2.ExitIndex(inlabel);
  return 0;
}

//
// Executes one step of the Lemke-Howson algorithm
//

template <class T> int LHTableau<T>::LemkePath(int dup)
{
//  if (!At_CBFS())  return 0;
  int enter, exit;
//  if(params.plev >=2) {
//    (*params.output) << "\nbegin path " << dup << "\n";
//    Dump(*params.output); 
//  }
//    (gout) << "\nbegin path " << dup << "\n";
//    Dump(gout); 
  enter = dup;
  if (Member(dup)) {
//    gout << "\ndup is member";
    enter = -dup;
  }
      // Central loop - pivot until another CBFS is found
  do  { 
    exit = PivotIn(enter);
//    if(params.plev >=2) 
//      Dump(*params.output);
//      Dump(gout);

    enter = -exit;
  } while ((exit != dup) && (exit != -dup));
      // Quit when at a CBFS.
//  if(params.plev >=2 ) (*params.output) << "\nend of path " << dup;
//  gout << "\nend of path " << dup;
  return 1;
}



template class LHTableau<double>;
template class LHTableau<gRational>;
template gMatrix<double> Make_A1(const Nfg &, const NFSupport &, const double &);
template gMatrix<gRational> Make_A1(const Nfg &, const NFSupport &, const gRational &);
template gVector<double> Make_b1(const Nfg &, const NFSupport &, const double &);
template gVector<gRational> Make_b1(const Nfg &, const NFSupport &, const gRational &);
template gMatrix<double> Make_A2(const Nfg &, const NFSupport &, const double &);
template gMatrix<gRational> Make_A2(const Nfg &, const NFSupport &, const gRational &);
template gVector<double> Make_b2(const Nfg &, const NFSupport &, const double &);
template gVector<gRational> Make_b2(const Nfg &, const NFSupport &, const gRational &);





