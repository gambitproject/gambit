//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of specialized containers for polynomials
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

#include "gpolyctr.h"

template <class T>
gbtPolyMultiArray<T>::gbtPolyMultiArray(const gbtPolySpace *s, const gbtPolyTermOrder *t, int len)
  : mindex(1), maxdex(len)
{
  assert(len>=0);
  data = (len) ? new gbtPolyMulti<T>*[len] - 1 : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gbtPolyMulti<T>(s, t);
}

template <class T>
gbtPolyMultiArray<T>::gbtPolyMultiArray(const gbtPolySpace *s, const gbtPolyTermOrder *t, int lo, int hi)
  : mindex(lo), maxdex(hi)
{
  assert(maxdex + 1 >= mindex);
  data = (maxdex>=mindex) ? new gbtPolyMulti<T>*[maxdex-mindex+1] - mindex : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gbtPolyMulti<T>(s, t);
}

template <class T> gbtPolyMultiArray<T>::gbtPolyMultiArray(const gbtPolyMultiArray<T> &a)
  : mindex(a.mindex), maxdex(a.maxdex)
{
  data = (maxdex>=mindex) ? new gbtPolyMulti<T>*[maxdex-mindex+1] - mindex : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gbtPolyMulti<T>(*a.data[i]);
}

template <class T> gbtPolyMultiArray<T>::~gbtPolyMultiArray()
{
  if (maxdex>=mindex)  {
    for (int i = mindex; i <= maxdex; delete data[i++]);
    delete [] (data + mindex);
  }
}

template <class T>
gbtPolyMultiArray<T> &gbtPolyMultiArray<T>::operator=(const gbtPolyMultiArray<T> &a)
{
  if (this != &a) {
    // We only reallocate if necessary.  This should be somewhat faster
    // if many objects are of the same length.  Furthermore, it is
    // _essential_ for the correctness of the gbtPVector and gbtDPVector
    // assignment operator, since it assumes the value of data does
    // not change.
    if (!data || (data && (mindex != a.mindex || maxdex != a.maxdex)))  {
      if (data)  {
        for (int i = mindex; i <= maxdex; delete data[i++]);
        delete [] (data + mindex);
      }
      mindex = a.mindex;   maxdex = a.maxdex;
      data = (maxdex >= mindex) ? new gbtPolyMulti<T>*[maxdex - mindex + 1] - mindex : 0;
    }
    for (int i = mindex; i <= maxdex; i++)
      data[i] = new gbtPolyMulti<T>(*a.data[i]);
  }
  return *this;
}

template <class T> int gbtPolyMultiArray<T>::Length(void) const
{
  return maxdex - mindex + 1;
}	

template <class T> int gbtPolyMultiArray<T>::First(void) const
{
  return mindex;
}	

template <class T> int gbtPolyMultiArray<T>::Last(void) const
{
  return maxdex;
}	

template <class T> const gbtPolyMulti<T> &gbtPolyMultiArray<T>::operator[](int index) const
{
  assert(index >= mindex && index <= maxdex);
  return *data[index];
}	

template <class T> gbtPolyMulti<T> &gbtPolyMultiArray<T>::operator[](int index)
{
  assert(index >= mindex && index <= maxdex);
  return *data[index];
}

template <class T> void gbtPolyMultiArray<T>::Dump(gbtOutput &f) const
{
  f << "{ ";
  for (int i = mindex; i <= maxdex; i++)
    f << *data[i] << ' ';
  f << '}';
}

template <class T>
gbtPolyMultiBlock<T>::gbtPolyMultiBlock(const gbtPolySpace *s, const gbtPolyTermOrder *t, int len)
  : gbtPolyMultiArray<T>(s, t, len)   { }

template <class T>
gbtPolyMultiBlock<T>::gbtPolyMultiBlock(const gbtPolySpace *s, const gbtPolyTermOrder *t, int lo, int hi)
  : gbtPolyMultiArray<T>(s, t, lo, hi)   { }

template <class T>
gbtPolyMultiBlock<T>::gbtPolyMultiBlock(const gbtPolyMultiBlock<T> &b) : gbtPolyMultiArray<T>(b)  { }

template <class T> gbtPolyMultiBlock<T>::~gbtPolyMultiBlock()   { }

template <class T> gbtPolyMultiBlock<T> &gbtPolyMultiBlock<T>::operator=(const gbtPolyMultiBlock<T> &b)
{
  gbtPolyMultiArray<T>::operator=(b);
  return *this;
}

template <class T> bool gbtPolyMultiBlock<T>::operator==(const gbtPolyMultiBlock<T> &b) const
{
  if (mindex != b.mindex || maxdex != b.maxdex) return 0;
  for (int i = mindex; i <= maxdex; i++) 
    if (*data[i] != *b.data[i]) return 0;
  return 1;
}

template <class T> bool gbtPolyMultiBlock<T>::operator!=(const gbtPolyMultiBlock<T> &b) const
{
  return !(*this == b);
}

template <class T> int gbtPolyMultiBlock<T>::InsertAt(const gbtPolyMulti<T> &t, int n)
{
  assert(mindex <=n && n <=maxdex+1);
  gbtPolyMulti<T> **new_data = new gbtPolyMulti<T>*[++maxdex-mindex+1] - mindex;

  int i;
  for (i = mindex; i <= n - 1; i++)       new_data[i] = data[i];
  new_data[i++] = new gbtPolyMulti<T>(t);
  for (; i <= maxdex; i++)       new_data[i] = data[i - 1];

  if (data)   delete [] (data + mindex);
  data = new_data;

  return n;
}

template <class T> int gbtPolyMultiBlock<T>::Append(const gbtPolyMulti<T> &t)
{
  return InsertAt(t, maxdex + 1);
}

template <class T> int gbtPolyMultiBlock<T>::Insert(const gbtPolyMulti<T> &t, int n)
{
  return InsertAt(t, (n < mindex) ? mindex : ((n > maxdex + 1) ? maxdex + 1 : n));
}

template <class T> gbtPolyMulti<T> gbtPolyMultiBlock<T>::Remove(int n)
{
  assert(n >= mindex && n <= maxdex);

  gbtPolyMulti<T> ret(*data[n]);

  gbtPolyMulti<T> **new_data = (--maxdex>=mindex) ? new gbtPolyMulti<T>*[maxdex-mindex+1] - mindex : 0;

  int i;
  for (i = mindex; i < n; i++)      new_data[i] = data[i];
  for (; i <= maxdex; i++)         new_data[i] = data[i + 1];

  delete data[n];
  delete [] (data + mindex);
  data = new_data;

  return ret;
}

template <class T> int gbtPolyMultiBlock<T>::Find(const gbtPolyMulti<T> &t) const
{
  int i;
  for (i = mindex; i <= maxdex && *data[i] != t; i++);
  return (i <= maxdex) ? i : 0;
} 

template <class T> int gbtPolyMultiBlock<T>::Contains(const gbtPolyMulti<T> &t) const
{ return Find(t); }

template <class T> void gbtBlock<T>::Flush(void)
{
  for (int i = mindex; i <= maxdex; delete data[i++]);
  maxdex = mindex - 1;
  if (data)   { delete [] (data + mindex);  data = 0; }
}

#include "math/gnumber.h"

template class gbtPolyMultiArray<gbtNumber>;
template class gbtPolyMultiBlock<gbtNumber>;


#include "base/glist.imp"

template class gbtList<gbtPolyMulti<gbtNumber> >;
