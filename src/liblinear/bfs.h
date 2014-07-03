//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/bfs.h
// Interface to basic feasible solution class
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

#include "libgambit/libgambit.h"
#include <map>

template <class T> class BFS {
private:
  std::map<int, T> m_map;
  T m_default;

public:
  // Lifecycle
  BFS(void) : m_default(0) { }
  ~BFS()  { }

  // define two BFS's to be equal if their bases are equal
  bool operator==(const BFS &M) const {
    if (m_map.size() != M.m_map.size())  return false;

    for (typename std::map<int, T>::const_iterator iter = m_map.begin();
	 iter != m_map.end(); iter++) {
      if (M.m_map.count((*iter).first) == 0) {
	return false;
      }
    }
    return true;
  }
  bool operator!=(const BFS &M) const  { return !(*this == M); }

  // Provide map-like operations
  int count(int key) const { return (m_map.count(key) > 0); }

  void insert(int key, const T &value) {
    m_map.erase(key);
    m_map.insert(std::pair<int, T>(key, value));
  }

  const T &operator[](int key) {
    if (m_map.count(key) == 1) {
      return m_map[key];
    }
    else {
      return m_default;
    }
  }
};

#endif   // BFS_H



