//
// FILE: gpreproc.cc -- Implementation of gPreprocessor class,
//                      preprocessor for the command language input
//
// $Id$
//



#include "gpreproc.h"




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
	    line += ' ';
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
	  if( bracket < 0 )
	    bracket = 0;
	  if( bracket == 0 )
	    SetPrompt( true );	  

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

	    if( c == '\"' )
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
	  break;

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
	      quote = !quote;
	  }
	  SetPrompt( true );
	  break;

	}
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

	  gInput* input = new gFileInput( filename );
	  if( input->IsValid() )
	  {
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

	  break;
	  
	}

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




