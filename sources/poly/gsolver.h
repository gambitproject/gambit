//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of gSolver
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

#ifndef GSOLVER_H
#define GSOLVER_H

#include "ideal.h"
#include "math/double.h"
#include "numerical/linrcomb.h"
#include "gpolylst.h"

template <class T> class gSolver {

private:
  const gPolyList<T>& InputList;
  const gIdeal<T>     TheIdeal;

// Conversion
  gList<gPoly<gDouble> > BasisTogDouble() const;

// Recursive Call in Solver
  gList<gVector<gDouble> >  
    ContinuationSolutions(const gList<gPoly<gDouble> >& list,
			  const int dmnsn,
			  const int curvar,
			  const gVector<gDouble>& knownvals);

public:

// Constructor and Destructor
  gSolver(const term_order* Order,const gPolyList<T>& Inputs);
  gSolver(const gSolver<T>&);
  ~gSolver();

  bool                     IsZeroDimensional();
  gList<gVector<gDouble> > Roots();
};

#endif // GSOLVER_H
