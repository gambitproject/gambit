/*
The files argcargv.c, winio.c and wmhandlr.c are all needed to simulate a
console under Windows for the GCL.
 $Id$

ARGCARGV.C -- WinMain->main startup for WINIO library
Changed considerably since the version in MSJ (May 1991)

from "Undocumented Windows"  (Addison-Wesley, 1992)
by Andrew Schulman, Dave Maxey and Matt Pietrek.

Copyright (c) Dave Maxey and Andrew Schulman 1991-1992

MSC/C++ 7.0 has QuickWin library.  It's not adequate for our
purposes, but its main() gets in the way of ours.  Need to
link with ARGCARGV.OBJ; can't put ARGCARGV.OBJ in a library

HACKED FOR WIN32
Andrew Schulman
Microsoft Systems Journal, April 1993
*/

#include "windows.h"
#include <stdlib.h>
#include <string.h>
#include "winio.h"

#define MAIN_BUFFER 32760

#if defined(__BORLANDC__) || defined(__SC__)
	// Borland and Symantec followed (incorrect) doc in SDK Guide, p. 14-3
	#define argc _argc
	#define argv _argv
	// #define argc _C0argc
	// #define argv _C0argv
	
	extern int _argc;
	extern char **_argv;
	// extern int _C0argc;
	// extern char **_C0argv;
#elif defined(__WATCOMC__)
	// for now!
	int argc = 0;
	char **argv = 0;
#else
	// Microsoft C code per MSJ, May 1991, pp. 135-6
	#define argc __argc
	#define argv __argv

	extern int __argc;
	extern char **__argv;
#endif

#if defined(__SC__) && !defined(_WIN32)
// Saw this in Symantec's source and added it - not used for Win32
// I'm not sure what this is for!!  Andy C. 6/17/95
short _acrtused_winc = 1234;  // Causes linker to pull in automatically for SC++
#endif

// weird! couldn't find environ
// oh well, nobody ever uses it!
#if ( defined(_MSC_VER) && (_MSC_VER >= 700) ) || defined(__SC__) || defined(__BORLANDC__)
extern int main(int argc, char **argv);
#else
extern int main(int argc, char **argv, char **envp);
#endif

void getexefilename(HANDLE hInst, char *strName);

HANDLE __hInst;
HANDLE __hPrevInst;
LPSTR __lpCmdLine;
int __nCmdShow;
HWND __hMainWnd;
UINT __hAppTimer;
char __szModule[9] = {0};

int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow)
		{
		int ret;

		__hInst = hInstance;
    __hPrevInst = hPrevInstance;
    __lpCmdLine = lpCmdLine;
    __nCmdShow = nCmdShow;
    
    getexefilename(__hInst, __szModule);
    
    winio_about(__szModule);    // default; can override
    
    if (! winio_init())
        {
        winio_warn(FALSE, __szModule, "Could not initialize");
        return 1;
        }
    
    if (__hMainWnd = winio_window((LPSTR) NULL, MAIN_BUFFER,
                WW_HASMENU | WW_EXITALLOWED))
        {
        // App timer to allow multitasking
        __hAppTimer = SetTimer(NULL, 0xABCD, 100, NULL);
    
				winio_setcurrent(__hMainWnd);

#if ( defined(_MSC_VER) && (_MSC_VER >= 700) ) || defined(__SC__) || defined(__BORLANDC__)
				ret = main(argc, argv);
#else
				ret = main(argc, argv, environ);
#endif
				printf("\n.....Program Terminated....\n");
				winio_doexit(__hMainWnd,1);

				if (__hAppTimer)
						KillTimer(NULL, __hAppTimer);
				}
		else
				{
				winio_warn(FALSE, __szModule, "Could not create main window");
        ret = -1;
        }
    
    return ret;
    }


void getexefilename(HANDLE hInst, char *strName)
    {
    char str[128];
    char *p;

    // can use hInst as well as hMod (GetExePtr does the trick)
    GetModuleFileName(hInst, str, 128);
    p = &str[strlen(str) - 1];
    
    for ( ; (p != str) && (*p != '\\'); p--)
        if (*p == '.') *p = 0;
        
    strcpy(strName, *p == '\\' ? ++p : p);
    }
    
