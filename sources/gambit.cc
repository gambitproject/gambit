//
// FILE: gambit.cc -- Main program for Gambit GUI
//
// $Id$
//
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "wx.h"
#ifdef wx_msw
#include "wx_bbar.h"
#else
#include "wx_tbar.h"
#endif
#pragma hdrstop
#include "wxio.h"
#include "gambit.h"
#include "wxmisc.h"
#include "normgui.h"
#include "extgui.h"
#include <signal.h>
#include <math.h>

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

#ifdef wx_msw // this handler is defined differently under win/unix
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
wxError(errMsg,"Numerical Error");
return 0;
}


char *wxStrLwr(char *s)
{for (int i=0;i<strlen(s);i++) s[i]=tolower(s[i]); return s;}

class GambitToolBar:	// no reason to have yet another .h file for just this
#ifdef wx_msw
								public wxButtonBar
#else
								public wxToolBar
#endif
{
private:
	wxFrame *parent;
public:
	GambitToolBar(wxFrame *frame);
	Bool OnLeftClick(int toolIndex, Bool toggled);
	void OnMouseEnter(int toolIndex);
};


//***************************************************************************
//                EXTENSIVE FORM TOOLBAR
//***************************************************************************

GambitToolBar::GambitToolBar(wxFrame *frame):
#ifdef wx_msw
	wxButtonBar(frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30)
#else
	wxToolBar(frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30)
#endif
{
parent=frame;
// Load palette bitmaps
#ifdef wx_msw
wxBitmap *ToolbarOpenBitmap=new wxBitmap("OPENTOOL");
wxBitmap *ToolbarHelpBitmap=new wxBitmap("HELPTOOL");
wxBitmap *ToolbarNfgBitmap=new wxBitmap("NFGTOOL");
wxBitmap *ToolbarEfgBitmap=new wxBitmap("EFGTOOL");
#endif
#ifdef wx_x
#include "bitmaps/open.xbm"
#include "bitmaps/help.xbm"
#include "bitmaps/efg.xbm"
#include "bitmaps/nfg.xbm"
wxBitmap *ToolbarHelpBitmap=new wxBitmap(HELP_bits,HELP_width,HELP_height);
wxBitmap *ToolbarNfgBitmap=new wxBitmap(NFG_bits,NFG_width,NFG_height);
wxBitmap *ToolbarEfgBitmap=new wxBitmap(EFG_bits,EFG_width,EFG_height);
wxBitmap *ToolbarOpenBitmap=new wxBitmap(OPEN_bits,OPEN_width,OPEN_height);
#endif
// Open | Efg,Nfg | Help
// Create the toolbar
SetMargins(2, 2);
#ifdef wx_msw
SetDefaultSize(33,30);
#endif
GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);
int dx = 3;
int gap = 8;
#ifdef wx_msw
	int width = 38;
#else
	int width = ToolbarOpenBitmap->GetWidth();
#endif
	int currentX = gap;
	AddTool(FILE_LOAD, ToolbarOpenBitmap,NULL,FALSE,(float)currentX,-1,NULL);
	currentX += width + dx+gap;
	AddTool(FILE_NEW_EFG, ToolbarEfgBitmap, NULL,FALSE, (float)currentX, -1, NULL);
	currentX += width + dx;
	AddTool(FILE_NEW_NFG, ToolbarNfgBitmap, NULL,FALSE, (float)currentX, -1, NULL);
	currentX += width + dx+gap;
	AddTool(GAMBIT_HELP_CONTENTS, ToolbarHelpBitmap, NULL,FALSE, (float)currentX, -1, NULL);
}

Bool GambitToolBar::OnLeftClick(int tool, Bool toggled)
{parent->OnMenuCommand(tool);return TRUE;}

void GambitToolBar::OnMouseEnter(int tool)
{parent->SetStatusText(parent->GetMenuBar()->GetHelpString(tool));}


//---------------------------------------------------------------------
//                     GAMBITFRAME: CONSTRUCTOR
//---------------------------------------------------------------------

// The `main program' equivalent, creating the windows and returning the
// main frame
wxFrame *GambitApp::OnInit(void)
{
// Create the main frame window
GambitFrame *gambit_frame = new GambitFrame(NULL, "Gambit", 0, 0, 220, 150,wxDEFAULT_FRAME);
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
gambit_frame->CreateStatusLine();
new GambitToolBar(gambit_frame);

// Set up the help system
wxInitHelp("gambit","Gambit -- Graphics User Interface, Version 0.9\n\nDeveloped by Richard D. McKelvey (rdm@hss.caltech.edu)\nMain Programmer:  Theodore Turocy (magyar@hss.caltech.edu)\nFront End: Eugene Grayver (egrayver@hss.caltech.edu)\nCalifornia Institute of Technology, 1995.\nFunding provided by the National Science Foundation");

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
	char *filename=copystring(FileNameFromPath(s));
	filename=wxStrLwr(filename); // ignore case
#ifndef EFG_ONLY
	if (strstr(filename,".nfg"))		// This must be a normal form
		{NfgGUI(0,s,0,this);return;}
#endif
#ifndef NFG_ONLY
	if (strstr(filename,".efg"))		// This must be an extensive form
		{ExtensiveFormGUI(0,s,0,this);return;}
#endif
printf("Neither an efg or a nfg\n");
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
		default: wxMessageBox("Error: Unknown Menu Selection"); break;
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
