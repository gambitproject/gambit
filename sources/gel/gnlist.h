//
// FILE: gnlist.h -- Nested list class for GCL
//
// $Id$
//

#ifndef GNLIST_H
#define GNLIST_H

#include "glist.h"

  //-------------------------------------------------------------------
  // Internally, this is how it works:
  //
  // For an example list such as { 7, 8, { 9, 10 }, 11 } 
  //   m_Data == { 7, 8, 9, 10, 11 }
  //   m_Dim == { 1, 3, -3, -2 }
  // Essentially, m_Data holds all the data values, while m_Dim holds
  //   the dimensionality of the list.
  // The structure of m_Dim is defined as follows:
  //   There is one number per each opening or closing brace in the list.
  //   The absolute value of each number is the number of data items
  //     in the list between this brace and the PREVIOUS brace, plus 1.
  //     The number is then given a positive sign if it is an opening brace,
  //     or a negative sign if it is a closing brace.
  //     Thus, the 1 corresponds to the fact that there are no data items
  //     before the first OPENING brace, while the 3 corresponds to the
  //     fact that there are two data items before the second OPENING brace.
  //------------------------------------------------------------------

template <class T> class gNestedList  {
private:
  gList<T> m_Data;
  gList<int> m_Dim;

  void GetElementInfo( int el, 
		       bool& islist, 
		       int& k_start, int& k_end, 
		       int& el_start, int& el_end ) const;
  

public:
    // NESTED LIST INTERNAL ERROR EXCEPTION CLASS
  class InternalError : public gException  {
    private:
      int line;

    public:
      InternalError(int l) : line(l)  { }
      virtual ~InternalError()  { }

      gText Description(void) const;
  };

    // CONSTRUCTORS, DESTRUCTOR, AND CONSTRUCTIVE OPERATORS
  gNestedList(void) { m_Dim.Append(0); }
  gNestedList(const gList<int>& dim)
    : m_Dim(dim) { }
  gNestedList(const gList<T> &data, const gList<int> &dim)
    : m_Data(data), m_Dim(dim) { }
  virtual ~gNestedList() {}

// gNestedList<T> &operator=(const gNestedList<T> &);

    // DATA ACCESS
  const gList<int> &Dim(void) const { return m_Dim; }
  const gList<T> &Data(void) const { return m_Data; }
  gList<T> &Data(void) { return m_Data; }
  
  gNestedList<T> NthElement(int i) const;
  int NumElements(void) const;
  bool Contains(const gNestedList<T> &) const;

  void Remove( int i );
  int Depth(void) const;

  bool operator==(const gNestedList<T> &b) const;
  bool operator!=(const gNestedList<T> &b) const;

  void Output(gOutput &out) const;
};


template <class T> gOutput &operator<<(gOutput &out, const gNestedList<T> &list)
{
  list.Output(out);
  return out;
}


#endif   // GNLIST_H


