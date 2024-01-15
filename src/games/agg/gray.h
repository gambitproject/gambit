//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: src/libagg/GrayComposition.h
// Interface file for GrayComposition
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

#ifndef GAMBIT_AGG_GRAY_H
#define GAMBIT_AGG_GRAY_H

#include <vector>

namespace Gambit {

namespace agg {

class GrayComposition {
  friend class AGG;

public:
  GrayComposition(int _n, int _k)
    : n(_n), k(_k), p(0), i(-1), d(-1), finished(false), current(k, 0)
  {
    current.at(0) = n;
  }

  bool eof() const { return finished; }

  const std::vector<int> &get() { return current; }

  void incr()
  {
    int b = -1;
    if (finished) {
      return;
    }
    if (current[k - 1] == n) {
      finished = true;
      return;
    }
    if (p == 0) {
      for (b = 1; b < k; ++b) {
        if (current[b] > 0) {
          break;
        }
      }
      if (b < k && b == 1) {
        if (d == 0 && current[0] == 1) {
          p = 1;
        }
      }
      else if ((n - current[0]) % 2 == 0) {
        d = 0;
        i = 1;
        p = 1;
      }
      else if (b < k && current[b] % 2 == 1) {
        d = 0;
        i = b;
        p = b;
      }
      else {
        i = 0;
        d = b;
      }
    }
    else {
      if ((n - current[p]) % 2 == 1) {
        d = p;
        i = p - 1;
        if (current[p] % 2 == 0) {
          i = 0;
        }
        p = i;
      }
      else if (current[p + 1] % 2 == 0) {
        i = p + 1;
        d = p;
        if (current[p] == 1) {
          p = p + 1;
        }
      }
      else {
        i = p;
        d = p + 1;
      }
    }
    current[i] += 1;
    current[d] -= 1;
    if (current[0] > 0) {
      p = 0;
    }
  }

private:
  int n, k;
  int p; // idx to first positive
  int i, d;
  bool finished;
  std::vector<int> current;
};

} // namespace agg

} // end namespace Gambit

#endif // GAMBIT_AGG_GRAY_H
