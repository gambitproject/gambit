/*
The files argcargv.c, winio.c and wmhandlr.c are all needed to simulate a
console under Windows for the GCL.
 $Id$

	WINIO.C -- Stdio (with extensions) for Windows
	
	Copyright (c) Dave Maxey & Andrew Schulman, 1990-1992.
	All Rights Reserved.
	
	Contact:  Andrew Schulman (CompuServe 76320,302)
	
	From Chapter 4 of "Undocumented Windows" (Addison-Wesley 1992)
	by Andrew Schulman, Dave Maxey and Matt Pietrek
		
	HACKED FOR WIN32S -- see #ifdef WIN32
	(There may still be some LOWORD, HIWORD that are wrong.)
	Microsoft Systems Journal, April 1993
*/

#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <conio.h>
#include <commdlg.h>

#include "wmhandlr.h"
#include "winio.h"

#ifndef __WATCOMC__
#if defined(WIN32) || defined(_WIN32)
#define far
/******************************************************************************
#define _fmemcpy(x,y,z)		memcpy((x),(y),(z))
#define _fmemset(x,y,z)		memset((x),(y),(z))
#define _fstrcat(x,y)		strcat((x),(y))
******************************************************************************/
#endif
#endif

#define			MAX_X					127
#define			TABSIZE					8
#define			TYPE_AHEAD				256
#define			WINIO_DEFAULT_BUFFER	8192
#define			WINIO_MIN_BUFFER		4096
#define			CARET_WIDTH				2
#define			MAXWINDOWS				128
#define			MAXAPPS					16
#define			DEFAULTWIDTH			80
#define			DEFAULTHEIGHT			25
#define			MAX_ABOUT				512

#define			ID_SAVE					33
#define			ID_EXIT					34
#define			ID_ABOUT				35
#define			MAX_MENU_ID				ID_ABOUT

#define			WINIO_ABOUT_TEXT		"Gambit -- Command Line Interface, Version 0.92\n\nDeveloped by Richard D. McKelvey (rdm@hss.caltech.edu)\nMain Programmer:  Theodore Turocy (magyar@hss.caltech.edu)\nFront End: Eugene Grayver (egrayver@hss.caltech.edu)\nCalifornia Institute of Technology, 1995.\nFunding provided by the National Science Foundation"

/* The following structure contains winio state about a window, and a
	pointer to it is stored as instance data within Windows. It can be
	retrieved using GetWindowLong(4) */
typedef struct {
	UINT			bufsize;
	UINT			bufused;
	UINT			curr_font;                         
	BOOL			tCaret, tCR;
	long			cLinesDiscarded;
	int				cxChar, cyChar, cxWidth, cyHeight;
	int				xWinWidth, yWinHeight, xCurrPos;
	int				xLeftOfWin, yTopOfWin, yCurrLine;
	UINT			pchKbIn, pchKbOut;
	LPSTR			fpBuffer, fpTopOfWin, fpCurrLine, fpCurr, fpSOI; 
	LPSTR			fpKeyboard;
	HANDLE			hBuffer, hKeyboard;
	HMENU			hMainMenu, hFileMenu, hHelpMenu;
	LINEHANDLER		pfnLine;
	BOOL			tPaint;
	BOOL			tEcho;
	PAINT_FUNC		paintentry_func, paintexit_func;
	DESTROY_FUNC	destroy_func;
	MENU_FUNC		menu_func[MAX_MENU_ID];
	int				cScrollLR[SB_ENDSCROLL + 1];
	int				cScrollUD[SB_ENDSCROLL + 1];
	char			strSaveFile[80];
	} WINIO_WNDDATA, FAR * WINIO_HWND;

/* PROTOTYPES in alphabetic order */

void			adjust_caret(WINIO_HWND);
void			append2buffer(char *, unsigned);
void			compute_repaint(void);
void			get_info(WINIO_HWND, PWINIOINFO);
int				initialize_buffers(WINIO_HWND, UINT, UINT);
int				initialize_class(HANDLE);
void			make_avail(unsigned);
void			new_rect(void);
HWND			new_window(LPCREATEPARAMS, LPSTR, UINT);
WMTAB			new_wmtab(void);
WINIO_HWND		new_wnddata(void);
char far *		nextline(char far *);
char far *		prevline(char far *);
void			set_font(HWND, WINIO_HWND);
void			winio_dosave(HWND hwnd, int nID);
void			winio_doexit(HWND hwnd, int nID);
void			winio_doabout(HWND hwnd, int wID);
long			winio_wmpaint(HWND, UINT, WPARAM, LPARAM);
long			winio_wmsize(HWND, UINT, WPARAM, LPARAM);
long			winio_wmcommand(HWND, UINT, WPARAM, LPARAM);
long			winio_wmdestroy(HWND, UINT, WPARAM, LPARAM);
long			winio_wmchar(HWND, UINT, WPARAM, LPARAM);
long			winio_wmkeydown(HWND, UINT, WPARAM, LPARAM);
long			winio_wmhscroll(HWND, UINT, WPARAM, LPARAM);
long			winio_wmvscroll(HWND, UINT, WPARAM, LPARAM);
long			winio_wmsetfocus(HWND, UINT, WPARAM, LPARAM);
long			winio_wmldblclk(HWND, UINT, WPARAM, LPARAM);
long			winio_wmkillfocus(HWND, UINT, WPARAM, LPARAM);

// this doesn't get declared in stdio.h if _WINDOWS is defined
// although it is in the Windows libraries!
int				vsprintf(char *, const char *, va_list);

#define			winio_caret_visible(p) \
				((p->yCurrLine <= (p->yTopOfWin + p->yWinHeight)) && \
				(p->xCurrPos <= (p->xLeftOfWin + p->xWinWidth)) && \
				(p->xCurrPos >= p->xLeftOfWin))

// For scrolling procedures
#define			USE_PARAM				10000
#define			DO_NOTHING				10001


typedef struct {
	int hSB, vSB;
	} recVKtoSB;


char			winio_deftitle[128] = "WINIO";
char			winio_error[15] = "WINIO ERROR";
char			*winio_wclass = "winio_wcmain";
char			*winio_icon = "winio_icon";
DWORD			winio_wstyle = WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL;
int				gcxMax, gcyMax, gcxScroll, gcyScroll,
				gcxWidth, gcyHeight, gcyCap, gcyMenu;
int				gcxDefFont = 0, gcyDefFont = 0;
int				cWindows = 0;
HWND			hCurrWnd = NULL;
HWND			hMaster = NULL;
WINIO_HWND		whwndCurr = NULL;
WINIO_HWND		whWndNew;
RECT			grectCurr;
int				gMenuID = ID_EXIT;
int				nBusyCount = 0;
HCURSOR			hCrsSave;
int				cxDefSize = 0, cyDefSize = 0;
char			strAbout[MAX_ABOUT + 1] = {0};
int				do_not_echo_next = 0;

/* This table defines, by scroll message, what increment to try */
/* and scroll horizontally. PGUP and PGDN entries are updated	*/
/* in the winio_wmsize function. Used as an initializer for		*/
/* new windows													*/
int				gcScrollLR[SB_ENDSCROLL + 1] =
//UP  DOWN PGUP		PGDN	POS		   TRACK	  TOP	  BOT	 ENDSCROLL
{ -1, +1,  -1,		+1,		USE_PARAM, USE_PARAM, -MAX_X, MAX_X, DO_NOTHING};
				
/* This table defines, by scroll message, what increment to try */
/* and scroll horizontally. PGUP and PGDN entries are updated	*/
/* in the winio_wmsize function, and the TOP & BOTTOM entries	*/
/* are updated by addchar function. Used as an initializer for	*/
/* new windows													*/
int				gcScrollUD[SB_ENDSCROLL + 1] =
//UP  DOWN PGUP		PGDN	POS		   TRACK	  TOP	  BOT	 ENDSCROLL
{ -1, +1,  -1,		+1,		USE_PARAM, USE_PARAM, -1,	  +1,	 DO_NOTHING};
				
/* This table associates horizontal and vertical scroll			*/
/* messages that should be generated by the arrow and page keys */
recVKtoSB		VKtoSB[VK_DOWN - VK_PRIOR + 1] =
//					VK_PRIOR					VK_NEXT
				{	{ DO_NOTHING, SB_PAGEUP },	{ DO_NOTHING, SB_PAGEDOWN },
//					VK_END						VK_HOME
					{ SB_TOP, SB_BOTTOM },		{ SB_TOP, SB_TOP },
//					VK_LEFT						VK_UP
					{ SB_LINEUP, DO_NOTHING },	{ DO_NOTHING, SB_LINEUP },
//					VK_RIGHT					VK_DOWN
					{ SB_LINEDOWN, DO_NOTHING },{ DO_NOTHING, SB_LINEDOWN } };
				

				
				
void abort_exit(void)
	{
#ifdef DEBUG
	winio_warn(FALSE, "WINDOS Kernel",
		"STDIO to/from non-existent window!\nProgram terminating");
#endif
	exit(0);
	}
				
				
/* ===================================================================	*/
/* the interface functions themselves.....								*/
/* ===================================================================	*/

int fscanf(FILE * __stream, const char * __format, ...)
{
va_list ap;
static char buffer[512];
va_start(ap, __format);
if (__stream==stdin)
{
	gets(buffer);
	return vsscanf(buffer,__format,ap) ;
}
else
	return vfscanf(__stream,__format,ap);
}

int fprintf(FILE _FAR *__stream, const char _FAR *fmt, ...)
{
va_list ap;
va_start(ap, fmt);
if (__stream==stdout || __stream==stderr)
	return vprintf(fmt,ap);
else
	return vfprintf(__stream,fmt,ap);
}

int gfgetc(FILE *__stream)
{
if (__stream==stdin)
	return winio_getchar();
else
	return fgetc(__stream);
}

int gungetc(int c,FILE *__stream)
{
if (__stream==stdin)
{
	char tmp[2];tmp[0]=c;tmp[1]='\0';
  do_not_echo_next=1;
	ungets(tmp);
	return c;
}
else
	return ungetc(c,__stream);
}

char * gets(char * strTmp)
	{
	char * str = strTmp;
	int c;

	if (! hCurrWnd)
		abort_exit();

	/* mark beginning of line to limit backspace */
	whwndCurr->fpSOI = whwndCurr->fpCurr;
	do {
		if ((c = winio_getchar()) == '\n')
			c = '\0';
		switch (c)
			{
			case '\b' :		if (str > strTmp) str--; break;
			case 0x1b :		str = strTmp; break;
			case EOF :		if (hCurrWnd)
								whwndCurr->fpSOI = NULL;
							return NULL;
			default :		*str = (BYTE) c; str++;
			}
		} while (c);
	whwndCurr->fpSOI = NULL;
	return strTmp;
	}

char * ungets(char * strTmp)
	{
	char * s;

	if (! hCurrWnd) abort_exit();

	for (s = strTmp; *s; s++)
		winio_wmchar(hCurrWnd, WM_CHAR, (UINT) *s, 0L);

	return strTmp;
	}

int printf(const char * strFmt, ...)
	{
	va_list marker;

	if (! whwndCurr) abort_exit();
	
	va_start(marker, strFmt);
	return vprintf(strFmt, marker);
	}

int vprintf(const char * strFmt, va_list marker)
	{
	static char s[4096] = {0};
	int len;

	if (! whwndCurr) abort_exit();
	
	len = vsprintf(s, strFmt, marker);
	addchars(s,len);
	return len;
	}

int winio_getchar(void)
	{
	int ch;

	if ((ch = getch()) == -1)
		return -1;

	if (whwndCurr->tEcho && !do_not_echo_next)
		winio_putchar(ch);
	if (do_not_echo_next) do_not_echo_next=0;
	return ch;
	}

int kbhit(void)
	{
	return (whwndCurr ?
		whwndCurr->pchKbIn == whwndCurr->pchKbOut : 0);
	}

int winio_putchar(int c)
	{
	if (! whwndCurr) abort_exit();
	
	addchars((char *) &c, 1);
	return c;
	}

int puts(const char * str)
	{
	static char nl[] = "\n";
	static char s[1024];

	if (! whwndCurr) abort_exit();
	lstrcpy(s, (LPSTR) str);
	lstrcat(s, nl);
	addchars((char *) s, lstrlen(s));

	return 0;
	}

/* ---------------------------------------------------------------	*/
/* pops up a message window											*/
/* ---------------------------------------------------------------	*/
BOOL winio_warn(BOOL confirm, char * strCaption, const char *strFmt, ...)
	{
	char * str;
	BOOL tWarn;
	va_list marker;

	str = malloc(4096);
	va_start(marker, strFmt);
	vsprintf(str, strFmt, marker);
	va_end(marker);
	
	tWarn = (MessageBox(NULL, str, strCaption, 
		confirm? MB_OKCANCEL : MB_OK) == IDOK);

	free(str);

	return tWarn;
	}

/* abort/retry/ignore */
int winio_ari(const char *msg)
{
int opt=MessageBox(NULL,msg,"Numerical Error",MB_TASKMODAL|MB_ABORTRETRYIGNORE|MB_ICONHAND);
switch(opt)
{
case IDABORT: return 2;
case IDIGNORE: return 1;
case IDRETRY: return 0;
}
return 0;
}

/* ---------------------------------------------------------------	*/
/* pops up a message window											*/
/* ---------------------------------------------------------------	*/
void fail(const char *strFmt, ...)
	{
	char * str;
	static char strCaption[128];
	va_list marker;

	if (cWindows)
		GetWindowText(winio_current(), strCaption, 128);
	str = malloc(4096);
	va_start(marker, strFmt);
	vsprintf(str, strFmt, marker);
	va_end(marker);
	
	winio_closeall();
	
	MessageBox(NULL, str, cWindows ? strCaption : __szModule, MB_OK);

	free(str);
	
	exit(1);
	}



/* ---------------------------------------------------------------	*/
/* The application must call this function before using any of the	*/
/* covered stdio type calls. It will normally be called from within */
/* WinMain() before main(), in ARGCARGV.C							*/
/* ---------------------------------------------------------------	*/
BOOL winio_init(void)
	{
		if (! __hPrevInst)
			if (! initialize_class(__hInst))
			{
				winio_warn(FALSE, winio_error, "Could not create class");
				return FALSE;
			}

	gcxMax = GetSystemMetrics(SM_CXSCREEN);
	gcyMax = GetSystemMetrics(SM_CYSCREEN);
	gcxWidth = 3 * (gcxMax / 4);
	gcyHeight = 3 * (gcyMax / 4);
	gcyCap = GetSystemMetrics(SM_CYCAPTION);
	gcyMenu = GetSystemMetrics(SM_CYMENU);
	gcxScroll = GetSystemMetrics(SM_CXVSCROLL);
	gcyScroll = GetSystemMetrics(SM_CYHSCROLL);

#if defined(WIN32) || defined(_WIN32)
	grectCurr.left = gcxMax >> 3;
	grectCurr.top = gcyMax >> 3;
#else
	if (__hPrevInst)
		{
		GetInstanceData(__hPrevInst, (NPSTR) &grectCurr, sizeof(RECT));
		grectCurr.left += gcxScroll;
		if (grectCurr.left >= gcxMax) grectCurr.left = gcxScroll;
		grectCurr.top += gcyScroll;
		if (grectCurr.top >= gcyMax) grectCurr.top = gcyScroll;
		}
	else
		{
		grectCurr.left = gcxMax >> 3;
		grectCurr.top = gcyMax >> 3;
		}
#endif		

	grectCurr.right = min(gcxMax - grectCurr.left, 3 * (gcxMax >> 2));
	grectCurr.bottom = min(gcyMax - grectCurr.top, 3 * (gcyMax >> 2));
	
	/* set up default title bar */
	strcpy(winio_deftitle, __szModule);
//	GetModuleFileName(__hInst, winio_deftitle, 128); // Gets unwieldy!
	if (__lpCmdLine && *__lpCmdLine)
		{
#if defined(WIN32) || defined(_WIN32)
		strcat(winio_deftitle, " ");
		strcat(winio_deftitle, __lpCmdLine);
#else
		_fstrcat(winio_deftitle, " ");
		_fstrcat(winio_deftitle, __lpCmdLine);
#endif
		}

	atexit((void (*)()) winio_end);
	
	return TRUE;
	}


/* ---------------------------------------------------------------	*/
/* Create a new window. This will be called once from ARGCARGV.C,	*/
/* after a call to winio_init(), to create the main application		*/
/* window, and thereafter by the app for 2nd and more windows.		*/
/* ---------------------------------------------------------------	*/
HWND winio_window(LPSTR strTitle, UINT wBufSize, UINT wFlags)
	{
	HWND hWnd;
	CREATEPARAMS cp;
	
	new_rect(); /* Get a new starting point for the window */
	
	if ((cp.lpData = (LPVOID) new_wnddata()) == NULL)
		return NULL;
	
	if ((cp.wmTab = new_wmtab()) == NULL)
		return NULL;
	
	if (! initialize_buffers((WINIO_HWND) cp.lpData, wBufSize, wFlags))
		return NULL;

	if ((hWnd = new_window((LPCREATEPARAMS) &cp,
			strTitle ? strTitle : winio_deftitle,
			wFlags)) == NULL)
		return NULL;
	
	cWindows++;

	wmhandler_yield();
	
	return hWnd;
	}


/* ---------------------------------------------------------------	*/
/* Calculate a nice new starting position for a new window!			*/
/* ---------------------------------------------------------------	*/
void new_rect()
	{
	if (hMaster)
		{
		GetWindowRect(hMaster, &grectCurr);
	
	/* new windows spiral out... ! */
		grectCurr.left =
			min(gcxMax,
				max(0, grectCurr.left +
				  (gcyCap *
					((1 + (cWindows >> 2)) * (cWindows & 0x0001 ? 2 : -2)))));
		grectCurr.top =
			min(gcyMax,
				max(0, grectCurr.top +
				  (gcyCap *
					((1 + (cWindows >> 2)) * (cWindows & 0x0002 ? 2 : -2)))));
		}
	// right & bottom fields are actually width & height
	grectCurr.right =
		cxDefSize
			? (cxDefSize * gcxDefFont) + gcxScroll
			: min(gcxMax, grectCurr.left + gcxWidth) - grectCurr.left;
	grectCurr.bottom =
		cyDefSize
			? ((cyDefSize + 1) * gcyDefFont) + gcyScroll + gcyCap + gcyMenu
			: min(gcyMax, grectCurr.top + gcyHeight) - grectCurr.top;
	
	gcxWidth = gcxMax >> 1;
	gcyHeight = gcyMax >> 1;
	}



/* ---------------------------------------------------------------	*/
/* Clear the contents of the buffer.								*/
/* ---------------------------------------------------------------	*/
WINIO_HWND new_wnddata(void)
	{
	WINIO_HWND whWnd;
	
	if ((whWnd = (WINIO_HWND) GlobalLock(
				GlobalAlloc(GMEM_MOVEABLE, sizeof(WINIO_WNDDATA)))) == NULL)
		return NULL;
	
	whWnd->cLinesDiscarded = 0;
	whWnd->curr_font = SYSTEM_FIXED_FONT;
	whWnd->cxChar = 10; // To avoid div by zero at 1st WM_SIZE
	whWnd->cyChar = 16; // To avoid div by zero at 1st WM_SIZE
	whWnd->tCaret = FALSE;
	whWnd->tEcho = TRUE;
	whWnd->tCR = FALSE;
	whWnd->tPaint = TRUE;
	whWnd->destroy_func = NULL;
	whWnd->paintentry_func = NULL;
	whWnd->paintexit_func = NULL;
	whWnd->pfnLine = NULL;
	whWnd->hMainMenu = NULL;
	whWnd->hFileMenu = NULL;
	lstrcpy(whWnd->strSaveFile, __szModule);
	lstrcat(whWnd->strSaveFile, ".LOG");
#if defined(WIN32) || defined(_WIN32)
	memset(whWnd->menu_func, 0, sizeof(whWnd->menu_func));
	memcpy(whWnd->cScrollUD, gcScrollUD, sizeof(gcScrollUD));
	memcpy(whWnd->cScrollLR, gcScrollLR, sizeof(gcScrollLR));
#else
	_fmemset(whWnd->menu_func, 0, sizeof(whWnd->menu_func));
	_fmemcpy(whWnd->cScrollUD, gcScrollUD, sizeof(gcScrollUD));
	_fmemcpy(whWnd->cScrollLR, gcScrollLR, sizeof(gcScrollLR));
#endif	
	return whWnd;
	}


/* ---------------------------------------------------------------	*/
/* Clear the contents of the buffer.								*/
/* ---------------------------------------------------------------	*/
void winio_clear(HWND hwnd)
	{
	WINIO_HWND whWnd;
	
	if (! IsWindow(hwnd))
		return;
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);

#if defined(WIN32) || defined(_WIN32)
	memset(whWnd->fpBuffer,0,whWnd->bufsize - 1);
#else
	_fmemset(whWnd->fpBuffer,0,whWnd->bufsize - 1);
#endif
	whWnd->fpCurr = whWnd->fpCurrLine =
		whWnd->fpTopOfWin = whWnd->fpBuffer;
	*(whWnd->fpBuffer) = '\0';
	whWnd->xCurrPos = 0;
	whWnd->yCurrLine = 0;
	whWnd->yTopOfWin = 0;
	whWnd->xLeftOfWin = 0;
	whWnd->bufused = 0;
	whWnd->cLinesDiscarded = 0;
	whWnd->pchKbIn = whWnd->pchKbOut = 0;

	if (whWnd->tPaint)
		InvalidateRect(hwnd, NULL, TRUE);
	SetScrollRange(hwnd, SB_VERT, 1, whWnd->yCurrLine + 1, FALSE);
	SetScrollPos(hwnd, SB_VERT, whWnd->yTopOfWin + 1, TRUE);
	}

/* ---------------------------------------------------------------	*/
/* Set the current window title										*/
/* ---------------------------------------------------------------	*/
void winio_settitle(HWND hWnd, char * strTitle)
	{
	SetWindowText(hWnd, strTitle);
	}

/* ---------------------------------------------------------------	*/
/* Return the window's output buffer size							*/
/* ---------------------------------------------------------------	*/
UINT winio_bufsize(HWND hwnd)
	{
	return IsWindow(hwnd) ? ((WINIO_HWND) GetWindowLong(hwnd, 4))->bufsize
							: 0;
	}

/* ---------------------------------------------------------------	*/
/* Return the window handle of this app's window with output focus	*/
/* ---------------------------------------------------------------	*/
HWND winio_current(void)
	{
	return hCurrWnd;
	}

/* ---------------------------------------------------------------	*/
/* Resizes the output buffer for the specified window. If the BOOL	*/
/* parameter is FALSE, the buffer cannot be shrunk to smaller than	*/
/* the amount currently in use. If the BOOL parameter is TRUE, the	*/
/* buffer will be cleared, and the new size can be anything over	*/
/* 4K. The return value is the new actual buffer size.				*/
/* ---------------------------------------------------------------	*/
UINT winio_setbufsize(HWND hwnd, UINT wSize, BOOL bClear)
	{
	WINIO_HWND whWnd;
	UINT bufsize;
		
	if (! IsWindow(hwnd))
		return 0;
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	
	bufsize =
		wSize ? max(max(wSize, (UINT) WINIO_MIN_BUFFER),
			(UINT) (bClear ? 0 : whWnd->bufused))
				: (UINT) WINIO_DEFAULT_BUFFER;

	if (! GlobalReAlloc(whWnd->hBuffer, (DWORD) bufsize, GMEM_MOVEABLE))
		return whWnd->bufsize;
		
	if (bClear)
		winio_clear(hwnd);
	
	return whWnd->bufsize = bufsize;
	}

/* ---------------------------------------------------------------	*/
/* Sets the default window width and height in chars for future		*/
/* windows created using winio_window(). Height is in high WORD,	*/
/* width is in low. A 0 in either position indicates 'don't care'.	*/
/* ---------------------------------------------------------------	*/
DWORD winio_defwindowsize(DWORD dwSize)
	{
	DWORD old = (cyDefSize << 16) | cxDefSize;
	
	cyDefSize = (int) ((DWORD) dwSize >> 16);
	cxDefSize = (int) ((DWORD) dwSize & 0x0000ffff);
	return old;
	}


/* ---------------------------------------------------------------	*/
/* This function allocates a new wmhandler table, and initializes	*/
/* it...															*/
/* ---------------------------------------------------------------	*/
WMTAB new_wmtab()
	{
	WMTAB wmTab;

	if ((wmTab = wmhandler_create()) == NULL)
		return NULL;
	
	/* set up our message handlers */
	wmhandler_settab(wmTab, WM_PAINT,				winio_wmpaint);
	wmhandler_settab(wmTab, WM_SIZE,				winio_wmsize);
	wmhandler_settab(wmTab, WM_COMMAND,				winio_wmcommand);
	wmhandler_settab(wmTab, WM_DESTROY,				winio_wmdestroy);
	wmhandler_settab(wmTab, WM_CHAR,				winio_wmchar);
	wmhandler_settab(wmTab, WM_HSCROLL,				winio_wmhscroll);
	wmhandler_settab(wmTab, WM_VSCROLL,				winio_wmvscroll);
	wmhandler_settab(wmTab, WM_SETFOCUS,			winio_wmsetfocus);
	wmhandler_settab(wmTab, WM_KILLFOCUS,			winio_wmkillfocus);
	wmhandler_settab(wmTab, WM_KEYDOWN,				winio_wmkeydown);
	wmhandler_settab(wmTab, WM_LBUTTONDBLCLK,		winio_wmldblclk);

	return wmTab;
	}

/* ---------------------------------------------------------------	*/
/* This function is called by winio_window(). It creates the new	*/
/* window, and attaches the wmhandler table pointer and the			*/
/* pointed to structure.											*/
/* ---------------------------------------------------------------	*/
	
#define MFES	(MF_ENABLED | MF_STRING)

HWND new_window(LPCREATEPARAMS cp, LPSTR strTitle, UINT wFlags)
	{
	HWND hwnd;
	WINIO_HWND whWnd = (WINIO_HWND) (cp->lpData);

	if (wFlags & WW_HASMENU)
	{
		if ((! (whWnd->hFileMenu = CreateMenu())) ||
			(! (whWnd->hMainMenu = CreateMenu())) ||
			(!hMaster && (! (whWnd->hHelpMenu = CreateMenu()))) ||
			(! AppendMenu(whWnd->hFileMenu, MFES,
				ID_SAVE, "&Save Buffer...")) ||
			(! AppendMenu(whWnd->hFileMenu, MF_SEPARATOR, (UINT) NULL, NULL)) ||
			(! AppendMenu(whWnd->hFileMenu, MFES, ID_EXIT, "E&xit")) ||
			(!hMaster && (! AppendMenu(whWnd->hHelpMenu,
				MFES, ID_ABOUT, "&About..."))) ||
			(! AppendMenu(whWnd->hMainMenu, MFES | MF_POPUP, 
				(UINT)whWnd->hFileMenu, "&File")) ||
			(! hMaster && (! AppendMenu(whWnd->hMainMenu, MFES | MF_POPUP,
				(UINT)whWnd->hHelpMenu, "&Help"))))
		{
			return FALSE;
		}
	}
	
	if (! __hInst)
	{
		return NULL;
	}

	hwnd = CreateWindow(
		(LPSTR) winio_wclass,		
		(LPSTR) strTitle,
		winio_wstyle,
		grectCurr.left, grectCurr.top, grectCurr.right, grectCurr.bottom,
		(wFlags & WW_STAYSONTOP) ? hMaster : NULL,
		whWnd->hMainMenu, 
		__hInst,
		(LPSTR) cp);

	if (hwnd == 0)
	{
		return NULL;
	}

	if (! hMaster) 
		winio_setmenufunc(hwnd, ID_ABOUT, winio_doabout);
	winio_setmenufunc(hwnd, ID_SAVE, winio_dosave);
	winio_setmenufunc(hwnd, ID_EXIT, winio_doexit);
	
	set_font(hwnd, whWnd);
	
	if (! gcxDefFont)
		{
		gcxDefFont = whWnd->cxChar;
		gcyDefFont = whWnd->cyChar;
		}
		
	winio_clear(hwnd);
	
	ShowWindow(hwnd, (hMaster) ? SW_SHOWNORMAL : __nCmdShow);
	UpdateWindow(hwnd);

	if (! hMaster)
		{
		winio_wstyle |= WS_POPUP;
		hMaster = hwnd;
		}
	else if (! (wFlags & WW_EXITALLOWED))
		EnableMenuItem(whWnd->hFileMenu, ID_EXIT, MF_DISABLED | MF_GRAYED);
	
	return hwnd;
	}

/* -----------------------------------------------------------------------	*/
/* Allows an app to get info about the current size and position of the		*/
/* window's view on the buffer.												*/
/* -----------------------------------------------------------------------	*/
void winio_getinfo(HWND hwnd, PWINIOINFO pwi)
	{
	if (! IsWindow(hwnd))
		return;
	
	get_info((WINIO_HWND) GetWindowLong(hwnd, 4), pwi);
	}



/* -----------------------------------------------------------------------	*/
/* Initializes Window Class													*/
/* -----------------------------------------------------------------------	*/
int initialize_class(HANDLE hInst)
	{
	WNDCLASS  wc;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS
				/* | CS_BYTEALIGNCLIENT */;
	// wc.lpfnWndProc = (long (FAR PASCAL *)()) wmhandler_wndproc;
	wc.lpfnWndProc = wmhandler_wndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 8;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
	wc.lpszMenuName = NULL;
	wc.lpszClassName = winio_wclass;

	return RegisterClass(&wc);
	}
	
/* -----------------------------------------------------------------------	*/
/* Uses GlobalAlloc() to allocate the display and keyboard buffers			*/
/* -----------------------------------------------------------------------	*/
BOOL initialize_buffers(WINIO_HWND whWnd, UINT wBufSize, UINT wFlags)
	{
	whWnd->bufsize = wBufSize ? max(wBufSize, WINIO_MIN_BUFFER)
								: WINIO_DEFAULT_BUFFER;

	if (! (whWnd->hBuffer =
			GlobalAlloc(GMEM_MOVEABLE, (DWORD) whWnd->bufsize)))
		return FALSE;
	
	// keep locked; assume protected mode
	whWnd->fpBuffer = GlobalLock(whWnd->hBuffer);
	
	if (! (wFlags & WW_OUTPUTONLY))
		{
		if (! (whWnd->hKeyboard = GlobalAlloc(GMEM_MOVEABLE, TYPE_AHEAD)))
			{
			GlobalFree(whWnd->hBuffer);
			return FALSE;
			}

		// keep locked; assume protected mode
		whWnd->fpKeyboard = GlobalLock(whWnd->hKeyboard);
		}
	
	*(whWnd->fpBuffer) = '\0';
	whWnd->fpBuffer++;

	return TRUE;
	}

/* -----------------------------------------------------------------------	*/
/* Allows an application to vegetate until the user closes all the windows	*/
/* -----------------------------------------------------------------------	*/
void winio_end()
	{
	while (cWindows)
		wmhandler_yield();
	}

/* -------------------------------------------------------------------	*/
/* Closes the window by sending it a WM_DESTROY message. Note that it	*/
/* does not disable the _onclose defined function. So the user program	*/
/* handler will be triggered.											*/
/* -------------------------------------------------------------------	*/
void winio_close(HWND hwnd)
	{
	DestroyWindow(hwnd);
	}


/* -------------------------------------------------------------------	*/
/* Shows the hourglass cursor and puts the window into a modal			*/
/* condition. Second and subsequent calls just increment a counter.		*/
/* -------------------------------------------------------------------	*/
void winio_setbusy(void)
	{
	if (! nBusyCount++)
		{
#if defined(WIN32) || defined(_WIN32)
		/* Portability docs are very inconsistent */
		hCrsSave = (HWND)SetClassLong(hCurrWnd, GCL_HCURSOR, (LONG)NULL);
#else
		hCrsSave = SetClassWord(hCurrWnd, GCW_HCURSOR, NULL);
#endif	
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		SetCapture(hCurrWnd);
		}
	}


/* -------------------------------------------------------------------	*/
/* Cancels the above. More accurately, cancels the above if the counter */
/* that it decrements (incremented in winio_setbusy) reaches zero.		*/
/* -------------------------------------------------------------------	*/
void winio_resetbusy(void)
	{
	if (! --nBusyCount)
		{
		ReleaseCapture();
		SetCursor(hCrsSave);
#if defined(WIN32) || defined(_WIN32)
		SetClassLong(hCurrWnd, GCL_HCURSOR, (LONG)hCrsSave);
#else	
		SetClassWord(hCurrWnd, GCW_HCURSOR, hCrsSave);
#endif
		}
	}


/* -------------------------------------------------------------------	*/
/* Closes all windows by sending the main window a WM_DESTROY message.	*/
/* Note that it does not disable the _onclose defined function. So the	*/
/* user program handler will be triggered.								*/
/* -------------------------------------------------------------------	*/
void winio_closeall()
	{
	if (hMaster)
		DestroyWindow(hMaster);
	}


/* -------------------------------------------------------------------	*/
/* Let the application install an exit routine, called back from		*/
/* winio_wmdestroy(). Deinstall by winio_onclose(NULL)					*/
/* -------------------------------------------------------------------	*/
void winio_onclose(HWND hwnd, DESTROY_FUNC exitfunc)
	{
	if (IsWindow(hwnd))
		((WINIO_HWND) GetWindowLong(hwnd, 4))->destroy_func = exitfunc;
	}

/* -------------------------------------------------------------------	*/
/* Let the application install a paint-time entry routine, called back	*/
/* from winio_wmpaint(). Deinstall by winio_onpaintentry(NULL).			*/
/* -------------------------------------------------------------------	*/
PAINT_FUNC winio_onpaintentry(HWND hwnd, PAINT_FUNC paintfunc)
	{
	WINIO_HWND whWnd;
	PAINT_FUNC old;
	
	if (! IsWindow(hwnd))
		return NULL;
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	old = whWnd->paintentry_func;
	whWnd->paintentry_func = paintfunc;
	return old;
	}

/* -------------------------------------------------------------------	*/
/* Let the application install a paint-time exit routine, called back	*/
/* from winio_wmpaint(). Deinstall by winio_onpaintexit(NULL).			*/
/* -------------------------------------------------------------------	*/
PAINT_FUNC winio_onpaintexit(HWND hwnd, PAINT_FUNC paintfunc)
	{
	WINIO_HWND whWnd;
	PAINT_FUNC old;
	
	if (! IsWindow(hwnd))
		return NULL;
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	old = whWnd->paintexit_func;
	whWnd->paintexit_func = paintfunc;
	return old;
	}

/* -------------------------------------------------------------------	*/
/* This function allows the font of the window to be modified.			*/
/* Currently, only SYSTEM_, ANSI_, and OEM_FIXED_FONTs are supported.	*/
/* -------------------------------------------------------------------	*/
UINT winio_setfont(HWND hwnd, UINT wFont)
	{
	UINT wOld;
	WINIO_HWND whWnd;
	
	if (! IsWindow(hwnd))
		return 0;
	
	if ((wFont != SYSTEM_FIXED_FONT) &&
		(wFont != ANSI_FIXED_FONT) &&
		(wFont != OEM_FIXED_FONT))
		return 0;
		
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	wOld = whWnd->curr_font;
	whWnd->curr_font = wFont;
	set_font(hwnd, whWnd);
	if (whWnd->tPaint)
		InvalidateRect(hwnd, NULL, TRUE);
	return wOld;
	}


/* -------------------------------------------------------------------	*/
/* This function allows a 'line handler' to be installed. When the user */
/* double-clicks on a line, the specified function is called with the	*/
/* arguments (HWND, LPSTR, int). Calling this function with NULL		*/
/* disables line handling...											*/
/* -------------------------------------------------------------------	*/
LINEHANDLER winio_setlinefn(HWND hwnd, LINEHANDLER pfnLine)
	{
	WINIO_HWND whWnd;
	LINEHANDLER old;
	
	if (! IsWindow(hwnd))
		return NULL;
	
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	old = whWnd->pfnLine;
	
	whWnd->pfnLine = pfnLine;
	return old;
	}


/* -------------------------------------------------------------------	*/
/* This function allows the caller to specify immediate or deferred		*/
/* screen updates. The call may not be issued before winio_init().		*/
/* -------------------------------------------------------------------	*/
BOOL winio_setpaint(HWND hwnd, BOOL on)
	{
	WINIO_HWND whWnd;
	BOOL ret;
	
	if (! IsWindow(hwnd))
		return FALSE;
	
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	ret = whWnd->tPaint;

	if (whWnd->tPaint = on)
		{
		whwndCurr->cScrollUD[SB_TOP]	   = -(whwndCurr->yCurrLine);
		whwndCurr->cScrollUD[SB_BOTTOM]	   = whwndCurr->yCurrLine;
		InvalidateRect(hwnd, NULL, TRUE);
		SetScrollRange(hwnd, SB_VERT, 1, whWnd->yCurrLine + 1, FALSE);
		SetScrollPos(hwnd, SB_VERT, whWnd->yTopOfWin + 1, TRUE);
//		wmhandler_yield();
		}
	
	return ret;
	}

/* -------------------------------------------------------------------	*/
/* This function allows the caller to scroll the window to top left of	*/
/* the buffer, as if the user had pressed the HOME key, which is		*/
/* effectively what this function does.									*/
/* -------------------------------------------------------------------	*/
void winio_home(HWND hwnd)
	{
	SendMessage(hwnd, WM_KEYDOWN, VK_HOME, 0L);
	SendMessage(hwnd, WM_KEYDOWN, VK_HOME, 0L);
	}

/* -------------------------------------------------------------------	*/
/* This function changes the behavior of getchar(), whose default		*/
/* is to echo characters, unlike in DOS. winio_setecho(FALSE) restores	*/
/* the non-echo DOS behavior.											*/
/* -------------------------------------------------------------------	*/
BOOL winio_setecho(HWND hwnd, BOOL flag)
	{
	WINIO_HWND whWnd;
	BOOL ret;
	
	if (! IsWindow(hwnd))
		return FALSE;
	
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	ret = whWnd->tEcho;
	whWnd->tEcho = flag;
	return ret;
	}

/* -------------------------------------------------------------------	*/
/* This function sets the window that is to be the target of the		*/
/* STDIO functions.														*/
/* -------------------------------------------------------------------	*/
HWND winio_setcurrent(HWND hwnd)
	{
	HWND h = hCurrWnd;
	
	if (! IsWindow(hwnd))
		return NULL;
	
	hCurrWnd = hwnd;
	if (hwnd)
		whwndCurr = (WINIO_HWND) GetWindowLong(hwnd, 4);
	else
		whwndCurr = NULL;
	
	return h;
	}

/* -------------------------------------------------------------------	*/
/* This function sets the Help About... text.							*/
/* -------------------------------------------------------------------	*/
void winio_about(char *str)
	{
	strncpy(strAbout, str, MAX_ABOUT);
	strAbout[MAX_ABOUT] = 0;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_PAINT handler. It sends the currrent 'in view' piece of	*/
/* the buffer to the window. Note that an embedded NULL character	*/
/* signifies an end of line, not '\n'.								*/
/* ---------------------------------------------------------------	*/
long winio_wmpaint(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	HDC hdc;
	PAINTSTRUCT ps;
	WINIOINFO wi;
	char far *pchEOL;
	int i, j, xStart;
	int xLeft, xRight, yTop, yBottom;
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	char far *pchSOL = whWnd->fpTopOfWin;

	hdc = BeginPaint(hwnd, &ps);

	get_info(whWnd, (PWINIOINFO) &wi);
	
	if ((whWnd->paintentry_func) &&
		(! (*whWnd->paintentry_func)
			(hwnd, hdc, (PAINTSTRUCT *) &ps, (PWINIOINFO) &wi)))
		goto labAfter;

	xLeft = (ps.rcPaint.left / whWnd->cxChar) + whWnd->xLeftOfWin;
	xRight = (ps.rcPaint.right / whWnd->cxChar) + whWnd->xLeftOfWin;
	yTop = ps.rcPaint.top / whWnd->cyChar;
	yBottom = ps.rcPaint.bottom / whWnd->cyChar;

	SelectObject(hdc, GetStockObject(whWnd->curr_font));
	
	for (i = 0; i < yTop; i++)		// lines above repaint region
		{
		while ((pchSOL < whWnd->fpCurrLine) && *pchSOL)
			pchSOL++;
		if (pchSOL < whWnd->fpCurrLine)
			pchSOL++;
		else
			break;
		}

	if ((i == yTop) && (i <= whWnd->yCurrLine)) // something needs repainting
		{
		for (; i <= yBottom; i++)	// lines in repaint region
			{
			for (j = 0; (j < xLeft) && (*pchSOL); j++, pchSOL++)
				; // Scroll right
			pchEOL = pchSOL;
			xStart = j - whWnd->xLeftOfWin;
			for (j = 0; (*pchEOL) ; j++, pchEOL++) ; // end of line
			TextOut(hdc, whWnd->cxChar * xStart, whWnd->cyChar * i,
				pchSOL, min(j, xRight - xLeft + 1));
			if ((unsigned) (pchEOL - whWnd->fpBuffer) >= whWnd->bufused)
				break;
			pchSOL = ++pchEOL;	
			}
		}
	
labAfter:	
	if (whWnd->paintexit_func)
		(*whWnd->paintexit_func)(hwnd, hdc,
			(PAINTSTRUCT *) &ps, (PWINIOINFO) &wi);
	
	EndPaint(hwnd, &ps);
	if (hwnd == GetFocus())
		adjust_caret(whWnd);
	return 0;
	}


/* ---------------------------------------------------------------	*/
/* Our WM_COMMAND handler. This fires off the appropriate handler	*/
/* for the menu option.												*/
/* ---------------------------------------------------------------	*/
long winio_wmcommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	/* Added for portability by Andy C. 6/17/95 */
	WORD itemID = LOWORD(wParam);

	if (LOWORD(lParam))			// Menu options only (dailog controls later ?)
		return 0;
	
	/* if (wParam > MAX_MENU_ID) */
	if (itemID > MAX_MENU_ID)
		return 0;
	
	/* Deleted this:	--wParam; */
	/* Added this: */
	--itemID;
	
	if (whWnd->menu_func[/* wParam */ itemID])
		(*whWnd->menu_func[/* wParam */ itemID])(hwnd, /* wParam */ itemID + 1);
	
	return 1;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_SIZE handler. It updates the internal record of our		*/
/* window size, minus the scroll bars, and recalcs the scroll bar	*/
/* ranges.															*/
/* ---------------------------------------------------------------	*/
long winio_wmsize(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	
	whWnd->cxWidth = LOWORD(lParam);
	whWnd->cyHeight = HIWORD(lParam);

	whWnd->xWinWidth   = (whWnd->cxWidth - gcxScroll ) / whWnd->cxChar;
	whWnd->yWinHeight  = (whWnd->cyHeight - gcyScroll ) / whWnd->cyChar;

	whWnd->cScrollLR[SB_PAGEUP]	   = -(whWnd->xWinWidth) / 2;
	whWnd->cScrollLR[SB_PAGEDOWN]  = +(whWnd->xWinWidth) / 2;
	whWnd->cScrollUD[SB_PAGEUP]	   = -(whWnd->yWinHeight) + 1;
	whWnd->cScrollUD[SB_PAGEDOWN]  = +(whWnd->yWinHeight) - 1;
	
	SetScrollRange(hwnd, SB_HORZ, 1, MAX_X, FALSE);
	SetScrollPos(hwnd, SB_HORZ, whWnd->xLeftOfWin + 1, TRUE);

	SetScrollRange(hwnd, SB_VERT, 1, whWnd->yCurrLine + 1, FALSE);
	SetScrollPos(hwnd, SB_VERT, whWnd->yTopOfWin + 1, TRUE);
	
	return 0;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_DESTROY handler. It frees up storage associated with the	*/
/* window. If it is the last window for the app, it exit()s...		*/
/* ---------------------------------------------------------------	*/
long winio_wmdestroy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	
	if (whWnd->destroy_func)
		(*(whWnd->destroy_func))(hwnd);

	if (hwnd == hMaster)
		hMaster = NULL;

	if (hwnd == hCurrWnd)
		{
		hCurrWnd = NULL;
		whwndCurr = NULL;
		}
	
	wmhandler_destroy(hwnd);
	GlobalFree(whWnd->hBuffer);
	GlobalFree(whWnd->hKeyboard);
#if defined(WIN32) || defined(_WIN32)
	GlobalFree(GlobalHandle(whWnd));
#else
	GlobalFree(LOWORD(GlobalHandle(HIWORD(whWnd))));
#endif
	--cWindows;

	return 0;
	}

/* --------------------------------------------------------------- */
/* Our WM_BYTE handler. It adds the BYTE to the internal kb buffer */
/* if there is room otherwise it queues a BEEP					   */
/* --------------------------------------------------------------- */
long winio_wmchar(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	char far *lpchKeybd = whWnd->fpKeyboard;
	unsigned pchSave = whWnd->pchKbIn;
	
	whWnd->pchKbIn++;
	if (whWnd->pchKbIn == TYPE_AHEAD)
		whWnd->pchKbIn = 0;
	if (whWnd->pchKbIn == whWnd->pchKbOut)
		{
		MessageBeep(0);
		whWnd->pchKbIn = pchSave;
		}
	else
		*(lpchKeybd + pchSave) = LOBYTE(wParam);

	return 0;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_LBUTTONDBLCLK handler. This handles the user double-		*/
/* clicking on a line. If the user has specified a 'line' handler	*/
/* we get the pointer to the beginning of the line, and pass that	*/
/* and the line number to the function specified...					*/
/* ---------------------------------------------------------------	*/
long winio_wmldblclk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	int cLine;
	int yRel;
	LPSTR lpLine;
	static char strTmp[128];
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	
	if (! whWnd->pfnLine)
		return 0;
	
	yRel = HIWORD(lParam) / whWnd->cyChar;
	cLine = whWnd->yTopOfWin + yRel;
	if (cLine > whWnd->yCurrLine)
		return 0;
	lpLine = whWnd->fpTopOfWin;
	for (; yRel > 0; yRel--)
		lpLine = nextline(lpLine);
	lstrcpy(strTmp, lpLine);
	(* (whWnd->pfnLine))(hwnd, strTmp, ++cLine);
	return 0;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_KEYDOWN handler. This handles what would be called		*/
/* function keys in the DOS world. In this case the function keys	*/
/* operate as scroll bar controls, so we generate messages to the	*/
/* scroll message handlers below.									*/
/* ---------------------------------------------------------------	*/
long winio_wmkeydown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	int hSB, vSB;
	
	if ((wParam < VK_PRIOR) || (wParam > VK_DOWN))
		return 0;
	
	hSB = VKtoSB[wParam - VK_PRIOR].hSB;
	vSB = VKtoSB[wParam - VK_PRIOR].vSB;
	if (hSB != DO_NOTHING)
		SendMessage(hwnd, WM_HSCROLL, hSB, 0L);
	if (vSB != DO_NOTHING)
		SendMessage(hwnd, WM_VSCROLL, vSB, 0L);
	return 0;
	}

/* --------------------------------------------------------------- */
/* Our WM_HSCROLL handler. It adjusts what part of the buffer	   */
/* is visible. It operates as left/right arrow keys.			   */
/* --------------------------------------------------------------- */
long winio_wmhscroll(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	int cxSave = whWnd->xLeftOfWin,
	/* Deleted this:	xInc = whWnd->cScrollLR[wParam]; */
	/* Added this: */
	xInc = whWnd->cScrollLR[LOWORD(wParam)];
	
	if (! whWnd->tPaint) return 0;
	
	if (xInc == DO_NOTHING)
		return 0;
	else if (xInc == USE_PARAM)
#if defined(WIN32) || defined(_WIN32)
		whWnd->xLeftOfWin = HIWORD(wParam) - 1;
#else
		whWnd->xLeftOfWin = LOWORD(lParam) - 1;
#endif
	else
		whWnd->xLeftOfWin += xInc;
	
	if ((whWnd->xLeftOfWin = max(0, min(MAX_X - 1, whWnd->xLeftOfWin)))
					== cxSave)
		return 0;

	ScrollWindow(hwnd, (cxSave - whWnd->xLeftOfWin) * whWnd->cxChar,
						0, NULL, NULL);
	SetScrollPos(hwnd, SB_HORZ, whWnd->xLeftOfWin + 1, TRUE);
	UpdateWindow(hwnd);

	return 0;
	}

/* --------------------------------------------------------------- */
/* Our WM_VSCROLL handler. It adjusts what part of the buffer	   */
/* is visible. It operates as page and line up/down keys.		   */
/* --------------------------------------------------------------- */
long winio_wmvscroll(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	int cySave = whWnd->yTopOfWin,
		/* Deleted this:	yInc = whWnd->cScrollUD[wParam], */
		/* Added this: */
		yInc = whWnd->cScrollUD[LOWORD(wParam)],
		i;
	
	if (! whWnd->tPaint) return 0;
	
	if (yInc == DO_NOTHING)
		return 0;
	else if (yInc == USE_PARAM)
#if defined(WIN32) || defined(_WIN32)
		whWnd->yTopOfWin = (wParam >> 16) - 1;
#else
		whWnd->yTopOfWin = LOWORD(lParam) - 1;
#endif
	else
		whWnd->yTopOfWin += yInc;

	if ((whWnd->yTopOfWin = max(0, min(whWnd->yCurrLine, whWnd->yTopOfWin)))
									== cySave)
		return 0;

	if(LOWORD(wParam) == SB_TOP)
		whWnd->yTopOfWin = 0;

	if (whWnd->yTopOfWin > cySave)
		for (i = cySave; i < whWnd->yTopOfWin; i++)
			whWnd->fpTopOfWin = nextline(whWnd->fpTopOfWin);
	else
		for (i = cySave; i > whWnd->yTopOfWin; i--)
			whWnd->fpTopOfWin = prevline(whWnd->fpTopOfWin);

	ScrollWindow(hwnd, 0,
				(cySave - whWnd->yTopOfWin) * whWnd->cyChar, NULL, NULL);
	SetScrollPos(hwnd, SB_VERT, whWnd->yTopOfWin + 1, TRUE);
	UpdateWindow(hwnd);

	return 0;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_SETFOCUS handler. It sets up the text caret.				*/
/* ---------------------------------------------------------------	*/
long winio_wmsetfocus(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	
	CreateCaret(hwnd, NULL, CARET_WIDTH, whWnd->cyChar);
	
	if ((whWnd->tCaret = winio_caret_visible(whWnd)))
		{
		SetCaretPos((whWnd->xCurrPos - whWnd->xLeftOfWin) * whWnd->cxChar,
					(whWnd->yCurrLine - whWnd->yTopOfWin) * whWnd->cyChar);
		ShowCaret(hwnd);
		}

	return 0;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_KILLFOCUS handler. It destroys the text caret.			*/
/* ---------------------------------------------------------------	*/
long winio_wmkillfocus(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	
	if (whWnd->tCaret)
		{
		HideCaret(hwnd);
		whWnd->tCaret = FALSE;
		}
	DestroyCaret();
	return 0;
	}

/* ---------------------------------------------------------------	*/
/* Our WM_COMMAND ID_EXIT handler. It will only be invoked from		*/
/* the main window. It simply destroys the main window and thereby	*/
/* automatically all the child windows.								*/
/* ---------------------------------------------------------------	*/
void winio_doexit(HWND hwnd, int nID)
	{
	DestroyWindow(hMaster);
	}
void winio_exit(void)
	{
	DestroyWindow(hMaster);
	}

/* ---------------------------------------------------------------	*/
/* Our WM_COMMAND ID_ABOUT handler. It will only be invoked from	*/
/* the main window. It simply destroys the main window and thereby	*/
/* automatically all the child windows.								*/
/* ---------------------------------------------------------------	*/
void winio_doabout(HWND hwnd, int nID)
	{
	winio_warn(FALSE, __szModule, "%s%s", strAbout, WINIO_ABOUT_TEXT);
	}

/* ---------------------------------------------------------------	*/
/* Tells the app how many windows are still open					*/
/* ---------------------------------------------------------------	*/
int winio_openwindows(void)
	{
	return cWindows;
	}


/* ---------------------------------------------------------------	*/
/* Sets up a handler for a menu option. An application that adds	*/
/* an option to the menu should specify the handler here, otherwise */
/* the option will never achieve anything...! User app IDs must be	*/
/* in the range 1 - 32.												*/
/* ---------------------------------------------------------------	*/
MENU_FUNC winio_setmenufunc(HWND hwnd, int id, MENU_FUNC menu_func)
	{
	MENU_FUNC old_menu_func;
	WINIO_HWND whWnd;
	
	if (! IsWindow(hwnd))
		return NULL;
	
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);

	if ((id < 1) || (id > MAX_MENU_ID))
		return (MENU_FUNC) -1;
	
	id--;
	old_menu_func = whWnd->menu_func[id];
	whWnd->menu_func[id] = menu_func;
	return old_menu_func;
	}


/* ---------------------------------------------------------------	*/
/* Returns the handle to the window's main menu, allowing an app	*/
/* to add options to it. It is NULL if the window was created		*/
/* without the WW_HASMENU flag.										*/
/* ---------------------------------------------------------------	*/
HMENU winio_hmenumain(HWND hwnd)
	{
	WINIO_HWND whWnd;
	
	if (! IsWindow(hwnd))
		return NULL;
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	return whWnd->hMainMenu;
	}


/* ---------------------------------------------------------------	*/
/* Returns the handle to the window's file popup menu, allowing an	*/
/* app to add options to it. It is NULL if the window was created	*/
/* without the WW_HASMENU flag.										*/
/* ---------------------------------------------------------------	*/
HMENU winio_hmenufile(HWND hwnd)
	{
	WINIO_HWND whWnd;
	
	if (! IsWindow(hwnd))
		return NULL;
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	return whWnd->hFileMenu;
	}


/* ---------------------------------------------------------------	*/
/* Returns the handle to the window's help popup menu, allowing an	*/
/* app to insert options into it. It is NULL if the window was		*/
/* created without the WW_HASMENU flag.								*/
/* ---------------------------------------------------------------	*/
HMENU winio_hmenuhelp(HWND hwnd)
	{
	WINIO_HWND whWnd;
	
	if (! IsWindow(hwnd))
		return NULL;
	whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	return whWnd->hHelpMenu;
	}


/* ---------------------------------------------------------------	*/
/* Our WM_COMMAND ID_SAVE handler. It uses COMMDLG.DLL to get a		*/
/* file name, and saves the contents of the hwnd's buffer into the	*/
/* file.															*/
/* ---------------------------------------------------------------	*/
void winio_dosave(HWND hwnd, int nID)
	{
	WINIO_HWND whWnd = (WINIO_HWND) GetWindowLong(hwnd, 4);
	OPENFILENAME ofn;
	LPSTR lpBuf, fpData;
	int lenLine, wr, h;

	memset(&ofn, 0, sizeof(ofn));	// zero out
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = __hInst;
	ofn.lpstrFile = (LPSTR) whWnd->strSaveFile;
	ofn.nMaxFile = 80;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_SHOWHELP;
	
	if (! GetSaveFileName(&ofn))
		return;
	
	if ((h = _lcreat(ofn.lpstrFile, (int)NULL)) == -1) return;

	if ((lpBuf = GlobalLock(GlobalAlloc(GMEM_MOVEABLE, MAX_X + 2))) == NULL)
		{
		winio_warn(FALSE, "Save Buffer", "Cannot find 150 bytes available!");
		_lclose(h);
		return;
		}

	GetWindowText(hwnd, lpBuf, MAX_X - 2);
	lstrcat(lpBuf, "\r\n\r\n");
	_lwrite(h, lpBuf, lstrlen(lpBuf));
	
	for (	fpData = whWnd->fpBuffer;
			fpData <= whWnd->fpCurrLine;
			fpData += lenLine + 1)
		{
		lenLine = lstrlen(fpData);
#if defined(WIN32) || defined(_WIN32)
		memcpy(lpBuf, fpData, lenLine);
#else
		_fmemcpy(lpBuf, fpData, lenLine);
#endif
		lpBuf[lenLine] = '\r';
		lpBuf[lenLine+1] = '\n';
		if ((wr = _lwrite(h, lpBuf, lenLine + 2)) < (lenLine + 2))
			{
			winio_warn(FALSE, "Save Buffer",
				"Problem writing %Fs\nInsufficient disk space ?",
					ofn.lpstrFile);
			break;
			}
		};
	_lclose(h); 
#if defined(WIN32) || defined(_WIN32)
	GlobalFree(GlobalHandle(lpBuf));
#else
	GlobalFree(LOWORD(GlobalHandle(HIWORD((DWORD) lpBuf))));
#endif
	}


/* ---------------------------------------------------------------	*/
/* Updates the window structure to reflect the new font selection.	*/
/* ---------------------------------------------------------------	*/
void set_font(HWND hwnd, WINIO_HWND whWnd)
	{
	HDC hdc;
	TEXTMETRIC tm;
	  
	hdc = GetDC(hwnd);
	SelectObject(hdc, GetStockObject(whWnd->curr_font));
	GetTextMetrics(hdc,&tm);
	ReleaseDC(hwnd,hdc);
	whWnd->cxChar = tm.tmAveCharWidth;
	whWnd->cyChar = tm.tmHeight+tm.tmExternalLeading;
	whWnd->xWinWidth   = (whWnd->cxWidth - gcxScroll ) / whWnd->cxChar;
	whWnd->yWinHeight  = (whWnd->cyHeight - gcyScroll ) / whWnd->cyChar;
	}


/* ---------------------------------------------------------------	*/
/* Updates the window structure to reflect the new font selection.	*/
/* ---------------------------------------------------------------	*/
void get_info(WINIO_HWND whWnd, PWINIOINFO pwi)
	{
	pwi->dimChar.x = whWnd->cxChar;
	pwi->dimChar.y = whWnd->cyChar;

	pwi->posCurr.x = whWnd->xCurrPos;
	pwi->posCurr.y = whWnd->yCurrLine;

	pwi->rectView.left = pwi->rectView.right = whWnd->xLeftOfWin;
	pwi->rectView.top = pwi->rectView.bottom = whWnd->yTopOfWin;
	pwi->rectView.right += whWnd->xWinWidth;
	pwi->rectView.bottom += whWnd->yWinHeight;

	pwi->cDiscarded = whWnd->cLinesDiscarded;
	}


/* ---------------------------------------------------------------	*/
/* Adjusts the position of the caret, and shows or hides it, as		*/
/* appropriate.														*/
/* ---------------------------------------------------------------	*/
void adjust_caret(WINIO_HWND whWnd)
	{
	int t = winio_caret_visible(whWnd);

	if (t)
		SetCaretPos((whWnd->xCurrPos - whWnd->xLeftOfWin) * whWnd->cxChar,
					(whWnd->yCurrLine - whWnd->yTopOfWin) * whWnd->cyChar);
	if (t && (! whWnd->tCaret))
		ShowCaret(hCurrWnd);
	if ((! t) && whWnd->tCaret)
		HideCaret(hCurrWnd);
	whWnd->tCaret = t;
	}

/* ---------------------------------------------------------------	*/
/* Computes, on the basis of what has just been updated, what area	*/
/* of the window needs to be repainted.								*/
/* ---------------------------------------------------------------	*/
void compute_repaint(void)
	{
	RECT rc;
	static int xCP = 0, yCL = 0;
	int tWholeWin = FALSE;

	if (whwndCurr->yCurrLine > (whwndCurr->yTopOfWin + whwndCurr->yWinHeight))
		{
		whwndCurr->yTopOfWin = 0;
		whwndCurr->fpTopOfWin = whwndCurr->fpBuffer;
		while (whwndCurr->yTopOfWin <
				(whwndCurr->yCurrLine - ((whwndCurr->yWinHeight + 1) / 2)))
			{
			whwndCurr->fpTopOfWin = nextline(whwndCurr->fpTopOfWin);
			whwndCurr->yTopOfWin++;
			}
		tWholeWin = TRUE;
		}

	if ((whwndCurr->xCurrPos < whwndCurr->xLeftOfWin) ||
		(whwndCurr->xCurrPos >
			(whwndCurr->xLeftOfWin + whwndCurr->xWinWidth)))
		{
		whwndCurr->xLeftOfWin = 0;
		while (whwndCurr->xLeftOfWin <
				(whwndCurr->xCurrPos - ((whwndCurr->xWinWidth + 1) / 2)))
			whwndCurr->xLeftOfWin++;
		tWholeWin = TRUE;
		}

	if (tWholeWin)
		{
		if (whwndCurr->tPaint)
			InvalidateRect(hCurrWnd, NULL, TRUE);
		}
	else
		{
		rc.left = ((yCL == whwndCurr->yCurrLine) && (!whwndCurr->tCR)) ?
			(min(xCP, whwndCurr->xCurrPos) -
				whwndCurr->xLeftOfWin) * whwndCurr->cxChar : 0;
		rc.top = (yCL - whwndCurr->yTopOfWin) * whwndCurr->cyChar;
		rc.right = (whwndCurr->xWinWidth + 3) * whwndCurr->cxChar;
		rc.bottom = (whwndCurr->yCurrLine - whwndCurr->yTopOfWin + 1 +
			whwndCurr->tCR)
			* whwndCurr->cyChar;
		if (whwndCurr->tPaint)
			InvalidateRect(hCurrWnd, &rc, TRUE);
		}

//	whwndCurr->tCR = FALSE;
	yCL = whwndCurr->yCurrLine;
	xCP = whwndCurr->xCurrPos;
	}

/* ---------------------------------------------------------------	*/
/* Adds the supplied cch-long string to the display buffer, and		*/
/* ensures any changed part of the window is repainted.				*/
/* ---------------------------------------------------------------	*/
void addchars(char *pch, unsigned cch)
	{
	int ycSave = whwndCurr->yCurrLine;
	int ytSave = whwndCurr->yTopOfWin;
	int xSave = whwndCurr->xLeftOfWin;

	make_avail(cch);

	append2buffer(pch, cch);

	if (whwndCurr->tPaint && (ycSave != whwndCurr->yCurrLine))
		SetScrollRange(hCurrWnd, SB_VERT, 1, whwndCurr->yCurrLine + 1, FALSE);

	compute_repaint();

	whwndCurr->cScrollUD[SB_BOTTOM] = whwndCurr->yCurrLine;
	
	if (whwndCurr->tPaint)
		{
		if (ytSave != whwndCurr->yTopOfWin)
			SetScrollPos(hCurrWnd, SB_VERT, whwndCurr->yTopOfWin + 1, TRUE);
		if (xSave != whwndCurr->xLeftOfWin)
			SetScrollPos(hCurrWnd, SB_HORZ, whwndCurr->xLeftOfWin + 1, TRUE);
// Move out of the 'if' ?
		wmhandler_yield();
		}
	}


/* ---------------------------------------------------------------	*/
/* Add chars onto the display buffer, wrapping at end of line,		*/
/* expanding tabs, etc.												*/
/* ---------------------------------------------------------------	*/
void append2buffer(char *pch, unsigned cch)
	{
	unsigned i;

	for (i = 0; i < cch; i++, pch++)
		{
		switch (*pch)
			{
			case '\r' :
				whwndCurr->fpCurr = whwndCurr->fpSOI;
				whwndCurr->xCurrPos = 0;
				whwndCurr->tCR = TRUE;
				break;
			case '\n' :
				*pch = '\0';
				*whwndCurr->fpCurr++ = '\0';
				whwndCurr->fpCurrLine = whwndCurr->fpCurr;
				whwndCurr->fpSOI = whwndCurr->fpCurr;
				whwndCurr->yCurrLine++;
				whwndCurr->xCurrPos = 0;
				break;
			case '\t' :
				do	{
				/* Special requirement of tabs... */
					make_avail(cch - i + TABSIZE);
					*whwndCurr->fpCurr++ = ' ';
					whwndCurr->xCurrPos++;
					} while ((whwndCurr->xCurrPos % TABSIZE) != 0);
				break;
			case 0xff /* EOF */ :
				break;
			case '\b' :
				if (whwndCurr->fpCurr > whwndCurr->fpSOI)
					{
					*(--whwndCurr->fpCurr) = 0;
					whwndCurr->bufused--;
					whwndCurr->xCurrPos--;
					}
				break;
			case 0x1b :
				while (whwndCurr->fpCurr > whwndCurr->fpSOI)
					{
					*(--whwndCurr->fpCurr) = 0;
					whwndCurr->bufused--;
					whwndCurr->xCurrPos--;
					}
				break;
			case 0x07 :
				MessageBeep(0);
				break;
			default :
				if (*pch > 0x1a)
					{
					if (whwndCurr->xCurrPos >= MAX_X)
						{
						*whwndCurr->fpCurr++ = 0;
						whwndCurr->xCurrPos = 0;
						whwndCurr->yCurrLine++;
						whwndCurr->fpCurrLine = whwndCurr->fpCurr;
						}
					whwndCurr->xCurrPos++;
					*whwndCurr->fpCurr++ = *pch;
					}
			}
		}

	if ((whwndCurr->fpBuffer + whwndCurr->bufused) < whwndCurr->fpCurr)
		{
		whwndCurr->bufused = (whwndCurr->fpCurr - whwndCurr->fpBuffer);
		*whwndCurr->fpCurr = '\0';
		}
	}

/* ---------------------------------------------------------------	*/
/* If we have run out of room in the display buffer, drop whole		*/
/* lines, and move the remaining buffer up.							*/
/* ---------------------------------------------------------------	*/
void make_avail(unsigned cch)
	{
	unsigned cDiscard = 0;
	char far *fpNew;
	char far *fpTmp;
	unsigned yDrop, min_discard;

	// Do we have enough room for what we want to add (allow 1 for \0) ?
	if ((unsigned long)(whwndCurr->bufused + cch + 1) < whwndCurr->bufsize)
		return;

	// Calulate how much should we should drop off the bottom
	min_discard = (UINT) whwndCurr->bufsize >> 3;

	// this gives a whole number of our allocation units.
	cDiscard = ((cch + min_discard - 1) / min_discard) * min_discard;
		
	// Adjust to line up at the next start of line
	fpNew = whwndCurr->fpBuffer;
	fpNew += (LONG) cDiscard;
	fpTmp = whwndCurr->fpBuffer;
	for (yDrop = 0; fpTmp < fpNew; fpTmp = nextline(fpTmp), yDrop++) ;
	cDiscard = fpTmp - whwndCurr->fpBuffer; 

	// Move the buffer 'up', and initialize vacated to zeros
#if defined(WIN32) || defined(_WIN32)
	memcpy(whwndCurr->fpBuffer, fpTmp, whwndCurr->bufused - cDiscard + 1);
#else
	_fmemcpy(whwndCurr->fpBuffer, fpTmp, whwndCurr->bufused - cDiscard + 1);
#endif
	whwndCurr->bufused -= cDiscard;
	whwndCurr->fpCurr -= cDiscard;
#if defined(WIN32) || defined(_WIN32)
	memset(whwndCurr->fpBuffer + whwndCurr->bufused, 0, cDiscard);
#else
	_fmemset(whwndCurr->fpBuffer + whwndCurr->bufused, 0, cDiscard);
#endif
	
	// Now we know how far we have moved, adjust other pointers
	if (whwndCurr->fpSOI != NULL) whwndCurr->fpSOI -= cDiscard;
	whwndCurr->fpCurrLine -= cDiscard;
	whwndCurr->yCurrLine -= yDrop;
	whwndCurr->fpTopOfWin -= cDiscard;
	whwndCurr->yTopOfWin -=yDrop;
	whwndCurr->cLinesDiscarded += yDrop;
	}


/* -------------------------------------------------------------------	*/
/* These two routines find the beginning of the next, and previous		*/
/* lines relative to their input pointer								*/
/* -------------------------------------------------------------------	*/

char far *nextline(char far *p) { while (*p) p++; return ++p; }
char far *prevline(char far *p) { p--; do p--; while (*p); return ++p; }

/* -------------------------------------------------------------------	*/
/* Waits for a character to appear in the keyboard buffer, yielding		*/
/* while nothing is available. Then inserts it into the buffer.			*/
/* -------------------------------------------------------------------	*/
int getch(void)
	{
	BYTE c;
	
	while (whwndCurr && (whwndCurr->pchKbIn == whwndCurr->pchKbOut))
		wmhandler_yield();
		
	if (! whwndCurr) 
		abort_exit();
	
	c = *(whwndCurr->fpKeyboard + whwndCurr->pchKbOut);

	whwndCurr->pchKbOut++;
	if (whwndCurr->pchKbOut == TYPE_AHEAD)
		whwndCurr->pchKbOut = 0;
	
	// Do CR/LF and EOF translation
	return (c == 0x1a) ? EOF : (c == '\r') ? '\n' : c;
	}

