//
// FILE: gpool.cc -- Implementation of memory pool
//
// $Id$
//

#include "base/gpool.h"

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

