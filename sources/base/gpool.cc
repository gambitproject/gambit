//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of memory pool
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

#include "gpool.h"

gPool::gPool(unsigned sz)
  : esize((sz < sizeof(gLink *)) ? sizeof(gLink *) : sz), head(0)
{ }

gPool::~gPool()
{
// need to go through and delete things really
}

void gPool::Grow(void)
{
  const int overhead = 12;
  const int chunk_size = 8 * 1024 - overhead;
  const int nelem = (chunk_size - esize) / esize;

  char *start = new char[chunk_size];
  char *last = &start[(nelem - 1) * esize];
  
  for (char *p = start; p < last; p += esize)
    ((gLink *) p)->next = (gLink *) (p + esize);
  ((gLink *) last)->next = 0;
  head = (gLink *) start;
}

