//#
//# FILE: bfs.h -- Implementation of tableau type
//#
//# $Id$
//#


#ifndef BFS_H
#define BFS_H

#include <assert.h>
#include "gmisc.h"
#include "gambitio.h"
#include "gmatrix.h"
#include "gblock.h"
#include "gmap.h"

#if defined(__BORLANDC__)
#include <stdio.h>
#include <conio.h>
#endif

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

// This is not the best way to do this, but you can't template typedefs...
#define BFS_List       gList< BFS<T> >

#endif   // BFS_H



