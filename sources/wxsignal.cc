//
// FILE: wxsignal.cc -- Definition of signal handler for the GUI
//
//  $Id$
//

#include "gsignal.h"

class WxSignal : public gSignal   {
private:
  bool sig;

public:
  WxSignal(void): sig(false) { }
  virtual ~WxSignal() { }

  void Get(void) { if (sig)  throw gSignalBreak(); } 
  void Reset(void) { sig = false; }
};

WxSignal _gbreak;
gSignal &gbreak = _gbreak;


