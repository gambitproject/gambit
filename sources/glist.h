//
// FILE: glist.h -- A generic array container class
//
// $Id$
//

#ifndef GLIST_H
#define GLIST_H

#include "gambitio.h"

template <class T> class gList;
template <class T> class gSortList;

template <class T> class gNode   {
  friend class gList<T>;
  friend class gSortList<T>;
  private:
    T data;
    gNode<T> *prev, *next;

 public:
    // Constructor
    gNode(const T &_data, gNode<T> *_prev, gNode<T> *_next);
};

template <class T> class gList  {
  protected:
    int length;
    gNode<T> *head, *tail;

    int CurrIndex;
    gNode<T> *CurrNode;

    int InsertAt(const T &t, int where);

  public:
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

