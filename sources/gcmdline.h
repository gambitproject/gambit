//
// FILE: gcmdline.h -- Command line interface for GCL
//
// $Id$
//

#ifndef GCMDLINE_H
#define GCMDLINE_H


#include <stdlib.h>
#ifdef __GNUG__
#include <termios.h>
#elif defined __BORLANDC__
// what goes here ?
#endif   // __GNUG__, __BORLANDC__

#include "gstream.h"
#include "gtext.h"
#include "glist.h"
#include "gstack.h"


class gCmdLineInput : public gInput
{
private:
  // the number of instances of thie object
  static int s_NumInstances;

  // the saved term attributes
  static struct termios s_TermAttr;

public:
  static void SaveTermAttr( void );
  static void RestoreTermAttr( void );
  static void SetRawTermAttr( void );

private:
  // the history of commands already executed
  gList< gText > m_History;
  
  // the maximum history size
  int m_HistoryDepth;
  int m_NumInvoke;

  // the last command being executed
  gText m_CmdExec;

  // whether EOF has been reached
  bool m_EOF;

  bool m_Verbose;
  bool m_Prompt;
  gStack< bool > m_PromptStack;

private:
  gCmdLineInput( const gCmdLineInput& );
  operator = ( const gCmdLineInput& );

  enum EscapeCode
  {
    ESC_ERROR,

    ESC_UP,
    ESC_DOWN,
    ESC_LEFT,
    ESC_RIGHT,

    ESC_DELETE
  };

  EscapeCode GetEscapeSequence( void ) const;

  // if the last executed command had been used up,
  //   wait until a new line has come in
  void GetCmdExec( void );

  // this will eat up the given number of characters
  //   from the start of m_CmdExec
  void EatSpace( int num );

  // this function strips old commands of their original
  //   prompt numbers and puts in the current one
  gText UpdatePromptNum( gText cmdBuf ) const;


public:
  gCmdLineInput( int historyDepth );
  virtual ~gCmdLineInput();


  virtual gInput& operator>>(int &x);
  virtual gInput& operator>>(unsigned int &x);
  virtual gInput& operator>>(long &x);
  virtual gInput& operator>>(char &x);
  virtual gInput& operator>>(double &x);
  virtual gInput& operator>>(float &x);
  virtual gInput& operator>>(char *x);
  

  virtual int get(char &c) 
  { 
    operator >> ( c ); 
    return !eof(); 
  }
  virtual void unget(char c) 
  {
    m_CmdExec.Insert( c, 0 ); 
  }

  virtual bool eof(void) const { return m_EOF; }
  virtual void seekp(long /* x */) const { }
  virtual long getpos(void) const { return 0; }
  virtual void setpos(long /* x */) const { }
  
  virtual bool IsValid(void) const { return !eof(); }

  void SetVerbose( bool verbose ) { m_Verbose = verbose; }
  bool Verbose( void ) { return m_Verbose; }

  void SetPrompt( bool prompt ) 
  {    
    if( prompt == false )
      m_PromptStack.Push( false );
    else
      m_PromptStack.Pop();
    
    if( m_PromptStack.Depth() == 0 )
      m_Prompt = true; 
    else
      m_Prompt = false;
  }
  bool Prompt( void ) { return m_Prompt; }

  void ResetPrompt( void )
  {
    m_PromptStack.Flush();
    m_Prompt = true;
  }
  
};



extern gCmdLineInput &gcmdline;

#endif   // GCMDLINE_H 
