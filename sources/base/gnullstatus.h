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
  gbtStatus &SetWidth(int) { return *this; }
  int GetPrec(void) const { return 0; }
  gbtStatus &SetPrec(int) { return *this; }
  gbtStatus &SetExpMode(void) { return *this; }
  gbtStatus &SetFloatMode(void) { return *this; }
  char GetRepMode(void) const { return 'f'; }

  gbtStatus &operator<<(int) { return *this; }
  gbtStatus &operator<<(unsigned int) { return *this; }
  gbtStatus &operator<<(bool) { return *this; }
  gbtStatus &operator<<(long) { return *this; }
  gbtStatus &operator<<(char) { return *this; }
  gbtStatus &operator<<(double) { return *this; }
  gbtStatus &operator<<(float) { return *this; }
  gbtStatus &operator<<(const char *) { return *this; }
  gbtStatus &operator<<(const void *) { return *this; }
  gbtStatus &operator<<(const std::string &) { return *this; }

  bool IsValid(void) const { return true; }
  void SetProgress(double) { }
  void SetProgress(double, const std::string &) { }

  void Get(void) const throw (gbtInterruptException) { }
  void Reset(void) { }
};

#endif  // GNULLSTATUS_H
