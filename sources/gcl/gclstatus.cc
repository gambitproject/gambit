//
// FILE: gclstatus.cc -- Status monitor for terminal-based GCL interface
//
// $Id$
//

#include <signal.h>
#include "gclstatus.h"
#include "gsmconsole.h"

gclStatus::gclStatus(void)
  : m_sig(false), m_width(0), m_prec(6), m_represent('f')
{ } 

gclStatus::~gclStatus()
{
#ifndef __BORLANDC__
  signal(SIGINT, SIG_DFL);
#endif
}

gclStatus gsmConsole::s_status;

gStatus &gsmConsole::GetStatusMonitor(void) 
{ return s_status; }

void gsmConsole::gclStatusHandler(int)
{
  s_status.m_sig = true;
// This is here because some systems (Solaris) reset the signal handler to
// default when using signal().
  signal(SIGINT, gclStatusHandler);
}


