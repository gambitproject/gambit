//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to a general stopwatch class
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

#ifndef GWATCH_H
#define GWATCH_H

//
// This is a general stopwatch class.  Under Unix, it returns the time elapsed
// in terms of the process' user time; under DOS and MS-Windows, it returns
// the total elapsed time.
//
// Note that time functions can be highly system-dependent, especially under
// Unix, so care should be taken when porting this code.
//
class gWatch    {
  private:
    bool running;
    long start, stop;
    char *time_str;

  public:
    gWatch(bool run = true);
    ~gWatch();

    bool IsRunning(void) const;

    void Start(void);
    void Stop(void);
    double Elapsed(void) const;
    
    char *const ElapsedStr(void);
};

#endif   // GWATCH_H
