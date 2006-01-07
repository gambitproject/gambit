//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Doubly-partitioned vector class
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

#ifndef GDPVECT_H
#define GDPVECT_H

#include "gpvector.h"

template <class T> class gbtDPVector : public gbtPVector<T>  {
  private:
    int sum(int part, const gbtPVector<int> &v) const;
    void setindex(void);

    bool Check(const gbtDPVector<T> &) const;

  protected:
    T ***dvptr;
    gbtArray<int> dvlen, dvidx;

  public:
    gbtDPVector(void);
    gbtDPVector(const gbtPVector<int> &sig);
    gbtDPVector(const Gambit::Vector<T> &val, const gbtPVector<int> &sig);
    gbtDPVector(const gbtDPVector<T> &v);
    virtual ~gbtDPVector();

    T &operator()(int a, int b, int c);
    const T &operator()(int a, int b, int c) const;

	// extract a subvector
    void CopySubRow(int row, int col,  const gbtDPVector<T> &v);

    gbtDPVector<T> &operator=(const gbtDPVector<T> &v);
    gbtDPVector<T> &operator=(const gbtPVector<T> &v);
    gbtDPVector<T> &operator=(const Gambit::Vector<T> &v);
    gbtDPVector<T> &operator=(T c);

    gbtDPVector<T> operator+(const gbtDPVector<T> &v) const;
    gbtDPVector<T> &operator+=(const gbtDPVector<T> &v);
    gbtDPVector<T> operator-(void) const;
    gbtDPVector<T> operator-(const gbtDPVector<T> &v) const;
    gbtDPVector<T> &operator-=(const gbtDPVector<T> &v);
    T operator*(const gbtDPVector<T> &v) const;
    gbtDPVector<T> &operator*=(const T &c);
    gbtDPVector<T> operator/(const T &c) const;
    
    bool operator==(const gbtDPVector<T> &v) const;
    bool operator!=(const gbtDPVector<T> &v) const;

    const gbtArray<int> &DPLengths(void) const;
};

#endif   // GDPVECTOR_H



