//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of a null status object
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

#ifndef GNULLSTATUS_H
#define GNULLSTATUS_H

#include "gstatus.h"

class gbtNullStatus : public gbtStatus  {
public:
  virtual ~gbtNullStatus(void) { }

  int GetWidth(void) const { return 0; }
  gbtOutput &SetWidth(int) { return *this; }
  int GetPrec(void) const { return 0; }
  gbtOutput &SetPrec(int) { return *this; }
  gbtOutput &SetExpMode(void) { return *this; }
  gbtOutput &SetFloatMode(void) { return *this; }
  char GetRepMode(void) const { return 'f'; }

  gbtOutput &operator<<(int) { return *this; }
  gbtOutput &operator<<(unsigned int) { return *this; }
  gbtOutput &operator<<(bool) { return *this; }
  gbtOutput &operator<<(long) { return *this; }
  gbtOutput &operator<<(char) { return *this; }
  gbtOutput &operator<<(double) { return *this; }
  gbtOutput &operator<<(float) { return *this; }
  gbtOutput &operator<<(const char *) { return *this; }
  gbtOutput &operator<<(const void *) { return *this; }

  bool IsValid(void) const { return true; }
  void SetProgress(double) { }
  void SetProgress(double, const gbtText &) { }

  void Get(void) const { }
  void Reset(void) { }
};

#endif  // GNULLSTATUS_H
