//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/funcmin.h
// Interface to N-dimensional function minimization routines
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

#include "libgambit/vector.h"

template <class T> class gFunction   {
  public:
    virtual ~gFunction()    { }
    virtual T Value(const Gambit::Vector<T> &) const = 0;
};

template <class T> class gC1Function : public gFunction<T> {
public:
  virtual ~gC1Function() { }

  virtual bool Gradient(const Gambit::Vector<T> &, Gambit::Vector<T> &) const = 0;
};

class gbtFuncMinError : public Gambit::Exception {
public:
  virtual ~gbtFuncMinError() throw() { }
  const char *what(void) const throw() { return "Internal error in minimization code"; }
};

//
// gFunctionMinimizer is an abstract base class for function minimization
//
class gFunctionMinimizer {
public:
  virtual ~gFunctionMinimizer() { }

  virtual void Set(const gC1Function<double> &fdf,
		   const Gambit::Vector<double> &x, double &f,
		   Gambit::Vector<double> &gradient, double step_size,
		   double p_tol) = 0;
  virtual void Restart(void) = 0;

  virtual bool Iterate(const gC1Function<double> &fdf,
		       Gambit::Vector<double> &x, double &f,
		       Gambit::Vector<double> &gradient, Gambit::Vector<double> &dx) = 0;
};

//
// gConjugatePR: implements Polak-Ribiere conjugate gradient descent
//
class gConjugatePR : public gFunctionMinimizer {
private:
  int iter;
  double step;
  double max_step;
  double m_tol;
  Gambit::Vector<double> x1;
  Gambit::Vector<double> dx1;
  Gambit::Vector<double> x2;
  double pnorm;
  Gambit::Vector<double> p;
  double g0norm;
  Gambit::Vector<double> g0;

public:
  gConjugatePR(int n);
  virtual ~gConjugatePR() { }

  void Set(const gC1Function<double> &fdf,
	   const Gambit::Vector<double> &x, double &f,
	   Gambit::Vector<double> &gradient, double step_size,
	   double p_tol);
  void Restart(void);

  bool Iterate(const gC1Function<double> &fdf,
	       Gambit::Vector<double> &x, double &f,
	       Gambit::Vector<double> &gradient, Gambit::Vector<double> &dx);
};

class gFuncMinException { };

#endif  // GFUNCMIN_H
