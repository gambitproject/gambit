//
// FILE: gpool.h -- Memory pool
//
// $Id$
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

