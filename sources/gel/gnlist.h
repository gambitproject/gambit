
// $Id$

#ifndef __gnlist_h__
#define __gnlist_h__

#include "glist.h"




template <class T> class gNestedList
{
private:
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
  gList< T > m_Data;
  gList< int > m_Dim;

  //------------------------------------------------------------------
  // GetElementInfo
  // 
  // Return values:
  //   If the element requested is itself a list, then:
  //     islist = true
  //     el_start and el_end point to the start and the ending index numbers
  //       in Data()
  //     k_start and k_end point to the opening and closing braces,
  //       respectively, corresponding to the sublist in Dim()
  //   otherwise
  //     islist = false
  //     el_start = el_end, both pointing to the element in Data()
  //     k_start and k_end are undefined
  //-----------------------------------------------------------------
  void GetElementInfo( int el, 
		       bool& islist, 
		       int& k_start, int& k_end, 
		       int& el_start, int& el_end ) const;
  

public:
  gNestedList( void ) { m_Dim.Append( 0 ); }

  gNestedList( const gList<int>& dim )
    : m_Dim( dim ) { }

  gNestedList( const gList<T>& data, const gList<int>& dim )
    : m_Data( data ), m_Dim( dim ) { }

  virtual ~gNestedList() {}

  const gList< int >& Dim( void ) const { return m_Dim; }
  const gList< T >& Data( void ) const { return m_Data; }
  gList< T >& Data( void ) { return m_Data; }
  

  gNestedList<T> NthElement( int i ) const;
  int NumElements( void ) const;
  bool Contains( const gNestedList<T>& t ) const;

  void Output( gOutput& out ) const; 

  // gNestedList<T> &operator=(const gNestedList<T> &);

  bool operator==(const gNestedList<T> &b) const
  { return Dim() == b.Dim() && Data() == b.Data(); }
  bool operator!=(const gNestedList<T> &b) const
  { return !operator==(b); }


  void Remove( int i );

  int Depth( void ) const
  {
    if( m_Dim.Length() <= 1 )
      return 0;

    int i = 0;
    int n = NumElements();
    int mindepth = 1;
    if( 0 < n )
      mindepth = NthElement( 1 ).Depth() + 1;
    for( i = 2; i <= n; ++i )
    {
      int depth = NthElement( i ).Depth() + 1;
      if( depth < mindepth )
	mindepth = depth;
    }
    return mindepth;
  }


  //----------------------------------------------------------
  // The following operations are not implemented (for now)
  // to prevent confusion 
  //----------------------------------------------------------

  // const T &operator[](int) const;
  // T &operator[](int);
  
  // gNestedList<T> operator+(const T &e) const;
  // gNestedList<T>& operator+=(const T &e);
  
  // gNestedList<T> operator+(const gNestedList<T>& b) const;
  // gNestedList<T>& operator+=(const gNestedList<T>& b);
  
  // gNestedList<T> &Combine(gNestedList<T> &b);
  
  // virtual int Append(const T &);
  // int Insert(const T &, int);
  // virtual T Remove(int);
  
  // int Find(const T &) const;
  // bool Contains(const T &t) const;
  // int Length(void) const;
  
  // void Flush(void);
  // void Dump(gOutput &) const;

};


template <class T> 
gOutput& operator<<( gOutput& out, const gNestedList<T>& list )
{
  list.Output( out );
  return out;
}


#endif // __gnlist_h__


