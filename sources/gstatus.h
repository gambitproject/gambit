//
// File: gstatus.h -- declaration of an abstract class to combine features
//                    of gProgress and gSignal. 
// 
// $Id$
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










