//#
//# FILE: gclsig.cc -- Definition of signal handler for the GCL
//#
//# $Id$
//#

#include "gsignal.h"
#include <signal.h>

class gGCLSignal : public gSignal   {
  private:
    static bool sig = false;

    static void Handler(int);

  public:
    gGCLSignal(void);
    virtual ~gGCLSignal();

    bool Get(void) const;
    void Reset(void);
};

gGCLSignal::gGCLSignal(void)
{
  signal(SIGINT, gGCLSignal::Handler);
}

gGCLSignal::~gGCLSignal()
{
  signal(SIGINT, SIG_DFL);
}

bool gGCLSignal::Get(void) const
{
  return sig;
}

void gGCLSignal::Reset(void)
{
  sig = false;
}

void gGCLSignal::Handler(int)
{
  sig = true;
}


gGCLSignal _gbreak;
gSignal &gbreak = _gbreak;


