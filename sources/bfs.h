//
// FILE: bfs.h -- Implementation of tableau type
//
// $Id$
//


#ifndef BFS_H
#define BFS_H

#include "gmap.h"

template <class T> class BFS : public gOrdMap<int, T>  {
  public:
    BFS(void);
    BFS(const T &d);
    BFS(const BFS<T> &m);
           // define two BFS's to be equal if their bases are equal
    int operator==(const BFS &M) const;
    int operator!=(const BFS &M) const;
};

template <class T> gOutput &operator<<(gOutput &, const BFS<T> &);

#endif   // BFS_H



