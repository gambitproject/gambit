//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of an abstract class combining features of gProgress and gSignal
// 

#ifndef GSTATUS_H
#define GSTATUS_H

#include "gsignal.h"
#include "gprogres.h"

class gStatus : public gSignal, public gProgress {
public:
  virtual ~gStatus() { }
};

#endif  // GSTATUS_H










