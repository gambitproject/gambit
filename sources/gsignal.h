//#
//# FILE: gsignal.h -- Declaration of abstract signal-handling class
//#
//# @(#)gsignal.h	1.1 5/9/95
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
