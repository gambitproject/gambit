//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of resizable 2-D array class
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

#ifndef GRBLOCK_H
#define GRBLOCK_H

#include "grarray.h"

template <class T> class gRectBlock : public gRectArray<T>  { 
 public:
  gRectBlock(void);
  gRectBlock(unsigned int nrows, unsigned int ncols);
  gRectBlock(int minr, int maxr, int minc, int maxc);
  gRectBlock(const gRectBlock<T> &);
  virtual ~gRectBlock();

  gRectBlock<T> &operator=(const gRectBlock<T> &);

  void AddRow(const gArray<T> &);
  void RemoveRow(int);
  void InsertRow(int, const gArray<T> &);
 
  void AddColumn(const gArray<T> &);
  void RemoveColumn(int);
  void InsertColumn(int, const gArray<T> &);
};

#endif   // GRBLOCK_H
