//#
//# FILE: gsignal.h -- Declaration of abstract signal-handling class
//#
//# $Id$
//#

#ifndef SIGNAL_H
#define SIGNAL_H

#include "basic.h"

class gSignal   {
  public:
    virtual ~gSignal()  { }
    virtual bool Get(void) const = 0;
    virtual void Reset(void) = 0;
};

extern gSignal &gbreak;

#endif   // SIGNAL_H
