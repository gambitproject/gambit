//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implements operating system specific functions
//

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef __GNUG__
#include <unistd.h>
#elif defined __BORLANDC__
#include <windows.h>
#endif   // __GNUG__, __BORLANDC__

#include "gstream.h"
#include "system.h"

const char *System::GetEnv(const char *name)
{
  assert(name != NULL);
  assert(strlen(name) > 0);
  return getenv(name);
}

int System::SetEnv(const char *name, const char *value)
{
  assert(name != NULL);
  assert(strlen(name) > 0);
  assert(value != NULL);
  char *envstr = new char[strlen(name) + strlen(value) + 2];
  strcpy(envstr, name);
  strcat(envstr, "=");
  strcat(envstr, value);
  return putenv(envstr);
}

int System::UnSetEnv(const char *name)
{
  assert(name != NULL);
  assert(strlen(name) > 0);  
  char *envstr = new char[strlen(name) + 1];
  strcpy(envstr, name);
  return putenv(envstr);
}


const char *System::GetCmdInterpreter(void)
{
#ifdef __GNUG__
  return System::GetEnv("SHELL");
#elif defined __BORLANDC__
  return System::GetEnv("COMSPEC");
#endif   // __GNUG__, __BORLANDC__
}

int System::Shell(const char *command)
{
  if (command == NULL)
    command = System::GetCmdInterpreter();
  
  int result = -1;
  if (command == NULL) {
    gerr << "System::Shell: Command interpreter not found or\n";
    gerr << "               feature not implemented for this compiler.\n";
  }
  else {
    assert(command != NULL);
    result = system(command);
  }
  return result;
}

int System::Spawn(const char *command)
{    
  if (command == 0)
    command = System::GetCmdInterpreter();
  
#ifdef __GNUG__
  pid_t pid = fork();
  if (pid == 0) {
    System::Shell(command);
    exit(0);
  }
  return 0;

#elif defined __BORLANDC__

  int result = WinExec(command, SW_SHOW);  
  if (0 <= result && result < 32) {
    gerr << "System::Spawn: Error executing \"" << command << "\"\n";
    return -1;
  }
  else
    return 0;
#endif   // __GNUG__, __BORLANDC__
}


// This returns the slash character for the system
char System::Slash(void)
{
  return '/';
}

char *System::Slashify(char *path)
{
  for (int i = 0; i < (int) strlen(path); i++)  {
    if (path[i] == '/' || path[i] == '\\')
      path[i] = Slash();
  }
  return path;
}




