
// $Id$

#ifndef __gnlist_h__
#define __gnlist_h__

#include "glist.h"




template <class T> class gNestedList : public gList< T >
{
private:
  gList< int > m_Dim;

public:
  gNestedList( void ) { m_Dim.Append( 0 ); }
  
  gNestedList( const gList<int>& dim )
    : m_Dim( dim ) { }

  gNestedList( const gList<T>& data, const gList<int>& dim )
    : gList<T>( data ), m_Dim( dim ) { }

  const gList< int >& Dim( void ) const { return m_Dim; }

  // this dumb second parameter is only here to facilitate printing
  //   of gTriState stuff
  void Output( gOutput& out, 
	       gOutput& (*disp_func)( gOutput& out, T ) = NULL ) const;

};


template <class T> 
gOutput& operator<<( gOutput& out, const gNestedList<T>& list )
{
  list.Output( out );
  return out;
}


#endif // __gnlist_h__


