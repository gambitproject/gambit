/*
The files argcargv.c, winio.c and wmhandlr.c are all needed to simulate a
console under Windows for the GCL.
 @(#)argcargv.c	1.1 6/12/96

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


void getargs(char *m_lpCmdLine,int *argc,char ***argv) // taken from the wx_win code
{
	// Split command line into tokens, as in usual main(argc, argv)
	char **command = (char **)malloc(sizeof(char *)*50);
	int count = 0;
	char *buf = (char *)malloc(sizeof(char)*(strlen(m_lpCmdLine) + 1));

	// Hangs around until end of app. in case
	// user carries pointers to the tokens

	/* Model independent strcpy */
	{
		int i;
		for (i = 0; (buf[i] = m_lpCmdLine[i]) != 0; i++)
		{
			/* loop */;
		}
	}

	// Get application name
	{
		char name[200];
		GetModuleFileName(__hInst, name, 199);

	// Is it only 16-bit Borland that already copies the program name
	// to the first command index?
// #if ! (defined(__BORLANDC__) && !defined(WIN32))
		command[count++] = strdup(name);
// #endif
	}

	/* Break-up string */
	{
		char *token;
		const char *IFS = " \t\r\n";
		if ((token = strtok(buf, IFS)) != NULL) {
			do {
				if (*token != '\0' && strchr(IFS, *token) == NULL)
					command[count++] = token;
			} while ((token = strtok(NULL, IFS)) != NULL);
		}
	}
	command[count] = NULL; /* argv[] is NULL terminated list! */
	*argv=command;
	*argc=count;
}

int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow)
		{
		int ret;
		char **argv;
		int argc;
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
				getargs(lpCmdLine,&argc,&argv);
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

