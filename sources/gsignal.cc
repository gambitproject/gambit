//#
//# FILE: gsignal.cc -- Implementation of "default" signal-handling class
//#
//# $Id$
//#

#include "gsignal.h"

class gNullSignal : public gSignal   {
  public:
    gNullSignal(void);
    virtual ~gNullSignal();

    bool Get(void) const;
    void Reset(void);
};

gNullSignal::gNullSignal(void)   { }

gNullSignal::~gNullSignal()   { }

bool gNullSignal::Get(void) const   { return false; }

void gNullSignal::Reset(void)    { }


gNullSignal _gbreak;
gSignal &gbreak = _gbreak;
