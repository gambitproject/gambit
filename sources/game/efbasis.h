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

#include "game/game.h"
#include "game/efgsupport.h"

class gbtGameNodeSet;

class gbtEfgBasis : public gbtEfgSupport {
protected:
  gbtArray <gbtGameNodeSet *> nodes;

  // This is scratch stuff for consistency computation.
  // FIXME: These shouldn't be members!!
  mutable gbtEfgBasis *bigbasis;
  mutable gbtMatrix<double> *A;
  mutable gbtVector<double> *b,*c;
  mutable gbtDPVector<int> *actIndex, *nodeIndex;
  mutable int num_eqs, num_ineqs, num_act_vars,num_node_vars;

  void MakeIndices(void) const;
  void MakeRowIndices(void) const;
  void MakeAb(void) const;
  void AddEquation1(int, const gbtGameAction &) const;
  void AddEquation2(int, gbtGameNode) const;
  void AddEquation3(int, const gbtGameNode &, const gbtGameNode &) const;
  void AddEquation4(int, const gbtGameNode &, const gbtGameNode &) const;
  int Col(const gbtGameAction &) const;
  int Col(const gbtGameNode &) const;
  void GetConsistencySolution(const gbtVector<double> &x) const;
  
public:
  gbtEfgBasis(const gbtGame &);
  gbtEfgBasis(const gbtEfgBasis &b); 
  virtual ~gbtEfgBasis();
  gbtEfgBasis &operator=(const gbtEfgBasis &b);

  bool operator==(const gbtEfgBasis &b) const;
  bool operator!=(const gbtEfgBasis &b) const;

  int NumNodes(int pl, int iset) const;
  int NumNodes(const gbtGameInfoset &) const;
  gbtPVector<int> TotalNumNodes(void) const;

  bool RemoveNode(const gbtGameNode &);
  void AddNode(const gbtGameNode &);
  bool IsReachable(gbtGameNode) const;

  // Returns the position of the node in the support.  Returns zero
  // if it is not there.
  int Find(const gbtGameNode &) const;

  const gbtArray<gbtGameNode> &Nodes(int pl, int iset) const;
  gbtGameNode GetNode(const gbtGameInfoset &, int index) const;

  bool IsValid(void) const;
  bool IsConsistent(void) const;
  void Dump(gbtOutput &) const;
};

gbtOutput &operator<<(gbtOutput &f, const gbtEfgBasis &);

#endif // EFBASIS_H
