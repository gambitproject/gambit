//
// FILE: gclstatus.h -- Status monitor for terminal-based GCL interface
//
// $Id$
//

#ifndef GCLSTATUS_H
#define GCLSTATUS_H

#include "gstatus.h"

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
  gOutput &operator<<(float) { return *this; }
  gOutput &operator<<(const char *) { return *this; }
  gOutput &operator<<(const void *) { return *this; }
  bool IsValid(void) const { return true; }

  void SetProgress(double) { }
  void Get(void) const 
    { if (m_sig) throw gSignalBreak(); }
  void Reset(void) { m_sig = false; }
};

#endif  // GCLSTATUS_H
