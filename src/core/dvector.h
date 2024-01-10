//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/dvector.h
// Doubly-partitioned vector class
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

#ifndef LIBGAMBIT_DVECTOR_H
#define LIBGAMBIT_DVECTOR_H

#include "pvector.h"

namespace Gambit {

template <class T> class DVector : public PVector<T>  {
private:
  void setindex();

protected:
  T ***dvptr;
  Array<int> dvlen, dvidx;

public:
  explicit DVector(const PVector<int> &sig);
  DVector(const DVector<T> &v);
  ~DVector() override;

  T &operator()(int a, int b, int c);
  const T &operator()(int a, int b, int c) const;

  DVector<T> &operator=(T c);
  DVector<T> &operator=(const Vector<T> &v)
  { static_cast<Vector<T> &>(*this) = v; return *this; }
};

} // end namespace Gambit

#endif // LIBGAMBIT_DVECTOR_H



