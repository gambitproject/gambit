//
// FILE: WinEdit.h -- main header file for the WINEDIT application
//
// $Id$
//

#if !defined(AFX_WINEDIT_H__51F1F616_9928_11D1_BCE8_006008049675__INCLUDED_)
#define AFX_WINEDIT_H__51F1F616_9928_11D1_BCE8_006008049675__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols



void WinGetArgs(int *argc,char ***argv); // taken from the wx_win code
int gcl_main( int argc, char* argv[] );



/////////////////////////////////////////////////////////////////////////////
// CWinEditApp:
// See WinEdit.cpp for the implementation of this class
//

#include <new.h>

#include "gtext.h"
#include "glist.h"
#include "gsm.h"
#include "gstack.h"
#include "gcompile.h"
#include "gcmdline.h"
#include "gpreproc.h"

class mfcCmdLineInput : public gCmdLineInput {
private:
  virtual char GetNextChar(void);

public:
  mfcCmdLineInput(int p_historyDepth);
  virtual ~mfcCmdLineInput() { }
};

class CWinEditApp : public CWinApp {
private:
   mfcCmdLineInput gcmdline;
   GCLCompiler C;
   gPreprocessor P;

public:
	CWinEditApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinEditApp)
	public:
	virtual BOOL InitInstance();
  virtual BOOL SaveAllModified();
  virtual BOOL OnIdle(LONG lcount);
//	virtual int Run();
	//}}AFX_VIRTUAL

   void ProcessMessages( void );

// Implementation

	//{{AFX_MSG(CWinEditApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINEDIT_H__51F1F616_9928_11D1_BCE8_006008049675__INCLUDED_)
