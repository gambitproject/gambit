//#
//# FILE: lemke.cc -- Lemke-Howson module
//#
//# $Id$
//#

#include "gambitio.h"
#include "normal.h"
#include "nfrep.h"
#include "gtableau.h"
#include "rational.h"
#include "mixed.h"
#include "solution.h"


//
// Here is the LemkeSolution class.  To be used, it needs to have private
// members added to store whatever information is interesting, beyond just
// the mixed strategy profile
//
class LemkeSolution : public MixedSolution   {
  public:
    LemkeSolution(const MixedProfile &mp) : MixedSolution(mp)   { }
    LemkeSolution(const LemkeSolution &s) : MixedSolution(s)   { }
    virtual ~LemkeSolution()  { }

    SolutionType Type(void) const   { return sLEMKE; }
    void Output(gOutput &f) const
      { gout << "Lemke solution: " << p << '\n'; }
};


class BaseLemke    {
  public:
    virtual int Lemke(int) = 0;
    virtual ~BaseLemke()   { }
};

template <class T> class LemkeTableau : public gTableau<T>, public BaseLemke  {
  private:
    int Lemke_Step(int);
    int At_CBFS(void) const;
    int All_Lemke(BFS_List &List, int j);
    int Exit_Row(int col);
    void Pivot(int, int);
 
  public:
    LemkeTableau(int i) : gTableau<T>(i)  { }
    LemkeTableau(const LemkeTableau<T> &t) : gTableau<T>(t)  { }
    virtual ~LemkeTableau()   { }

    int Lemke(int);
};


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
  Tableau(row, Num_Strats + 1) = 1;

     // Now pivot.
  Tableau.Pivot(row, col);

     // swap row and column labels
  int temp = Row_Labels[row];
  Row_Labels[row] = Col_Labels[col];
  Col_Labels[col] = temp;
 
#ifdef UNUSED
   Dump(gout);
#endif   // UNUSED

     // Now switch the col column and the Num_Strat + 1 (scratch) column,
     // then set the row'th entry to 0, making the whole column zero.
  Tableau.SwitchColumns(col, Num_Strats + 1);
  Tableau(row, Num_Strats + 1) = 0;
}

//
// Lemke is the most important routine.
// It implements the Lemke-Howson algorithm, as refined by Eaves.
// It is assumed that the starting point is a complementary basic
// feasible solution.  If not it returns 0 without doing anything.
//
template <class T> int LemkeTableau<T>::Lemke(int Duplicate_Label)
{
  BFS_List List;
  BFS<T> cbfs((T) 0);
  int i;

  if (Duplicate_Label == 0)
    All_Lemke(List, 0);
  else  {
    Lemke_Step(Duplicate_Label);
    for (i = 1; i <= Num_Strats; i++)
      if (Row_Labels[i] > 0)
	cbfs.Define(Row_Labels[i], Tableau(i,0));
    List.Append(cbfs);
  }

#ifdef UNUSED
  for (i = 1; i <= List.Length(); i++)   {
    List[i].Dump(gout);
    gout << "\n";
  }
#endif   // UNUSED

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
#ifdef UNUSED
    Dump(gout);
    gout << "\npivot row = " << row << " col = " << col << "\n";
#endif   // UNUSED
    Pivot(row, col);
#ifdef UNUSED
    Dump(gout);
#endif   // UNUSED
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
template <class T> int LemkeTableau<T>::All_Lemke(BFS_List &List, int j)
{
  BFS<T> cbfs((T) 0);
  int i;

  for (i = 1; i <= Num_Strats; i++)
    if (Row_Labels[i] > 0)
      cbfs.Define(Row_Labels[i], Tableau(i,0));

#ifdef UNUSED
  gout << "\npath " << j ;
  gout << " Basis:  " ;
  for (i = 1; i <= Num_Strats; i++)
    gout << Row_Labels[i] << " " ;
  gout << "\n";	
#endif   // UNUSED

  if (List.Contains(cbfs))  return 1;
  List.Append(cbfs);
  cbfs.Dump(gout);

#ifdef UNUSED
  gout << "\npath " << j << " Basis:  " ;
  for (i = 1; i <= Num_Strats; i++)
    gout << Row_Labels[i] << " " ;
#endif   // UNUSED
  gout << "\n";
#ifdef UNUSED
  Dump(gout);
#endif   // UNUSED

  for (i = 1; i <= Num_Strats; i++)
    if (i != j)  {
      LemkeTableau<T> Tcopy(*this);
      Tcopy.Lemke_Step(i);
#ifdef UNUSED
      Tcopy.Dump(gout);
#endif   // UNUSED
      Tcopy.All_Lemke(List, i);
#ifdef UNUSED
      Tcopy.Dump(gout);
#endif   // UNUSED
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
  for (i = 1; i <= Num_Strats; i++)
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
    assert(c <= Num_Strats); 	
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



int NormalForm::Lemke(int dup_strat)
{
  int i, j;
  StrategyProfile s(2);

  if (NumPlayers() != 2)   return 0;
  int n1 = strategies[1]->NumStrats();
  int n2 = strategies[2]->NumStrats();
  int n = n1+n2;

  for (i = 1; i <= strategies.Length(); i++)
    s.SetStrategy(strategies[i]->GetStrategy(1));

  switch (array->Type())   {
    case nfDOUBLE:   {
      double min = 0.0, x;
      LemkeTableau<double> T(n);
      NFRep<double> *N = (NFRep<double> *) array;

//      N->WriteNfgFile(gout);

      for (i = 1; i <= n1; i++)
 	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  x = (*N)(s,1);
//	  gout << i << ' ' << j << ' ' << s << " -> " << x << '\n';
	  if (x < min)  min = x;
	  x = (*N)(s,2);
//	  gout << i << ' ' << j << ' ' << s << " -> " << x << '\n';
	  if (x < min)  min = x;
	}

      for (i = 1; i <= n1; i++) 
	for (j = 1; j <= n1; j++)
	  T.Entry(i,j) = 0.0;
      
      for (i = n1 + 1; i <= n; i++)  
	for (j = n1 + 1; j <= n; j++) 
	  T.Entry(i,j) = 0.0;

      for (i = 1; i <= n1; i++)  
	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  T.Entry(i, n1 + j) = (*N)(s,1) - min;
	  T.Entry(n1 + j, i) = (*N)(s,2) - min;
	}

      for (i = 1; i <= n; T.Entry(i++, 0) = -1.0);
      for (i = 1; i <= n; T.Entry(i++, n + 1) = 0.0);
      T.Lemke((dup_strat <= 0 || dup_strat > n) ? 0 : dup_strat); 
      return 1;
    }

    case nfRATIONAL:   {
      Rational min = 0, x;
      LemkeTableau<Rational> T(n);
      NFRep<Rational> *N = (NFRep<Rational> *) array;

      for (i = 1; i <= n1; i++)
	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  x = (*N)(s,1);
	  if (x < min)  min = x;
	  x = (*N)(s,2);
	  if (x < min)  min = x;
	}

      for (i = 1; i <= n1; i++) 
	for (j = 1; j <= n1; j++)
	  T.Entry(i,j) = 0.0;
      
      for (i = n1 + 1; i <= n; i++)  
	for (j = n1 + 1; j <= n; j++) 
	  T.Entry(i,j) = 0.0;

      for (i = 1; i <= n1; i++)  
	for (j = 1; j <= n2; j++)  {
	  s.SetStrategy(strategies[1]->GetStrategy(i));
	  s.SetStrategy(strategies[2]->GetStrategy(j));
	  T.Entry(i, n1 + j) = (*N)(s,1) - min;
	  T.Entry(n1 + j, i) = (*N)(s,2) - min;
	}

      for (i = 1; i <= n; T.Entry(i++, 0) = -1.0);
      for (i = 1; i <= n; T.Entry(i++, n + 1) = 0.0);
      T.Lemke((dup_strat <= 0 || dup_strat > n) ? 0 : dup_strat); 
      return 1;
    }
  }
}




