//
// FILE: gpreproc.h -- Implementation of gPreprocessor class,
//                     preprocessor for the command language input
//
// $Id$
//



#ifndef GPREPROC_H
#define GPREPROC_H


#include <assert.h>
#include <string.h>

#include "gcmdline.h"
#include "gtext.h"
#include "gstack.h"


class gPreprocessor
{
private:
  gCmdLineInput* m_CmdLine;

  gStack< gInput* > m_InputStack;
  gStack< int >     m_LineNumberStack;
  gStack< gText > m_FileNameStack;

  gText           m_RawLine;
  gText           m_PrevFileName;
  int               m_PrevLineNumber;

  gText           m_StartupString;
  


  gInput* LoadInput( gText& name );
  bool ExpectText( const char* text );
  
  bool EOL( char c ) const { return ( c == '\n' || c == '\r' ); }

  void SetPrompt( bool prompt )
  {
    if( m_InputStack.Peek() == m_CmdLine )
      m_CmdLine->SetPrompt( prompt );	  
  }

  void GetChar( char& c )
  {
    if( m_StartupString.Length() > 0 )
    {
      c = m_StartupString[0];
      m_StartupString = m_StartupString.Right( m_StartupString.Length() - 1 );
    }
    else
    {
      assert( !eof() );
      m_InputStack.Peek()->get( c );
      if( EOL( c ) )
	++m_LineNumberStack.Peek();
    }

    m_RawLine += c;
  }

  bool IsQuoteEscapeSequence( const gText& line ) const
  {
    bool backslash = false;
    assert( line.Length() > 0 );
    char c = line[ line.Length() - 1 ];
    if( c == '\"' )
    {
      // if there's an odd number of consecutive backslashes,
      //   then it's an escape sequence 
      int i = 2;
      while( line.Length() >= i && line[ line.Length() - i ] == '\\' )
      {
	backslash = !backslash;
	++i;
      }
    }
    return backslash;
  }


public:

  gPreprocessor( gCmdLineInput* cmdline, const char* cmd = NULL ) 
    : m_CmdLine( cmdline ), m_PrevFileName( "console" ), m_PrevLineNumber( 1 ),
      m_StartupString( cmd )
  {
    assert( m_CmdLine );
    m_InputStack.Push( m_CmdLine );
    m_FileNameStack.Push( m_PrevFileName );
    m_LineNumberStack.Push( m_PrevLineNumber );

    if(cmd && !EOL( m_StartupString[ m_StartupString.Length() - 1 ] ) )
      m_StartupString += '\n';
  }

  ~gPreprocessor()
  {
    while( m_InputStack.Depth() > 1 )
      delete m_InputStack.Pop();
  }


  //----------------------------------------------
  // eof
  //   If true, then there is no more input from
  //   the preprocessor.
  //-----------------------------------------------
  bool eof( void ) const { return m_InputStack.Depth() == 0; }


  //---------------------------------------------
  // GetLine
  //   Returns the preprocessed lines.  See 
  //   gpreproc.cc for details.
  //----------------------------------------------
  gText GetLine( void );


  //---------------------------------------------
  // GetRawLine
  //   Returns the un-preprocessed line corresponding to
  //   the previous GetLine() call.
  //----------------------------------------------
  const gText& GetRawLine( void ) const
  { return m_RawLine; }



  //---------------------------------------------
  // GetFileName
  //   Returns the file name corresponding to
  //   the previous GetLine() call.
  //---------------------------------------------
  const gText& GetFileName( void ) const 
  { return m_PrevFileName; }
  

  //---------------------------------------------
  // GetLineNumber
  //   Returns the line number corresponding to
  //   the previous GetLine() call.
  //---------------------------------------------
  int GetLineNumber( void ) const 
  { return m_PrevLineNumber; }




};



#endif // __gpreproc_h__
