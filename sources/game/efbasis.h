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

  // This is scratch stuff for consistency computation.
  // FIXME: These shouldn't be members!!
  mutable EFBasis *bigbasis;
  mutable gMatrix<double> *A;
  mutable gVector<double> *b,*c;
  mutable gDPVector<int> *actIndex, *nodeIndex;
  mutable int num_eqs, num_ineqs, num_act_vars,num_node_vars;

  void MakeIndices(void) const;
  void MakeRowIndices(void) const;
  void MakeAb(void) const;
  void AddEquation1(int, const gbtEfgAction &) const;
  void AddEquation2(int, gbtEfgNode) const;
  void AddEquation3(int, const gbtEfgNode &, const gbtEfgNode &) const;
  void AddEquation4(int, const gbtEfgNode &, const gbtEfgNode &) const;
  int Col(const gbtEfgAction &) const;
  int Col(const gbtEfgNode &) const;
  void GetConsistencySolution(const gVector<double> &x) const;
  
public:
  EFBasis(const gbtEfgGame &);
  EFBasis(const EFBasis &b); 
  virtual ~EFBasis();
  EFBasis &operator=(const EFBasis &b);

  bool operator==(const EFBasis &b) const;
  bool operator!=(const EFBasis &b) const;

  int NumNodes(int pl, int iset) const;
  int NumNodes(const gbtEfgInfoset &) const;
  gPVector<int> NumNodes(void) const;

  bool RemoveNode(const gbtEfgNode &);
  void AddNode(const gbtEfgNode &);
  bool IsReachable(gbtEfgNode) const;

  // Returns the position of the node in the support.  Returns zero
  // if it is not there.
  int Find(const gbtEfgNode &) const;

  const gArray<gbtEfgNode> &Nodes(int pl, int iset) const;
  gbtEfgNode GetNode(const gbtEfgInfoset &, int index) const;

  bool IsValid(void) const;
  bool IsConsistent(void) const;
  void Dump(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const EFBasis &);

#endif // EFBASIS_H



