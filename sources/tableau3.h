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
  gMatrix<gRational> InvDat;

protected:
public:
      // constructors and destructors
  Tableau(const gMatrix<gRational> &A, const gVector<gRational> &b); 
  Tableau(const Tableau<gRational>&);
  virtual ~Tableau();
  
  Tableau<gRational>& operator=(const Tableau<gRational>&);
  
  // pivoting
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

