//
// FILE: glist.h -- A generic array container class
//
// $Id$
//

#ifndef GLIST_H
#define GLIST_H

#include "base/gmisc.h"
#include "base/gstream.h"

template <class T> class gList  {
  protected:

  class gNode   {
  public:
    T data;
    gNode *prev, *next;
 
    // CONSTRUCTOR
    gNode(const T &_data, gNode *_prev, gNode *_next);
  };

    int length;
    gNode *head, *tail;

    int CurrIndex;
    gNode *CurrNode;

    int InsertAt(const T &t, int where);

  public:
    class BadIndex : public gException   {
      public:
        virtual ~BadIndex()   { }
        gText Description(void) const;
    };

    gList(void);
    gList(const gList<T> &);
    virtual ~gList();

    gList<T> &operator=(const gList<T> &);

    bool operator==(const gList<T> &b) const;
    bool operator!=(const gList<T> &b) const;

    const T &operator[](int) const;
    T &operator[](int);

    gList<T> operator+(const T &e) const;
    gList<T>& operator+=(const T &e);

    gList<T> operator+(const gList<T>& b) const;
    gList<T>& operator+=(const gList<T>& b);

    gList<T> &Combine(gList<T> &b);
    gList<T> InteriorSegment(int, int) const;

    virtual int Append(const T &);
    int Insert(const T &, int);
    virtual T Remove(int);

    bool HasARedundancy();
    void RemoveRedundancies();

    int Find(const T &) const;
    bool Contains(const T &t) const;
    int Length(void) const;

    virtual void Flush(void);
    void Dump(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &f, const gList<T> &b);

#endif    // GLIST_H

