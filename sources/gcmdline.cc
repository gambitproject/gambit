//
// FILE: gcmdline.cc -- Implementation of editing interface for GCL
//
//  $Id$
//

#include <ctype.h>

#ifdef __GNUG__
#include <unistd.h>
#endif   // __GNUG__

#include "gcmdline.h"

int gCmdLineInput::s_NumInstances = 0;
#ifdef __GNUG__
struct termios gCmdLineInput::s_TermAttr;
#endif // __GNUG__

void gCmdLineInput::SaveTermAttr( void )
{
#ifdef __GNUG__
  // save the current terminal attributes
  tcgetattr( STDIN_FILENO, &s_TermAttr );
#endif // __GNUG__
}

void gCmdLineInput::RestoreTermAttr( void )
{
#ifdef __GNUG__
  // restore the terminal attributes to the original
  tcsetattr( STDIN_FILENO, TCSANOW, &s_TermAttr );
#endif // __GNUG__
}



void gCmdLineInput::SetRawTermAttr( void )
{
#ifdef __GNUG__
  struct termios rawTerm;
  tcgetattr( STDIN_FILENO, &rawTerm );

  // system( "stty raw -echo opost" );
  /*
     stty [-]raw
     Same  as  -ignbrk  -brkint  -ignpar  -parmrk -inpck
     -istrip -inlcr -igncr -icrnl  -ixon  -ixoff  -iuclc
     -ixany  -imaxbel  -opost -isig -icanon -xcase min 1
     time 0.  With `-', same as cooked.
     */

  rawTerm.c_iflag &= ~IGNBRK;
  rawTerm.c_iflag &= ~BRKINT;
  rawTerm.c_iflag &= ~IGNPAR;
  rawTerm.c_iflag &= ~PARMRK;
  rawTerm.c_iflag &= ~INPCK;
  rawTerm.c_iflag &= ~ISTRIP;
  rawTerm.c_iflag &= ~INLCR;
  rawTerm.c_iflag &= ~IGNCR;
  rawTerm.c_iflag &= ~ICRNL;
  rawTerm.c_iflag &= ~IXON;
  rawTerm.c_iflag &= ~IXOFF;
  rawTerm.c_iflag &= ~IUCLC;
  rawTerm.c_iflag &= ~IXANY;
  rawTerm.c_iflag &= ~IMAXBEL;

  rawTerm.c_oflag |= OPOST;

  // rawTerm.c_lflag &= ~ISIG;
  rawTerm.c_lflag |= ISIG;
  rawTerm.c_lflag &= ~ICANON;
  rawTerm.c_lflag &= ~XCASE;
  rawTerm.c_lflag &= ~ECHO;

  rawTerm.c_cc[VTIME] = 0;
  rawTerm.c_cc[VMIN] = 1;
  tcsetattr( STDIN_FILENO, TCSANOW, &rawTerm );



#endif // __GNUG__
}

gCmdLineInput::gCmdLineInput( int historyDepth )
  : GCL::CommandLine(historyDepth),
    m_NumInvoke(0), m_EOF(false), m_Verbose(true), m_Prompt(true)
{
  if( s_NumInstances == 0 )
  {
    SaveTermAttr();
    SetRawTermAttr();
  }
  ++s_NumInstances;
}

gCmdLineInput::~gCmdLineInput()
{
  --s_NumInstances;
  if( s_NumInstances == 0 )
    RestoreTermAttr();
}

char gCmdLineInput::GetNextChar(void)
{
  char c;
  gin.get(c);
  return c;
}

void gCmdLineInput::GetCmdExec( void )
{
  // number of invokations thus far

  int i = 0;

  if( m_CmdExec.Length() > 0 )
    return;


  // set up the editable prompt
  char buf[32];
  if( m_Prompt )
  {
    ++m_NumInvoke;
    if( m_Verbose )
    {
      if( m_NumInvoke < HistoryDepth() )
	sprintf( buf, "GCL%d:= << ", m_NumInvoke );
      else
	sprintf( buf, "GCL%d:=; GCL%d:= << ",
		m_NumInvoke - HistoryDepth(),
		m_NumInvoke );
    }
    else
    {
      if( m_NumInvoke < HistoryDepth() )
	sprintf( buf, "GCL%d:= ", m_NumInvoke );
      else
	sprintf( buf, "GCL%d:=; GCL%d:= ",
		m_NumInvoke - HistoryDepth(),
		m_NumInvoke );
    }
  }
  else
    sprintf( buf, "%s","" );


  gText cmdBuf = buf;
  gText cmdBufOld;
  int curPos = cmdBuf.Length();
  char c = 0;

  int historyPos = m_History.Length() + 1;

  gout << cmdBuf;

  for( ; ; ) { // infinite loop
    c = GetNextChar();
    if( c == EOF || c == '\r' || c == '\n' )
      break;


    if( c == 27 ) // escape sequences
    {
      EscapeCode code = GetEscapeSequence();

      switch( code )
      {
      case ESC_LEFT:
	if( curPos > 0 )
	{
	  --curPos;
	  gout << '\b';
	}
	else // can't move left, beep
	  gout << '\a';
	break;

      case ESC_RIGHT:
	if( curPos < cmdBuf.Length() )
	{
	  gout << cmdBuf[curPos];
	  ++curPos;
	}
	else // can't move right, beep
	  gout << '\a';
	break;

      case ESC_UP:
	if( historyPos > 1 )
	{
	  // clear the current line
#ifdef USE_CR
	  gout << '\r';
#else
	  for( i = 0; i < curPos; ++i )
	    gout << '\b';
#endif
	  for( i = 0; i < cmdBuf.Length(); ++i )
	    gout << ' ';
#ifndef USE_CR
	  for( i = 0; i < cmdBuf.Length(); ++i )
	    gout << '\b';
#endif

	  // save the latest line
	  if( historyPos > m_History.Length() )
	    cmdBufOld = cmdBuf;

	  --historyPos;
	  cmdBuf = m_History[historyPos];
#ifndef USE_OLD_PROMPT
	  cmdBuf = UpdatePromptNum( cmdBuf );
#endif
	  curPos = cmdBuf.Length();

	  // display the new line
#ifdef USE_CR
	  gout << '\r';
#endif
	  gout << cmdBuf;
	}
	else // can't go up, beep
	  gout << '\a';
	break;

      case ESC_DOWN:
	if( historyPos <= m_History.Length() )
	{
	  // clear the current line
#ifdef USE_CR
	  gout << '\r';
#else
	  for( i = 0; i < curPos; ++i )
	    gout << '\b';
#endif
	  for( i = 0; i < cmdBuf.Length(); ++i )
	    gout << ' ';
#ifndef USE_CR
	  for( i = 0; i < cmdBuf.Length(); ++i )
	    gout << '\b';
#endif

	  ++historyPos;
	  if( historyPos <= m_History.Length() )
	  {
	    cmdBuf = m_History[historyPos];
#ifndef USE_OLD_PROMPT
	    cmdBuf = UpdatePromptNum( cmdBuf );
#endif
	  }
	  else // restore the latest line
	    cmdBuf = cmdBufOld;
	  curPos = cmdBuf.Length();
	  
	  // display the new line
#ifdef USE_CR
	  gout << '\r';
#endif
	  gout << cmdBuf;
	}
	else // can't go up, beep
	  gout << '\a';
	break;

      case ESC_DELETE:
	if( curPos < cmdBuf.Length() )
	{
	  cmdBuf.Remove( curPos );

	  // print the entire string after the current cursor position
	  gout << cmdBuf.Right( cmdBuf.Length() - curPos );
	  gout << ' '; // this to erase the last character

	  // reposition the cursor
	  for( i = 0; i < cmdBuf.Length() - curPos + 1; ++i )
	    gout << '\b';
	}
	else // nothing to delete, beep
	  gout << '\a';
	break;

      default: // bad escape sequence, beep
	gout << '\a';
      }
    }
    else if( c == 'R' - 'A' + 1 ) // ^R, refreshes the line
    {
      gout << "^R\n";
      gout << cmdBuf;
      curPos = cmdBuf.Length();
    }
    else if( c == '\b' || c == 127 ) // backspace
    {
      if( curPos > 0 )
      {
	cmdBuf.Remove( curPos - 1 );
	--curPos;

	// print the entire string after the current cursor position
	gout << '\b';
	gout << cmdBuf.Right( cmdBuf.Length() - curPos );
	gout << ' '; // this to erase the last character

	// reposition the cursor
	for( i = 0; i < cmdBuf.Length() - curPos + 1; ++i )
	  gout << '\b';
      }
    }
    else if( isprint( c ) )// normal characters
    {
      cmdBuf.Insert( c, curPos );
      ++curPos;
      
      // print the entire string after the current cursor position
      gout << cmdBuf.Right( cmdBuf.Length() - curPos + 1 );
      
      // reposition the cursor
      for( i = 0; i < cmdBuf.Length() - curPos; ++i )
	gout << '\b';
    }
  }


#ifdef USE_OLD_PROMPT
  if( historyPos < m_History.Length() + 1 )
    --m_NumInvoke;
#endif


  if (c == EOF) {
    m_EOF = true;
  }
  m_History.Append( cmdBuf );
  m_CmdExec = cmdBuf + "\n";
  gout << '\n';

  // if the history is too long now, get rid of the first one
  if( m_History.Length() > HistoryDepth() )
    m_History.Remove( 1 );
  
}


gCmdLineInput::EscapeCode gCmdLineInput::GetEscapeSequence(void)
{
  char c1 = 0;
  char c2 = 0;
  char c3 = 0;

  // remember that the first Escape has already been caught

  // the second char must be '[' in an escape sequence
  if (!gin.eof()) {
    c1 = GetNextChar();
  }
  if (c1 != '[') {
    return ESC_ERROR;
  }

  if (!gin.eof()) {
    c2 = GetNextChar();
  }

  switch( c2 )
  {
  case 65: // up arrow
    return ESC_UP;
  case 66: // down arrow
    return ESC_DOWN;
  case 68: // left arrow
    return ESC_LEFT;
  case 67: // right arrow
    return ESC_RIGHT;

  case 51: // delete key, if followed by 126
    if (!gin.eof()) {
      c3 = GetNextChar();
    }
    if( c3 == 126 )
      return ESC_DELETE;
    else
      return ESC_ERROR;
    break;

  default:
    return ESC_ERROR;
  }
}

void gCmdLineInput::EatSpace( int num )
{
  for (int i = 0; i < num; i++)
    m_CmdExec.Remove(0);
}

gInput& gCmdLineInput::operator >> (int &x)
{
  int tokens = 0;
  while (tokens == 0) {
    GetCmdExec();

    int num = 0;
    tokens = sscanf( m_CmdExec, "%d%n", &x, &num );
    EatSpace( num );
  }
  return *this; 
}

gInput& gCmdLineInput::operator >> (unsigned int &x)
{
  int tokens = 0;
  while (tokens == 0) {
    GetCmdExec();
    
    int num = 0;
    tokens = sscanf( m_CmdExec, "%d%n", &x, &num );
    EatSpace( num );
  }
  return *this; 
}

gInput& gCmdLineInput::operator >> (long &x)
{
  int tokens = 0;
  while( tokens == 0 )
  {
    GetCmdExec();
    
    int num = 0;
    tokens = sscanf( m_CmdExec, "%ld%n", &x, &num );
    EatSpace( num );
  }
  return *this; 
}

gInput& gCmdLineInput::operator >> (char &x) 
{
  int tokens = 0;
  while( tokens == 0 )
  {
    GetCmdExec();
    
    int num = 0;
    tokens = sscanf( m_CmdExec, "%c%n", &x, &num );
    EatSpace( num );
  }
  return *this; 
}

gInput& gCmdLineInput::operator >> (double &x)
{
  int tokens = 0;
  while( tokens == 0 )
  {
    GetCmdExec();

    int num = 0;
    tokens = sscanf( m_CmdExec, "%lf%n", &x, &num );
    EatSpace( num );
  }
  return *this;
}

gInput& gCmdLineInput::operator >> (float &x)
{
  int tokens = 0;
  while( tokens == 0 )
  {
    GetCmdExec();
    
    int num = 0;
    tokens = sscanf( m_CmdExec, "%f%n", &x, &num );
    EatSpace( num );
  }
  return *this; 
}

gInput& gCmdLineInput::operator >> (char *x)
{
  int tokens = 0;
  while( tokens == 0 )
  {
    GetCmdExec();

    int num = 0;
    tokens = sscanf( m_CmdExec, "%s%n", x, &num );
    EatSpace( num );
  }
  return *this; 
}




gText gCmdLineInput::UpdatePromptNum( gText cmdBuf ) const
{
  char buf[512] = "";
  int val0 = 0;
  int val1 = 0;
  int numchars0 = 0;
  int numchars1 = 0;
  int match = sscanf( cmdBuf, "GCL%d:=%n; GCL%d:=%n", 
		     &val0, &numchars0, &val1, &numchars1 );
  if( match == 1 )
  {
    sprintf( buf, "GCL%d:=%s", m_NumInvoke, &cmdBuf[numchars0] );
    cmdBuf = buf;
  }
  else if( match == 2 )
  {
    sprintf( buf, "GCL%d:=; GCL%d:=%s",
	    m_NumInvoke - HistoryDepth(), m_NumInvoke, 
	    &cmdBuf[numchars1] );
    cmdBuf = buf;
  }
  return cmdBuf;
}

#include "base/gstack.imp"
template class gStack<bool>;
