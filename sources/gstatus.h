//
// File: gstatus.h -- declaration of an abstract class to combine features
//                    of gprogress and gsignal.  Used by most algorithms.
// 
// $Id$
//

#ifndef GSTATUS_H
#define GSTATUS_H

#include "gsignal.h"
#include "gprogres.h"

class gStatus: public gSignal, public gProgress
{
private:

public:
	virtual ~gStatus(void) { }
};

extern gStatus &gstatus;
#endif



