//#
//# FILE: gclsig.cc -- Definition of signal handler for the GCL
//#
//# $Id$
//#

#include "gsignal.h"
#include <signal.h>

class gGCLSignal : public gSignal   {
  private:
    bool sig;

    friend void gGCLSignalHandler(int);

  public:
    gGCLSignal(void);
    virtual ~gGCLSignal();

    void Get(void) const;
    void Reset(void);
};

gGCLSignal::gGCLSignal(void) : sig(false)
{
  signal(SIGINT, gGCLSignalHandler);
}

gGCLSignal::~gGCLSignal()
{
  signal(SIGINT, SIG_DFL);
}

void gGCLSignal::Get(void) const
{
  if (sig)
    throw gSignalBreak();
}

void gGCLSignal::Reset(void)
{
  sig = false;
}


gGCLSignal _gbreak;
gSignal &gbreak = _gbreak;


void gGCLSignalHandler(int)
{
  _gbreak.sig = true;
// This is here because some systems (Solaris) reset the signal handler to
// default when using signal().
  signal(SIGINT, gGCLSignalHandler);
}


