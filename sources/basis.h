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

class Basis {

protected:

  gBlock<int> label;        // labels of variables in basis (neg for slacks)
  gBlock<int> cols;         // location of col in basis (0 if not in basis)
  gBlock<int> slacks;       // location of slacks in basis
  gBlock<int> artUnitEntry; // artificial variable label
  gArray<bool> colBlocked;  
  gArray<bool> rowBlocked;
  bool IsBasisIdent;

public:
    class BadIndex : public gException  {
    public:
      virtual ~BadIndex();
      gText Description(void) const;
    };

  //-------------------------------------------
  // Constructors, Destructor, Operators
  //-------------------------------------------
  
  Basis(int first, int last, int firstlabel, int lastlabel);
  Basis(const Basis &);
  virtual ~Basis();
  
  Basis& operator=(const Basis&);

  //------------------------------
  // Public Members
  //------------------------------
  
  int First() const;         // First basis index
  int Last() const;          // Last  basis index
  int FirstLabel() const;    // First Column label
  int LastLabel() const;     // Last Column label

  // returns true if the column is slack
  inline bool IsSlackColumn( int col ) const 
    {return  -col >= label.First() && -col <= label.Last();} 
  
  // returns true if the column is a regular column
  inline bool IsRegColumn( int col ) const
    {return col >= cols.First() && col <= cols.Last();} 
  
  // returns true if the column is artificial
  inline bool IsArtifColumn( int col ) const
    {return col >= artUnitEntry.First() && col <= cols.Last();} 

  inline int UnitEntry(int col ) const
    {assert(IsArtifColumn(col)); return artUnitEntry[col]; }

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

  // returns whether the basis is the identity matrix
  bool IsIdent();

  // Append an artificial variable.  Returns the last col index
  // ( ie where the artificial variable was appended ).
  int AppendArtificial( int art );

  // Remove an artificial variable, returns which artificial variable
  // was removed.
  void RemoveArtificial( int col );
  
  void Dump(gOutput &) const;

  // ----------------
  // Private members
  // ----------------

  virtual void CheckBasis();

};

#endif // BASIS_H

