

// $Id$

#include <assert.h>
#include <stdlib.h>

#include "tristate.h"
#include "gtext.h"
#include "gnumber.h"


#include "gnlist.h"


//---------------------------------------------------------------
//                       CAUTION
// 
//  Be sure you read the description of the data structure
//  in gnlist.h before changing these methods!
//
//---------------------------------------------------------------


template <class T> 
void gNestedList<T>::GetElementInfo( int el, 
				     bool& islist, 
				     int& k_start, int& k_end, 
				     int& el_start, int& el_end ) const
{
  // el_* are for indices into the data list
  el_start = 0;
  el_end = 0;

  // k_* are for indices into the dimention list
  k_start = 0;
  k_end = 0;

  // whether the element requested is itself a list or not
  islist = false;


  assert( 0 < el );
  assert( el <= NumElements() );
  assert( m_Dim.Length() > 1 );
  assert( m_Dim[1] == 1 );

  k_end = 1;
  int depth = 1;
  int steps = abs( Dim()[2] ) - 1;

  for (int index = 1; index <= el; ++index )
  {
    k_start = k_end+1;
    el_start = el_end+1;      

    while( steps == 0 )
    {
      ++k_end;
      if( Dim()[k_end] > 0 )
	++depth;
      else
	--depth;
      steps = abs( Dim()[k_end+1] ) - 1;
    }

    ++el_end;
    --steps;

    if( depth == 1 )
    {
      islist = false;
    }
    else
    {
      islist = true;

      while( depth != 1 )
      {
	++k_end;
	if( Dim()[k_end] > 0 )
	  ++depth;
	else
	  --depth;
	el_end += steps;
	steps = abs( Dim()[k_end+1] ) - 1;
      }
    }
  }

}


template <class T> 
void gNestedList<T>::Output( gOutput& out ) const 
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    assert( m_Dim.Length() == 1 );
    if (Data()[1] != NULL)
      out << Data()[1];    
    else
      out << "(NULL)";
  }
  else // is a list
  {
    for (i = 1; i <= m_Dim.Length(); ++i )
    {
      assert( m_Dim[i] != 0 );
      for (j = 0; j < abs( m_Dim[i] ) - 1; ++j )
      {
	if (Data()[el] != NULL)
	  out << Data()[el];
	else
	  out << "(NULL)";

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



template <class T> 
gNestedList<T> gNestedList<T>::NthElement( int el ) const
{
  // el_* are for indices into the data list
  int el_start = 0;
  int el_end = 0;

  // k_* are for indices into the dimention list
  int k_start = 0;
  int k_end = 0;
  
  // whether the element requested is itself a list or not
  bool islist = false;

  // get the index values for the given element number
  GetElementInfo( el, islist, k_start, k_end, el_start, el_end );  


  gList<int> dim;
  if( !islist )
    dim.Append( 0 );
  else
  {
    for(int k = k_start; k <= k_end; ++k )
      dim.Append( m_Dim[k] );
    dim[1] = 1;
  }

  gNestedList<T> ret(dim);
  for(int i = el_start; i <= el_end; ++i )
    ret.Data().Append( Data()[ i ] );
  return ret;
}



template <class T>
int gNestedList<T>::NumElements( void ) const
{
  int num = 0;
  
  // current depth
  int depth = 0;

  if( m_Dim.Length() <= 1 )
    return 0;

  assert( m_Dim.Length() > 1 );
  assert( m_Dim[1] == 1 );
  // k is for index into the dimention list
  for (int k = 1; k <= m_Dim.Length(); ++k )
  {
    assert( m_Dim[k] != 0 );
    if( depth == 1 )
      num += abs( m_Dim[k] ) - 1;
    else if( m_Dim[k] < 0 && depth == 2)
      ++num;
      
    if( m_Dim[k] > 0 )
      ++depth;
    else
      --depth;
  }

  assert( num > 0 );
  return num;
}




template <class T> 
bool gNestedList<T>::Contains( const gNestedList<T>& t ) const
{
  int i;
  for (i = 1; i <= NumElements(); ++i )
    if( NthElement( i ) == t )
      return true;
  return false;
}


template <class T> 
void gNestedList<T>::Remove( int el )
{
  // el_* are for indices into the data list
  int el_start = 0;
  int el_end = 0;

  // k_* are for indices into the dimention list
  int k_start = 0;
  int k_end = 0;
  
  // whether the element requested is itself a list or not
  bool islist = false;

  // get the index values for the given element number
  GetElementInfo( el, islist, k_start, k_end, el_start, el_end );


  if( !islist )
  {
    assert( el_start == el_end );
    assert( k_end < Dim().Length() );
    Data().Remove( el_start );
    m_Dim[k_end+1] += (m_Dim[k_end+1]>0?1:-1) * -1;
  }
  else
  {
    assert( 1 < k_start );
    assert( k_end < Dim().Length() );
    for( int i = el_end; i >= el_start; --i )
      Data().Remove( i );
    m_Dim[k_end+1] += (m_Dim[k_end+1]>0?1:-1)*( abs( Dim()[k_start] ) - 1 );
    for( int k = k_end; k >= k_start; --k )
      m_Dim.Remove( k );
  }
}

template <class T> int gNestedList<T>::Depth( void ) const
{
  if( m_Dim.Length() <= 1 )
    return 0;

  int i = 0;
  int n = NumElements();
  int mindepth = 1;
  if( 0 < n )
    mindepth = NthElement( 1 ).Depth() + 1;
  for( i = 2; i <= n; ++i )   {
    int depth = NthElement( i ).Depth() + 1;
    if( depth < mindepth )
	    mindepth = depth;
  }
  return mindepth;
}


void gNestedList<gTriState *>::Output( gOutput& out ) const
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    assert( m_Dim.Length() == 1 );
    if (!Data()[1])
      out << "(NULL)";
    else  { 
      switch (*Data()[1])  {
      case triTRUE:
	out << "True";
	break;
      case triFALSE:
	out << "False";
	break;
      case triMAYBE:
	out << "Maybe";
	break;
      }
    }
  }
  else  // is a list
  {
    for (i = 1; i <= m_Dim.Length(); ++i )
    {
      assert( m_Dim[i] != 0 );
      for (j = 0; j < abs( m_Dim[i] ) - 1; ++j )
      {
	if (!Data()[el])
	  out << "(NULL)";
	else  {
	  switch (*Data()[el])  {
	  case triTRUE:
	    out << "True";
	    break;
	  case triFALSE:
	    out << "False";
	    break;
	  case triMAYBE:
	    out << "Maybe";
	    break;
	  }
	}

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

void gNestedList<gNumber *>::Output( gOutput& out ) const
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    assert( m_Dim.Length() == 1 );

    if (!Data()[1])
      out << "(NULL)";
    else
      out << *Data()[1];    
  }
  else // is a list
  {
    for (i = 1; i <= m_Dim.Length(); ++i )
    {
      assert( m_Dim[i] != 0 );
      for (j = 0; j < abs( m_Dim[i] ) - 1; ++j )
      {
	if (!Data()[el])
	  out << "(NULL)";
	else
	  out << *Data()[el];

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


void gNestedList<gText *>::Output( gOutput& out ) const 
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    assert( m_Dim.Length() == 1 );

    if (!Data()[1])
      out << "(NULL)";
    else
      out << *Data()[1];    
  }
  else // is a list
  {
    for (i = 1; i <= m_Dim.Length(); ++i )
    {
      assert( m_Dim[i] != 0 );
      for (j = 0; j < abs( m_Dim[i] ) - 1; ++j )
      {
	if (!Data()[el])
	  out << "(NULL)";
	else
	  out << *Data()[el];

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


class Efg;
class Node;
class Action;
class Infoset;
class EFOutcome;
class EFPlayer;
class EFSupport;
class BehavSolution;
class Nfg;
class Strategy;
class NFOutcome;
class NFPlayer;
class NFSupport;
class MixedSolution;
class gInput;
class gOutput;

template class gNestedList<gNumber *>;
template class gNestedList<gText *>;
template class gNestedList<gTriState *>;
template class gNestedList<Efg *>;
template class gNestedList<Node *>;
template class gNestedList<Infoset *>;
template class gNestedList<Action *>;
template class gNestedList<EFPlayer *>;
template class gNestedList<EFOutcome *>;
template class gNestedList<EFSupport *>;
template class gNestedList<BehavSolution *>; 
template class gNestedList<Nfg *>;
template class gNestedList<Strategy *>;
template class gNestedList<NFPlayer *>;
template class gNestedList<NFOutcome *>;
template class gNestedList<NFSupport *>;
template class gNestedList<MixedSolution *>;
template class gNestedList<gInput *>;
template class gNestedList<gOutput *>;

template gOutput &operator<<(gOutput &, const gNestedList<gNumber *> &);
template gOutput &operator<<(gOutput &, const gNestedList<gText *> &);
template gOutput &operator<<(gOutput &, const gNestedList<gTriState *> &);
template gOutput &operator<<(gOutput &, const gNestedList<Efg *> &);
template gOutput &operator<<(gOutput &, const gNestedList<Node *> &);
template gOutput &operator<<(gOutput &, const gNestedList<Action *> &);
template gOutput &operator<<(gOutput &, const gNestedList<Infoset *> &);
template gOutput &operator<<(gOutput &, const gNestedList<EFPlayer *> &);
template gOutput &operator<<(gOutput &, const gNestedList<EFOutcome *> &);
template gOutput &operator<<(gOutput &, const gNestedList<EFSupport *> &);
template gOutput &operator<<(gOutput &, const gNestedList<BehavSolution *> &);
template gOutput &operator<<(gOutput &, const gNestedList<Nfg *> &);
template gOutput &operator<<(gOutput &, const gNestedList<Strategy *> &);
template gOutput &operator<<(gOutput &, const gNestedList<NFPlayer *> &);
template gOutput &operator<<(gOutput &, const gNestedList<NFOutcome *> &);
template gOutput &operator<<(gOutput &, const gNestedList<NFSupport *> &);
template gOutput &operator<<(gOutput &, const gNestedList<MixedSolution *> &);
template gOutput &operator<<(gOutput &, const gNestedList<gInput *> &);
template gOutput &operator<<(gOutput &, const gNestedList<gOutput *> &);
