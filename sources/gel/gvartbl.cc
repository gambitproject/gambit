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
  if (!IsDefined(name))  {
    switch (type)   {
      case gelNUMBER:
        m_NumberNames.Append(name);
        m_NumberValues.Append(0);
	break;
      case gelBOOLEAN:
	m_BooleanNames.Append(name);
	m_BooleanValues.Append(triMAYBE);
	break;
      case gelTEXT:
	m_TextNames.Append(name);
	m_TextValues.Append("");
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

void gelVariableTable::Value(const gText &name, gNumber &value) const
{
  if (m_NumberNames.Contains(name))  {
    value = m_NumberValues[m_NumberNames.Find(name)];
    return;
  }

  assert(0);
}

void gelVariableTable::Value(const gText &name, gTriState &value) const
{
  if (m_BooleanNames.Contains(name))  {
    value = m_BooleanValues[m_BooleanNames.Find(name)];
    return;
  }

  assert(0);
}

void gelVariableTable::Value(const gText &name, gText &value) const
{
  if (m_TextNames.Contains(name))  {
    value = m_TextValues[m_TextNames.Find(name)];
    return;
  }

  assert(0);
}

void gelVariableTable::SetValue(const gText &name, const gNumber &value)
{
  assert(m_NumberNames.Contains(name));
  m_NumberValues[m_NumberNames.Find(name)] = value;
}

void gelVariableTable::SetValue(const gText &name, const gTriState &value)
{
  assert(m_BooleanNames.Contains(name));
  m_BooleanValues[m_BooleanNames.Find(name)] = value;
}

void gelVariableTable::SetValue(const gText &name, const gText &value)
{
  assert(m_TextNames.Contains(name));
  m_TextValues[m_TextNames.Find(name)] = value;
}


#include "glist.imp"

template class gList<gText>;
template class gNode<gText>;

template class gList<gNumber>;
template class gNode<gNumber>;

template class gList<gTriState>;
template class gNode<gTriState>;


