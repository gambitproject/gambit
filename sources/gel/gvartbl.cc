//
// FILE: gvartbl.cc -- Implementation of variable lookup table
//
// $Id$
//

#include "gvartbl.h"

gelVariableTable::gelVariableTable(void)
{ }

void gelVariableTable::Define(const gText &name, const gelType type)
{
  gList< gNumber >   list_gNumber; 
  gList< gTriState > list_gTriState;
  gList< gText >     list_gText;

  if (!IsDefined(name))  
  {
    switch (type)   
    {
      case gelNUMBER:
        m_NumberNames.Append(name);
        list_gNumber.Append( 0 );
        m_NumberValues.Append( list_gNumber );
	break;
      case gelBOOLEAN:
	m_BooleanNames.Append(name);
        list_gTriState.Append( triMAYBE );
	m_BooleanValues.Append( list_gTriState );
	break;
      case gelTEXT:
	m_TextNames.Append(name);
        list_gText.Append( "" );
	m_TextValues.Append( list_gText );
	break;
      default:
	assert(0);
    }
  }
}

bool gelVariableTable::IsDefined(const gText &name) const
{
  return m_NumberNames.Contains(name) || m_BooleanNames.Contains(name) ||
         m_TextNames.Contains(name);
}

gelType gelVariableTable::Type(const gText &name) const
{
  if (m_NumberNames.Contains(name))
    return gelNUMBER;
  else if (m_BooleanNames.Contains(name))
    return gelBOOLEAN;
  else if (m_TextNames.Contains(name))
    return gelTEXT;

  assert(0);
  return gelBOOLEAN;
}

void gelVariableTable::Value(const gText &name, gList<gNumber>& value) const
{
  if (m_NumberNames.Contains(name))  {
    value = m_NumberValues[m_NumberNames.Find(name)];
    return;
  }

  assert(0);
}

void gelVariableTable::Value(const gText &name, gList<gTriState>& value) const
{
  if (m_BooleanNames.Contains(name))  {
    value = m_BooleanValues[m_BooleanNames.Find(name)];
    return;
  }

  assert(0);
}

void gelVariableTable::Value(const gText &name, gList<gText>& value) const
{
  if (m_TextNames.Contains(name))  {
    value = m_TextValues[m_TextNames.Find(name)];
    return;
  }

  assert(0);
}

void 
gelVariableTable::SetValue(const gText &name, const gList<gNumber>& value)
{
  assert(m_NumberNames.Contains(name));
  m_NumberValues[m_NumberNames.Find(name)] = value;
}

void 
gelVariableTable::SetValue(const gText &name, const gList<gTriState>& value)
{
  assert(m_BooleanNames.Contains(name));
  m_BooleanValues[m_BooleanNames.Find(name)] = value;
}

void 
gelVariableTable::SetValue(const gText &name, const gList<gText>& value)
{
  assert(m_TextNames.Contains(name));
  m_TextValues[m_TextNames.Find(name)] = value;
}


#include "glist.imp"

template class gList<gText>;
template class gList<gNumber>;
template class gList<gTriState>;

template class gList< gList<gText> >;
template class gList< gList<gNumber> >;
template class gList< gList<gTriState> >;

template operator<<( gOutput&, const gList<gText>& );
template operator<<( gOutput&, const gList<gNumber>& );
template operator<<( gOutput&, const gList<gTriState>& );
