//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to basic feasible solution class
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

#ifndef BFS_H
#define BFS_H

#include "base/gmap.h"

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



