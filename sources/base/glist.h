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
#include "gstream.h"

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
    class BadIndex : public gbtException   {
      public:
        virtual ~BadIndex()   { }
        gbtText Description(void) const;
    };

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
    void Dump(gbtOutput &) const;
};

template <class T> gbtOutput &operator<<(gbtOutput &f, const gbtList<T> &b);

#endif    // GLIST_H
