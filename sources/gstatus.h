//
// File: gstatus.h -- declaration of an abstract class to combine features
//										of gprogress and gsignal.  Used by most algorithms.
// @(#)gstatus.h	1.3 6/21/95
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



