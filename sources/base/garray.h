//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A basic bounds-checked array type
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

#ifndef GARRAY_H
#define GARRAY_H

#include "gmisc.h"
#include <stdlib.h>

template <class T> class gbtArray  {
protected:
  int m_mindex, m_maxdex;
  T *m_data;

public:
  //!
  //! @name Lifecycle
  //!
  //@{

  //! Constructs an array of the specified length, indexing first element as 1
  
  //!
  //! Constructs an array of the specified length, with indexing of
  //! the first element as element 1.  The array contents are not initialized.
  //! @param p_length The length of the array (defaults to zero)
  //!
  gbtArray(unsigned int p_length = 0)
    : m_mindex(1), m_maxdex(p_length), 
      m_data((p_length) ? new T[p_length] - 1 : 0)   
  { }

  //! Constructs an array indexed by [p_low, p_high]

  //!
  //! Constructs an array whose first element is indexed by p_low,
  //! and whose last element is indexed by p_high.  The array contents are
  //! not initialized.
  //!
  gbtArray(int p_low, int p_high)
    : m_mindex(p_low), m_maxdex(p_high)
  {
    if (m_maxdex + 1 < m_mindex)   throw gbtRangeException();
    m_data = (m_maxdex >= m_mindex) ? new T[m_maxdex -m_mindex + 1] - m_mindex : 0;
  }

  /// Copy constructor
  gbtArray(const gbtArray<T> &a) 
    : m_mindex(a.m_mindex), m_maxdex(a.m_maxdex),
      m_data((m_maxdex >= m_mindex) ? new T[m_maxdex - m_mindex + 1] - m_mindex : 0)
  {
    for (int i = m_mindex; i <= m_maxdex; i++) m_data[i] = a.m_data[i];
  }

  //! Cleans up the array
  
  //!
  //! Cleans up the array by deallocating the storage.  Note that
  //! if the contents of the array are pointers, that the delete operator
  //! is not called on the contained pointers.
  //!
  virtual ~gbtArray()
  {
    if (m_maxdex >= m_mindex)  delete [] (m_data + m_mindex);
  }

  //! Copies the contents of an array
  gbtArray<T> &operator=(const gbtArray<T> &a)
  {
    if (this != &a) {
      // We only reallocate if necessary.  This should be somewhat faster
      // if many objects are of the same length.  Furthermore, it is
      // _essential_ for the correctness of the gbtPVector and gbtDPVector
      // assignment operator, since it assumes the value of m_data does
      // not change.
      if (!m_data || (m_data && (m_mindex != a.m_mindex || m_maxdex != a.m_maxdex)))  {
	if (m_data)   delete [] (m_data + m_mindex);
	m_mindex = a.m_mindex;   m_maxdex = a.m_maxdex;
	m_data = (m_maxdex >= m_mindex) ? new T[m_maxdex - m_mindex + 1] - m_mindex : 0;
      }

      for (int i = m_mindex; i <= m_maxdex; i++) m_data[i] = a.m_data[i];
    }

    return *this;
  }

  //@}

  //!
  //! @name Information about the array
  //!
  //@{
  /// Returns the number of elements in this array
  int Length(void) const throw () { return m_maxdex - m_mindex + 1; }

  /// Returns the index of the first element of the array
  int First(void) const throw () { return m_mindex; }

  /// Returns the index of the last element of the array
  int Last(void) const throw () { return m_maxdex; }

  //@}

  //!
  //! @name Subscripting the array
  //!
  //@{
  //! Returns the element at the p_index'th location in the array
  const T &operator[](int p_index) const throw (gbtIndexException)
  {
    if (p_index < m_mindex || p_index > m_maxdex) throw gbtIndexException();
    return m_data[p_index];
  }

  //! Returns the element at the p_index'th location in the array
  T &operator[](int p_index) throw (gbtIndexException)
  {
    if (p_index < m_mindex || p_index > m_maxdex) throw gbtIndexException();
    return m_data[p_index];
  }
  //@}
};

//! Test if two arrays are equal

//!
//! Test if two arrays are equal.  Defined external to array to avoid
//! requiring the == operator to be defined on the contained type;
//! this function requires the == operator.
//!
template <class T> bool operator==(const gbtArray<T> &a, const gbtArray<T> &b)
{
  if (a.First() != b.First() || a.Last() != b.Last())   return false;
  for (int i = a.First(); i <= a.Last(); i++)
    if (a[i] != b[i])   return false;
  return true;
}

//! Test if two arrays are not equal
template <class T> bool operator!=(const gbtArray<T> &a, const gbtArray<T> &b)
{ return !(a == b); }


#endif	// GARRAY_H
