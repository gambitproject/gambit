//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to N-dimensional function minimization routines
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

#ifndef GFUNCMIN_H
#define GFUNCMIN_H

#include "math/gvector.h"
#include "gfunc.h"

//
// gFunctionMinimizer is an abstract base class for function minimization
//
class gFunctionMinimizer {
public:
  virtual ~gFunctionMinimizer() { }

  virtual void Set(const gC1Function<double> &fdf,
		   const gVector<double> &x, double &f,
		   gVector<double> &gradient, double step_size,
		   double p_tol) = 0;
  virtual void Restart(void) = 0;

  virtual bool Iterate(const gC1Function<double> &fdf,
		       gVector<double> &x, double &f,
		       gVector<double> &gradient, gVector<double> &dx) = 0;
};

//
// gConjugatePR: implements Polak-Ribiere conjugate gradient descent
//
class gConjugatePR : public gFunctionMinimizer {
private:
  int iter;
  double step;
  double max_step;
  double tol;
  gVector<double> x1;
  gVector<double> dx1;
  gVector<double> x2;
  double pnorm;
  gVector<double> p;
  double g0norm;
  gVector<double> g0;

public:
  gConjugatePR(int n);
  virtual ~gConjugatePR() { }

  void Set(const gC1Function<double> &fdf,
	   const gVector<double> &x, double &f,
	   gVector<double> &gradient, double step_size,
	   double p_tol);
  void Restart(void);

  bool Iterate(const gC1Function<double> &fdf,
	       gVector<double> &x, double &f,
	       gVector<double> &gradient, gVector<double> &dx);
};

class gFuncMinException { };

#endif  // GFUNCMIN_H
