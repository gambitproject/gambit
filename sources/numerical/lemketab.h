//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of Lemke tableau class
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

#ifndef LEMKETAB_H
#define LEMKETAB_H

#include "base/gstatus.h"
#include "numerical/tableau.h"

template <class T> class LTableau : public Tableau<T> {
protected:
//  T eps2;
public:
//   LTableau(void);
  class BadPivot : public gException  {
  public:
    virtual ~BadPivot();
    gText Description(void) const;
  };
  class BadExitIndex : public gException  {
  public:
    virtual ~BadExitIndex();
    gText Description(void) const;
  };
  LTableau(const gMatrix<T> &A, const gVector<T> &b);
  LTableau(Tableau<T> &);
  virtual ~LTableau();

  int SF_PivotIn(int i);
  int SF_ExitIndex(int i);
  int SF_LCPPath(int dup, gStatus &); // follow a path of ACBFS's from one CBFS to another
  int PivotIn(int i);
  int ExitIndex(int i);
  int LemkePath(int dup); // follow a path of ACBFS's from one CBFS to another
};

#endif     // LEMKETAB_H










