//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of a generic array container class
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

#ifndef GBLOCK_H
#define GBLOCK_H

#include <stdlib.h>
#include "garray.h"

template <class T> class gbtBlock : public gbtArray<T>   {
private:
  //
  // Insert the given element at the specified location.  Does not do any
  // error checking as it assumes that the members which call it have already
  // determined the proper location.  Returns the location at which the new
  // element was placed.
  //
  int InsertAt(const T &t, int where);

public:
  //
  // Constructs a block of the given length.  All elements of the block
  // are constructed according to the default constructor for type T.
  //
  gbtBlock(unsigned int len = 0) : gbtArray<T>(len) { }
      
  //
  // Constructs a block starting at lo, and going to hi.
  //
  gbtBlock(int lo, int hi) : gbtArray<T>(lo, hi) { }
  //
  // Constructs a block to have the same contents as another block.  This
  // uses copy semantics.
  //
  gbtBlock(const gbtBlock<T> &b) : gbtArray<T>(b) { }
  //
  // Deallocates the block of memory, calling the destructors for any
  // elements still within the block.
  //
  virtual ~gbtBlock() { }
  
  //
  // Sets the block to have the same contents as another block.
  //
  gbtBlock<T> &operator=(const gbtBlock<T> &b)
  {
    gbtArray<T>::operator=(b);  return *this; 
  }
  
  //
  // Append an element to a gbtBlock.  Operator overloaded for ease of use.
  //+grp
  gbtBlock<T> operator+(const T &e) const;
  gbtBlock<T>& operator+=(const T &e);
  //-grp
  
  //
  // Concatenate two gbtBlocks.  + puts the result in a separate gbtBlock,
  // while += puts it in the first argument.
  //+grp
  gbtBlock<T> operator+(const gbtBlock<T>& b) const;
  gbtBlock<T>& operator+=(const gbtBlock<T>& b);
  //-grp

  //
  // Append a new element to the block, and return the index at which the
  // element can be found.  Note that this index is guaranteed to be the
  // last (highest) index in the block.
  //
  int Append(const T &);
  //
  // Insert a new element into the block at a given index.  If the index is
  // less than 1, the element is inserted at index 1; if the index is greater
  // than the highest index, the element is appended to the end of the block.
  // Returns the index at which the element actually is placed.
  //
  int Insert(const T &, int);
  //
  // Remove the element at a given index from the block.  Returns the value
  // of the element removed.
  //
  T Remove(int);
  
  //
  // Return the index at which a given element resides in the block, or zero
  // if the element is not found.
  //
  int Find(const T &) const;
  //
  // Return true (nonzero) if the element is currently residing in the block.
  //
  bool Contains(const T &t) const { return Find(t); }
  //
  // Empty the block
  //
  void Flush(void);
};

template <class T> gbtBlock<T> gbtBlock<T>::operator+(const gbtBlock<T>& b) const
{
  gbtBlock<T> result(*this);
  for (int i = b.m_mindex; i <= b.m_maxdex; i++)
    result.Append(b[i]);
  return result;
}

template <class T> gbtBlock<T> gbtBlock<T>::operator+(const T &e) const
{
  gbtBlock<T> result(*this);
  result.Append(e);
  return result;
}

template <class T> gbtBlock<T> &gbtBlock<T>::operator+=(const T &e)
{
  Append(e);
  return *this;
}

template <class T> gbtBlock<T> &gbtBlock<T>::operator+=(const gbtBlock<T> &b)
{
  *this = *this + b;
  return *this;
}

template <class T> int gbtBlock<T>::InsertAt(const T &t, int n)
{
  if (m_mindex > n || n > m_maxdex + 1)   throw gbtIndexException();
    
  T *new_data = new T[++m_maxdex - m_mindex + 1] - m_mindex;

  int i;
  for (i = m_mindex; i <= n - 1; i++)       new_data[i] = m_data[i];
  new_data[i++] = t;
  for (; i <= m_maxdex; i++)       new_data[i] = m_data[i - 1];

  if (m_data)   delete [] (m_data + m_mindex);
  m_data = new_data;

  return n;
}

template <class T> int gbtBlock<T>::Append(const T &t)
{
  return InsertAt(t, m_maxdex + 1);
}

template <class T> int gbtBlock<T>::Insert(const T &t, int n)
{
  return InsertAt(t, (n < m_mindex) ? m_mindex : ((n > m_maxdex + 1) ? m_maxdex + 1 : n));
}

template <class T> T gbtBlock<T>::Remove(int n)
{
  if (n < m_mindex || n > m_maxdex)    throw gbtIndexException();

  T ret(m_data[n]);
  T *new_data = (--m_maxdex>=m_mindex) ? new T[m_maxdex-m_mindex+1] - m_mindex : 0;

  int i;
  for (i = m_mindex; i < n; i++)     new_data[i] = m_data[i];
  for (; i <= m_maxdex; i++)         new_data[i] = m_data[i + 1];

  delete [] (m_data + m_mindex);
  m_data = new_data;

  return ret;
}

template <class T> int gbtBlock<T>::Find(const T &t) const
{
  int i;
  for (i = m_mindex; i <= m_maxdex && m_data[i] != t; i++);
  return (i <= m_maxdex) ? i : 0;
} 

template <class T> void gbtBlock<T>::Flush(void)
{ 
  m_maxdex = m_mindex - 1;  
  if (m_data)  delete [] (m_data + m_mindex);  
  m_data = 0; 
}

#endif    // GBLOCK_H
