//
// FILE: gambit.cc -- Main program for Gambit GUI
//
// @(#)gambit.cc	1.4 4/7/94
//
#include <assert.h>
#include "wx.h"
#pragma hdrstop
#include "wxio.h"
#include "gambit.h"
#include "wxmisc.h"
#include "normgui.h"
#include "extgui.h"
#include <signal.h>

#ifdef _AIX
extern wxApp *wxTheApp=1;
#endif
GambitApp gambitApp;

typedef void (*fptr)(int);

void SigFPEHandler(int a)
{
signal(SIGFPE, (fptr)SigFPEHandler);  //  reinstall signal handler
wxMessageBox("A floating point error has occured!\nThe results returned may be invalid");
}
//---------------------------------------------------------------------
//                     GAMBITFRAME: CONSTRUCTOR
//---------------------------------------------------------------------

// The `main program' equivalent, creating the windows and returning the
// main frame
wxFrame *GambitApp::OnInit(void)
{
// Create the main frame window
GambitFrame *gambit_frame = new GambitFrame(NULL, "Gambit", 0, 0, 200, 100,wxMDI_PARENT | wxDEFAULT_FRAME);
// Give it an icon
wxIcon *frame_icon;
#ifdef wx_msw
	frame_icon = new wxIcon("gambit_icn");
#else
	#include "gambit.xbm"
	frame_icon = new wxIcon(gambit_bits, gambit_width, gambit_height);
#endif

gambit_frame->SetIcon(frame_icon);

// Make a menubar
wxMenu *file_menu = new wxMenu;
	wxMenu *new_menu=new wxMenu;
		new_menu->Append(FILE_NEW_NFG,"Normal",						"Normal Form file");
		new_menu->Append(FILE_NEW_EFG,"Extensive",					"Extensive Form file");
	file_menu->Append(FILE_NEW,"&New",new_menu,						"Create new file");
	file_menu->Append(FILE_LOAD,"&Open",									"Open a file");
	file_menu->Append(FILE_QUIT, "&Quit",                	"Quit program");
wxMenu *help_menu = new wxMenu;
	help_menu->Append(GAMBIT_HELP_CONTENTS,"&Contents",						"Table of contents");
	help_menu->Append(GAMBIT_HELP_ABOUT,"&About",								"About this program");

wxMenuBar *menu_bar = new wxMenuBar;
menu_bar->Append(file_menu, "&File");
menu_bar->Append(help_menu,	"&Help");

// Associate the menu bar with the frame
gambit_frame->SetMenuBar(menu_bar);

// Set up the help system
wxInitHelp("gambit","Gambit -- Graphics User Interface, Version 2.0\n\nDeveloped by Richard D. McKelvey (rdm@hss.caltech.edu)\nMain Programmer:  Theodore Turocy (magyar@hss.caltech.edu)\nFront End: Eugene Grayver (egrayver@hss.caltech.edu)\nCalifornia Institute of Technology, 1995.\nFunding provided by the National Science Foundation");

gambit_frame->Show(TRUE);
// Set up the error handling functions:
signal(SIGFPE, (fptr)SigFPEHandler);
// Set up the input/output default windows
wout=new gWxOutput(gWXOUT);wout->Show(FALSE);
werr=new gWxOutput(gWXERR);werr->Show(FALSE);

// Process command line arguments, if any
if (argc>1) gambit_frame->LoadFile(argv[1]);

// Return the main frame window
return gambit_frame;
}

// Define my frame constructor
GambitFrame::GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type):
	wxFrame(frame, title, x, y, w, h)
{}

//--------------------------------------------------------------------
//              GAMBITFRAME: EVENT-HANDLING MEMBERS
//--------------------------------------------------------------------

//********************************************************************
//                       FILE-LOAD MENU HANDLER
//********************************************************************

void GambitFrame::LoadFile(char *s)
{
if (!s)
	s=wxFileSelector("Load data file", NULL, NULL, NULL, "*.?fg");
if (!s) return;
s=copystring(s);
if (strcmp(s,"")!=0)
{
	char *filename=FileNameFromPath(s);
#ifndef EFG_ONLY
	if (StringMatch(".nfg",filename))		// This must be a normal form
		{NfgGUI(0,s,0,this);return;}
#endif
#ifndef NFG_ONLY
	if (StringMatch(".efg",filename))		// This must be an extensive form
		{ExtensiveFormGUI(0,s,0,this);return;}
#endif
	wxMessageBox("Unknown file type");	// If we got here, there is something wrong
}
delete [] s;
}

//*******************************************************************
//                    COMMAND EVENT HANDLER
//*******************************************************************

void GambitFrame::OnMenuCommand(int id)
{
	switch (id)
	{
		case FILE_QUIT: OnClose(); delete this;	break;
		case FILE_LOAD:	LoadFile();	break;
#ifndef EFG_ONLY
		case FILE_NEW_NFG: NfgGUI(0,gString(),0,this);	break;
#endif
#ifndef NFG_ONLY
		case FILE_NEW_EFG: ExtensiveFormGUI(0,gString(),0,this); break;
#endif
		case GAMBIT_HELP_ABOUT:	wxHelpAbout(); break;
		case GAMBIT_HELP_CONTENTS: wxHelpContents(GAMBIT_GUI_HELP);	break;
		default: assert(0); break;
	}
}
#ifdef wx_x
extern void wxFlushResources(void);
#endif

Bool GambitFrame::OnClose()
{
#ifdef wx_x
	wxFlushResources();
#endif
	wxKillHelp();
	delete wout;delete werr;
	return TRUE;
}
