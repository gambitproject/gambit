//
// $Source$
// $Date$
// $Revision$
// 
// DESCRIPTION:
// Implements operating system specific functions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
  static char *Slashify(char *path);
};

#endif  // SYSTEM_H

