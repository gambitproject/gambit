//#
//# FILE: gnulib.h -- Utilities from the g++ library
//#
//# $Id$
//#

#ifndef GNULIB_H
#define GNULIB_H

#ifdef __GNUG__
#include <_G_config.h>
#endif   // __GNUG__
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#ifdef __GNUG__
#include <unistd.h>
#endif  // __GNUG__
#include <stdio.h> 
#include <errno.h>
#include <fcntl.h>

#ifdef __GNUG__
extern "C" {
int strcasecmp _G_ARGS((const char*, const char*));
}
#endif   // __GNUG__


#include <stddef.h>
#include <math.h>

#ifdef __GNUG__
#define _VOLATILE_VOID volatile void
#else
#define _VOLATILE_VOID void
#endif

typedef void (*one_arg_error_handler_t)(const char*);
typedef void (*two_arg_error_handler_t)(const char*, const char*);

long         gcd(long, long);
long         lg(unsigned long); 
double       pow(double, long);
long         pow(long, long);

double       start_timer();
double       return_elapsed_time(double last_time = 0.0);

char*        dtoa(double x, char cvt = 'g', int width = 0, int prec = 6);

unsigned int hashpjw(const char*);
unsigned int multiplicativehash(int);
unsigned int foldhash(double);

extern _VOLATILE_VOID default_one_arg_error_handler(const char*);
extern _VOLATILE_VOID default_two_arg_error_handler(const char*, const char*);

//extern two_arg_error_handler_t lib_error_handler;

extern two_arg_error_handler_t 
       set_lib_error_handler(two_arg_error_handler_t f);


double abs(double arg);
float abs(float arg);
short abs(short arg);
long abs(long arg);
int sign(long arg);
int sign(double arg);
long sqr(long arg);
double sqr(double arg);
int even(long arg);
int odd(long arg);
long lcm(long x, long y);
void (setbit)(long& x, long b);
void clearbit(long& x, long b);
int testbit(long x, long b);

#if !defined(IV)

#if ! _G_MATH_H_INLINES /* hpux and SCO define this in math.h */
inline double abs(double arg) 
{
  return (arg < 0.0)? -arg : arg;
}
#endif

inline float abs(float arg) 
{
  return (arg < 0.0)? -arg : arg;
}

inline short abs(short arg) 
{
  return (arg < 0)? -arg : arg;
}

inline long abs(long arg) 
{
  return (arg < 0)? -arg : arg;
}

inline int sign(long arg)
{
  return (arg == 0) ? 0 : ( (arg > 0) ? 1 : -1 );
}

inline int sign(double arg)
{
  return (arg == 0.0) ? 0 : ( (arg > 0.0) ? 1 : -1 );
}

inline long sqr(long arg)
{
  return arg * arg;
}

#if ! _G_MATH_H_INLINES /* hpux and SCO define this in math.h */
inline double sqr(double arg)
{
  return arg * arg;
}
#endif

inline int even(long arg)
{
  return !(arg & 1);
}

inline int odd(long arg)
{
  return (int) (arg & 1L);
}

inline long lcm(long x, long y)
{
  return x / gcd(x, y) * y;
}

inline void (setbit)(long& x, long b)
{
  x |= (long) (1 << (int) b);
}

inline void clearbit(long& x, long b)
{
  x &= ~((long) (1 << (int) b));
}

inline int testbit(long x, long b)
{
  return ((x & (1 << (int) b)) != 0);
}

#endif


/*
  An AllocRing holds the last n malloc'ed strings, reallocating/reusing 
  one only when the queue wraps around. It thus guarantees that the
  last n allocations are intact. It is useful for things like I/O
  formatting where reasonable restrictions may be made about the
  number of allowable live allocations before auto-deletion.
*/

class AllocRing
{

  struct AllocQNode
  {
    void*  ptr;
    int    sz;
  };

  AllocQNode* nodes;
  int         n;
  int         current;

  int         find(void* p);

public:

              AllocRing(int max);
             ~AllocRing();

  void*       alloc(int size);
  int         contains(void* ptr);
  void        clear();
  void        free(void* p);
};

class Obstack
{
  struct _obstack_chunk
  {
    char*           limit;
    _obstack_chunk* prev;
    char            contents[4];
  };

protected:
  long	          chunksize;
  _obstack_chunk* chunk;
  char*	          objectbase;
  char*	          nextfree;
  char*	          chunklimit;
  int             alignmentmask;

  void  _free(void* obj);
  void  newchunk(int size);

public:
        Obstack(int size = 4080, int alignment = 4); // 4080=4096-mallocslop

        ~Obstack();

  void* base();
  void* next_free();
  int   alignment_mask();
  long  chunk_size();
  int   size();
  int   room();
  int   contains(void* p);      // does Obstack hold pointer p?

  void  grow(const void* data, int size);
  void  grow(const void* data, int size, char terminator);
  void  grow(const char* s);
  void  grow(char c);
  void  grow_fast(char c);
  void  blank(int size);
  void  blank_fast(int size);

  void* finish();
  void* finish(char terminator);

  void* copy(const void* data, int size);
  void* copy(const void* data, int size, char terminator);
  void* copy(const char* s);
  void* copy(char c);
  void* alloc(int size);

  void  free(void* obj);
  void  shrink(int size = 1); // suggested by ken@cs.rochester.edu

  int   OK();                   // rep invariant
};


inline Obstack::~Obstack()
{
  _free(0);
}

inline void* Obstack::base()
{
  return objectbase;
}

inline void* Obstack::next_free()
{
  return nextfree;
}

inline int Obstack::alignment_mask()
{
  return alignmentmask;
}

inline long Obstack::chunk_size()
{
  return chunksize;
}

inline int Obstack::size()
{
  return (int) (nextfree - objectbase);
}

inline int Obstack::room()
{
  return (int) (chunklimit - nextfree);
}

inline void Obstack:: grow(const void* data, int size)
{
  if (nextfree+size > chunklimit) 
    newchunk(size);
  memcpy(nextfree, data, size);
  nextfree += size; 
}

inline void Obstack:: grow(const void* data, int size, char terminator)
{
  if (nextfree+size+1 > chunklimit) 
    newchunk(size+1);
  memcpy(nextfree, data, size);
  nextfree += size; 
  *(nextfree)++ = terminator; 
}

inline void Obstack:: grow(const char* s)
{
  grow((const void*)s, strlen(s), 0); 
}

inline void Obstack:: grow(char c)
{
  if (nextfree+1 > chunklimit) 
    newchunk(1); 
  *(nextfree)++ = c; 
}

inline void Obstack:: blank(int size)
{
  if (nextfree+size > chunklimit) 
    newchunk(size);
  nextfree += size; 
}

inline void* Obstack::finish(char terminator)
{
  grow(terminator); 
  return finish(); 
}

inline void* Obstack::copy(const void* data, int size)
{
  grow (data, size);
  return finish(); 
}

inline void* Obstack::copy(const void* data, int size, char terminator)
{
  grow(data, size, terminator); 
  return finish(); 
}

inline void* Obstack::copy(const char* s)
{
  grow((const void*)s, strlen(s), 0); 
  return finish(); 
}

inline void* Obstack::copy(char c)
{
  grow(c);
  return finish(); 
}

inline void* Obstack::alloc(int size)
{
  blank(size);
  return finish(); 
}

inline void Obstack:: free(void* obj)     
{
  if (obj >= (void*)chunk && obj<(void*)chunklimit)
    nextfree = objectbase = (char *) obj;
  else 
    _free(obj); 
}

inline void Obstack:: grow_fast(char c)
{
  *(nextfree)++ = c; 
}

inline void Obstack:: blank_fast(int size)
{
  nextfree += size; 
}

inline void Obstack:: shrink(int size) // from ken@cs.rochester.edu
{
  if (nextfree >= objectbase + size)
    nextfree -= size;
}


#endif   // GNULIB_H

