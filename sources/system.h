//
// FILE: system.h -- Imeplements operating system specific functions
//
// $Id$
//

#ifndef SYSTEM_H
#define SYSTEM_H

class System {
private:
  System(void);

public:
  static const char *GetEnv(const char *name);
  static int SetEnv(const char *name, const char *value);
  static int UnSetEnv(const char *name);

  static const char *GetCmdInterpreter(void);
  static int Shell(const char *command);
  static int Spawn(const char *command);

  // This returns the slash character for the system;
  //   forward '/' for UNIX, backward '\' for DOS/Windows
  static char Slash(void);
};

#endif  // SYSTEM_H

