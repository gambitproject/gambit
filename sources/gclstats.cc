//
// FILE: gclstats.cc -- gStatus definition for the CommandLine gambit.
//
// $Id$
//

#include "gstatus.h"
#include <signal.h>

class gGCLStatus : public gStatus  {
friend void gGCLStatusHandler(int);
private:
  bool m_sig;
  int m_width, m_prec;
  char m_represent;

public:
  // CONSTRUCTOR, DESTRUCTOR
  gGCLStatus(void); 
  ~gGCLStatus();

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
  bool Get(void) const 
    { if (m_sig) throw gSignalBreak(); return m_sig; }
  void Reset(void) { m_sig = false; }
};

gGCLStatus::gGCLStatus(void)
  : m_sig(false), m_width(0), m_prec(6), m_represent('f')
{
#ifndef __BORLANDC__
  signal(SIGINT, gGCLStatusHandler);
#endif
}

gGCLStatus::~gGCLStatus()
{
#ifndef __BORLANDC__
  signal(SIGINT, SIG_DFL);
#endif
}


gGCLStatus _gstatus;
gStatus &gstatus = _gstatus;


void gGCLStatusHandler(int)
{
  _gstatus.m_sig = true;
// This is here because some systems (Solaris) reset the signal handler to
// default when using signal().
  signal(SIGINT, gGCLStatusHandler);
}


