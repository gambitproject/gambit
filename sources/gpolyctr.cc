//
// FILE: gpolyctr.cc -- Specialized container types for polynomials
//
// $Id$
//

#include "gpolyctr.h"

template <class T>
gPolyArray<T>::gPolyArray(const gSpace *s, const term_order *t, int len)
  : mindex(1), maxdex(len)
{
  assert(len>=0);
  data = (len) ? new gPoly<T>*[len] - 1 : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gPoly<T>(s, t);
}

template <class T>
gPolyArray<T>::gPolyArray(const gSpace *s, const term_order *t, int lo, int hi)
  : mindex(lo), maxdex(hi)
{
  assert(maxdex + 1 >= mindex);
  data = (maxdex>=mindex) ? new gPoly<T>*[maxdex-mindex+1] - mindex : 0;
  for (int i = mindex; i <= maxdex; i++)
    data[i] = new gPoly<gNumber>(s, t);
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
    // _essential_ for the correctness of the gPVector and gDPVector
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
  assert(index >= mindex && index <= maxdex);
  return *data[index];
}	

template <class T> gPoly<T> &gPolyArray<T>::operator[](int index)
{
  assert(index >= mindex && index <= maxdex);
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
  assert(mindex <=n && n <=maxdex+1);
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
  assert(n >= mindex && n <= maxdex);

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

#include "gnumber.h"

template class gPolyArray<gNumber>;
template class gPolyBlock<gNumber>;


#include "glist.imp"

template class gList<gPoly<gNumber> >;
template class gNode<gPoly<gNumber> >;
