//
// FILE: basis.h -- Definition of the Basis class for tableaus
//
//


#ifndef BASIS_H
#define BASIS_H

#include "gmatrix.h"
#include "gvector.h"
#include "gblock.h"

//---------------------------------------------------------------------------
// Class Basis
//---------------------------------------------------------------------------

template <class T> class LUdecomp;

template <class T> class Basis {

friend class LUdecomp<T>;
  
protected:

  const gMatrix<T> *A;
  gBlock<int> label;
  gBlock<int> cols;
  gBlock<int> slacks;
  gBlock<int> arts;
  gBlock<int> artUnitEntry;
  gArray<bool> colBlocked;
  gArray<bool> rowBlocked;
  bool IsBasisIdent;

  // returns true if the column is slack
  inline bool IsSlackColumn( int col ) const 
    {return  -col >= label.First() && -col <= label.Last();} 
  
  // returns true if the column is artificial
  inline bool IsArtifColumn( int col ) const
    {return col > cols.Last() && col <= cols.Last() + arts.Length();} 

  // returns true if the column is a regular column
  inline bool IsRegColumn( int col ) const
    {return col >= cols.First() && col <= cols.Last();} 
  
public:

  //-------------------------------------------
  // Constructors, Destructor, Operators
  //-------------------------------------------
  

  Basis(const gMatrix<T> &A);
  Basis(const Basis<T> &);
  virtual ~Basis();
  
  Basis<T>& operator=(const Basis<T>&);

  //------------------------------
  // Public Members
  //------------------------------
  
  //remove outindex, insert label, return outlabel
  int Pivot(int outindex, int col); 

  // return true iff label is a Basis member
  bool Member(int label) const;

  // finds Basis index corresponding to label number,
  // fails assert if label not in Basis
  int Find(int label) const;

  // finds label of variable corresponding to Basis index
  int Label(int index) const;

  // marks/unmarks label to block it from entering basis
  void Mark(int label);
  void UnMark(int label);

  // returns true if label is blocked from entering basis
  bool IsBlocked(int label) const;

  // select Basis elements according to Tableau rows and cols
  void BasisSelect(const gBlock<T>&rowv, gVector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const gBlock<T>&unitv,
		   const gBlock<T>&rowv,
		   gVector<T>&colv
		   ) const; 
  

  // returns whether the basis is the identity matrix
  bool IsIdent();

  // returns a column from the matrix  
  void GetColumn( int col, gVector<T> & ) const;
  
  // Insert an artificial variable at column col
  void InsertArtificial( int art, int col );

  // Append an artificial variable.  Returns the last col index
  // ( ie where the artificial variable was appended ).
  int AppendArtificial( int art );

  // Remove an artificial variable, returns which artificial variable
  // was removed.
  void RemoveArtificial( int col );
  
  // Clears out all the artificial variables
  void FlushArtificial();

  // Returns the index of the last artificial variable
  int LastArtificial();

  void Dump(gOutput &) const;

  // ----------------
  // Private members
  // ----------------

  virtual void CheckBasis();

};

#endif // BASIS_H

