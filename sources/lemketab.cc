//#
//# FILE: lemketab.cc -- Lemke tableau
//#
//# $Id$
//#

#include "lemketab.h"


//---------------------------------------------------------------------------
//                        Lemke Tableau: member functions
//---------------------------------------------------------------------------

// template <class T> LTableau<T>::LTableau(void)
//   : Tableau<T>()
// { } 

template <class T> LTableau<T>::LTableau(const gMatrix<T> &A, 
					 const gVector<T> &b)
  : Tableau<T>(A,b)
{ } 

template <class T> LTableau<T>::LTableau(Tableau<T> &tab)
  : Tableau<T>(tab) 
{ }

template <class T> LTableau<T>::~LTableau(void) 
{ }

template <class T> int LTableau<T>::SF_PivotIn(int inlabel)
{ 
  //* gout << "\n inlabel = " << inlabel;
  int outindex = SF_ExitIndex(inlabel);
//  gout << " outindex = " << outindex;
  if(outindex==0) {
  //* gout << "\nPivotIn: outindex=0, inlabel=" << inlabel;
    return inlabel;
  }
  int outlabel = Label(outindex);
  //* gout << " outlabel = " << outlabel;
  //* gout << " outindex = " << outindex;
  Pivot(outindex,inlabel);
  return outlabel;
}

template <class T> int LTableau<T>::PivotIn(int inlabel)
{ 
  //   gout << "\n inlabel = " << inlabel;
  int outindex = ExitIndex(inlabel);
//  gout << " outindex = " << outindex;
  if(outindex==0) return inlabel;
  int outlabel = Label(outindex);
  if(outlabel==0){assert(0);return 0;}
  //   gout << " outlabel = " << outlabel;
  //   gout << " outindex = " << outindex;
  Pivot(outindex,inlabel);
  return outlabel;
}

template <class T> int LTableau<T>::SF_ExitIndex(int inlabel)
{
  gBlock<int> BestSet;
  int i, c;
  T ratio, tempmax;
  gVector<T> incol(MinRow(), MaxRow());
  gVector<T> col(MinRow(), MaxRow());
  
  SolveColumn(inlabel,incol);
  
  gout << "\nincol = " << incol << '\n';;
      // Find all row indices for which column col has positive entries.
  for (i = MinRow(); i <= MaxRow(); i++)
    if (incol[i] > eps2)
      BestSet.Append(i);
  if(BestSet.Length()==0) {
    //* gout << "\nBestSet.Length() == 0, Find(0): " << Find(0);
    return 0;
  }
  // Is this really needed?  
  /*
    if(BestSet.Length()==0  
       && incol[Find(0)]<=eps2 && incol[Find(0)] >= (-eps2) )
      return Find(0);
  */
  assert(BestSet.Length() > 0);
  
      // If there are multiple candidates, break ties by
      // looking at ratios with other columns,
      // eliminating nonmaximizers of 
      // a similar ratio, until only one candidate remains.
  c = MinRow()-1;
  BasisVector(col);
  // gout << "\nLength = " <<  BestSet.Length();
  //* gout << "\n x =     " << col << "\n";
  while (BestSet.Length() > 1)   {
    assert(c <= MaxRow());
    if(c>=MinRow()) {
      SolveColumn(-c,col);
      // gout << "\n-c = " << -c << " col = " << col;
    }
	// Initialize tempmax.
    tempmax = col[BestSet[1]] / incol[BestSet[1]];
	// Find the maximum ratio. 
    for (i = 2; i <= BestSet.Length(); i++)  {
      ratio = col[BestSet[i]] / incol[BestSet[i]];
//*      if (ratio > tempmax)  tempmax = ratio;
      if (ratio < tempmax)  tempmax = ratio;
    }
//    assert(tempmax > (T 2)*eps1);
    
	// Remove nonmaximizers from the list of candidate columns.
    for (i = BestSet.Length(); i >= 1; i--)  {
      ratio = col[BestSet[i]] / incol[BestSet[i]];
//*      if (ratio < tempmax -eps1)
      if (ratio > tempmax +eps2)
	BestSet.Remove(i);
    }
//    else  {
//      assert(Member(FindColumn(c)));
//      if (BestSet.Contains(c_row)) return c_row;
//    }
    c++;
  }
  assert(BestSet.Length() > 0);
  return BestSet[1];
}

//
// ExitIndex determines, for the current tableau and variable to
// to be added to the basis, which element should leave the basis.
// The choice is the one specified by Eaves, which is guaranteed
// to not cycle, even if the problem is degenerate.
//


template <class T> int LTableau<T>::ExitIndex(int inlabel)
{
  gBlock<int> BestSet;
  int i, c;
  T ratio, tempmax;
  gVector<T> incol(MinRow(), MaxRow());
  gVector<T> col(MinRow(), MaxRow());
  
  SolveColumn(inlabel,incol);
  //   gout << "\nincol = " << incol;
      // Find all row indices for which column col has positive entries.
  for (i = MinRow(); i <= MaxRow(); i++)
    if (incol[i] > eps2)
      BestSet.Append(i);
  // Is this really needed?  
  if(BestSet.Length()==0)
    //   gout << "\nBestSet.Length() == 0, Find(0):\n" << Find(0);
  if(BestSet.Length()==0  
     && incol[Find(0)]<=eps2 && incol[Find(0)] >= (-eps2) )
    return Find(0);
  assert(BestSet.Length() > 0);
  
      // If there are multiple candidates, break ties by
      // looking at ratios with other columns, 
      // eliminating nonmaximizers of 
      // a similar ratio, until only one candidate remains.
  c = MinRow()-1;
  BasisVector(col);
  // gout << "\nLength = " <<  BestSet.Length();
  //   gout << "\n x =     " << col << "\n";
  while (BestSet.Length() > 1)   {
    assert(c <= MaxRow());
    if(c>=MinRow()) {
      SolveColumn(-c,col);
      // gout << "\n-c = " << -c << " col = " << col;
    }
	// Initialize tempmax.
    tempmax = col[BestSet[1]] / incol[BestSet[1]];
	// Find the maximum ratio. 
    for (i = 2; i <= BestSet.Length(); i++)  {
      ratio = col[BestSet[i]] / incol[BestSet[i]];
      if (ratio > tempmax)  tempmax = ratio;
    }
//    assert(tempmax > (T 2)*eps1);
    
	// Remove nonmaximizers from the list of candidate columns.
    for (i = BestSet.Length(); i >= 1; i--)  {
      ratio = col[BestSet[i]] / incol[BestSet[i]];
      if (ratio < tempmax -eps1)
	BestSet.Remove(i);
    }
//    else  {
//      assert(Member(FindColumn(c)));
//      if (BestSet.Contains(c_row)) return c_row;
//    }
    c++;
  }
  assert(BestSet.Length() > 0);
  return BestSet[1];
}

//
// Executes one step of the Lemke-Howson algorithm
//

template <class T> int LTableau<T>::SF_LCPPath(int dup, gStatus &status)
{
  int enter, exit;
  enter = dup;
/*
  if(dup)
    Pivot(dup,0);
  else {
    enter = -SF_PivotIn(dup);
    assert(enter!=0);
  }      
*/
      // Central loop - pivot until another CBFS is found
  long nits = 0;
  do  {
    // Talk about optimism! This is dumb, but better than nothing (I guess):
    status.SetProgress((double)nits/(double)(nits+1)); 
    nits++;
    //* gout << "\nBasis:\n";
    //* Dump(gout);
    exit = SF_PivotIn(enter);
    if(exit==enter) {
      //* gout << "\nenter, exit: " << enter << " " << exit;
      return 0;
    }
    enter = -exit;
    status.Get();
  } while (exit != 0);
  return 1;
}

template <class T> int LTableau<T>::LemkePath(int dup)
{
//  if (!At_CBFS())  return 0;
  int enter, exit;
//  if(params.plev >=2) {
//    (*params.output) << "\nbegin path " << dup << "\n";
//    Dump(*params.output); 
//  }
    //   gout << "\nbegin path " << dup << "\n";
    //   Dump(gout); 
  enter = dup;
  if (Member(dup))
    enter = -dup;
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
  //   gout << "\nend of path " << dup;
  return 1;
}


template class LTableau<double>;
template class LTableau<gRational>;





