//
// FILE: basis.h -- Definition of the Basis class for tableaus
//
// $Id$
//

#ifndef BASIS_H
#define BASIS_H

#include "base/base.h"
#include "math/gvector.h"
#include "math/gmatrix.h"

//---------------------------------------------------------------------------
// Class Basis
//---------------------------------------------------------------------------

class Basis {

private:
  gBlock<int> basis;        // current members of basis (neg for slacks)
  gBlock<int> cols;         // location of col in basis (0 if not in basis)
  gBlock<int> slacks;       // location of slacks in basis
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
  int MinCol() const;    // First Column label
  int MaxCol() const;     // Last Column label

  inline bool IsRegColumn( int col ) const;
  inline bool IsSlackColumn( int col ) const; 

  //remove outindex, insert label, return outlabel
  int Pivot(int outindex, int col); 

  // return true iff label is a Basis member
  bool Member(int label) const;

  // finds Basis index corresponding to label number,
  int Find(int label) const;

  // finds label of variable corresponding to Basis index
  int Label(int index) const;

  // marks/unmarks label to block it from entering basis
  void Mark(int label);
  void UnMark(int label);

  // returns true if label is blocked from entering basis
  bool IsBlocked(int label) const;

  // Check if Basis is Ident
  virtual void CheckBasis();
  // returns whether the basis is the identity matrix
  bool IsIdent();

  void Dump(gOutput &) const;
};

#endif // BASIS_H

