//
// File: gprogress.h -- Declaration of an abstract class to handle progress
// 											updates
// @(#)gprogres.h	1.3 6/21/95
//

#ifndef GPROGRES_H
#define GPROGRES_H
#include "gambitio.h"
#include "gstring.h"

// This class is designed to give the use some feedback about the operation of
// an algorithm.  It implements all of the functions of gOutput, and one extra
// function to explicitly state the % done.

class gProgress : public gOutput
{
private:

public:
	virtual ~gProgress(void) { }
// functions for gOutput
	// see gambitio.h
// functions to show progress
	virtual void SetProgress(double p)=0;
};

#endif
