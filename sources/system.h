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
  static const char* GetCmdInterpreter( void );
  static int Shell( const char* command );
  static int Spawn( const char* command );
  
};


#endif // __system_h__

