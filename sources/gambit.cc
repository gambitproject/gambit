//
// FILE: gambit.cc -- Main program for Gambit GUI
//
// @(#)gambit.cc	1.4 4/7/94
//

#pragma hdrstop
#include "wx.h"
#include "wx_help.h"
#include "extform.h"
#include "const.h"
#include "gambit.h"
#include "normgui.h"
#include "extgui.h"

GambitFrame   *gambit_frame = NULL;
wxHelpInstance *help_instance = NULL;
wxList 		my_children;
wxCursor *arrow_cursor;
#ifdef _AIX
extern wxApp *wxTheApp=1;
#endif
GambitApp gambitApp;
//---------------------------------------------------------------------
//                     GAMBITFRAME: CONSTRUCTOR
//---------------------------------------------------------------------

// The `main program' equivalent, creating the windows and returning the
// main frame
wxFrame *GambitApp::OnInit(void)
{
arrow_cursor = new wxCursor(wxCURSOR_ARROW);
// Create the main frame window
gambit_frame = new GambitFrame(NULL, "Gambit", 0, 0, 200, 100,wxMDI_PARENT | wxDEFAULT_FRAME);
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
	help_menu->Append(HELP_GAMBIT,"&Contents",						"Table of contents");
	help_menu->Append(HELP_ABOUT,"&About",								"About this program");

wxMenuBar *menu_bar = new wxMenuBar;
menu_bar->Append(file_menu, "&File");
menu_bar->Append(help_menu,	"&Help");

// Associate the menu bar with the frame
gambit_frame->SetMenuBar(menu_bar);
// Process command line arguments, if any
gambit_frame->Show(TRUE);
if (argc>1) gambit_frame->file_load(argv[1]);

// Return the main frame window
return gambit_frame;
}

// Define my frame constructor
GambitFrame::GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type):
	wxFrame(frame, title, x, y, w, h)
{}

//--------------------------------------------------------------------
//              GAMBITFRAME: EVENT-HANDLING HOOK MEMBERS
//--------------------------------------------------------------------

//********************************************************************
//                       FILE-LOAD MENU HANDLER
//********************************************************************

void GambitFrame::file_load(char *s)
{
if (!s) s=copystring(wxFileSelector("Load data file", NULL, NULL, NULL, "*.?fg"));
if (s)
{
	char *filename=FileNameFromPath(s);
#ifndef EFG_ONLY
	if (StringMatch(".nfg",filename))		// This must be a normal form
		{NormalFormGUI(0,s,0,this);return;}
#endif
#ifndef NFG_ONLY
	if (StringMatch(".efg",filename))		// This must be an extensive form
		{ExtensiveFormGUI(0,s,0,this);return;}
#endif
	wxMessageBox("Unknown file type");	// If we got here, there is something wrong
	delete [] s;
}
}

//*******************************************************************
//                    COMMAND EVENT HANDLER
//*******************************************************************

void GambitFrame::OnMenuCommand(int id)
{
	switch (id)  {
		case FILE_QUIT:
			OnClose();
			delete this;
			break;
		case FILE_LOAD:
			file_load();
			break;
#ifndef EFG_ONLY
		case FILE_NEW_NFG:
			NormalFormGUI(0,gString(),0,this);
			break;
#endif
#ifndef NFG_ONLY
		case FILE_NEW_EFG:
			ExtensiveFormGUI(0,gString(),0,this);
			break;
#endif
		case HELP_ABOUT:
			(void)wxMessageBox("Gambit Front End\nAuthor: Eugene Grayver egrayver@cco.caltech.edu\n(c) Caltech EPS, 1994", "About Gambit");
			break;
		case HELP_GAMBIT:
			if (help_instance==NULL)
			{
				help_instance = new wxHelpInstance(TRUE);
				help_instance->Initialize("gambit");
			}
			help_instance->LoadFile("gambit");
			help_instance->DisplayContents();
			break;
		default:
			(void)wxMessageBox("Internal Error!\nContact the author\negrayver@cco.caltech.edu","Error");
			break;
	}
}

Bool GambitFrame::OnClose()
{
	if (help_instance) help_instance->Quit();
	return TRUE;
}
