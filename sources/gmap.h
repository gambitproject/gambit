//
// FILE: gmap.h -- Declaration of Map container type
//
// $Id$
//

#ifndef GMAP_H
#define GMAP_H

#include <assert.h>
#include "basic.h"

// What is the difference between a gMap and a gSet?
// Essentially that a gMap allows arbitrary (integer) numbering, while a gSet
// only supports sequential numbering

template <class T> class gMap  {
  private:
    int length;
    int *numbers;
    T *contents;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// initialize an empty gMap
    gMap(void) : length(0), contents(0), numbers(0)   { }
	// copy constructor
    gMap(const gMap<T> &);
	// clean up after a gMap
    ~gMap()   { delete [] contents;  delete [] numbers; }

	// OPERATOR OVERLOADING
	// assignment operator
    gMap<T> &operator=(const gMap<T> &);
	// returns the element with index number 'index'
    T operator[](uint index) const;
	// returns the indexth entry in the gMap
    T operator()(uint index) const;
	// determines the equality of two gMaps
    int operator==(const gMap<T> &) const;
    int operator!=(const gMap<T> &) const;

	// ADDING AND DELETING ELEMENTS
	// add a new element to the map, at the lowest available index
	//  returns the index at which the element is stored
    int Append(const T &new_member);
	// add a new element to the map at a specified index
    int Insert(const T &new_member, uint as_number);
	// remove element number 'number' from the map
	//  returns the element which was removed
    T Remove(uint number);
	// remove element from the map
    T Remove(const T &p);

	// GENERAL INFORMATION
	// returns the number of elements in the map
    uint Length(void) const   { return length; }
	// returns the number corresponding to an element in the map
	//   or zero if the element is not a member
    uint ElNumber(const T &p) const;
	// returns nonzero if the number is in use
    int Contains(int number) const;
	// returns nonzero if the element is in the map
    int Contains(const T &p) const  { return ElNumber(p); }
	// returns lowest unused index greater than zero
    int GetFirstVacancy(void) const;
	// returns index number of the ith element
    int GetIndex(int i) const;

	// DEBUGGING
    void Dump(void) const;
};


#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type
#endif   // __GNUC__, __BORLANDC__

template <class T> INLINE gMap<T>::gMap(const gMap<T> &m) : length(m.length)
{
  numbers = new int[length];
  contents = new T[length];
  
  for (uint i = 0; i < length; i++)   {
    numbers[i] = m.numbers[i];
    contents[i] = m.contents[i];
  }
}

template <class T> INLINE gMap<T> &gMap<T>::operator=(const gMap<T> &m)
{
  if (this != &m)   {
    delete [] numbers;
    delete [] contents;

    length = m.length;
    numbers = new int[length];
    contents = new T[length];
    
    for (uint i = 0; i < length; i++)   {
      numbers[i] = m.numbers[i];
      contents[i] = m.contents[i];
    }
  }
  return *this;
}

template <class T> inline T gMap<T>::operator[](uint index) const
{
  return contents[Contains(index) - 1];
}

template <class T> inline T gMap<T>::operator()(uint index) const
{
  return contents[index - 1];
}

template <class T> INLINE int gMap<T>::operator==(const gMap<T> &m) const
{
  if (length != m.length)   return 0;

  for (int i = 0; i < length; i++)
    if (numbers[i] != m.numbers[i] || contents[i] != m.contents[i]) return 0;

  return 1;
}

template <class T> inline int gMap<T>::operator!=(const gMap<T> &m) const
{
  return !(*this == m);
}

template <class T>
INLINE int gMap<T>::Insert(const T &new_member, uint as_number)
{
  if (Contains(as_number))  {
    contents[Contains(as_number) - 1] = new_member;
    return as_number;
  }

  T *new_contents = new T[++length];
  int *new_numbers = new int[length];

  if (length == 1)   {
    new_contents[0] = new_member;
    new_numbers[0] = as_number;
    contents = new_contents;
    numbers = new_numbers;
    return as_number;
  }

  for (uint i = 0; i < length - 1 && numbers[i] < as_number; i++)  {
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

  return as_number;
}

template <class T> INLINE int gMap<T>::Append(const T &new_member)
{
  for (uint i = 1; i <= length && numbers[i - 1] == i; i++);
  Insert(new_member, i);
  return i;
}

template <class T> INLINE T gMap<T>::Remove(uint number)
{
  assert(Contains(number));

  T return_value = contents[Contains(number) - 1];

  if (length == 1)  {
    delete [] contents;
    delete [] numbers;
    length--;
    contents = 0;
    numbers = 0;
    return return_value;
  }

  T *new_contents = new T[--length];
  int *new_numbers = new int[length];

  for (uint i = 0; numbers[i] < number; i++)   {
    new_contents[i] = contents[i];
    new_numbers[i] = numbers[i];
  }

  for (; i < length; i++)  {
    new_contents[i] = contents[i + 1];
    new_numbers[i] = numbers[i + 1];
  }

  delete [] contents;
  delete [] numbers;

  contents = new_contents;
  numbers = new_numbers;

  return return_value;
}

template <class T> inline T gMap<T>::Remove(const T &p)
{
  return Remove(ElNumber(p));
}

template <class T> INLINE uint gMap<T>::ElNumber(const T &p) const
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

template <class T> INLINE int gMap<T>::GetFirstVacancy(void) const
{
  for (uint i = 0; i < length; i++)
    if (numbers[i] != i + 1)   return ++i;
  return length + 1;
}

template <class T> inline int gMap<T>::GetIndex(int i) const
{
  return numbers[i - 1];
}



template <class T> INLINE void gMap<T>::Dump(void) const
{
  for (uint i = 0; i < length; i++)
    printf("(%d) %d: %p\n", i + 1, numbers[i], contents[i]);
  printf("\n");
}

#endif   // GMAP_H


