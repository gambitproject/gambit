// File: gcl.cc -- top level of the Gambit Command Line
// $Id$
#include "rational.h"
#include "gstring.h"
#include "glist.h"
#include "gsm.h"
#include "ggrstack.h"
#include "gcompile.h"
#include <signal.h>
#include <values.h>
#include <math.h>

typedef void (*fptr)(int);

void SigFPEHandler(int a)
{
if (a==SIGFPE)
	gerr<<"A floating point error has occured!  The results returned may be invalid\n";
signal(SIGFPE, (fptr)SigFPEHandler);  //  reinstall signal handler
}

#define MATH_CONTINUE	0
#define	MATH_IGNORE		1
#define	MATH_QUIT			2
#ifdef __BORLANDC__ // this handler is defined differently windows
extern "C" int winio_ari(const char *msg);
extern "C" void winio_closeall(void);
int _RTLENTRY _matherr (struct exception *e)
#else
int matherr(struct exception *e)
#endif
{
char *whyS [] =
{
		"argument domain error",
		"argument singularity ",
		"overflow range error ",
		"underflow range error",
		"total loss of significance",
		"partial loss of significance"
};
static option=MATH_CONTINUE;
char errMsg[ 80 ];
if (option!=MATH_IGNORE)
{
sprintf (errMsg,
			"%s (%8g,%8g): %s\n", e->name, e->arg1, e->arg2, whyS [e->type - 1]);
gerr<<errMsg;
// define this to pop up a dialog for math errors under windows.  allows to quit.
#if defined(__WIN32__) && defined(MATH_ERROR_DIALOG)
option=winio_ari(errMsg);
if (option==MATH_QUIT) { winio_closeall(); exit(1);}
#endif
}

if (e->type == SING)
	if (!strcmp(e->name,"log"))
		if(e->arg1 == 0.0) {
e->retval = LN_MINDOUBLE;
return 1;
		}

return 1;	// we did not really fix anything, but want no more warnings
}

int main(int ,char **)
{
	// Set up the error handling functions:
	signal(SIGFPE, (fptr)SigFPEHandler);

	GCLCompiler C;

	C.Parse();

	return 0;
}

