#include "rational.h"
#include "gstring.h"
#include "glist.h"
#include "gsm.h"
#include "ggrstack.h"
#include "gcompile.h"
#include <signal.h>

typedef void (*fptr)(int);

void SigFPEHandler(int a)
{
signal(SIGFPE, (fptr)SigFPEHandler);  //  reinstall signal handler
gerr<<"A floating point error has occured!  The results returned may be invalid\n";
}
#ifdef wx_msw // if running under MsWin, different entry point (wxgclio.cc)
void gcl_main(void)
#else
main()                                               
#endif
{
	// Set up the error handling functions:
	signal(SIGFPE, (fptr)SigFPEHandler);

	GCLCompiler C;

	C.Parse();
}

