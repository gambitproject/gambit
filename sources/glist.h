//
// FILE: glist.h -- A generic array container class
//
// $Id$
//

#ifndef GLIST_H
#define GLIST_H

#include "gmisc.h"
#include "gstream.h"

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

    virtual int Append(const T &);
    int Insert(const T &, int);
    virtual T Remove(int);

    int Find(const T &) const;
    bool Contains(const T &t) const;
    int Length(void) const;

    void Flush(void);
    void Dump(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &f, const gList<T> &b);

#endif    // GLIST_H

