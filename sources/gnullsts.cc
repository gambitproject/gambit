//
// File gnullsts: instantiation of a null gStatus.  This gStatus does NOTHING!
//
// $Id$
//

#include "gstatus.h"

class gNullStatus : public gStatus
{
public:
	~gNullStatus(void) { ;}
	// functions for gProgress::gOutput
	int GetWidth(void) { ;}
	gOutput &SetWidth(int w) { ;}
	int GetPrec(void) { ;}
	gOutput &SetPrec(int p) { ;}
	gOutput &SetExpMode(void) { ;}
	gOutput &SetFloatMode(void) { ;}
	char GetRepMode(void) { ;}

	gOutput &operator<<(int x) { ;}
	gOutput &operator<<(unsigned int x) { ;}
	gOutput &operator<<(long x) { ;}
	gOutput &operator<<(char x) { ;}
	gOutput &operator<<(double x) { ;}
	gOutput &operator<<(float x) { ;}
	gOutput &operator<<(const char *x) { ;}
	gOutput &operator<<(const void *x) { ;}

	bool IsValid(void) const { ;}
	// functions for gProgress
	void	SetProgress(double p) { ;}
	// functions for gSignal
	bool Get(void) const {return false;}
	void Reset(void) { ;}
};

gNullStatus _gstatus;
gStatus &gstatus=_gstatus;
//gSignal &gbreak=_gstatus;


