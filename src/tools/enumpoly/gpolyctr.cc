//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpolyctr.cc
// Implementation of specialized containers for polynomials
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

#include "gpolyctr.h"

template <class T>
gPolyArray<T>::gPolyArray(const gSpace *s, const term_order *t, int len)
  : mindex(1), maxdex(len)
{
  //assert(len>=0);
  data = (len) ? new gPoly<T>*[len] - 1 : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gPoly<T>(s, t);
}

template <class T>
gPolyArray<T>::gPolyArray(const gSpace *s, const term_order *t, int lo, int hi)
  : mindex(lo), maxdex(hi)
{
  //assert(maxdex + 1 >= mindex);
  data = (maxdex>=mindex) ? new gPoly<T>*[maxdex-mindex+1] - mindex : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gPoly<T>(s, t);
}

template <class T> gPolyArray<T>::gPolyArray(const gPolyArray<T> &a)
  : mindex(a.mindex), maxdex(a.maxdex)
{
  data = (maxdex>=mindex) ? new gPoly<T>*[maxdex-mindex+1] - mindex : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gPoly<T>(*a.data[i]);
}

template <class T> gPolyArray<T>::~gPolyArray()
{
  if (maxdex>=mindex)  {
    for (int i = mindex; i <= maxdex; delete data[i++]);
    delete [] (data + mindex);
  }
}

template <class T>
gPolyArray<T> &gPolyArray<T>::operator=(const gPolyArray<T> &a)
{
  if (this != &a) {
    // We only reallocate if necessary.  This should be somewhat faster
    // if many objects are of the same length.  Furthermore, it is
    // _essential_ for the correctness of the Gambit::PVector and Gambit::DVector
    // assignment operator, since it assumes the value of data does
    // not change.
    if (!data || (data && (mindex != a.mindex || maxdex != a.maxdex)))  {
      if (data)  {
        for (int i = mindex; i <= maxdex; delete data[i++]);
        delete [] (data + mindex);
      }
      mindex = a.mindex;   maxdex = a.maxdex;
      data = (maxdex >= mindex) ? new gPoly<T>*[maxdex - mindex + 1] - mindex : 0;
    }
    for (int i = mindex; i <= maxdex; i++)
      data[i] = new gPoly<T>(*a.data[i]);
  }
  return *this;
}

template <class T> int gPolyArray<T>::Length(void) const
{
  return maxdex - mindex + 1;
}	

template <class T> int gPolyArray<T>::First(void) const
{
  return mindex;
}	

template <class T> int gPolyArray<T>::Last(void) const
{
  return maxdex;
}	

template <class T> const gPoly<T> &gPolyArray<T>::operator[](int index) const
{
  if (index < mindex || index > maxdex) {
    throw Gambit::IndexException();
  }
  return *data[index];
}	

template <class T> gPoly<T> &gPolyArray<T>::operator[](int index)
{
  if (index < mindex || index > maxdex) {
    throw Gambit::IndexException();
  }
  return *data[index];
}

template <class T> void gPolyArray<T>::Dump(gOutput &f) const
{
  f << "{ ";
  for (int i = mindex; i <= maxdex; i++)
    f << *data[i] << ' ';
  f << '}';
}

template <class T>
gPolyBlock<T>::gPolyBlock(const gSpace *s, const term_order *t, int len)
  : gPolyArray<T>(s, t, len)   { }

template <class T>
gPolyBlock<T>::gPolyBlock(const gSpace *s, const term_order *t, int lo, int hi)
  : gPolyArray<T>(s, t, lo, hi)   { }

template <class T>
gPolyBlock<T>::gPolyBlock(const gPolyBlock<T> &b) : gPolyArray<T>(b)  { }

template <class T> gPolyBlock<T>::~gPolyBlock()   { }

template <class T> gPolyBlock<T> &gPolyBlock<T>::operator=(const gPolyBlock<T> &b)
{
  gPolyArray<T>::operator=(b);
  return *this;
}

template <class T> bool gPolyBlock<T>::operator==(const gPolyBlock<T> &b) const
{
  if (mindex != b.mindex || maxdex != b.maxdex) return 0;
  for (int i = mindex; i <= maxdex; i++) 
    if (*data[i] != *b.data[i]) return 0;
  return 1;
}

template <class T> bool gPolyBlock<T>::operator!=(const gPolyBlock<T> &b) const
{
  return !(*this == b);
}

template <class T> int gPolyBlock<T>::InsertAt(const gPoly<T> &t, int n)
{
  if (mindex > n || n > maxdex+1) {
    throw Gambit::IndexException();
  }
  gPoly<T> **new_data = new gPoly<T>*[++maxdex-mindex+1] - mindex;

  int i;
  for (i = mindex; i <= n - 1; i++)       new_data[i] = data[i];
  new_data[i++] = new gPoly<T>(t);
  for (; i <= maxdex; i++)       new_data[i] = data[i - 1];

  if (data)   delete [] (data + mindex);
  data = new_data;

  return n;
}

template <class T> int gPolyBlock<T>::Append(const gPoly<T> &t)
{
  return InsertAt(t, maxdex + 1);
}

template <class T> int gPolyBlock<T>::Insert(const gPoly<T> &t, int n)
{
  return InsertAt(t, (n < mindex) ? mindex : ((n > maxdex + 1) ? maxdex + 1 : n));
}

template <class T> gPoly<T> gPolyBlock<T>::Remove(int n)
{
  if (n < mindex || n > maxdex) {
    throw Gambit::IndexException();
  }
  gPoly<T> ret(*data[n]);

  gPoly<T> **new_data = (--maxdex>=mindex) ? new gPoly<T>*[maxdex-mindex+1] - mindex : 0;

  int i;
  for (i = mindex; i < n; i++)      new_data[i] = data[i];
  for (; i <= maxdex; i++)         new_data[i] = data[i + 1];

  delete data[n];
  delete [] (data + mindex);
  data = new_data;

  return ret;
}

template <class T> int gPolyBlock<T>::Find(const gPoly<T> &t) const
{
  int i;
  for (i = mindex; i <= maxdex && *data[i] != t; i++);
  return (i <= maxdex) ? i : 0;
} 

template <class T> int gPolyBlock<T>::Contains(const gPoly<T> &t) const
{ return Find(t); }

template <class T> void gBlock<T>::Flush(void)
{
  for (int i = mindex; i <= maxdex; delete data[i++]);
  maxdex = mindex - 1;
  if (data)   { delete [] (data + mindex);  data = 0; }
}

#include "math/gnumber.h"

template class gPolyArray<gbtNumber>;
template class gPolyBlock<gbtNumber>;


#include "base/glist.imp"

template class Gambit::List<gPoly<gbtNumber> >;
