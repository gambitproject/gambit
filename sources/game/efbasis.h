//
// FILE: efbasis.h -- Declaration of EFBasis class
//
// $Id$ 
//

#ifndef EFBASIS_H
#define EFBASIS_H

#include "base/base.h"
#include "math/gdpvect.h"
#include "game/efg.h"
#include "game/efstrat.h"

class EFNodeSet;

class EFBasis : public EFSupport {
protected:
  gArray <EFNodeSet *> nodes;

  // This is scratch stuff for consistency computation:
  EFBasis *bigbasis;
  gMatrix<double> *A;
  gVector<double> *b,*c;
  gDPVector<int> *actIndex, *nodeIndex;
  int num_eqs, num_ineqs, num_act_vars,num_node_vars;

  void MakeIndices();
  void MakeRowIndices();
  void MakeAb();
  void AddEquation1(int,Action *) const;
  void AddEquation2(int,Node *) const;
  void AddEquation3(int,Node *,Node *) const;
  void AddEquation4(int,Node *,Node *) const;
  int Col(Action *) const;
  int Col(Node *) const;
  void GetConsistencySolution(const gVector<double> &x);
  
public:
  EFBasis(const FullEfg &);
  EFBasis(const EFBasis &b); 
  virtual ~EFBasis();
  EFBasis &operator=(const EFBasis &b);

  bool operator==(const EFBasis &b) const;
  bool operator!=(const EFBasis &b) const;

  int NumNodes(int pl, int iset) const;
  gPVector<int> NumNodes(void) const;

  bool RemoveNode(Node *);
  void AddNode(Node *);
  bool IsReachable(Node *) const;

  // Returns the position of the node in the support.  Returns zero
  // if it is not there.
  int Find(Node *) const;

  const gArray<Node *> &Nodes(int pl, int iset) const;

  bool IsValid(void) const;
  bool IsConsistent();
  void Dump(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const EFBasis &);

#endif // EFBASIS_H



