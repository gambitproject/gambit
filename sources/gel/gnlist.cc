

// $Id$

#include <assert.h>

#include "tristate.h"
#include "gtext.h"
#include "gnumber.h"


#include "gnlist.h"
#include "glist.imp"




// this dumb second parameter is only here to facilitate printing
//   of gTriState stuff
template <class T> 
void gNestedList<T>::Output( gOutput& out, 
			     gOutput& (*disp_func)( gOutput& out, T ) ) const
{
  int i = 0;
  int j = 0;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    assert( m_Dim.Length() == 1 );

    if( disp_func )
      disp_func( out, operator[](1) );
    else
      out << operator[](1);    
  }
  else // is a list
  {
    for( i = 1; i <= m_Dim.Length(); ++i )
    {
      assert( m_Dim[i] != 0 );
      for( j = 0; j < abs( m_Dim[i] ) - 1; ++j )
      {
	if( disp_func )
	  disp_func( out, operator[](el) );
	else
	  out << operator[](el);

	if( j != abs( m_Dim[i] ) - 2 || m_Dim[i] > 0 )
	  out << ", ";
	else
	  out << ' ';
	++el;
      }
      if( m_Dim[i] > 0 )
	out << "{ ";
      else if( i == m_Dim.Length() || 
	       ( i < m_Dim.Length() && m_Dim[i+1] == -1 ) )
	out << "} ";
      else
	out << "}, ";
    }
  }
}




template class gList<int>;

template class gNestedList< gNumber >;
template class gNestedList< gText >;
template class gNestedList< gTriState >;

template class 
gOutput& operator<<( gOutput& out, const gNestedList< gNumber >& list );
template class 
gOutput& operator<<( gOutput& out, const gNestedList< gText >& list );
template class 
gOutput& operator<<( gOutput& out, const gNestedList< gTriState >& list );

