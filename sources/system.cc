//
// FILE: system.cc -- Imeplements operating system specofic functions
//
// $Id$
//





#include <stdlib.h>
#include <assert.h>
#include <string.h>


#ifdef __GNUG__
  #include <unistd.h>
#elif defined __BORLANDC__
  #include <windows.h>
#endif   // __GNUG__, __BORLANDC__


#include "gambitio.h"
#include "system.h"



System::System( void )
{
  assert( 0 );
}


const char* System::GetEnv( const char* name )
{
  return getenv( name );
}



const char* System::GetCmdInterpreter( void )
{
  const char* command = NULL;
#ifdef __GNUG__
  command = System::GetEnv( "SHELL" );
#elif defined __BORLANDC__
  command = System::GetEnv( "COMSPEC" );
#endif   // __GNUG__, __BORLANDC__
  return command;
}

int System::Shell( const char* command )
{
  
  if( command == NULL )
    command = System::GetCmdInterpreter();
  
  int result = -1;
  if( command == NULL )
  {
    gerr << "System::Shell: Command interpreter not found or\n";
    gerr << "               feature not implemented for this compiler.\n";
  }
  else
  {
    assert( command != NULL );
    result = system( command );
  }
  return result;
}


int System::Spawn( const char* command )
{    
  if( command == 0 )
    command = System::GetCmdInterpreter();
  
#ifdef __GNUG__
  pid_t pid = fork();
  if( pid == 0 )
  {
    System::Shell( command );
    exit( 0 );
  }
  return 0;
#elif defined __BORLANDC__

  int result = WinExec( command, SW_SHOW );  
  if( 0 <= result && result < 32 )
  {
    gerr << "System::Spawn: Error executing \"" << command << "\"\n";
    return -1;
  }
  else
    return 0;
#endif   // __GNUG__, __BORLANDC__
}
