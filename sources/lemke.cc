//#
//# FILE: lemke.cc -- Lemke-Howson module
//#
//# $Id$
//#

#include "gambitio.h"
#include "normal.h"
#include "nfrep.h"
#include "normiter.h"
#include "gtableau.h"
#include "rational.h"
#include "mixed.h"
#include "solution.h"


//
// Here is the LemkeSolution class.  To be used, it needs to have private
// members added to store whatever information is interesting, beyond just
// the mixed strategy profile
//
template <class T> class LemkeSolution : public MixedSolution<T>   {
  public:
    LemkeSolution(const MixedProfile<T> &mp) : MixedSolution<T>(mp)   { }
    LemkeSolution(const LemkeSolution<T> &s) : MixedSolution<T>(s)   { }
    virtual ~LemkeSolution()  { }

    SolutionType Type(void) const   { return sLEMKE; }
    void Output(gOutput &f) const
      { f << "Lemke solution: " << p << '\n'; }
};


class BaseLemke    {
  public:
    virtual int Lemke(int) = 0;
    virtual gBlock<Solution *> GetSolutions(void) const = 0;
    virtual ~BaseLemke()   { }
};

template <class T> class LemkeTableau
  : public gTableau<T>, public BaseLemke, public SolutionModule  {
  private:
    const NFRep<T> &rep;
    BFS_List List;
    
    int Lemke_Step(int);
    int At_CBFS(void) const;
    int All_Lemke(BFS_List &List, int j);
    int Exit_Row(int col);
    void Pivot(int, int);
 
  public:
    LemkeTableau(const NFRep<T> &, gOutput &ofile, gOutput &efile, int plev);
    virtual ~LemkeTableau()   { }

    int Lemke(int);
    gBlock<Solution *> GetSolutions(void) const;
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
 
  if (printlevel >= 3)
    Dump(output);

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
  BFS<T> cbfs((T) 0);
  int i;

  List = BFS_List();

  if (Duplicate_Label == 0)
    All_Lemke(List, 0);
  else  {
    Lemke_Step(Duplicate_Label);
    for (i = 1; i <= Num_Strats; i++)
      if (Row_Labels[i] > 0)
	cbfs.Define(Row_Labels[i], Tableau(i,0));
    List.Append(cbfs);
  }

  if (printlevel >= 1)  {
    for (i = 1; i <= List.Length(); i++)   {
      List[i].Dump(output);
      output << "\n";
    }
  }

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
template <class T> int LemkeTableau<T>::All_Lemke(BFS_List &List, int j)
{
  BFS<T> cbfs((T) 0);
  int i;

  for (i = 1; i <= Num_Strats; i++)
    if (Row_Labels[i] > 0)
      cbfs.Define(Row_Labels[i], Tableau(i,0));

  if (printlevel >= 3)  {
    output << "\npath " << j ;
    output << " Basis:  " ;
    for (i = 1; i <= Num_Strats; i++)
      output << Row_Labels[i] << " " ;
    output << "\n";	
  }

  if (List.Contains(cbfs))  return 1;
  List.Append(cbfs);

  if (printlevel >= 2)
    cbfs.Dump(output);

  if (printlevel >= 3)  {
    output << "\npath " << j << " Basis:  " ;
    for (i = 1; i <= Num_Strats; i++)
      output << Row_Labels[i] << " " ;
  }

  if (printlevel >= 2)
    output << "\n";

  if (printlevel >= 3)
    Dump(output);

  for (i = 1; i <= Num_Strats; i++)
    if (i != j)  {
      LemkeTableau<T> Tcopy(*this);
      Tcopy.Lemke_Step(i);

      if (printlevel >= 3)      Tcopy.Dump(output);
      Tcopy.All_Lemke(List, i);

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

template <class T> gBlock<Solution *> LemkeTableau<T>::GetSolutions(void) const
{
  gBlock<Solution *> solutions;
  
  for (int i = 1; i <= List.Length(); i++)    {
    MixedProfile<T> prof(rep);

    gVector<T> strat1(1, rep.NumStrats(1)), strat2(1, rep.NumStrats(2));
    T sum = (T) 0;

    for (int j = 1; j <= rep.NumStrats(1); j++)
      if (List[i].IsDefined(j))   sum += List[i](j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= rep.NumStrats(1); j++) 
      if (List[i].IsDefined(j))   strat1[j] = List[i](j) / sum;
      else  strat1[j] = (T) 0;

    sum = (T) 0;

    for (j = 1; j <= rep.NumStrats(2); j++)
      if (List[i].IsDefined(rep.NumStrats(1) + j))  
	sum += List[i](rep.NumStrats(1) + j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= rep.NumStrats(2); j++)
      if (List[i].IsDefined(rep.NumStrats(1) + j))
	strat2[j] = List[i](rep.NumStrats(1) + j) / sum;
      else
	strat2[j] = (T) 0;

    prof.SetMixedStrategy(1, strat1);
    prof.SetMixedStrategy(2, strat2);
    solutions.Append(new LemkeSolution<T>(prof));
  }

  return solutions;
}

template <class T>
LemkeTableau<T>::LemkeTableau(const NFRep<T> &r,
			      gOutput &ofile, gOutput &efile, int plev)
     : gTableau<T>(r.NumStrats(1) + r.NumStrats(2)), 
       SolutionModule(ofile, efile, plev), rep(r)
{
  NormalIter<T> iter(r);
  T min = (T) 0, x;
  int n1 = r.NumStrats(1), n2 = r.NumStrats(2);

  for (int i = 1; i <= n1; i++)   {
    for (int j = 1; j <= n2; j++)  {
      x = iter.Evaluate(1);
      if (x < min)   min = x;
      x = iter.Evaluate(2);
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


  for (i = 1; i <= n1; i++)  {
    for (int j = 1; j <= n2; j++)  {
      Tableau(i, n1 + j) = iter.Evaluate(1) - min;
      Tableau(n1 + j, i) = iter.Evaluate(2) - min;
      iter.Next(2);
    }
    iter.Next(1);
  }

  for (i = 1; i <= n1 + n2; Tableau(i++, 0) = -1.0);
  for (i = 1; i <= n1 + n2; Tableau(i++, n1 + n2 + 1) = 0.0);
}

int NormalForm::Lemke(int dup_strat)
{
  if (NumPlayers() != 2)   return 0;

  if (dup_strat < 0 || dup_strat > data->NumStrats(1)+data->NumStrats(2))
    dup_strat = 0;

  BaseLemke *T;

  switch (type)   {
    case nfDOUBLE:
      T = new LemkeTableau<double>((NFRep<double> &) *data, gout, gerr, 0);
      break;

    case nfRATIONAL:
      T = new LemkeTableau<Rational>((NFRep<Rational> &) *data,
				     gout, gerr, 0);
      break;
  }

  T->Lemke(dup_strat);

  gBlock<Solution *> solutions(T->GetSolutions());
  gout << "Equilibria found:\n";
  for (int i = 1; i <= solutions.Length(); i++)
    gout << *solutions[i];

  while (solutions.Length())   delete solutions.Remove(1);
}





