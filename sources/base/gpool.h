//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Class implementing a memory pool
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

#ifndef GPOOL_H
#define GPOOL_H

class gPool   {
private:
  struct gLink  { gLink *next; };

  const unsigned esize;
  gLink *head;

  // these are here to protect against copying -- shouldn't be done with this
  // class!
  gPool(const gPool &);
  void operator=(const gPool &);
  
  void Grow(void);

public:
  gPool(unsigned n);
  ~gPool();

  void *Alloc(void);
  void Free(void *);
};

inline void *gPool::Alloc(void)
{
  if (head == 0)  Grow();
  gLink *p = head;
  head = p->next;
  return p;
}

inline void gPool::Free(void *b)
{
  gLink *p = (gLink *) b;
  p->next = head;
  head = p;
}

#endif   // GPOOL_H

