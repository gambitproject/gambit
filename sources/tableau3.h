//
// FILE: tableau3.h:  gRational Tableau class 
//
// $Id$
//

#ifndef TABLEAU3_H  
#define TABLEAU3_H

class Tableau<gRational> : public TableauInterface<gRational>{
private:
  gRational Det;
  gRational totdenom;
  gMatrix<gRational> InvDat;

  gVector<gRational> tmpcol; // temporary column vector, to avoid allocation

protected:
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
  void Solve(const gVector<gRational> &b, gVector<gRational> &x);  // solve M x = b
  void SolveT(const gVector<gRational> &c, gVector<gRational> &y);  // solve y M = c
  
  bool IsFeasible();
  bool IsLexMin();
};

#endif     // TABLEAU3_H

