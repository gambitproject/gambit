/*
The files argcargv.c, winio.c and wmhandlr.c are all needed to simulate a
console under Windows for the GCL.
 $Id$

	WMHANDLR.C -- Windows message handling without switch statements
	Part of the WINIO Library
	
	Copyright (c) Dave Maxey & Andrew Schulman, 1990-1992.
	All Rights Reserved.
	
	Contact:  Andrew Schulman (CompuServe 76320,302)
	
	From Chapter 4 of "Undocumented Windows" (Addison-Wesley 1992)
	by Andrew Schulman, Dave Maxey and Matt Pietrek
*/

#include <windows.h>
#include <string.h>
#include "wmhandlr.h"

long  defwmhandler(HWND, UINT, WPARAM, LPARAM);


/* -----------------------------------------------------------------------	*/
/* Sees if this is one of the 16 allowable extra message handlers handling	*/
/* messages above WM_USER. If so, returns TRUE, and puts the index of the	*/
/* handler in the wmhandlrtable into pwExtraMsg. Otherwise returns FALSE	*/
/* -----------------------------------------------------------------------	*/
BOOL wmhandler_isextramsg(WMTAB wmTab, UINT wMsg, UINT *pwExtraMsg)
	{
	UINT i;
	
	for (i = 0; i < MAX_EXTRA_WMS; i++)
		if (wmTab->wExtraMsg[i] == wMsg)
			{
			*pwExtraMsg = i + WM_USER;
			return TRUE;
			}
	return FALSE;
	}


/* -----------------------------------------------------------------------	*/
/* Gets the current handler for the supplied message above WM_USER if one	*/
/* is registered. If none is registered, returns NULL.						*/
/* -----------------------------------------------------------------------	*/
WMHANDLER wmhandler_getextratab(WMTAB wmTab, UINT wMsg)
	{
	UINT i;
	
	for (i = 0; i < MAX_EXTRA_WMS; i++)
		if (wmTab->wExtraMsg[i] == wMsg)
			return wmTab->wmhandlertable[WM_USER+i];
	return NULL;
	}


/* -----------------------------------------------------------------------	*/
/* Sets the current handler for the supplied message above WM_USER to the	*/
/* specified function. If the message was already registered, updates the	*/
/* handler and returns the previous. Otherwise, registers the message at	*/
/* the first free entry in the table, and returns defwmhandler as the		*/
/* previous handler. If none available, returns NULL.						*/
/* -----------------------------------------------------------------------	*/
WMHANDLER wmhandler_setextratab(WMTAB wmTab, UINT wMsg, WMHANDLER f)
	{
	UINT i, iUse = 0xffff;
	WMHANDLER old = defwmhandler;
	
	// Is it in the table of extra wmhandlers
	for (i = 0; i < MAX_EXTRA_WMS; i++)
		if (wmTab->wExtraMsg[i] == wMsg)
			break;
		else
		// while we're going through, identify a free entry
		if ((wmTab->wExtraMsg[i] == 0) && (iUse == 0xffff)) 
			iUse = i;

	if ( i < MAX_EXTRA_WMS)
		iUse = i;
	
	if (iUse == 0xffff)
		return NULL;
	
	// Out with the old, in with the new
	old = wmTab->wmhandlertable[WM_USER+iUse];
	wmTab->wmhandlertable[WM_USER+iUse] = f ? f : defwmhandler;
	
	// If the entry is being NULLed, remove it
	if ((! f) || (f == defwmhandler))
		wmTab->wExtraMsg[iUse] = 0;
	else
		wmTab->wExtraMsg[iUse] = wMsg;
	
	return old;
	}


/* -----------------------------------------------------------------------	*/
/* This is our event processor. It is the dispatcher for the handlers set	*/
/* using SetHandler. An Application plugs this function into its			*/
/* window, sets handlers for messages, and wndproc handles the rest.		*/
/* This window procecedure should never need to be changed!					*/
/* -----------------------------------------------------------------------	*/
LRESULT CALLBACK wmhandler_wndproc(HWND hWnd, UINT wMsg,
		WPARAM wParam, LPARAM lParam)
	{
	LPCREATEPARAMS lpcp;
	WMTAB wmTab;
	UINT wExtraMsg;
	
	// On a CREATE, the message table needs to be installed
	// and a WM_GETMINMAXINFO is the first message received

	if (wMsg == WM_CREATE)
		{
		lpcp = (LPCREATEPARAMS) ((LPCREATESTRUCT) lParam)->lpCreateParams;
		SetWindowLong(hWnd, 0, (DWORD) (lpcp->wmTab));
		SetWindowLong(hWnd, 4, (DWORD) (lpcp->lpData));
		}

	if ((! hWnd) || ((wmTab = (WMTAB) GetWindowLong(hWnd, 0)) == NULL))
		return DefWindowProc(hWnd, wMsg, wParam, lParam);
	
	if (wMsg < WM_USER)
		return (*(wmTab->wmhandlertable[wMsg]))(hWnd, wMsg, wParam, lParam);
	else
	if (wmhandler_isextramsg(wmTab, wMsg, &wExtraMsg))
		return (*(wmTab->wmhandlertable[wExtraMsg]))
			(hWnd, wMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, wMsg, wParam, lParam);
	}

/* ---------------------------------------------------------------- */
/* Routines to get and set the message handlers. Setting to NULL	*/
/* uninstalls a handler, by setting the handler to the default		*/
/* which calls Windows own DefWndProc.								*/
/* ---------------------------------------------------------------- */
WMHANDLER wmhandler_get(HWND hWnd, UINT wMsg)
	{
	WMTAB wmTab = (WMTAB) GetWindowLong(hWnd, 0);
	return (wMsg < WM_USER) ?
		wmTab->wmhandlertable[wMsg] :
		wmhandler_getextratab(wmTab, wMsg);
	}

WMHANDLER wmhandler_settab(WMTAB wmTab, UINT wMsg, WMHANDLER f)
	{
	WMHANDLER oldf;

	if (wMsg < WM_USER)
		{
		oldf = wmTab->wmhandlertable[wMsg];
		wmTab->wmhandlertable[wMsg] = f ? f : defwmhandler;
		return (oldf ? oldf : defwmhandler);
		}
	else
		return wmhandler_setextratab(wmTab, wMsg, f);
	}

WMHANDLER wmhandler_set(HWND hWnd, UINT wMsg, WMHANDLER f)
	{
	WMTAB wmTab = (WMTAB) GetWindowLong(hWnd, 0);
	return wmhandler_settab(wmTab, wMsg, f);
	}

/* ----------------------------------------------------------------------- */
/* This is a default handler so that an application chain on to a previous */
/* handler from their current one without having to worry what was there   */
/* before. All this default handler does is to call DefWindowProc.		   */
/* ----------------------------------------------------------------------- */
long defwmhandler(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
	{	
	return DefWindowProc(hwnd, wMsg, wParam, lParam);
	}

/* -------------------------------------------------------------------	*/
/* Create a new wm handler function disatch table.						*/
/* -------------------------------------------------------------------	*/
WMTAB wmhandler_create(void)
	{
	int i;
	WMTAB wmTab;
	
	if (! (wmTab = (WMTAB) GlobalLock(
						GlobalAlloc(GMEM_MOVEABLE, sizeof(WMHANDLERDATA)))))
		return NULL;
	
	for (i = 0; i < WM_USER + MAX_EXTRA_WMS; i++) 
		wmTab->wmhandlertable[i] = defwmhandler;
	
	for (i = 0; i < MAX_EXTRA_WMS; i++)
		wmTab->wExtraMsg[i] = 0;
	
	return wmTab;
	}


/* -------------------------------------------------------------------	*/
/* Destroy a wm handler function disatch table.							*/
/* -------------------------------------------------------------------	*/
void wmhandler_destroy(HWND hwnd)
	{
#if defined(WIN32) || defined(_WIN32)
	GlobalFree( GlobalHandle( (LPVOID)GetWindowLong(hwnd, 0) ) );
#else
	GlobalFree(LOWORD(GlobalHandle(HIWORD(GetWindowLong(hwnd, 0)))));
#endif
	SetWindowLong(hwnd, 0, (LONG)NULL);
	}


/* -------------------------------------------------------------------	*/
/* Returns an HWND that can be used as a message recicipent. The		*/
/* window is invisible.	 Somewhat like "object windows" in PM.			*/
/* -------------------------------------------------------------------	*/
HWND wmhandler_hwnd(char *name)
{
	extern HANDLE __hPrevInst;	// in ARGCARGV
	extern HANDLE __hInst;
	HWND hWnd;
	CREATEPARAMS cp;
		
	if (! __hPrevInst) // NOT VALID! ASSUMES LAST APP DID WMHANDLER_HWND()
		{
		WNDCLASS wndclass;
		memset(&wndclass, 0, sizeof(wndclass));
      wndclass.lpfnWndProc = (WNDPROC)wmhandler_wndproc;
		wndclass.hInstance = __hInst;
		wndclass.lpszClassName = "OBJECTWND";
		wndclass.cbWndExtra = 8;
		RegisterClass(&wndclass);
		}
	
	if (! (cp.wmTab = wmhandler_create()))
		return NULL;
	
	/* MAYBE SHOULD HAVE ICON, WITH EXIT/ABOUT... */

	if (! (hWnd = CreateWindow("OBJECTWND", name, WS_DISABLED,
		0, 0, 0, 0, NULL, NULL, __hInst, (LPSTR) &cp)))
		return NULL;
	
	return hWnd;
	}

/* -------------------------------------------------------------------	*/
/* This is where messages get received and dispatched. In cooperative	*/
/* multitasking of Windows, this function MUST be called periodically.	*/
/* Same as winio_yield(); probably ought to have been here instead.		*/
/* -------------------------------------------------------------------	*/
void wmhandler_yield(void)
	{
	MSG msg;
#ifdef POWER_UNFRIENDLY
	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
#else
#ifdef PRE_APP_TIMER
	if (InSendMessage())
		return;
	for (;;)
		{
		if (! GetMessage(&msg, NULL, 0, 0))
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (! PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
			break;
		}
#else
	if (GetMessage(&msg, NULL, 0, 0))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
#endif
#endif
	}
		
