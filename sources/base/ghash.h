//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of hash table class
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

#ifndef GHASH_H
#define GHASH_H

#include "base.h"

template <class K, class T> class HashTable {
private:
  int m_numBuckets;
  gList<K> *m_keyBucket;
  gList<T> *m_valueBucket;

  int ValidatedHash(const K &) const;
  virtual int Hash(const K &) const = 0;

  // The derived classes need to define this function to do clean ups when
  // a T type member is being removed.
  virtual void DeleteAction(T) = 0;

  // these two are here for copy protection
  HashTable(const HashTable<K, T> &);
  HashTable<K, T> &operator=(const HashTable<K, T> &);

protected:
  // CONSTRUCTOR
  HashTable(unsigned int);

public:
  class BadKey : public gException {
  public:
    virtual ~BadKey() { }
    gText Description(void) const { return "Bad key passed to HashTable"; }
  };

  class InternalError : public gException {
  private:
    gText m_message;
    
  public:
    InternalError(const gText &p_message) : m_message(p_message) { }
    virtual ~InternalError() { }
    gText Description(void) const { return m_message; }
  };

  // DESTRUCTOR
  virtual ~HashTable();

  int NumBuckets(void) const { return m_numBuckets; }
  int IsDefined(K) const;
  void Define(K, T);
  T Remove(K);
  void Remove(T);
  T operator()(K) const;
  T &operator()(K);
  const gList<K> *Key(void) const { return m_keyBucket; }
  const gList<T> *Value(void) const { return m_valueBucket; }

  // This function should be called in the destructor of decendents classes
  void Flush(void);
};


#endif  // GHASH_H
