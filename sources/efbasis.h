//
// FILE: efbasis.h -- Declaration of EFBasis class
//
// @(#)efbasis.h	1.2 02/10/98 
//

#ifndef EFBASIS_H
#define EFBASIS_H

#include "gtext.h"
#include "gblock.h"
#include "gstream.h"
#include "glist.h"
#include "efg.h"

class EFNodeSet;

class EFBasis : public EFSupport {
protected:
  gArray <EFNodeSet *> nodes;
  
public:
  EFBasis(const Efg &);
  EFBasis(const EFBasis &b); 
  virtual ~EFBasis();
  EFBasis &operator=(const EFBasis &b);

  bool operator==(const EFBasis &b) const;
  bool operator!=(const EFBasis &b) const;

  int NumNodes(int pl, int iset) const;
  gPVector<int> NumNodes(void) const;

  bool RemoveNode(Node *);
  void AddNode(Node *);

  // Returns the position of the node in the support.  Returns zero
  // if it is not there.
  int Find(Node *) const;

  const gArray<Node *> &Nodes(int pl, int iset) const;

  bool IsValid(void) const;
  bool IsConsistent() const;
  void Dump(gOutput &) const;
};

#endif // EFBASIS_H

