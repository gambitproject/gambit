//#
//# FILE: lemke.cc -- Lemke-Howson module
//#
//# $Id$
//#

#include "rational.h"
#include "gwatch.h"
#include "gpvector.h"

#include "nfg.h"
#include "nfgiter.h"

#include "gtableau.h"

#include "lemke.h"

//---------------------------------------------------------------------------
//                        LemkeParams: member functions
//---------------------------------------------------------------------------

LemkeParams::LemkeParams(gStatus &status_) 
  : trace(0), stopAfter(0), output(&gnull),status(status_)
{ }

//---------------------------------------------------------------------------
//                    LemkeTableau<T>: class definition
//---------------------------------------------------------------------------

template <class T> class LemkeTableau : public gTableau<T> 
{
  private:
    const Nfg<T> &N;
    const NFSupport &support;
    int num_strats;
    gOutput &output;
    int printlevel;
    long npivots;
    BFS_List List;
    gStatus &status;
   
    int Lemke_Step(int);
    int At_CBFS(void) const;
    int All_Lemke(BFS_List &List, int j, long &np);
    int Exit_Row(int col);
    void Pivot(int, int);
 
  public:
    LemkeTableau(const Nfg<T> &, const NFSupport &, 
		 gOutput &ofile, int trace, gStatus &status_=gstatus);
    virtual ~LemkeTableau();

    int Lemke(int);
    long NumPivots(void) const;
    long &NumPivots(void);
    void GetSolutions(gList<MixedProfile<T> > &) const;
};

//-------------------------------------------------------------------------
//               LemkeTableau<T>: constructor and destructor
//-------------------------------------------------------------------------

template <class T>
LemkeTableau<T>::LemkeTableau(const Nfg<T> &NF, const NFSupport &S,
			      gOutput &ofile, int trace, 
			      gStatus &status_)
     : gTableau<T>(1, S.NumStrats(1) + S.NumStrats(2),
		   S.NumStrats(1) + S.NumStrats(2),
		   0, S.NumStrats(1) + S.NumStrats(2) + 1,
		   S.NumStrats(1) + S.NumStrats(2)),
		   N(NF), support(S),
		   num_strats(S.NumStrats(1) + S.NumStrats(2)),
		   output(ofile), printlevel(trace),
		   npivots(0), status(status_)
{
  NfgIter<T> iter(&S);
  T min = (T) 0, x;
  int n1 = S.NumStrats(1), n2 = S.NumStrats(2);
  int i;

  for (i = 1; i <= n1 + n2; i++)  {
    Col_Labels[i] = i;
    Row_Labels[i] = -i;
  }

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

  for (i = 1; i <= n1; i++) 
    for (int j = 1; j <= n1; j++) 
      Tableau(i, j) = 0.0;

  for (i = n1 + 1; i <= n1 + n2; i++)
    for (int j = n1 + 1; j <= n1 + n2; j++)
      Tableau(i, j) = 0.0;

  min-= (T)1;

  for (i = 1; i <= n1; i++)  {
    for (int j = 1; j <= n2; j++)  {
      Tableau(i, n1 + j) = iter.Payoff(1) - min;
      Tableau(n1 + j, i) = iter.Payoff(2) - min;
      iter.Next(2);
    }
    iter.Next(1);
  }

  for (i = 1; i <= n1 + n2; Tableau(i++, 0) = -1.0);
  for (i = 1; i <= n1 + n2; Tableau(i++, n1 + n2 + 1) = 0.0);
}

template <class T> LemkeTableau<T>::~LemkeTableau()
{ }


//-------------------------------------------------------------------------
//                    LemkeTableau<T>: member functions
//-------------------------------------------------------------------------

//
// Pivot implements the pivoting procedure.
// The last column begins with all 0 's, and the first step
// is to place a 1 in the row 'th entry of this column.
// The pivoting procedure from the class gMatrix is then
// invoked, the labels of the row amd col are swapped,
// the n + 2 'th and col' th columns are interchanged,
// and the(row, n + 2) entry is set to 0, which should
// result in all entries in the final column being 0.
//
template <class T> void LemkeTableau<T>::Pivot(int row, int col)
{
     // On entry the n + 1 column should be all zero.
     // To begin with we put a 1 in the row'th entry of this column
  Tableau(row, num_strats + 1) = 1;

     // Now pivot.
  Tableau.Pivot(row, col);

     // swap row and column labels
  int temp = Row_Labels[row];
  Row_Labels[row] = Col_Labels[col];
  Col_Labels[col] = temp;
 
  if (printlevel >= 3)
    Dump(output);


     // Now switch the col column and the Num_Strat + 1 (scratch) column,
     // then set the row'th entry to 0, making the whole column zero.
  Tableau.SwitchColumns(col, num_strats + 1);
  Tableau(row, num_strats + 1) = 0;
  npivots++;
}

//
// Lemke is the most important routine.
// It implements the Lemke-Howson algorithm, as refined by Eaves.
// It is assumed that the starting point is a complementary basic
// feasible solution.  If not it returns 0 without doing anything.
//
template <class T> int LemkeTableau<T>::Lemke(int Duplicate_Label)
{
  BFS<T> cbfs((T) 0);
  int i;

  List = BFS_List();
  
  if (Duplicate_Label == 0)
    All_Lemke(List, 0, npivots);
  else  {
    Lemke_Step(Duplicate_Label);
    for (i = 1; i <= num_strats && !status.Get(); i++) {
      status.SetProgress((double)(i-1)/(double)num_strats);
      if (Row_Labels[i] > 0)
	cbfs.Define(Row_Labels[i], Tableau(i,0));
    }
    List.Append(cbfs);
  }

  if(status.Get()) status.Reset();

  if (printlevel >= 2)  {
    for (i = 1; i <= List.Length(); i++)   {
      List[i].Dump(output);
      output << "\n";
    }

  }
  if(printlevel >= 1)output << "\nN Pivots = " << npivots << "\n";

  return List.Length();
}

//
// Executes one step of the Lemke-Howson algorithm
//
template <class T> int LemkeTableau<T>::Lemke_Step(int Duplicate_Label)
{
  int row, col, newcol, exit_label;

     // check if at complementary basic feasible sol
  if (!At_CBFS())  return 0;

     // Initially Duplicate_Label is thought of as a
     // strategy.  We now set it to the corresponding
     // column label.
  if (!Col_Labels.Contains(Duplicate_Label))
    Duplicate_Label = -Duplicate_Label;

     // Now find the index of Dup Label
  col = Col_Labels.Find(Duplicate_Label);

     // Central loop - pivot until another CBFS is found
  do  {
    row = Exit_Row(col);                    // The row that will leave.
    exit_label = Row_Labels[row];           // The corresponding label.
    newcol = Col_Labels.Find(-exit_label);  // The column corresponding to 
                                            // The dual label of exit_label.
    if (printlevel >= 3)  {
      Dump(output);
      output << "\npivot row = " << row << " col = " << col << "\n";
    }

    Pivot(row, col);

    if (printlevel >= 3) 
      Dump(output);

    col = newcol;                   // The new column to enter the basis.
  } while (exit_label != -Duplicate_Label && exit_label != Duplicate_Label);
	                                              // Quit when at a CBFS.

  return 1;
}

//
// At_CBFS returns true if the current basis is complementary and feasible
//
template <class T> int LemkeTableau<T>::At_CBFS(void) const
{
  // To be written later
  return 1;
}

//
// All_Lemke finds all accessible Nash equilibria by recursively 
// calling itself.  List maintains the list of basic variables 
// for the equilibria that have already been found.  
// From each new accessible equilibrium, it follows
// all possible paths, adding any new equilibria to the List.  
//
template <class T> int LemkeTableau<T>::All_Lemke(BFS_List &List, int j, long &np)
{
  BFS<T> cbfs((T) 0);
  int i,len;
  T p1,p2,aa;

  np+=NumPivots();

  for (i = 1; i <= num_strats; i++)
    if (Row_Labels[i] > 0)
      cbfs.Define(Row_Labels[i], Tableau(i,0));

  if (printlevel >= 3)  {
    output << "\npath " << j ;
    output << " Basis:  " ;
    for (i = 1; i <= num_strats; i++)
      output << Row_Labels[i] << " " ;
    output << "\n";	
  }

  if (List.Contains(cbfs))  return 1;
  List.Append(cbfs);

  if (printlevel >= 2)
    cbfs.Dump(output);

  if (printlevel >= 3)  {
    output << "\npath " << j << " Basis:  " ;
    for (i = 1; i <= num_strats; i++)
      output << Row_Labels[i] << " " ;
  }

  if (printlevel >= 2)
    output << "\n";

  if (printlevel >= 3)
    Dump(output);

  for (i = 1; i <= num_strats  && !status.Get(); i++)
    if (i != j)  {
      len=List.Length()-1;
      p1=(double)len/(double)(len+1);
      p2=(double)(len+1)/(double)(len+2);
      aa=(double)(i)/(double)num_strats;
      status.SetProgress(p1+aa*(p2-p1));
//      gout << "\n " << p1+aa*(p2-p1);

      LemkeTableau<T> Tcopy(*this);
      Tcopy.NumPivots()= 0;
      Tcopy.Lemke_Step(i);
      if (printlevel >= 3)      Tcopy.Dump(output);
      Tcopy.All_Lemke(List, i,np);
      if (printlevel >= 3)      Tcopy.Dump(output);
    }
  return 1;
}

//
// Exit_Row determines, for the current tableau and given column to
// to be added to the basis, which row should leave the basis.
// The choice is the one specified by Eaves, which is guaranteed
// to not cycle, even if the problem is degenerate.
//
template <class T> int LemkeTableau<T>::Exit_Row(int col)
{
  gBlock<int> BestSet;
  int i, c, c_col,c_row;
  T ratio, tempmax;

  // Find all row indices for which column col has positive entries.
  for (i = 1; i <= num_strats; i++)
    if (Tableau(i, col) > (T) 0)
      BestSet.Append(i);
  assert(BestSet.Length() > 0);

      // If there are multiple candidates, break ties by
      // looking at ratios with other columns, 
      // eliminating nonmaximizers of 
      // a similar ratio, until only one candidate remains.
  c = 0;
  while (BestSet.Length() > 1)   {
              // Initialize tempmax.
    assert(c <= num_strats); 	
    c_col = Col_Labels.Find(-c);
    if (c_col > 0 || c == 0)   {
      tempmax = Tableau(BestSet[1], c_col) / Tableau(BestSet[1], col);

             // Find the maximum ratio. 
      for (i = 2; i <= BestSet.Length(); i++)  {
	ratio = Tableau(BestSet[i], c_col) / Tableau(BestSet[i], col);
	if (ratio > tempmax)  tempmax = ratio;
      }

             // Remove nonmaximizers from the list of candidate columns.
      for (i = BestSet.Length(); i >= 1; i--)  {
	ratio = Tableau(BestSet[i], c_col) / Tableau(BestSet[i], col);
	if (ratio < tempmax)
	  BestSet.Remove(i);
      }
    }
    else  {
      c_row = Row_Labels.Find(-c);
      assert(c_row > 0);
      if (BestSet.Contains(c_row)) return c_row;
    }
    c++;
  }
  assert(BestSet.Length() > 0);
  return BestSet[1];
}

//-------------------------------------------------------------------------
//                   LemkeTableau<T>: Returning solutions
//-------------------------------------------------------------------------

template <class T>
void LemkeTableau<T>::GetSolutions(gList<MixedProfile<T> > &solutions) const
{
  solutions.Flush();

  for (int i = 1; i <= List.Length(); i++)    {
    MixedProfile<T> profile(N, support);
    T sum = (T) 0;
    int j;

    for (j = 1; j <= support.NumStrats(1); j++)
      if (List[i].IsDefined(j))   sum += List[i](j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= support.NumStrats(1); j++) 
      if (List[i].IsDefined(j))   profile(1, j) = List[i](j) / sum;
      else  profile(1, j) = (T) 0;

    sum = (T) 0;

    for (j = 1; j <= support.NumStrats(2); j++)
      if (List[i].IsDefined(support.NumStrats(1) + j))  
	sum += List[i](support.NumStrats(1) + j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= support.NumStrats(2); j++)
      if (List[i].IsDefined(support.NumStrats(1) + j))
	profile(2, j) = List[i](support.NumStrats(1) + j) / sum;
      else
	profile(2, j) = (T) 0;

    solutions.Append(profile);
  }
}

template <class T> long LemkeTableau<T>::NumPivots(void) const
{
  return npivots;
}

template <class T> long &LemkeTableau<T>::NumPivots(void)
{
  return npivots;
}

#ifdef __GNUG__
template class LemkeTableau<double>;
template class LemkeTableau<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class LemkeTableau<double>;
class LemkeTableau<gRational>;
#pragma option -Jgx
#endif   // __GNUG__

//-------------------------------------------------------------------------
//                    LemkeModule<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
LemkeModule<T>::LemkeModule(const Nfg<T> &N, const LemkeParams &p,
			    const NFSupport &S)
  : nf(N), support(S), params(p), npivots(0)
{ }

template <class T> LemkeModule<T>::~LemkeModule()
{ }

template <class T> int LemkeModule<T>::Lemke(void)
{
  if (nf.NumPlayers() != 2 || !params.output)   return 0;

  gWatch watch;

  LemkeTableau<T> LT(nf, support, *params.output, params.trace, params.status);
//  LT.Lemke((params.stopAfter == 1) ? 1 : 0);
  LT.Lemke(params.stopAfter);

  time = watch.Elapsed();
  npivots += LT.NumPivots();
  LT.GetSolutions(solutions);

  return 1;
}

template <class T> long LemkeModule<T>::NumPivots(void) const
{
  return npivots;
}

template <class T> double LemkeModule<T>::Time(void) const
{
  return time;
}

template <class T>
const gList<MixedProfile<T> > &LemkeModule<T>::GetSolutions(void) const
{
  return solutions;
}

#ifdef __GNUG__
template class LemkeModule<double>;
template class LemkeModule<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class LemkeModule<double>;
class LemkeModule<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__


//-------------------------------------------------------------------------
//                    Convenience functions for Lemke
//-------------------------------------------------------------------------

template <class T>
int Lemke(const Nfg<T> &N, const LemkeParams &p,
	  gList<MixedProfile<T> > &solutions,
	  long &npivots, double &time)
{
  NFSupport S(N);
  LemkeModule<T> LM(N, p, S);
  int result = LM.Lemke();

  npivots = LM.NumPivots();
  time = LM.Time();
  
  solutions = LM.GetSolutions();

  return result;
}

#ifdef __GNUG__
template int Lemke(const Nfg<double> &, const LemkeParams &,
		   gList<MixedProfile<double> > &, long &, double &);
template int Lemke(const Nfg<gRational> &, const LemkeParams &,
		   gList<MixedProfile<gRational> > &, long &, double &);
#elif defined __BORLANDC__
#pragma option -Jgd
int Lemke(const Nfg<double> &, const LemkeParams &,
	  gList<MixedProfile<double> > &, long &, double &);
int Lemke(const Nfg<gRational> &, const LemkeParams &,
	  gList<MixedProfile<gRational> > &, long &, double &);
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__




