//
// FILE: system.h -- Imeplements operating system specofic functions
//
// $Id$
//




#ifndef __system_h__
#define __system_h__


class System
{
private:
  System( void );

public:
  static const char* GetEnv( const char* name );
  static int SetEnv( const char* name, const char* value );
  static int UnSetEnv( const char* name );

  static const char* GetCmdInterpreter( void );
  static int Shell( const char* command );
  static int Spawn( const char* command );

  // This returns the slash character for the system;
  //   forward '/' for UNIX, backward '\' for DOS/Windows
  static char Slash( void );
  
};


#endif // __system_h__

