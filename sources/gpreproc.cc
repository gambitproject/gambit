//
// FILE: gpreproc.cc -- Implementation of gPreprocessor class,
//                      preprocessor for the command language input
//
// $Id$
//



#include "gpreproc.h"
#include "system.h"



//-------------------------------------------------------------------
// GetLine
//   Does the main pre-processing job.
//
//  This function handles the following:
//
//  o Include[] statements.  If the file exists, it is included, and the
//    Include[] function call is replaced with "True".  If the file is
//    not found or there is a syntax error, it is replaced with "False".
//   
//  o Strips all comments.  The comment types processed are C and C++ 
//    style comments.
//  
//  o Filter out explicit continuations, i.e., a backslash at the end 
//    of the line.  Only works if the backslash is the last character
//    on the line; trailing spaces are not accepted.
//
//  o Handle implicit continuations (bracket matching)
//    If brackets are open at a carriage return, then the CR is
//    replaced with a space.
//---------------------------------------------------------------------    

gString gPreprocessor::GetLine( void )
{

  // If no more input available, return nothing.
  if( eof() )
    return gString( "" );

  // Record the current file name and line number.
  m_PrevFileName = m_FileNameStack.Peek();
  m_PrevLineNumber = m_LineNumberStack.Peek();



  // This is initialized to work with the explicit continuation
  //   processing code.  The backslask will be stripped.
  gString line = '\\';
  
  gString errorMsg;

  char c = 0;
  bool quote = false;
  bool error = false;
  int bracket = 0;
  bool continuation = false;


  while( line.right( 1 ) == '\\' && !error )
  {
    // Strip the trailing backslash
    line = line.left( line.length() - 1 );

    while( !m_InputStack.Peek()->eof() )
    {
      GetChar( c );
      
      if( !m_InputStack.Peek()->eof() )
      {
	// Ignore CR or replace with space as appropriate.
	if( !EOL( c ) )
	  line += c;
	else
	{ 
	  if( bracket > 0 )
	  {
	    if( line.right( 1 ) == '\\' )
	      line = line.left( line.length() - 1 );
	    line += ' ';
	  }
	  else
	    break;
	}


	if( c == '[' )
	{
	  // Turn of prompts until the bracket is matched.
	  if( bracket == 0 )
	    SetPrompt( false );	  
	  ++bracket;

	}
	else if( c == ']' )
	{
	  // If brackets are matched, turn prompt back on.
	  --bracket;
	  if( bracket == 0 )
	    SetPrompt( true );	  

	  if( bracket < 0 )
	    bracket = 0;

	}
	else if( c == '\"' )
	{
	  // In the middle of a string; accept everything
	  //   until the closing quote mark is found.
	  SetPrompt( false );	  
	  quote = !quote;
	  while( !m_InputStack.Peek()->eof() && quote )
	  {
	    GetChar( c );
	    line += c;

	    if( c == '\"' && !IsQuoteEscapeSequence( line ) )
	      quote = !quote;
	  }
	  SetPrompt( true );	  

	}


	assert( !quote );

	if( line.right( 2 ) == "//" )
	{
	  // In a line comment.  Ignore everything until
	  //   end of line.
	  line = line.left( line.length() - 2 );

	  while( !m_InputStack.Peek()->eof() && !EOL( c ) )
	    GetChar( c );
	}
	else if( line.right( 2 ) == "/*" )
	{
	  // In a C style comment.  Ignore everything until
	  //   comment closing is found.
	  line = line.left( line.length() - 2 );

	  SetPrompt( false );
	  gString comment = "  ";
	  while( !m_InputStack.Peek()->eof() && 
		( comment.right( 2 ) != "*/" || quote ) )
	  {
	    GetChar( c );
	    comment += c;

	    if( c == '\"' )
	    {
	      bool escapeSeq = false;
	      if( quote )
		escapeSeq = IsQuoteEscapeSequence( comment );
	      if( !escapeSeq )
		quote = !quote;
	    }
	  }
	  SetPrompt( true );

	}

	//------------------- Include -------------------------------
	else if( line.right( 7 ) == "Include" )
	{
	  // Process Include[] calls.

	  line = line.left( line.length() - 7 );

	  c = ' ';
	  while( !m_InputStack.Peek()->eof() && c == ' ' )
	    GetChar( c );
	  if( c != '[' )
	  {
	    line += "False";
	    errorMsg = "Include[] syntax error; opening '[' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while( !m_InputStack.Peek()->eof() && c == ' ' )
	    GetChar( c );
	  if( c != '\"' )
	  {
	    line += "False";
	    errorMsg = "Include[] syntax error; opening '\"' not found";
	    error = true;
	    break;
	  }

	  gString filename;
	  c = ' ';
	  while( !m_InputStack.Peek()->eof() && c != '\"' && !EOL( c ) )
	  {
	    GetChar( c );
	    if( c != '\"' )
	      filename += c;
	  }
	  if( EOL( c ) )
	  {
	    line += "False";
	    errorMsg = "Include[] syntax error; closing '\"' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while( !m_InputStack.Peek()->eof() && c == ' ' )
	    GetChar( c );
	  if( c != ']' )
	  {
	    line += "False";
	    errorMsg = "Include[] syntax error; closing ']' not found";
	    error = true;
	    break;
	  }

	  // bring in the rest of this line
	  gString restOfLine;
	  c = ' ';
	  while( !m_InputStack.Peek()->eof() && !EOL( c ) )
	  {
	    GetChar( c );
	    restOfLine += c;
	  }

	  // note: filename might be changed after this call
	  gInput* input = LoadInput( filename );
	  if( input )
	  {
	    assert( input->IsValid() );
	    line += "True";
	    m_InputStack.Push( input );
	    m_FileNameStack.Push( filename );
	    m_LineNumberStack.Push( 1 );
	  }
	  else
	  {
	    line += "False";
	    errorMsg = "Include file \"" + filename + "\" not found";
	    error = true;
	  }

	  line += restOfLine;
	  break;
	  
	} // "Include"

	//------------------- GetPath -------------------------------
	else if( line.right( 7 ) == "GetPath" )
	{
	  // Process GetPath[] calls.

	  bool text_found = false;
	  bool closed_bracket = false;
	  bool file = true;
	  bool path = true;

	  line = line.left( line.length() - 7 );

	  c = ' ';
	  while( !m_InputStack.Peek()->eof() && c == ' ' )
	    GetChar( c );
	  if( c != '[' )
	  {
	    line += "\"\"";
	    errorMsg = "GetPath[] syntax error; opening '[' not found";
	    error = true;
	    break;
	  }

	  c = ' ';
	  while( !m_InputStack.Peek()->eof() && c == ' ' )
	    GetChar( c );
	  text_found = false;
	  if( c == 'T' ) // True
	  {
	    text_found = ExpectText( "rue" );
	    if( text_found )
	      file = true;
	  }
	  else if( c == 'F' )
	  {
	    text_found = ExpectText( "alse" );
	    if( text_found )
	      file = false;
	  }
	  else if ( c == ']' )
	  {
	    text_found = true;
	    closed_bracket = true;
	  }
	  if( !text_found )
	  {
	    line += "\"\"";
	    errorMsg = "GetPath[] syntax error (1)";
	    error = true;
	    break;
	  }

	  if( !closed_bracket )
	  {
	    c = ' ';
	    while( !m_InputStack.Peek()->eof() && c == ' ' )
	      GetChar( c );
	    text_found = false;
	    if( c == ',' )
	    {
	      text_found = true;
	    }
	    else if( c == ']' )
	    {
	      text_found = true;
	      closed_bracket = true;
	    }
	    if( !text_found )
	    {
	      line += "\"\"";
	      errorMsg = "GetPath[] syntax error (2)";
	      error = true;
	      break;
	    }
	  }

	  if( !closed_bracket )
	  {
	    c = ' ';
	    while( !m_InputStack.Peek()->eof() && c == ' ' )
	      GetChar( c );
	    text_found = false;
	    if( c == 'T' ) // True
	    {
	      text_found = ExpectText( "rue" );
	      if( text_found )
		path = true;
	    }
	    else if( c == 'F' )
	    {
	      text_found = ExpectText( "alse" );
	      if( text_found )
		path = false;
	    }
	    if( !text_found )
	    {
	      line += "\"\"";
	      errorMsg = "GetPath[] syntax error (3)";
	      error = true;
	      break;
	    }
	  }

	  if( !closed_bracket )
	  {
	    c = ' ';
	    while( !m_InputStack.Peek()->eof() && c == ' ' )
	      GetChar( c );
	    if( c != ']' )
	    {
	      line += "\"\"";
	      errorMsg = "GetPath[] syntax error; closing ']' not found";
	      error = true;
	      break;
	    }
	  }

	  // now the file and path variables are defined,
	  //   determine return value
	  gString txt = GetFileName();
	  const char SLASH = System::Slash();

	  if( file && path )
	  {
	  }
	  else if( file )
	  {
	    if( txt.lastOccur( SLASH ) > 0 )
	      txt = txt.right( txt.length() - txt.lastOccur( SLASH ) );
	  }
	  else if( path )
	  {
	    if( txt.lastOccur( SLASH ) > 0 )
	      txt = txt.left( txt.lastOccur( SLASH ) );
	    else
	      txt = "";
	  }
	  else
	    txt = "";
	  
	  line += (gString) '\"' + (gString) txt + (gString) '\"';

	} // "GetPath"

      }
    }

    // This outer loop deals with explicit line continuation characters
    if( line.right( 1 ) == '\\' )
    {
      if( !continuation )
	SetPrompt( false );
      continuation = true;
    }
  }

  if( continuation )
    SetPrompt( true );

  
  if( error )
    gerr << "GCL Error: " << errorMsg << '\n';


  if( m_InputStack.Peek()->eof() )
  {
    delete m_InputStack.Pop();
    (void) m_FileNameStack.Pop();
    (void) m_LineNumberStack.Pop();
  }

  return line;
}






// note: filename might be changed after this call
gInput* gPreprocessor::LoadInput( gString& name )
{
  gInput* _Input = NULL;
  extern char* _SourceDir;
  const char* SOURCE = _SourceDir; 
  assert( SOURCE );
  
  const char SLASH = System::Slash();
  
  bool search = false;
  if( strchr( (char *) name, SLASH ) == NULL )
    search = true;
  gString IniFileName;
  
  IniFileName = name;
  _Input = new gFileInput( IniFileName );
  if( _Input->IsValid() )
  {
    name = IniFileName;
    return _Input;
  }
  else
    delete _Input;

  if( search )
  {
    if( System::GetEnv( "HOME" ) != NULL )
    {
      IniFileName = (gString) System::GetEnv( "HOME" ) + (gString) SLASH + (gString) name;
      _Input = new gFileInput( IniFileName );
      if( _Input->IsValid() )
      {
	name = IniFileName;
	return _Input;
      }
      else
	delete _Input;
    }

    if( System::GetEnv( "GCLLIB" ) != NULL )
    {
      IniFileName = (gString) System::GetEnv( "GCLLIB" ) + (gString) SLASH + (gString) name;
      _Input = new gFileInput( IniFileName );
      if( _Input->IsValid() )
      {
	name = IniFileName;
	return _Input;
      }
      else
	delete _Input;
    }

    if( SOURCE != NULL )
    {
      IniFileName = (gString) SOURCE + (gString) SLASH + (gString) name;
      _Input = new gFileInput( IniFileName );
      if( _Input->IsValid() )
      {
	name = IniFileName;
	return _Input;
      }
      else
	delete _Input;
    }
  }

  return NULL;
}




bool gPreprocessor::ExpectText( const char* text )
{
  int length = strlen( text );
  int i = 0;
  char c = 0;

  if( m_InputStack.Peek()->eof() )
    return false;

  for( i = 0; i < length; ++i )
  {
    GetChar( c );
    if( m_InputStack.Peek()->eof() || c != text[i] )
      return false;
  }
  return true;
}
