//
// FILE: gmap.h -- Declaration of Map container type
//
// $Id$
//

#ifndef GMAP_H
#define GMAP_H

#include "basic.h"
#include "ghandle.h"

// What is the difference between a gMap and a gSet?
// Essentially that a gMap allows arbitrary (integer) numbering, while a gSet
// only supports sequential numbering

template <class T> class gMap  {
  private:
    int length;
    int *numbers;
    gHandle<T> *contents;

  public:
    gMap(void) : length(0), contents(0), numbers(0)   { }
    gMap(const gMap<T> &);

    uint Length(void) const   { return length; }
    void Insert(gHandle<T>& new_member, uint as_number);
    gHandle<T> Remove(uint number);
    void Remove(gHandle<T>& p);

    uint ElNumber(gHandle<T>& p) const;
    int Contains(int number) const;

    gHandle<T> operator[](uint index) const;

    void Dump(void) const;

    ~gMap()    { delete [] contents;  delete [] numbers; }
};


#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type
#endif   // __GNUC__, __BORLANDC__

template <class T> INLINE gMap<T>::gMap(const gMap<T> &m)
{
  length = m.length;
  numbers = new int[length];
  contents = new gHandle<T>[length];
  
  for (uint i = 0; i < length; i++)   {
    numbers[i] = m.numbers[i];
    contents[i] = m.contents[i];
  }
}

template <class T> INLINE void gMap<T>::Dump(void) const
{
  for (uint i = 0; i < length; i++)
    printf("%d: %p\n", numbers[i], contents[i].ptr());
  printf("\n");
}

template <class T> INLINE uint gMap<T>::ElNumber(gHandle<T> &p) const
{
  for (uint i = 0; i < length; i++)
    if (contents[i] == p)  return numbers[i];
  return 0;
}

template <class T> INLINE int gMap<T>::Contains(int number) const
{
  for (uint i = 0; i < length; i++)
    if (numbers[i] == number)  return i + 1;
  return 0;
}

template <class T>
INLINE void gMap<T>::Insert(gHandle<T> &new_member, uint as_number)
{
  if (Contains(as_number))  {
    contents[Contains(as_number) - 1] = new_member;
    return;
  }

  gHandle<T> *new_contents = new gHandle<T>[++length];
  int *new_numbers = new int[length];

  if (length == 1)   {
    new_contents[0] = new_member;
    new_numbers[0] = as_number;
    contents = new_contents;
    numbers = new_numbers;
    return;
  }

  for (uint i = 0; numbers[i] < as_number; i++)  {
    new_contents[i] = contents[i];
    new_numbers[i] = numbers[i];
  }

  new_contents[i] = new_member;
  new_numbers[i] = as_number;

  for (i++; i < length; i++)  {
    new_contents[i] = contents[i - 1];
    new_numbers[i] = numbers[i - 1];
  }

  delete [] contents;
  delete [] numbers;

  contents = new_contents;
  numbers = new_numbers;
}

template <class T> INLINE gHandle<T> gMap<T>::Remove(uint number)
{
  if (!Contains(number))   return gHandle<T>(0);

  gHandle<T> return_value = contents[Contains(number) - 1];

  if (length == 1)  {
    delete [] contents;
    delete [] numbers;
    length--;
    contents = 0;
    numbers = 0;
    return return_value;
  }

  gHandle<T> *new_contents = new gHandle<T>[--length];
  int *new_numbers = new int[length];

  for (uint i = 0; numbers[i] < number; i++)   {
    new_contents[i] = contents[i];
    new_numbers[i] = numbers[i];
  }

  for (i++; i < length; i++)  {
    new_contents[i] = contents[i + 1];
    new_numbers[i] = numbers[i + 1];
  }

  delete [] contents;
  delete [] numbers;

  contents = new_contents;
  numbers = new_numbers;

  return return_value;
}

template <class T> INLINE void gMap<T>::Remove(gHandle<T> &p)
{
  Remove(ElNumber(p));
}


template <class T> INLINE gHandle<T> gMap<T>::operator[](uint index) const
{
  if (Contains(index))
    return contents[Contains(index) - 1];
  else
    return gHandle<T>(0);
}


#endif   // GMAP_H


