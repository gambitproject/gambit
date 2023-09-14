//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/list.h
// A generic (doubly) linked-list container class
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

#ifndef LIBGAMBIT_LIST_H
#define LIBGAMBIT_LIST_H

namespace Gambit {

/// A doubly-linked list container.
///
/// This implements a doubly-linked list.  A special feature of this
/// class is that it caches the last item accessed by indexing via
/// operator[], meaning that, if accesses are done in sequential order,
/// indexing time is constant.
///
/// This index-cacheing feature was implemented very early in the development
/// of Gambit, before STL-like concepts of iterators had been fully
/// developed.  An iterator approach is a better and more robust solution
/// to iterating lists, both in terms of performance and encapsulation.
/// Therefore, it is recommended to avoid operator[] and use the provided
/// iterator classes in new code, and to upgrade existing code to the
/// iterator idiom as practical.
template <class T> class List {
protected:
  class Node {
  public:
    T m_data;
    Node *m_prev, *m_next;
    
    // CONSTRUCTOR
    Node(const T &p_data, Node *p_prev, Node *p_next);
  };

  int m_length;
  Node *m_head, *m_tail;

  int m_currentIndex;
  Node *m_currentNode;

  int InsertAt(const T &t, int where);

public:
  class iterator {
  private:
    List &m_list;
    Node *m_node;
  public:
    iterator(List &p_list, Node *p_node)
      : m_list(p_list), m_node(p_node)  { }
    T &operator*()  { return m_node->m_data; }
    iterator &operator++()  { m_node = m_node->m_next; return *this; }
    bool operator==(const iterator &it) const
    { return (m_node == it.m_node); }
    bool operator!=(const iterator &it) const
    { return (m_node != it.m_node); }
  };

  class const_iterator {
  private:
    const List &m_list;
    Node *m_node;
  public:
    const_iterator(const List &p_list, Node *p_node)
      : m_list(p_list), m_node(p_node)  { }
    const T &operator*() const { return m_node->m_data; }
    const_iterator &operator++()  { m_node = m_node->m_next; return *this; }
    bool operator==(const const_iterator &it) const
    { return (m_node == it.m_node); }
    bool operator!=(const const_iterator &it) const
    { return (m_node != it.m_node); }
  };

  List();
  List(const List<T> &);
  virtual ~List();
  
  List<T> &operator=(const List<T> &);
  
  bool operator==(const List<T> &b) const;
  bool operator!=(const List<T> &b) const;

  /// Return a forward iterator starting at the beginning of the list
  iterator begin()  { return iterator(*this, m_head); }
  /// Return a forward iterator past the end of the list
  iterator end()    { return iterator(*this, 0); }
  /// Return a const forward iterator starting at the beginning of the list
  const_iterator begin() const { return const_iterator(*this, m_head); }
  /// Return a const forward iterator past the end of the list
  const_iterator end() const   { return const_iterator(*this, 0); }
  /// Return a const forward iterator starting at the beginning of the list
  const_iterator cbegin() const { return const_iterator(*this, m_head); }
  /// Return a const forward iterator past the end of the list
  const_iterator cend() const   { return const_iterator(*this, 0); }

  const T &operator[](int) const;
  T &operator[](int);

  List<T> operator+(const List<T>& b) const;
  List<T>& operator+=(const List<T>& b);

  int Insert(const T &, int);
  T Remove(int);

  int Find(const T &) const;
  bool Contains(const T &t) const;
  int Length() const { return m_length; }

  /// @name STL-style interface
  ///
  /// These operations are a partial implementation of operations on
  /// STL-style list containers.  It is suggested that future code be
  /// written to use these, and existing code ported to use them as
  /// possible.
  ///@{
  /// Return whether the list container is empty (has size 0).
  bool empty() const { return (m_length == 0); }
  /// Return the number of elements in the list container.
  size_t size() const { return m_length; }
  /// Adds a new element at the end of the list container, after its
  /// current last element.
  void push_back(const T &val);
  /// Removes all elements from the list container (which are destroyed),
  /// leaving the container with a size of 0.
  void clear();
  /// Returns a reference to the first elemnet in the list container.
  T &front()             { return m_head->m_data; }
  /// Returns a reference to the first element in the list container.
  const T &front() const { return m_head->m_data; }
  /// Returns a reference to the last element in the list container.
  T &back()             { return m_tail->m_data; }
  /// Returns a reference to the last element in the list container.
  const T &back() const { return m_tail->m_data; }
  ///@}
};


//--------------------------------------------------------------------------
//                 Node: Member function implementations
//--------------------------------------------------------------------------

template <class T> 
List<T>::Node::Node(const T &p_data,
		    typename List<T>::Node *p_prev, typename List<T>::Node *p_next)
  : m_data(p_data), m_prev(p_prev), m_next(p_next)
{ }

//--------------------------------------------------------------------------
//                 List<T>: Member function implementations
//--------------------------------------------------------------------------

template <class T> List<T>::List() 
  : m_length(0), m_head(0), m_tail(0), m_currentIndex(0), m_currentNode(0)
{ }

template <class T> List<T>::List(const List<T> &b)
  : m_length(b.m_length)
{
  if (m_length)  {  
    Node *n = b.m_head;
    m_head = new Node(n->m_data, 0, 0);
    n = n->m_next;
    m_tail = m_head;
    while (n)  { 
      m_tail->m_next = new Node(n->m_data, m_tail, 0);
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

template <class T> List<T>::~List()
{
  Node *n = m_head;
  while (n)  {
    Node *m_next = n->m_next;
    delete n;
    n = m_next;
  }
}

template <class T> int List<T>::InsertAt(const T &t, int num)
{
  if (num < 1 || num > m_length + 1)   throw IndexException();
  
  if (!m_length)  {
    m_head = m_tail = new Node(t, 0, 0);
    m_length = 1;
    m_currentIndex = 1;
    m_currentNode = m_head;
    return m_length;
  }

  Node *n;
  int i;
  
  if( num <= 1 )  {
    n = new Node(t, 0, m_head);
    m_head->m_prev = n;
    m_currentNode = m_head = n;
    m_currentIndex = 1;
  }
  else if( num >= m_length + 1)  {
    n = new Node(t, m_tail, 0);
    m_tail->m_next = n;
    m_currentNode = m_tail = n;
    m_currentIndex = m_length + 1;
  }
  else  {
    if( num < m_currentIndex )
      for (i = m_currentIndex, n = m_currentNode; i > num; i--, n = n->m_prev);
    else
      for (i = m_currentIndex, n = m_currentNode; i < num; i++, n = n->m_next);
    n = new Node(t, n->m_prev, n);
    m_currentNode = n->m_prev->m_next = n->m_next->m_prev = n;
    m_currentIndex = num;
  }

  m_length++;
  return num;
}

//--------------------- visible functions ------------------------

template <class T> List<T> &List<T>::operator=(const List<T> &b)
{
  if (this != &b)   {
    Node *n = m_head;
    while (n) {
      Node *m_next = n->m_next;
      delete n;
      n = m_next;
    }

    m_length = b.m_length;
    m_currentIndex = b.m_currentIndex;
    if (m_length)   {
      Node *n = b.m_head;
      m_head = new Node(n->m_data, 0, 0);
      if (b.m_currentNode == n) m_currentNode = m_head;
      n = n->m_next;
      m_tail = m_head;
      while (n)  {
	m_tail->m_next = new Node(n->m_data, m_tail, 0);
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

template <class T> bool List<T>::operator==(const List<T> &b) const
{
  if (m_length != b.m_length) return false;
  for (Node *m = m_head, *n = b.m_head; m; m = m->m_next, n = n->m_next)
    if (m->m_data != n->m_data)  return false;
  return true;
}

template <class T> bool List<T>::operator!=(const List<T> &b) const
{
  return !(*this == b);
}

template <class T> const T &List<T>::operator[](int num) const
{
  if (num < 1 || num > m_length)    throw IndexException();

  int i;
  Node *n;
  if( num < m_currentIndex )
    for (i = m_currentIndex, n = m_currentNode; i > num; i--, n = n->m_prev);
  else
    for (i = m_currentIndex, n = m_currentNode; i < num; i++, n = n->m_next);
  return n->m_data;
}

template <class T> T &List<T>::operator[](int num)
{
  if (num < 1 || num > m_length)   throw IndexException();
  Node *n;
  int i;
  if( num < m_currentIndex )
    for (i = m_currentIndex, n = m_currentNode; i > num; i--, n = n->m_prev);
  else
    for (i = m_currentIndex, n = m_currentNode; i < num; i++, n = n->m_next);
  m_currentIndex = i;
  m_currentNode = n;
  return n->m_data;
}

template <class T> List<T> List<T>::operator+(const List<T> &b) const
{
  List<T> result(*this);
  Node *n = b.m_head;
  while (n)  {
    result.Append(n->data);
    n = n->m_next;
  }
  return result;
}

template <class T> List<T> &List<T>::operator+=(const List<T> &b)
{
  Node *n = b.m_head;
  
  while (n)  {
    push_back(n->m_data);
    n = n->m_next;
  }
  return *this;
}

template <class T> int List<T>::Insert(const T &t, int n)
{
  return InsertAt(t, (n < 1) ? 1 : ((n > m_length + 1) ? m_length + 1 : n));
}

template <class T> T List<T>::Remove(int num)
{
  if (num < 1 || num > m_length)   throw IndexException();
  Node *n;
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

template <class T> int List<T>::Find(const T &t) const
{
  if (m_length == 0)  return 0;
  Node *n = m_head;
  for (int i = 1; n; i++, n = n->m_next)
    if (n->m_data == t)   return i;
  return 0;
}

template <class T> bool List<T>::Contains(const T &t) const
{
  return (Find(t) != 0);
}

template <class T> void List<T>::push_back(const T &val)
{
  InsertAt(val, m_length + 1);
}

template <class T> void List<T>::clear()
{
  Node *n = m_head;
  while (n)  {
    Node *m_next = n->m_next;
    delete n;
    n = m_next;
  }
  m_length = 0;
  m_head = 0;
  m_tail = 0;
  m_currentIndex = 0;
  m_currentNode = 0;
}

}

#endif // LIBGAMBIT_LIST_H

