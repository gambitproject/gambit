

#include <assert.h>
#include <ctype.h>

#ifdef __GNUG__

#include <unistd.h>
#define TEMPLATE template

#elif defined __BORLANDC__

#include "winio.h"
#pragma option -Jgd
#define TEMPLATE

#endif   // __GNUG__, __BORLANDC__

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
  rawTerm.c_iflag &= ~IGNBRK;
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

  rawTerm.c_lflag &= ~ISIG;
  rawTerm.c_lflag &= ~ICANON;
  rawTerm.c_lflag &= ~XCASE;
  rawTerm.c_lflag &= ~ECHO;

  rawTerm.c_cc[VTIME] = 0;
  rawTerm.c_cc[VMIN] = 1;
  tcsetattr( STDIN_FILENO, TCSANOW, &rawTerm );



#endif // __GNUG__
}





gCmdLineInput::gCmdLineInput( int historyDepth )
: m_HistoryDepth( historyDepth ),
  m_Verbose( true ),
  m_Prompt( true )
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



void gCmdLineInput::GetCmdExec( void )
{
  // number of invokations thus far
  static int numInvoke = 0;

  int i = 0;

  if( m_CmdExec.length() > 0 )
    return;


  // set up the editable prompt
  char buf[16];
  if( m_Prompt )
  {
    if( m_Verbose )
    {
      if( numInvoke < m_HistoryDepth )
	sprintf( buf, "GCL%d:= << ", numInvoke );
      else
	sprintf( buf, "GCL%d:=; GCL%d:= << ",
		numInvoke - m_HistoryDepth, 
		numInvoke );
    }
    else
    {
		if( numInvoke < m_HistoryDepth )
	sprintf( buf, "GCL%d:= ", numInvoke );
      else
	sprintf( buf, "GCL%d:=; GCL%d:= ",
		numInvoke - m_HistoryDepth,
		numInvoke );
	 }
	 ++numInvoke;
  }
  else
	 sprintf( buf, "" );


  gString cmdBuf = buf;
  gString cmdBufOld;
  int curPos = cmdBuf.length();
  char c = 0;

  int historyPos = m_History.Length() + 1;

  gout << cmdBuf;

  for( ; ; ) // infinite loop
  {
	 assert( 0 <= curPos );
	 assert( curPos <= cmdBuf.length() );

#ifdef __BORLANDC__
	 winio_setecho( winio_current(), false );
	 c = winio_getchar();
#else
	 gin.get( c );
#endif // __BORLANDC__
	 if( c == EOF || c == '\r' || c == '\n' )
		break;


	 if( c == 27 ||
		  c == VK_LEFT || c == VK_RIGHT ||
		  c == VK_DOWN || c == VK_UP ||
		  c == VK_DELETE ) // escape sequences
	 {
		EscapeCode code = ESC_ERROR;
		if( c == 27 )
		  code = GetEscapeSequence();
		else
		{
		  switch( c )
		  {
		  case VK_LEFT:
			 code = ESC_LEFT;
			 break;
		  case VK_RIGHT:
			 code = ESC_RIGHT;
			 break;
		  case VK_UP:
			 code = ESC_UP;
			 break;
		  case VK_DOWN:
			 code = ESC_DOWN;
			 break;
		  case VK_DELETE:
			 code = ESC_DELETE;
          break;

		  default:
			 // do nothing
          ;
		  }
		}

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
	if( curPos < cmdBuf.length() )
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
	  for( i = 0; i < cmdBuf.length(); ++i )
		 gout << ' ';
#ifndef USE_CR
	  for( i = 0; i < cmdBuf.length(); ++i )
		 gout << '\b';
#endif

	  // save the latest line
	  if( historyPos > m_History.Length() )
		 cmdBufOld = cmdBuf;

	  --historyPos;
	  cmdBuf = m_History[historyPos];
	  curPos = cmdBuf.length();

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
	  for( i = 0; i < cmdBuf.length(); ++i )
		 gout << ' ';
#ifndef USE_CR
	  for( i = 0; i < cmdBuf.length(); ++i )
	    gout << '\b';
#endif

	  ++historyPos;
	  if( historyPos <= m_History.Length() )
	    cmdBuf = m_History[historyPos];
	  else // restore the latest line
		 cmdBuf = cmdBufOld;
	  curPos = cmdBuf.length();
	  
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
	if( curPos < cmdBuf.length() )
	{
	  cmdBuf.remove( curPos );

	  // print the entire string after the current cursor position
	  gout << cmdBuf.right( cmdBuf.length() - curPos );
	  gout << ' '; // this to erase the last character

	  // reposition the cursor
	  for( i = 0; i < cmdBuf.length() - curPos + 1; ++i )
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
		curPos = cmdBuf.length();
    }
	 else if( c == '\b' || c == 127 ) // backspace
    {
      if( curPos > 0 )
      {
	cmdBuf.remove( curPos - 1 );
	--curPos;

	// print the entire string after the current cursor position
	gout << '\b';
	gout << cmdBuf.right( cmdBuf.length() - curPos );
	gout << ' '; // this to erase the last character

	// reposition the cursor
	for( i = 0; i < cmdBuf.length() - curPos + 1; ++i )
	  gout << '\b';
      }
		else // nothing to delete, beep
	gout << '\a';
    }
    else if( isprint( c ) )// normal characters
    {
      cmdBuf.insert( c, curPos );
      ++curPos;
      
		// print the entire string after the current cursor position
      gout << cmdBuf.right( cmdBuf.length() - curPos + 1 );
      
      // reposition the cursor
      for( i = 0; i < cmdBuf.length() - curPos; ++i )
	gout << '\b';
    }
  }

  if( historyPos < m_History.Length() + 1 )
    --numInvoke;
  
  if( c == EOF )
    m_EOF = true;
  m_History.Append( cmdBuf );
  m_CmdExec = cmdBuf + "\n";
  gout << '\n';

  // if the history is too long now, get rid of the first one
  if( m_History.Length() > m_HistoryDepth )
    m_History.Remove( 1 );
  
}







gCmdLineInput::EscapeCode gCmdLineInput::GetEscapeSequence( void ) const
{
  char c1 = 0;
  char c2 = 0;
  char c3 = 0;

  // remember that the first Escape has already been caught

  // the second char must be '[' in an escape sequence
  if( !gin.eof() )
#ifdef __BORLANDC__
	 winio_setecho( winio_current(), false );
	 c1 = winio_getchar();
#else
	 gin.get( c1 );
#endif // __BORLANDC__
  if( c1 != '[' )
	 return ESC_ERROR;


  if( !gin.eof() )
#ifdef __BORLANDC__
	 winio_setecho( winio_current(), false );
	 c2 = winio_getchar();
#else
	 gin.get( c2 );
#endif // __BORLANDC__
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
	 if( !gin.eof() )
#ifdef __BORLANDC__
	 winio_setecho( winio_current(), false );
	 c3 = winio_getchar();
#else
	 gin.get( c3 );
#endif // __BORLANDC__
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
  assert( num <= m_CmdExec.length() );
  int i = 0;
  for( i = 0; i < num; ++i )
    m_CmdExec.remove( 0 );
}





gInput& gCmdLineInput::operator >> (int &x)
{
  int tokens = 0;
  while( tokens == 0 )
  {
    GetCmdExec();
    assert( m_CmdExec.length() > 0 );
    
	 int num = 0;
    tokens = sscanf( m_CmdExec.stradr(), "%d%n", &x, &num );
    EatSpace( num );
  }
  return *this; 
}

gInput& gCmdLineInput::operator >> (unsigned int &x)
{
  int tokens = 0;
  while( tokens == 0 )
  {
	 GetCmdExec();
    assert( m_CmdExec.length() > 0 );
    
    int num = 0;
	 tokens = sscanf( m_CmdExec.stradr(), "%d%n", &x, &num );
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
    assert( m_CmdExec.length() > 0 );
    
    int num = 0;
    tokens = sscanf( m_CmdExec.stradr(), "%ld%n", &x, &num );
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
    assert( m_CmdExec.length() > 0 );
    
    int num = 0;
    tokens = sscanf( m_CmdExec.stradr(), "%c%n", &x, &num );
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
    assert( m_CmdExec.length() > 0 );
    
	 int num = 0;
    tokens = sscanf( m_CmdExec.stradr(), "%lf%n", &x, &num );
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
    assert( m_CmdExec.length() > 0 );
    
    int num = 0;
    tokens = sscanf( m_CmdExec.stradr(), "%f%n", &x, &num );
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
	 assert( m_CmdExec.length() > 0 );

    int num = 0;
    tokens = sscanf( m_CmdExec.stradr(), "%s%n", x, &num );
    EatSpace( num );
  }
  return *this; 
}



gCmdLineInput _gcmdline( 20 );
gCmdLineInput& gcmdline( _gcmdline );



#include "gstack.imp"

TEMPLATE class gStack< bool >;
