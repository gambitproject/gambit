//
// FILE: tableau2.h:  gRational Tableau class 
//
//

#ifndef TABLEAU2_H  
#define TABLEAU2_H

class Tableau<gRational> : public TableauInterface<gRational>{
private:
  int remap(int col_index) const;  // aligns the column indexes
  gMatrix<gRational> GetInverse();

  gMatrix<gInteger> Tabdat;  // This caries the full tableau
  gVector<gInteger> Coeff;   // and coeffieient vector
  gVector<int> nonbasic;     //** nonbasic variables -- should be moved to Basis
  gInteger totdenom;  // This carries the denominator for Q data or 1 for Z
  gInteger denom;  // This is the denominator for the simplex

  gVector<gRational> tmpcol; // temporary column vector, to avoid allocation

public:
      // constructors and destructors
  Tableau(const gMatrix<gRational> &A, const gVector<gRational> &b); 
  Tableau(const Tableau<gRational>&);
  virtual ~Tableau();
  
  Tableau<gRational>& operator=(const Tableau<gRational>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void SolveColumn(int, gVector<gRational> &);  // column in new basis 
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const gVector<gRational> &bnew);
  void SetBasis( const Basis<gRational> &); // set new Tableau
  void Solve(const gVector<gRational> &b, gVector<gRational> &x) const;  // solve M x = b
  void SolveT(const gVector<gRational> &c, gVector<gRational> &y) const;  // solve y M = c
  
  bool IsFeasible();
  bool IsLexMin();
};

#endif     // TABLEAU2_H

