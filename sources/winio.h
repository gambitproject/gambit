/*
The files argcargv.c, winio.c and wmhandlr.c are all needed to simulate a
console under Windows for the GCL.
 $Id$
WINIO.H
Stdio (e.g. printf) functionality for Windows - definition

from "Undocumented Windows" (Addison-Wesley, 1992)
by Andrew Schulman, Dave Maxey and Matt Pietrek.

Copyright (c) Dave Maxey and Andrew Schulman 1991-1992. All rights reserved.
*/

#ifndef WINIO_H
#define WINIO_H 1


#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
/* ==== STDIO.H for Windows ==== */
#include <stdarg.h>
#ifdef __BORLANDC__
/* Borland C++ STDIO.H doesn't do gets, etc. #if defined(_Windows) */
#undef _Windows
#include <stdio.h>
#define _Windows
#else
#include <stdio.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 700)
#define main	winio_main
#endif

#ifndef MK_FP
#define MK_FP(s,o) ((void far *) (((DWORD) (s) << 16) | (o)))
#endif

#undef putchar
#define putchar winio_putchar
#undef getchar
#define getchar winio_getchar
// BOOL kbhit(void);

/* these were missing from original MSJ version, which assumes inclusion
   of WINDOWS.H as well as WINIO.H.	 #ifndef PASCAL is a semi-unreliable
   way to check for inclusion of any vendor's WINDOWS.H */
#ifndef PASCAL
typedef unsigned HANDLE;
typedef unsigned char BYTE;
typedef unsigned WORD;
typedef unsigned BOOL;
typedef unsigned long DWORD;
typedef unsigned HWND;
typedef long LONG;
#define PASCAL pascal
#define FAR far
#endif

/* winio_init() must be called before any of the above listed
functions to introduce the app to WINIO. This function should only be
called from WinMain before main() is invoked */
int winio_init(void);

/* Signals WINIO that the task is terminating. Makes all windows for the 
task inactive, and allows the user to view and play with
them until double-clicking on the system menu of each to close. Will
only be called from WinMain() upon return from main() */
void winio_end(void);

/* closes a window and frees up buffers */
void winio_close(HWND hWnd);

/* closes all windows for the app and frees up all buffers. Will usually
only be called by winio_app_exit() which itself is registered via atexit() */
void winio_closeall(void);

#define		WW_HASMENU			0x0001 // new window has standard menu
#define		WW_EXITALLOWED		0x0002 // ignored unless WW_HASMENU
#define		WW_STAYSONTOP		0x0004 // window will always overlay parent
#define		WW_OUTPUTONLY		0x0008 // window will not take input

/* Creates a new window. strTitle is the initial title. The bufsize
parameter if 0 means default (16k). wFlags are one or more of the
above flags ORed together */
HWND winio_window(LPSTR strTitle, UINT wBufSize, UINT wFlags);

/* Tells an app how many windows are still open */
int winio_openwindows(void);

/* Sets the current window. This is the window that will be the target
for STDIO calls. Returns the previous current. */
HWND winio_setcurrent(HWND);

/* Resizes the output buffer for the specified window. If the BOOL
parameter is FALSE, the buffer cannot be shrunk to smaller than the amount
currently in use. If the BOOL parameter is TRUE, the buffer will be
cleared, and the new size can be anything over 4K. The return value is
the new actual buffer size. */
UINT winio_setbufsize(HWND, UINT, BOOL);

/* Sets the default window size (height in high order WORD, width
in low order, both in chars) to have an effect in future
winio_window calls. Returns previous in effect. */
DWORD winio_defwindowsize(DWORD);

/* Return the window's output buffer size */
UINT winio_bufsize(HWND);

/* Sets the text for the Help About... box which is only present
on the main window - truncated to a maximum of 512 chars. */
void winio_about(char *);

/* Sets the text for the specified window. This is here only for consistency
since it is a direct cover on SetWindowText().... */
void winio_settitle(HWND, char *);

/* May be SYSTEM_FIXED_FONT (default), ANSI_FIXED_FONT, or OEM_FIXED_FONT */
UINT winio_setfont(HWND, UINT);

typedef void (* LINEHANDLER)(HWND, LPSTR, int);

/* Sets up a handler for a user double-click. May be cancelled using
	winio_linefn(hwnd, NULL). Returns previous handler...  */
LINEHANDLER winio_setlinefn(HWND, LINEHANDLER);

/* To turn automatic updating of window off and on */
BOOL winio_setpaint(HWND, BOOL);

/* Goes to top left of window buffer */
void winio_home(HWND);

/* Captures the cursor and turns it into an hourglass. Calling this
function repeatedly increments a counter. */
void winio_setbusy(void);

/* Decrements the counter incremented above. Returns the cursor to the
arrow when the counter reaches 0. */
void winio_resetbusy(void);

/* Replacement for standard I/O library function putchar() */
int winio_putchar(int c);

/* Replacement for standard I/O library function getchar() */
int winio_getchar(void);

/* A getch() - like function to get a character from the keyboard */
int chInput(void);

/* A puts() function with user-specified string length */
void addchars(char *, unsigned);

/* To change the behavior of getchar(). Note. This is a task global,
because console input is only ever coming from one Window at a time */
BOOL winio_setecho(HWND, BOOL);

/* clear out the contents of the buffer and start over fresh */
void winio_clear(HWND);

/* Returns the current WINIO window handle */
HWND winio_current(void);

/* ==== User definable exit routine ==== */

typedef void (* DESTROY_FUNC)(HWND);

/* Optional notification function; without it, there is no way for your
application to know if the user has double-clicked the system menu box */
void winio_onclose(HWND, DESTROY_FUNC);

/* ==== User definable paint involvement routines ==== */

typedef struct {
	POINT dimChar;			// dimensions of a character cell
	POINT posCurr;			// curr pos from top of buffer in chars.
	RECT rectView;			// frame in view from top of buffer in chars.
	long cDiscarded;		// lines discarded from buffer so far
	}	WINIOINFO,
		* PWINIOINFO,
		FAR * LPWINIOINFO;


typedef BOOL (* PAINT_FUNC)(HWND, HDC, PAINTSTRUCT *, PWINIOINFO);

/* Optional paint daemon installation functions; with these, it is
possible to install routines that can prepare the DC on entry, or
redraw some graphics on exit, or whatever. */
PAINT_FUNC winio_onpaintentry(HWND, PAINT_FUNC);
PAINT_FUNC winio_onpaintexit(HWND, PAINT_FUNC);

/* To obtain WINIOINFO at non-WM_PAINT times */
void winio_getinfo(HWND, PWINIOINFO);


/* ==== Utility functions built on message box ==== */

BOOL winio_warn(BOOL, char *, const char *, ...);

/* fail() is a format/var.args. dialog box and exit routine */
void fail(const char *strFmt, ...);

/* ==== STDIO extension function ==== */

char *ungets(char *);


typedef void (* MENU_FUNC)(HWND, int);


/* The following functions allow submenus to be added to a window's
existing menu, and for handler functions to be registered against new
options. winio_hmenumain returns the handle to the window's main
menu, and winio_hmenufile returns the handle to the window's file
popup menu. The application can then use AppendMenu/InsertMenu to add
additional menus/choices. winio_setmenufunc allows a handler function
to be registered for a menu option id. NOTE that if a menu option id
is not registered with a handler in this way, the app will never know
that the option has been selected... NOTE ALSO that application menu
option IDs must be allocated from 1 thru 32 (giving a total of 32
possible additional menu options per window). */

HMENU winio_hmenumain(HWND);

HMENU winio_hmenufile(HWND);

HMENU winio_hmenuhelp(HWND);

MENU_FUNC winio_setmenufunc(HWND, int, MENU_FUNC);


// The following are defined in argcargv.c 
extern HANDLE __hInst;
extern HANDLE __hPrevInst;
extern LPSTR __lpCmdLine;
extern int __nCmdShow;
extern HWND __hMainWnd;
extern UINT __hAppTimer;
extern char __szModule[];

#ifdef __cplusplus
}
#endif

#endif
