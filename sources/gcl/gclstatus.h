//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Status monitor for terminal-based GCL interface
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

#ifndef GCLSTATUS_H
#define GCLSTATUS_H

#include "base/gstatus.h"

class gclStatus : public gStatus  {
friend class gsmConsole;
private:
  bool m_sig;
  int m_width, m_prec;
  char m_represent;

public:
  // CONSTRUCTOR, DESTRUCTOR
  gclStatus(void); 
  virtual ~gclStatus();

  // OUTPUT DISPLAY FORMATS
  int GetWidth(void) const { return m_width; }
  gOutput &SetWidth(int w) { m_width = w; return *this; }
  int GetPrec(void) const { return m_prec; }
  gOutput &SetPrec(int p) { m_prec = p; return *this; }
  gOutput &SetExpMode(void) { m_represent = 'e'; return *this; }
  gOutput &SetFloatMode(void) { m_represent = 'f'; return *this; }
  char GetRepMode(void) const { return m_represent; } 

  // OUTPUT OPERATORS
  gOutput &operator<<(int) { return *this; }
  gOutput &operator<<(unsigned int) { return *this; }
  gOutput &operator<<(bool) { return *this; }
  gOutput &operator<<(long) { return *this; }
  gOutput &operator<<(char) { return *this; }
  gOutput &operator<<(double) { return *this; }
  gOutput &operator<<(long double) { return *this; }
  gOutput &operator<<(float) { return *this; }
  gOutput &operator<<(const char *) { return *this; }
  gOutput &operator<<(const void *) { return *this; }
  bool IsValid(void) const { return true; }

  void SetProgress(double) { }
  void SetProgress(double, const gText &) { }
  void Get(void) const 
    { if (m_sig) throw gSignalBreak(); }
  void Reset(void) { m_sig = false; }
};

#endif  // GCLSTATUS_H
