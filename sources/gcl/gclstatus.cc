//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Status monitor for terminal-based GCL
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


