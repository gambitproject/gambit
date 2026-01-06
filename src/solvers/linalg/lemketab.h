//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/lcp/lemketab.h
// Declaration of Lemke tableau class
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

#ifndef GAMBIT_LINALG_LEMKETAB_H
#define GAMBIT_LINALG_LEMKETAB_H

#include "tableau.h"

namespace Gambit::linalg {

template <class T> class LemkeTableau : public Tableau<T> {
public:
  class BadPivot final : public std::runtime_error {
  public:
    BadPivot() : std::runtime_error("Bad pivot in LTableau") {}
    ~BadPivot() noexcept override = default;
  };
  class BadExitIndex final : public std::runtime_error {
  public:
    BadExitIndex() : std::runtime_error("Bad exit index in LTableau") {}
    ~BadExitIndex() noexcept override = default;
  };
  LemkeTableau(const Matrix<T> &A, const Vector<T> &b) : Tableau<T>(A, b) {}
  ~LemkeTableau() override = default;

  int SF_PivotIn(int i);
  int SF_ExitIndex(int i);
  int SF_LCPPath(int dup); // follow a path of ACBFS's from one CBFS to another
  int ExitIndex(int i);
};

} // end namespace Gambit::linalg

#endif // GAMBIT_LINALG_LEMKETAB_H
