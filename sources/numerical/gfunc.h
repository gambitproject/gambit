//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Abstract base classes representing functions
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


#ifndef GFUNC_H
#define GFUNC_H

#include "base/base.h"

template <class T> class gVector;
template <class T> class gMatrix;

template <class T> class gFunction   {
  public:
    virtual ~gFunction()    { }
    virtual T Value(const gVector<T> &) const = 0;
};

template <class T> class gC1Function : public gFunction<T> {
public:
  virtual ~gC1Function() { }

  virtual bool Gradient(const gVector<T> &, gVector<T> &) const = 0;
};

class gFuncMinError : public gException {
public:
  virtual ~gFuncMinError() { }
  gText Description(void) const 
    { return "Internal error in minimization code"; }
};


#endif   // GFUNC_H
