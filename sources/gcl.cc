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

#ifdef __BORLANDC__ // this handler is defined differently windows
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

char errMsg[ 80 ];
sprintf (errMsg,
			"%s (%8g,%8g): %s\n", e->name, e->arg1, e->arg2, whyS [e->type - 1]);
gerr<<errMsg;

if (e->type == SING)
	if (!strcmp(e->name,"log"))
		if(e->arg1 == 0.0) {
e->retval = LN_MINDOUBLE;
return 1;
		}

return 0;
}

int main(int ,char **)
{
	// Set up the error handling functions:
	signal(SIGFPE, (fptr)SigFPEHandler);

	GCLCompiler C;

	C.Parse();

	return 0;
}

