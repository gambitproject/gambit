//
// FILE: gprogress.h -- Declaration of an abstract class to handle progress
// 			updates
// $Id$
//

#ifndef GPROGRES_H
#define GPROGRES_H

#include "gambitio.h"

// This class is designed to give the use some feedback about the operation of
// an algorithm.  It implements all of the functions of gOutput, and one extra
// function to explicitly state the % done.

class gProgress : public gOutput  {
  public:
    virtual ~gProgress(void) { }
    virtual void SetProgress(double p) = 0;
};

#endif    // GPROGRES_H
