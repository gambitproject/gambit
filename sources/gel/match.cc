//
// FILE: match.cc -- Function matching code
//
// $Id$
//

#include <ctype.h>

#include "match.h"
#include "funcmisc.h"

#include "gnumber.h"
#include "glist.h"
#include "garray.h"


gelParamInfo::gelParamInfo(const gText &s)
  : m_Optional( false ), m_ByReference( false )
{
  int index = 0;
  int length = s.Length();
  gText word;
  bool quote = false;
  
  assert( index < length );
  if( s[index] == '{' )
  {
    m_Optional = true;
    ++index;
  }
  
  word = "";
  while( isalnum( s[index] ) )
  {
    assert( index < length );
    word += s[index];
    ++index;
  }
  m_Name = word;
  

  assert( index < length );
  if( s[index] == '<' )
  {
    m_ByReference = true;
    ++index;
  }
  
  assert( index < length && s[index] == '-' );
  ++index;
  assert( index < length && s[index] == '>' );
  ++index;
  
  
  word = "";
  quote = false;
  while( isalnum( s[index] ) || 
	 ( ispunct( s[index] ) && s[index] != '}' ) ||
	 quote )
  {
    assert( index < length );
    if( s[index] == '\"' )
      quote = !quote;
    word += s[index];
    ++index;
  }
  
  if( m_Optional )
  {
    assert( index < length && s[index] == '}' );
    m_Default = word;
  }
  else
  {
    if (word == "NUMBER")
      m_Type = gelNUMBER;
    else if (word == "BOOLEAN")
      m_Type = gelBOOLEAN;
    else if (word == "TEXT")
      m_Type = gelTEXT;
  }
  
}







gOutput &operator<<(gOutput &f, const gelSignature &)  { return f; }
gelSignature::gelSignature(const gText &s)
{ 

  gText word;
  int index = 0;
  int param = 0;
  bool quote = false;
  bool done = false;
  int length = s.Length();



  word = "";
  assert( index < length );
  while( s[index] != '[' )
  {
    assert( index < length );
    if( isalnum( s[index] ) )
      word += s[index];
    ++index;
  }
  assert( s[index] == '[' );
  assert( index < length );
  m_Name = word;
  
  
  ++index;
  param = 0;
  done = false;
  while( !done )
  {
    assert( index < length );

    word = "";
    quote = false;
    while( (s[index] != ',' && s[index] != ']') || quote )
    {
      assert( index < length );

      if( isalnum( s[index] ) || ispunct( s[index] ) || quote )
      {
	if( s[index] == '\"' )
	  quote = !quote;
	word += s[index];
      }
      ++index;	

    }
    
    if (word != "")   {
      m_Parameters.Append( new gelParamInfo( word ) );
      ++param;
    }

    assert( index < length );
    if( s[index] == ']' )
      done = true;
    ++index;
  }


  while( index < length && isspace( s[index] ) )
    ++index;
  if( index >= length )
  {
    // NOTICE
    // this is for those "incomplete functions" without return types
    // should be elimited in the future

    word = "ANYTYPE";
  }
  else
  {
    assert( index < length && s[index] == '=' );
    ++index;
    assert( index < length && s[index] == ':' );
    ++index;
    
    word = "";
    while( index < length )
    {
      if( isalnum( s[index] ) || ispunct( s[index] ) )
	word += s[index];
      ++index;
    }
  }

  if (word == "NUMBER")
    m_Type = gelNUMBER;
  else if (word == "BOOLEAN")
    m_Type = gelBOOLEAN;
  else if (word == "TEXT")
    m_Type = gelTEXT;

}

gelSignature::~gelSignature()
{
  for (int i = 1; i <= m_Parameters.Length(); i++)
    delete m_Parameters[i];
}

bool
gelSignature::Matches(const gText &n,
		      const gArray<gelExpr *> &actuals) const
{
  if (n != m_Name ||
      actuals.Length() != m_Parameters.Length())
    return false;

  for (int i = 1; i <= actuals.Length(); i++)  {
    if (actuals[i]->Type() != m_Parameters[i]->Type())
      return false;
  }

  return true;
}

extern void gelMathInit(gelEnvironment *);

gelEnvironment::gelEnvironment(void)
{
  gelMathInit(this);
}

gelEnvironment::~gelEnvironment()
{ } 

void gelEnvironment::Register(gelAdapter *func, const gText &sig)
{
  signatures.Append(new gelSignature(sig));
  functions.Append(func);
}

gelExpr *gelEnvironment::Match(const gText &name, 
			       const gArray<gelExpr *> &actuals)
{
  for (int i = 1; i <= actuals.Length(); i++)   {
    if (!actuals[i])  return 0;
  }

  for (int i = 1; i <= signatures.Length(); i++)  {
    if (signatures[i]->Matches(name, actuals))
      return (*functions[i])(actuals);
  }

  return 0;
}

gelExpr *gelEnvironment::Match(const gText &name, gelExpr *op1, gelExpr *op2)
{
  gArray<gelExpr *> actuals(2);
  actuals[1] = op1;
  actuals[2] = op2;

  if (!op1 || !op2)  return 0;

  for (int i = 1; i <= signatures.Length(); i++)  {
    if (signatures[i]->Matches(name, actuals))
      return (*functions[i])(actuals);
  }
  
  return 0;
}

gelExpr *gelEnvironment::Match(const gText &name, gelExpr *op)
{
  gArray<gelExpr *> actuals(1);
  actuals[1] = op;

  if (!op)  return 0;

  for (int i = 1; i <= signatures.Length(); i++)  {
    if (signatures[i]->Matches(name, actuals))
      return (*functions[i])(actuals);
  }

  return 0;
}
  

#include "garray.imp"

template class gArray<gelExpr*>;

#include "glist.imp"

template class gList<gelSignature *>;
template class gNode<gelSignature *>;
template class gList<gelAdapter *>;
template class gNode<gelAdapter *>;
template class gList<gelParamInfo *>;
template class gNode<gelParamInfo *>;

