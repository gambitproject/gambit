//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to extensive form basis class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
  EFBasis(const efgGame &);
  EFBasis(const EFBasis &b); 
  virtual ~EFBasis();
  EFBasis &operator=(const EFBasis &b);

  bool operator==(const EFBasis &b) const;
  bool operator!=(const EFBasis &b) const;

  int NumNodes(int pl, int iset) const;
  int NumNodes(Infoset *) const;
  gPVector<int> NumNodes(void) const;

  bool RemoveNode(Node *);
  void AddNode(Node *);
  bool IsReachable(Node *) const;

  // Returns the position of the node in the support.  Returns zero
  // if it is not there.
  int Find(Node *) const;

  const gArray<Node *> &Nodes(int pl, int iset) const;
  Node *GetNode(Infoset *, int index) const;

  bool IsValid(void) const;
  bool IsConsistent();
  void Dump(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const EFBasis &);

#endif // EFBASIS_H



