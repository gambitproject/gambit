//
// FILE: gclstats.cc -- gStatus definition for the CommandLine gambit.
//
// $Id$
//

#include "gstatus.h"
#include <signal.h>

char tmp_str[200];
class gGCLStatus : public gStatus
{
private:
	// gSignal Stuff
	bool sig;
	friend void gGCLStatusHandler(int);
	// gProgress stuff
	int Width,Prec;
	char Represent;
public:
	// Constructor
	gGCLStatus(void) : sig(false),Width(0),Prec(6),Represent('f')	{signal(SIGINT, gGCLStatusHandler);}
	// Destructor
	~gGCLStatus(void) {signal(SIGINT, SIG_DFL);}
	// functions for gProgress::gOutput
	int GetWidth(void) {return Width;}
	gOutput &SetWidth(int w) {Width=w;return *this;}
	int GetPrec(void) {return Prec;}
	gOutput &SetPrec(int p) {Prec=p;return *this;}
	gOutput &SetExpMode(void) {Represent='e';return *this;}
	gOutput &SetFloatMode(void){Represent='f';return *this;}
	char GetRepMode(void){return Represent;}

	gOutput &operator<<(int x);
	gOutput &operator<<(unsigned int x);
	gOutput &operator<<(long x);
	gOutput &operator<<(char x);
	gOutput &operator<<(double x);
	gOutput &operator<<(float x);
	gOutput &operator<<(const char *x);
	gOutput &operator<<(const void *x);
	bool IsValid(void) const {return true;}
	// functions for gProgress
	void	SetProgress(double p) {/*gout<<(int)(p*100)<<"% Done\n";*/}
	// functions for gSignal
	bool Get(void) const {return sig;}
	void Reset(void) {sig=false;}

};

//****************************** gGclProgress stuff ********************
gOutput &gGCLStatus::operator<<(int x)
{/*sprintf(tmp_str, "%*d", Width,  x);gout<<tmp_str;*/return *this;}
gOutput &gGCLStatus::operator<<(unsigned int x)
{/*sprintf(tmp_str, "%*d", Width,  x);gout<<tmp_str;*/return *this;}
gOutput &gGCLStatus::operator<<(long x)
{/*sprintf(tmp_str, "%*ld", Width, x);gout<<tmp_str;*/return *this;}
gOutput &gGCLStatus::operator<<(char x)
{/*sprintf(tmp_str, "%c", x);gout<<tmp_str;*/return *this;}
gOutput &gGCLStatus::operator<<(double x)
  {/*
	switch (Represent) {
	case 'f':
		sprintf(tmp_str, "%*.*lf", Width, Prec, x);
		gout<<tmp_str;
		break;
	case 'e':
		sprintf(tmp_str, "%*.*le", Width, Prec, x);
		gout<<tmp_str;
	      }*/
return *this;
}
gOutput &gGCLStatus::operator<<(float x)
  {/*
	switch (Represent) {
	case 'f':
		sprintf(tmp_str, "%*.*lf", Width, Prec, x);
		gout<<tmp_str;
		break;
	case 'e':
		sprintf(tmp_str, "%*.*le", Width, Prec, x);
		gout<<tmp_str;
	      }*/
return *this;
}
gOutput &gGCLStatus::operator<<(const char *x)
{/*sprintf(tmp_str, "%s", x);gout<<tmp_str;*/return *this;}
gOutput &gGCLStatus::operator<<(const void *x)
{/*sprintf(tmp_str, "%p", x);gout<<tmp_str;*/return *this;}

gGCLStatus _gstatus;
gStatus &gstatus=_gstatus;

// ***************************** gGclSignal Stuff
void gGCLStatusHandler(int)
{
	_gstatus.sig = true;
// This is here because some systems (Solaris) reset the signal handler to
// default when using signal().
	signal(SIGINT, gGCLStatusHandler);
}

