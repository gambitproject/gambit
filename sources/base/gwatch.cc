//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of portable stopwatch class
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

#include <stdio.h>
#include <math.h>

#ifdef __GNUG__
#include <time.h>
#include <sys/times.h>
#include <limits.h>

#ifndef CLK_TCK
// This is correct for SunOS; Solaris has CLK_TCK predefined.  Other
// systems might have problems with this.
#define CLK_TCK      60
#endif   // CLK_TCK

extern "C" long _sysconf(int);

#endif   // __GNUG__

#ifdef __BORLANDC__
#include <sys\timeb.h>
#endif    // __BORLANDC__

#include "gwatch.h"


gWatch::gWatch(bool run /* = true */)
  : running(run), time_str(new char[30])
{
  if (run)
    Start();
  else
    start = 0;
  stop = 0;
}

gWatch::~gWatch()
{
  delete time_str;
}

bool gWatch::IsRunning(void) const
{
  return running;
}

void gWatch::Start(void)
{
  running = 1;

#ifdef __GNUG__
  struct tms buffer;
  times(&buffer);
  start = buffer.tms_utime;
#elif defined __BORLANDC__
  struct timeb buffer;
  ftime(&buffer);
  start = buffer.time * 1000 + buffer.millitm;
#endif   // __GNUG__, __BORLANDC__
}

void gWatch::Stop(void)
{
  if (!running)   return;

  running = 0;

#ifdef __GNUG__
  struct tms buffer;
  times(&buffer);
  stop = buffer.tms_utime;
#elif defined __BORLANDC__
  struct timeb buffer;
  ftime(&buffer);
  stop = buffer.time * 1000 + buffer.millitm;
#endif   // __GNUG__, __BORLANDC__
}

double gWatch::Elapsed(void) const
{
#ifdef __GNUG__
  if (running)   {
    struct tms buffer;
    times(&buffer);
    return (((double) (buffer.tms_utime - start)) / ((double) (CLK_TCK)));
  }
  
  return (((double) (stop - start)) / ((double) (CLK_TCK)));
#elif defined __BORLANDC__
  struct timeb buffer;
  ftime(&buffer);
  return (((double) (buffer.time * 1000 + buffer.millitm - start)) / 1000);
#endif  // __GNUG__, __BORLANDC__
}

char *const gWatch::ElapsedStr(void)
{
  double et = Elapsed();
  int total = (int) floor(et);

  int secs = total % 60;
  int temp = (total - secs) / 60;
  int mins = temp % 60;
  int hours = (temp - mins) % 60;
  
  int mus = (int) ((et * 1000) / floor(et));
  sprintf(time_str, "%02d:%02d:%02d.%03d", hours, mins, secs, mus);
  return time_str;
}



