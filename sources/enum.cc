//#
//# FILE: enum.cc -- Enum module
//#
//# @(#)enum.cc	1.36 3/2/95
//#

#include "rational.h"
#include "gwatch.h"
#include "gpvector.h"

#include "normal.h"
#include "normiter.h"

#include "enum.h"

//---------------------------------------------------------------------------
//                        EnumParams: member functions
//---------------------------------------------------------------------------

EnumParams::EnumParams(void) : plev(0), nequilib(0),outfile(0), 
errfile(0),sig(gbreak)
{ }

//-------------------------------------------------------------------------
//               EnumTableau<T>: constructor and destructor
//-------------------------------------------------------------------------

template <class T> EnumTableau<T>::EnumTableau(void) 
  : Tableau<T>(), n1(0), n2(0)
{ } 

template <class T> EnumTableau<T>::EnumTableau(const NormalForm<T>&NF) 
  : Tableau<T>(NF.NumStrats(1) + NF.NumStrats(2),
	       NF.NumStrats(1) + NF.NumStrats(2)), 
	       n1(NF.NumStrats(1)), n2(NF.NumStrats(2))
{ 
  NormalIter<T>
    iter(NF); T min = (T) 0, x; int i;
  
  for (i = 1; i <= n1; i++)   {
    for (int j = 1; j <= n2; j++)  {
      x = iter.Payoff(1);
      if (x < min)   min = x;
      x = iter.Payoff(2);
      if (x < min)   min = x;
      iter.Next(2);
    }
    iter.Next(1);
  }

  min-=(T)1;
  
  for (i = 1; i <= n1; i++) 
    for (int j = 1; j <= n1; j++) 
      A(i, j) = (T)0;
  
  for (i = n1 + 1; i <= n1 + n2; i++)
    for (int j = n1 + 1; j <= n1 + n2; j++)
      A(i, j) = (T)0;
  
  for (i = 1; i <= n1; i++)  {
    for (int j = 1; j <= n2; j++)  {
      A(i, n1 + j) = iter.Payoff(1) - min;
      A(n1 + j, i) = iter.Payoff(2) - min;
      iter.Next(2);
    }
    iter.Next(1);
  }
  for (i = 1; i <= n1 + n2; i++) 
    b[i]=-1.0;
//  A.Dump(gout);
//  b.Dump(gout);

}

template <class T> int EnumTableau<T>::MinRow(void) const
{
return A.MinRow();
}

template <class T> int EnumTableau<T>::MaxRow(void) const
{
return A.MaxRow();
}

template <class T> EnumTableau<T>::~EnumTableau()
{ }


#ifdef __GNUG__
template class EnumTableau<double>;
template class EnumTableau<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class EnumTableau<double>;
class EnumTableau<gRational>;
#pragma option -Jgx
#endif   // __GNUG__

//-------------------------------------------------------------------------
//                    EnumModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
EnumModule<T>::EnumModule(const NormalForm<T> &N, const EnumParams &p)
  : tab(N), params(p), rows(N.NumStrats(1)), cols(N.NumStrats(2)), 
    npivots(0),level(0),count(0),List(),players(N.NumPlayers())
{ }

template <class T> int EnumModule<T>::Enum(void)
{
      // Ted -- is there a better way to do this?  A lot of 
      //        allocation before finding out there are too 
      //        many players. (Same in Lemke module I think)
  if (players != 2)   return 0;  

  gWatch watch;

  gTuple<int> target(rows+cols);
  for(int i=1;i<=target.Length();i++) 
    target[i]=i;

  Basis<T> basis(tab);

  i = rows+1;
  
  while(i<=rows+cols && !params.sig.Get()) 
    if(params.nequilib==0 || List.Length()<params.nequilib) {
      SubSolve(rows,i,basis,target);
      i++;
    }
  
  if(params.sig.Get()) {
    gout << "\n User Break \n";
    params.sig.Reset();
  }
  for(i=1;i<=List.Length();i++) {
    gout << "\n";
    List[i].Dump(gout);
  }
  time = (gRational) watch.Elapsed();
  return 1;
}


template <class T> void EnumModule<T>
::SubSolve(int pr, int pcl, Basis<T> &B1, gTuple<int> &targ1)
{
  int i,j,ii,jj,pc;
  count++;

  Basis<T> B2(B1);

      // construct new target basis
  gTuple<int> targ2(targ1);  
  pc = targ1.Find(pcl);
  targ2[pc] = targ2[pr];
  targ2[pr] = pcl;

//  gout << "\n targ = ";
//  targ2.Dump(gout);


      /* pivot to target */
  int flag = 1;
  int piv = 1;
  while(piv && flag) {
    piv=0;
    flag=0;
    for(i=1;i<=rows;i++) {
      ii = targ2[i];
      if(ii<=rows)ii=-ii;
      if(!B2.Member(ii)) {
//	gout << " i,ii : " << i << ii;
	j=rows+1;
	jj = targ2[j];
	if(jj<=rows)jj=-jj;
	while(j<=rows+cols && !B2.Member(jj)) {
	  j++;
	  jj = targ2[j];
	  if(jj<=rows)jj=-jj;
	}
//	gout << " j,jj : " << j << jj;
	if(j<=rows+cols) {
	      // note: may want to pivot for 1 and 2 separately to pick 
	      // up additional possible feasible solutions.  
	  if(B2.CanPivot(jj,ii) && B2.CanPivot(-ii,-jj))  {
//	    gout << " jj,ii, : " << i << j << ii << jj;
	    B2.CompPivot(jj,ii);
	    piv=1;
	  }
	  else flag=1;
	}
      }
    }
  }
//  gout << "\n";
//  B2.Dump(gout);


  j=0;
  if(B2.IsNash()) {
    List.Append(B2.GetBFS());
    j=1;
  }
     
  if(params.plev>=3) {
    printf("\nPass# %3ld, Depth =%3d, Target = ",
	   count, rows-pr+1);
    for(i=1;i<=rows;i++)
      printf("%3d", targ2[i]);
    if(flag) {
      printf("  Infeasible");
//      B2.Dump(gout); 
    }
    if(j) {
      printf("  Nash equilib");    
      B2.Dump(gout); 
    }
  }
  
  if(flag) B2=B1;

  if(pr>1) {
    i=targ2[pr-1];
    while(i+1<targ2[pr] && !params.sig.Get())
      if(params.nequilib==0 || List.Length()<params.nequilib) {
	i++;
	SubSolve(pr-1,i,B2,targ2);
      }
  }
}

/*
template <class T>
gList<gPVector<T> > &EnumTableau<T>::AddSolution(void) const
{
  gTuple<int> dim(2);
  dim[1] = n1;
  dim[2] = n2;
  
  gPVector<T> profile(dim);
  T sum = (T) 0;
  
  for (int j = 1; j <= N.NumStrats(1); j++)
    if (List.IsDefined(j))   sum += List(j);
  
  if (sum == (T) 0)  continue;
  
  for (j = 1; j <= N.NumStrats(1); j++) 
    if (List.IsDefined(j))   profile(1, j) = List(j) / sum;
    else  profile(1, j) = (T) 0;
  
  sum = (T) 0;
  
  for (j = 1; j <= N.NumStrats(2); j++)
    if (List[i].IsDefined(N.NumStrats(1) + j))  
      sum += List[i](N.NumStrats(1) + j);
  
  if (sum == (T) 0)  continue;
  
  for (j = 1; j <= N.NumStrats(2); j++)
    if (List[i].IsDefined(N.NumStrats(1) + j))
      profile(2, j) = List[i](N.NumStrats(1) + j) / sum;
    else
      profile(2, j) = (T) 0;
  
  solutions.Append(profile);
}
*/

template <class T> int EnumModule<T>::NumPivots(void) const
{
  return npivots;
}

template <class T> gRational EnumModule<T>::Time(void) const
{
  return time;
}

template <class T> EnumParams &EnumModule<T>::Parameters(void)
{
  return params;
}

//template <class T> void EnumModule<T>
//::GetSolutions(gList<gPVector<T> > &solutions) const

template <class T> gList<gPVector<T> > &EnumModule<T>
::GetSolutions(void) const
{
  gList<gPVector<T> > *solutions;
  solutions = new gList<gPVector<T> >;
//  solutions.Flush();

  for (int i = 1; i <= List.Length(); i++)    {
    gTuple<int> dim(2);
    dim[1] = rows;
    dim[2] = cols;

    gPVector<T> profile(dim);
    T sum = (T) 0;

    for (int j = 1; j <= rows; j++)
      if (List[i].IsDefined(j))   sum += List[i](j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= rows; j++) 
      if (List[i].IsDefined(j))   profile(1, j) = List[i](j) / sum;
      else  profile(1, j) = (T) 0;

    sum = (T) 0;

    for (j = 1; j <= cols; j++)
      if (List[i].IsDefined(rows + j))  
	sum += List[i](rows + j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= cols; j++)
      if (List[i].IsDefined(rows + j))
	profile(2, j) = List[i](rows + j) / sum;
      else
	profile(2, j) = (T) 0;

    solutions->Append(profile);
  }
  return *solutions;
}


/*
template <class T>
const gList<gPVector<T> > &EnumModule<T>::GetSolutions(void) const
{
  return solutions;
}


template <class T>
void EnumModule<T>::AddSolution(const gPVector<T> &s)
{
  solutions.Append(s);
}
*/
#ifdef __GNUG__
template class EnumModule<double>;
template class EnumModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class EnumModule<double>;
class EnumModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


//-------------------------------------------------------------------------
//                    Convenience functions for Enum
//-------------------------------------------------------------------------

template <class T>
int Enum(const NormalForm<T> &N, const EnumParams &p,
	  gList<gPVector<T> > &solutions,
	  int &npivots, gRational &time)
{
  EnumModule<T> LM(N, p);
  int result = LM.Enum();

  npivots = LM.NumPivots();
  time = LM.Time();
  
  LM.GetSolutions();

  return result;
}

#ifdef __GNUG__
template int Enum(const NormalForm<double> &, const EnumParams &,
		   gList<gPVector<double> > &, int &, gRational &);
template int Enum(const NormalForm<gRational> &, const EnumParams &,
		   gList<gPVector<gRational> > &, int &, gRational &);
#elif defined __BORLANDC__
#pragma option -Jgd
int Enum(const NormalForm<double> &, const EnumParams &,
	  gList<gPVector<double> > &, int &, gRational &);
int Enum(const NormalForm<gRational> &, const EnumParams &,
	  gList<gPVector<gRational> > &, int &, gRational &);
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__










