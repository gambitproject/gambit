//
// $Source$
// $Date$
// $Revision$
// 
// DESCRIPTION:
// Utilities from the g++ library
//

#include <values.h>
#include "gnulib.h"

Obstack::Obstack(int size, int alignment)
{
  alignmentmask = alignment - 1;
  chunksize = size;
  chunk = 0;
  nextfree = objectbase = 0;
  chunklimit = 0;
}

void Obstack::_free(void* obj)
{
  _obstack_chunk*  lp;
  _obstack_chunk*  plp;

  lp = chunk;
  while (lp != 0 && ((void*)lp > obj || (void*)(lp)->limit < obj))
  {
    plp = lp -> prev;
    delete [] (char*)lp;
    lp = plp;
  }
  if (lp)
  {
    objectbase = nextfree = (char *)(obj);
    chunklimit = lp->limit;
    chunk = lp;
  }
}

void Obstack::newchunk(int size)
{
  _obstack_chunk*	old_chunk = chunk;
  _obstack_chunk*	new_chunk;
  long	new_size;
  int obj_size = (int) (nextfree - objectbase);

  new_size = (obj_size + size) << 1;
  if (new_size < chunksize)
    new_size = chunksize;

  new_chunk = chunk = (_obstack_chunk*)(new char[(int) new_size]);
  new_chunk->prev = old_chunk;
  new_chunk->limit = chunklimit = (char *) new_chunk + (int) new_size;

  memcpy((void*)new_chunk->contents, (void*)objectbase, obj_size);
  objectbase = new_chunk->contents;
  nextfree = objectbase + obj_size;
}

void* Obstack::finish()
{
  void* value = (void*) objectbase;
  nextfree = (char*)((int)(nextfree + alignmentmask) & ~(alignmentmask));
  if (nextfree - (char*)chunk > chunklimit - (char*)chunk)
    nextfree = chunklimit;
  objectbase = nextfree;
  return value;
}

int Obstack::contains(void* obj) // true if obj somewhere in Obstack
{
  _obstack_chunk *ch;
  for (ch = chunk; 
       ch != 0 && (obj < (void*)ch || obj >= (void*)(ch->limit)); 
       ch = ch->prev);

  return ch != 0;
}
         
int Obstack::OK()
{
  int v = chunksize > 0;        // valid size
  v &= alignmentmask != 0;      // and alignment
  v &= chunk != 0;
  v &= objectbase >= chunk->contents;
  v &= nextfree >= objectbase;
  v &= nextfree <= chunklimit;
  v &= chunklimit == chunk->limit;
  _obstack_chunk* p = chunk;
  // allow lots of chances to find bottom!
  long x = MAXLONG;
  while (p != 0 && x != 0) { --x; p = p->prev; }
  v &= x > 0;
  return v;
}

AllocRing::AllocRing(int max)
  : nodes(new AllocQNode[max]), n(max), current(0)
{
  for (int i = 0; i < n; ++i)
  {
    nodes[i].ptr = 0;
    nodes[i].sz = 0;
  }
}

int AllocRing::find(void* p)
{
  if (p == 0) return -1;

  for (int i = 0; i < n; ++i)
    if (nodes[i].ptr == p)
      return i;

  return -1;
}


void AllocRing::clear()
{
  for (int i = 0; i < n; ++i)
  {
    if (nodes[i].ptr != 0)
    {
      delete(nodes[i].ptr);
      nodes[i].ptr = 0;
    }
    nodes[i].sz = 0;
  }
  current = 0;
}


void AllocRing::free(void* p)
{
  int idx = find(p);
  if (idx >= 0)
  {
    delete nodes[idx].ptr;
    nodes[idx].ptr = 0;
  }
}

AllocRing::~AllocRing()
{
  clear();
}

int AllocRing::contains(void* p)
{
  return find(p) >= 0;
}

static unsigned int good_size(unsigned int s)
{
  unsigned int req = s + 4;
  unsigned int good = 8;
  while (good < req) good <<= 1;
  return good - 4;
}

void* AllocRing::alloc(int s)
{
  unsigned int size = good_size(s);

  void* p;
  if (nodes[current].ptr != 0 && 
      nodes[current].sz >= int(size) && 
      nodes[current].sz < int(4 * size))
    p = nodes[current].ptr;
  else
  {
    if (nodes[current].ptr != 0) delete nodes[current].ptr;
    p = new char[size];
    nodes[current].ptr = p;
    nodes[current].sz = size;
  }
  ++current;
  if (current >= n) current = 0;
  return p;
}

/*
 common functions on built-in types
*/

long gcd(long x, long y)        // euclid's algorithm
{
  long a = abs(x);
  long b = abs(y);

  long tmp;
  
  if (b > a)
  {
    tmp = a; a = b; b = tmp;
  }
  for(;;)
  {
    if (b == 0)
      return a;
    else if (b == 1)
      return b;
    else
    {
      tmp = b;
      b = a % b;
      a = tmp;
    }
  }
}

