

// $Id$

#include <assert.h>
#include <stdlib.h>

#include "tristate.h"
#include "gtext.h"
#include "gnumber.h"


#include "gnlist.h"
#include "glist.imp"


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

  gout << "islist: " << islist << " k_start: " << k_start << " k_end: " << k_end << " el_start: " << el_start << " el_end: " << el_end << '\n';
}




// this dumb second parameter is only here to facilitate printing
//   of gTriState stuff
template <class T> 
void gNestedList<T>::Output( gOutput& out, 
			     gOutput& (*disp_func)( gOutput& out, T ) ) const
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    assert( m_Dim.Length() == 1 );

    if( disp_func )
      disp_func( out, Data()[1] );
    else
      out << Data()[1];    
  }
  else // is a list
  {
    for (i = 1; i <= m_Dim.Length(); ++i )
    {
      assert( m_Dim[i] != 0 );
      for (j = 0; j < abs( m_Dim[i] ) - 1; ++j )
      {
	if( disp_func )
	  disp_func( out, Data()[el] );
	else
	  out << Data()[el];

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




template class gList<int>;

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

template class gNestedList<gNumber>;
template class gNestedList<gText>;
template class gNestedList<gTriState>;
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

template class gList<gNestedList<Efg *> >;
template class gList<gNestedList<Node *> >;
template class gList<gNestedList<Infoset *> >;
template class gList<gNestedList<Action *> >;
template class gList<gNestedList<EFPlayer *> >;
template class gList<gNestedList<EFOutcome *> >;
template class gList<gNestedList<EFSupport *> >;
template class gList<gNestedList<BehavSolution *> >;
template class gList<gNestedList<Nfg *> >;
template class gList<gNestedList<Strategy *> >;
template class gList<gNestedList<NFPlayer *> >;
template class gList<gNestedList<NFOutcome *> >;
template class gList<gNestedList<NFSupport *> >;
template class gList<gNestedList<MixedSolution *> >;


template gOutput &operator<<(gOutput &, const gNestedList<gNumber> &);
template gOutput &operator<<(gOutput &, const gNestedList<gText> &);
template gOutput &operator<<(gOutput &, const gNestedList<gTriState> &);
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

