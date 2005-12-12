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

template <class T> class gbtList  {
protected:
  class gNode   {
  public:
    T m_data;
    gNode *m_prev, *m_next;
    
    // CONSTRUCTOR
    gNode(const T &p_data, gNode *p_prev, gNode *p_next);
  };

  int m_length;
  gNode *m_head, *m_tail;

  int m_currentIndex;
  gNode *m_currentNode;

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

  gbtList<T> operator+(const gbtList<T>& b) const;
  gbtList<T>& operator+=(const gbtList<T>& b);

  virtual int Append(const T &);
  int Insert(const T &, int);
  virtual T Remove(int);

  int Find(const T &) const;
  bool Contains(const T &t) const;
  int Length(void) const { return m_length; }
};


//--------------------------------------------------------------------------
//                 gNode: Member function implementations
//--------------------------------------------------------------------------

template <class T> 
gbtList<T>::gNode::gNode(const T &p_data,
		       gbtList<T>::gNode *p_prev, gbtList<T>::gNode *p_next)
  : m_data(p_data), m_prev(p_prev), m_next(p_next)
{ }

//--------------------------------------------------------------------------
//                 gbtList<T>: Member function implementations
//--------------------------------------------------------------------------

template <class T> gbtList<T>::gbtList(void) 
: m_length(0), m_head(0), m_tail(0), m_currentIndex(0), m_currentNode(0)
{ }

template <class T> gbtList<T>::gbtList(const gbtList<T> &b)
 : m_length(b.m_length)
{
  if (m_length)  {  
    gNode *n = b.m_head;
    m_head = new gNode(n->m_data, 0, 0);
    n = n->m_next;
    m_tail = m_head;
    while (n)  { 
      m_tail->m_next = new gNode(n->m_data, m_tail, 0);
      n = n->m_next;
      m_tail = m_tail->m_next;
    }
    m_currentIndex = 1;
    m_currentNode = m_head;
  }
  else  {
    m_head = m_tail = 0;
    m_currentIndex = 0;
    m_currentNode = 0;
  }
}

template <class T> gbtList<T>::~gbtList()
{
  gNode *n = m_head;
  while (n)  {
    gNode *m_next = n->m_next;
    delete n;
    n = m_next;
  }
}

template <class T> int gbtList<T>::InsertAt(const T &t, int num)
{
  if (num < 1 || num > m_length + 1)   throw gbtIndexException();
  
  if (!m_length)  {
    m_head = m_tail = new gNode(t, 0, 0);
    m_length = 1;
    m_currentIndex = 1;
    m_currentNode = m_head;
    return m_length;
  }

  gNode *n;
  int i;
  
  if( num <= 1 )  {
    n = new gNode(t, 0, m_head);
    m_head->m_prev = n;
    m_currentNode = m_head = n;
    m_currentIndex = 1;
  }
  else if( num >= m_length + 1)  {
    n = new gNode(t, m_tail, 0);
    m_tail->m_next = n;
    m_currentNode = m_tail = n;
    m_currentIndex = m_length + 1;
  }
  else  {
    assert( m_currentIndex >= 1 && m_currentIndex <= m_length );
    if( num < m_currentIndex )
      for (i = m_currentIndex, n = m_currentNode; i > num; i--, n = n->m_prev);
		else
      for (i = m_currentIndex, n = m_currentNode; i < num; i++, n = n->m_next);
    n = new gNode(t, n->m_prev, n);
    m_currentNode = n->m_prev->m_next = n->m_next->m_prev = n;
    m_currentIndex = num;
  }

  m_length++;
  return num;
}

//--------------------- visible functions ------------------------

template <class T> gbtList<T> &gbtList<T>::operator=(const gbtList<T> &b)
{
  if (this != &b)   {
    gNode *n = m_head;
    while (n) {
      gNode *m_next = n->m_next;
      delete n;
      n = m_next;
    }

    m_length = b.m_length;
    m_currentIndex = b.m_currentIndex;
    if (m_length)   {
      gNode *n = b.m_head;
      m_head = new gNode(n->m_data, 0, 0);
      if (b.m_currentNode == n) m_currentNode = m_head;
      n = n->m_next;
      m_tail = m_head;
      while (n)  {
	m_tail->m_next = new gNode(n->m_data, m_tail, 0);
	if (b.m_currentNode == n) m_currentNode = m_tail->m_next;
	n = n->m_next;
	m_tail = m_tail->m_next;
      }
    }
    else
      m_head = m_tail = 0;
  }
  return *this;
}

template <class T> bool gbtList<T>::operator==(const gbtList<T> &b) const
{
  if (m_length != b.m_length) return false;
  for (gNode *m = m_head, *n = b.m_head; m; m = m->m_next, n = n->m_next)
    if (m->m_data != n->m_data)  return false;
  return true;
}

template <class T> bool gbtList<T>::operator!=(const gbtList<T> &b) const
{
  return !(*this == b);
}

template <class T> const T &gbtList<T>::operator[](int num) const
{
  if (num < 1 || num > m_length)    throw gbtIndexException();

  int i;
  gNode *n;
  if( num < m_currentIndex )
    for (i = m_currentIndex, n = m_currentNode; i > num; i--, n = n->m_prev);
  else
    for (i = m_currentIndex, n = m_currentNode; i < num; i++, n = n->m_next);
  return n->m_data;
}

template <class T> T &gbtList<T>::operator[](int num)
{
  if (num < 1 || num > m_length)   throw gbtIndexException();
  gNode *n;
  int i;
  if( num < m_currentIndex )
    for (i = m_currentIndex, n = m_currentNode; i > num; i--, n = n->m_prev);
  else
    for (i = m_currentIndex, n = m_currentNode; i < num; i++, n = n->m_next);
  m_currentIndex = i;
  m_currentNode = n;
  return n->m_data;
}

template <class T> gbtList<T> gbtList<T>::operator+(const gbtList<T> &b) const
{
  gbtList<T> result(*this);
  gNode *n = b.m_head;
  while (n)  {
    result.Append(n->data);
    n = n->m_next;
  }
  return result;
}

template <class T> gbtList<T> &gbtList<T>::operator+=(const gbtList<T> &b)
{
  gNode *n = b.m_head;
  
  while (n)  {
    Append(n->m_data);
    n = n->m_next;
  }
  return *this;
}

template <class T> int gbtList<T>::Append(const T &t)
{
  return InsertAt(t, m_length + 1);
}

template <class T> int gbtList<T>::Insert(const T &t, int n)
{
  return InsertAt(t, (n < 1) ? 1 : ((n > m_length + 1) ? m_length + 1 : n));
}

template <class T> T gbtList<T>::Remove(int num)
{
  if (num < 1 || num > m_length)   throw gbtIndexException();
  gNode *n;
  int i;

  if( num < m_currentIndex )
    for (i = m_currentIndex, n = m_currentNode; i > num; i--, n = n->m_prev);
  else
    for (i = m_currentIndex, n = m_currentNode; i < num; i++, n = n->m_next);

  if (n->m_prev)
    n->m_prev->m_next = n->m_next;
  else
    m_head = n->m_next;
  if (n->m_next)
    n->m_next->m_prev = n->m_prev;
  else
    m_tail = n->m_prev;

  m_length--;
  m_currentIndex = i;
  m_currentNode = n->m_next;
  if (m_currentIndex > m_length) {
    m_currentIndex = m_length;
    m_currentNode = m_tail;
  }
  T ret = n->m_data;
  delete n;
  return ret;
}

template <class T> int gbtList<T>::Find(const T &t) const
{
  if (m_length == 0)  return 0;
  gNode *n = m_head;
  for (int i = 1; n; i++, n = n->m_next)
    if (n->m_data == t)   return i;
  return 0;
}

template <class T> bool gbtList<T>::Contains(const T &t) const
{
  return (Find(t) != 0);
}

#endif    // GLIST_H

