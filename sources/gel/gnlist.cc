//
// FILE: gnlist.cc -- Instantiation of gNestedList classes
//
// $Id$
//

#include "gnlist.imp"

#include "tristate.h"
#include "gtext.h"
#include "gnumber.h"


//
// When instantiating for certain types T, we need to override the default
// behavior of some list operations
//

//---------------------------------------------------
// Overriden functions for gNestedList<gTriState *>
//---------------------------------------------------

void gNestedList<gTriState *>::Output( gOutput& out ) const
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    if (m_Dim.Length() != 1)
      throw InternalError(__LINE__);
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
      if (m_Dim[i] == 0)
        throw InternalError(__LINE__);
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

bool gNestedList<gTriState *>::operator==(const gNestedList<gTriState *> &l) const
{
  if (m_Dim != l.m_Dim)   return false;
  for (int i = 1; i <= m_Data.Length(); i++)
    if (*m_Data[i] != *l.m_Data[i])  return false;
  return true;
}

//---------------------------------------------------
// Overriden functions for gNestedList<gNumber *>
//---------------------------------------------------

void gNestedList<gNumber *>::Output( gOutput& out ) const
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    if (m_Dim.Length() != 1)
      throw InternalError(__LINE__);

    if (!Data()[1])
      out << "(NULL)";
    else
      out << *Data()[1];    
  }
  else // is a list
  {
    for (i = 1; i <= m_Dim.Length(); ++i )
    {
      if (m_Dim[i] == 0)
        throw InternalError(__LINE__);
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

bool gNestedList<gNumber *>::operator==(const gNestedList<gNumber *> &l) const
{
  if (m_Dim != l.m_Dim)   return false;
  for (int i = 1; i <= m_Data.Length(); i++)
    if (*m_Data[i] != *l.m_Data[i])  return false;
  return true;
}

//---------------------------------------------------
// Overriden functions for gNestedList<gText *>
//---------------------------------------------------

void gNestedList<gText *>::Output( gOutput& out ) const 
{
  int i;
  int j;
  int el = 1;

  if( m_Dim[1] == 0 ) // not a list
  {
    if (m_Dim.Length() != 1)
      throw InternalError(__LINE__);

    if (!Data()[1])
      out << "(NULL)";
    else
      out << *Data()[1];    
  }
  else // is a list
  {
    for (i = 1; i <= m_Dim.Length(); ++i )
    {
      if (m_Dim[i] == 0)
        throw InternalError(__LINE__);
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

bool gNestedList<gText *>::operator==(const gNestedList<gText *> &l) const
{
  if (m_Dim != l.m_Dim)   return false;
  for (int i = 1; i <= m_Data.Length(); i++)
    if (*m_Data[i] != *l.m_Data[i])  return false;
  return true;
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
