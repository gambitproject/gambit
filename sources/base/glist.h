//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A generic (doubly) linked-list container class
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

#ifndef GLIST_H
#define GLIST_H

#include "gmisc.h"

template <class T> class gbtList  {
  protected:

  class gbtNode   {
  public:
    T data;
    gbtNode *prev, *next;
 
    // CONSTRUCTOR
    gbtNode(const T &_data, gbtNode *_prev, gbtNode *_next);
  };

    int length;
    gbtNode *head, *tail;

    int CurrIndex;
    gbtNode *CurrNode;

    int InsertAt(const T &t, int where);

  public:
    gbtList(void);
    gbtList(const gbtList<T> &);
    virtual ~gbtList();

    gbtList<T> &operator=(const gbtList<T> &);

    bool operator==(const gbtList<T> &b) const;
    bool operator!=(const gbtList<T> &b) const;

    const T &operator[](int) const;
    T &operator[](int);

    gbtList<T> operator+(const T &e) const;
    gbtList<T>& operator+=(const T &e);

    gbtList<T> operator+(const gbtList<T>& b) const;
    gbtList<T>& operator+=(const gbtList<T>& b);

    gbtList<T> &Combine(gbtList<T> &b);
    gbtList<T> InteriorSegment(int, int) const;

    virtual int Append(const T &);
    int Insert(const T &, int);
    virtual T Remove(int);

    bool HasARedundancy();
    void RemoveRedundancies();

    int Find(const T &) const;
    bool Contains(const T &t) const;
    int Length(void) const;

    virtual void Flush(void);
};

//--------------------------------------------------------------------------
//                 gbtNode: Member function implementations
//--------------------------------------------------------------------------

template <class T> 
gbtList<T>::gbtNode::gbtNode(const T &_data,
		       gbtList<T>::gbtNode *_prev, gbtList<T>::gbtNode *_next)
  : data(_data), prev(_prev), next(_next)
{ }

//--------------------------------------------------------------------------
//                 gbtList<T>: Member function implementations
//--------------------------------------------------------------------------

template <class T> gbtList<T>::gbtList(void) 
: length(0), head(0), tail(0), CurrIndex(0), CurrNode(0)
{ }

template <class T> gbtList<T>::gbtList(const gbtList<T> &b)
 : length(b.length)
{
  if (length)  {  
    gbtNode *n = b.head;
    head = new gbtNode(n->data, 0, 0);
    n = n->next;
    tail = head;
    while (n)  { 
      tail->next = new gbtNode(n->data, tail, 0);
      n = n->next;
      tail = tail->next;
    }
    CurrIndex = 1;
    CurrNode = head;
  }
  else  {
    head = tail = 0;
    CurrIndex = 0;
    CurrNode = 0;
  }
}

template <class T> gbtList<T>::~gbtList()
{
  Flush();
}

template <class T> int gbtList<T>::InsertAt(const T &t, int num)
{
  if (num < 1 || num > length + 1)   throw gbtIndexException();
  
  if (!length)  {
    head = tail = new gbtNode(t, 0, 0);
    length = 1;
    CurrIndex = 1;
    CurrNode = head;
    return length;
  }

  gbtNode *n;
  int i;
  
  if( num <= 1 )  {
    n = new gbtNode(t, 0, head);
    head->prev = n;
    CurrNode = head = n;
    CurrIndex = 1;
  }
  else if( num >= length + 1)  {
    n = new gbtNode(t, tail, 0);
    tail->next = n;
    CurrNode = tail = n;
    CurrIndex = length + 1;
  }
  else  {
    if( num < CurrIndex )
      for (i = CurrIndex, n = CurrNode; i > num; i--, n = n->prev);
		else
      for (i = CurrIndex, n = CurrNode; i < num; i++, n = n->next);
    n = new gbtNode(t, n->prev, n);
    CurrNode = n->prev->next = n->next->prev = n;
    CurrIndex = num;
  }

  length++;
  return num;
}

//--------------------- visible functions ------------------------

template <class T> gbtList<T> &gbtList<T>::operator=(const gbtList<T> &b)
{
  if (this != &b)   {
    Flush();
    length = b.length;
    CurrIndex = b.CurrIndex;
    if (length)   {
      gbtNode *n = b.head;
      head = new gbtNode(n->data, 0, 0);
      if (b.CurrNode == n) CurrNode = head;
      n = n->next;
      tail = head;
      while (n)  {
	tail->next = new gbtNode(n->data, tail, 0);
	if (b.CurrNode == n) CurrNode = tail->next;
	n = n->next;
	tail = tail->next;
      }
    }
    else
      head = tail = 0;
  }
  return *this;
}

template <class T> bool gbtList<T>::operator==(const gbtList<T> &b) const
{
  if (length != b.length) return false;
  for (gbtNode *m = head, *n = b.head; m; m = m->next, n = n->next)
    if (m->data != n->data)  return false;
  return true;
}

template <class T> bool gbtList<T>::operator!=(const gbtList<T> &b) const
{
  return !(*this == b);
}

template <class T> const T &gbtList<T>::operator[](int num) const
{
  if (num < 1 || num > length)  throw gbtIndexException();

  int i;
  gbtNode *n;
  if( num < CurrIndex )
    for (i = CurrIndex, n = CurrNode; i > num; i--, n = n->prev);
  else
    for (i = CurrIndex, n = CurrNode; i < num; i++, n = n->next);
  // CurrIndex = i;
  // CurrNode = n;
  return n->data;
}

template <class T> T &gbtList<T>::operator[](int num)
{
  if (num < 1 || num > length)   throw gbtIndexException();
  gbtNode *n;
  int i;
  if( num < CurrIndex )
    for (i = CurrIndex, n = CurrNode; i > num; i--, n = n->prev);
  else
    for (i = CurrIndex, n = CurrNode; i < num; i++, n = n->next);
  CurrIndex = i;
  CurrNode = n;
  return n->data;
}

template <class T> gbtList<T> gbtList<T>::operator+(const T &e) const
{
  gbtList<T> result(*this);
  result.Append(e);
  return result;
}

template <class T> gbtList<T> &gbtList<T>::operator+=(const T &e)
{
  Append(e);
  return *this;
}

template <class T> gbtList<T> gbtList<T>::operator+(const gbtList<T> &b) const
{
  gbtList<T> result(*this);
  gbtNode *n = b.head;
  while (n)  {
    result.Append(n->data);
    n = n->next;
  }
  return result;
}

template <class T> gbtList<T> &gbtList<T>::operator+=(const gbtList<T> &b)
{
  gbtNode *n = b.head;
  
  while (n)  {
    Append(n->data);
    n = n->next;
  }
  return *this;
}

template <class T> gbtList<T> &gbtList<T>::Combine(gbtList<T> &b)
{
  if (this == &b)   return *this;

  if (!head)   {
    head = b.head;
    tail = b.tail;
    length = b.length;
    b.head = 0;
    b.tail = 0;
    b.length = 0;
    b.CurrIndex = 0;
    b.CurrNode = 0;
    return *this;
  }

  tail->next = b.head;
  if (b.head)  b.head->prev = tail;
  length += b.length;
  if (b.tail)  tail = b.tail;
  b.head = 0;
  b.tail = 0;
  b.length = 0;
  b.CurrIndex = 0;
  b.CurrNode = 0;
  return *this;
}

template <class T> 
gbtList<T> gbtList<T>::InteriorSegment(int first, int last) const
{
  gbtList<T> answer;

  for (int i = first; i <= last; i++)
    answer += (*this)[i];

  return answer;
}

template <class T> int gbtList<T>::Append(const T &t)
{
  return InsertAt(t, length + 1);
}

template <class T> int gbtList<T>::Insert(const T &t, int n)
{
  return InsertAt(t, (n < 1) ? 1 : ((n > length + 1) ? length + 1 : n));
}

template <class T> T gbtList<T>::Remove(int num)
{
  if (num < 1 || num > length)   throw gbtIndexException();
  gbtNode *n;
  int i;

  if( num < CurrIndex )
    for (i = CurrIndex, n = CurrNode; i > num; i--, n = n->prev);
  else
    for (i = CurrIndex, n = CurrNode; i < num; i++, n = n->next);

  if (n->prev)
    n->prev->next = n->next;
  else
    head = n->next;
  if (n->next)
    n->next->prev = n->prev;
  else
    tail = n->prev;

  length--;
  CurrIndex = i;
  CurrNode = n->next;
  if( CurrIndex > length )
  {
    CurrIndex = length;
    CurrNode = tail;
  }
  T ret = n->data;
  delete n;
  return ret;
}

template <class T> bool gbtList<T>::HasARedundancy()
{
  int i = 1; int j = 2;		
  while (i < Length()) {
    if ((*this)[i] == (*this)[j])
      return true;
    else 
      j++;
    if (j > Length()) { i++; j = i+1; }
  }
  return false;
}

template <class T> void gbtList<T>::RemoveRedundancies()
{
  int i = 1; int j = 2;		
  while (i < Length()) {
    if ((*this)[i] == (*this)[j])
      Remove(j);
    else 
      j++;
    if (j > Length()) { i++; j = i+1; }
  }
}

template <class T> int gbtList<T>::Find(const T &t) const
{
  if (length == 0)  return 0;
  gbtNode *n = head;
  for (int i = 1; n; i++, n = n->next)
    if (n->data == t)   return i;
  return 0;
}

template <class T> bool gbtList<T>::Contains(const T &t) const
{
  return (Find(t) != 0);
}

template <class T> int gbtList<T>::Length(void) const
{
  return length;
}

template <class T> void gbtList<T>::Flush(void)
{
  length = 0;
  gbtNode *n = head;
  while (n)  {
    gbtNode *next = n->next;
    delete n;
    n = next;
  }
  head = tail = 0;
  CurrIndex = 0;
  CurrNode = 0;
}


#endif    // GLIST_H
