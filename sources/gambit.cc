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
#include "nfggui.h"
#include "efggui.h"
#include <signal.h>
#include <math.h>

#ifdef _AIX
extern wxApp *wxTheApp=1;
#endif
GambitApp gambitApp;

typedef void ( *fptr)(int);
void SigFPEHandler(int type)
{
signal(SIGFPE, (fptr)SigFPEHandler);  //  reinstall signal handler
wxMessageBox("A floating point error has occured!\nThe results returned may be invalid");
}

#define MATH_CONTINUE	0
#define	MATH_IGNORE		1
#define	MATH_QUIT			2
class MathErrHandl : public wxDialogBox
{
private:
	wxRadioBox *opt_box;
	int opt;
	static void ok_func(wxButton &ob,wxEvent &)
	{((MathErrHandl *)ob.GetClientData())->OnOk();}
	void OnOk(void)
	{opt=opt_box->GetSelection();Show(FALSE);}
public:
	MathErrHandl(const char *err):wxDialogBox(0,"Numerical Error",TRUE)
	{
	char *options[3]={"Continue","Ignore","Quit"};
	new wxMessage(this,(char *)err);
	this->NewLine();
	opt_box=new wxRadioBox(this,0,"What now?",-1,-1,-1,-1,3,options,1,wxVERTICAL);
	this->NewLine();
	wxButton *ok=new wxButton(this,(wxFunction)ok_func,"OK");
	ok->SetClientData((char *)this);
	Fit();
	ok->Centre(wxHORIZONTAL);
	Show(TRUE);
	}
	int Option(void) {return opt;}
};

/*
#ifdef wx_msw // this handler is defined differently under win/unix
int _RTLENTRY _matherr (struct exception *e)
#else
#ifdef _LINUX
struct exception {char *name;double arg1,arg2;int type;};
#endif
int matherr(struct exception *e)
#endif
{
static char *whyS [] =
{
		"argument domain error",
		"argument singularity ",
		"overflow range error ",
		"underflow range error",
		"total loss of significance",
		"partial loss of significance"
};
static option=MATH_CONTINUE;
char errMsg[ 80 ];
if (option!=MATH_IGNORE)
{
sprintf (errMsg,
			"%s (%8g,%8g): %s\n", e->name, e->arg1, e->arg2, whyS [e->type - 1]);
MathErrHandl E(errMsg);
option=E.Option();
if (option==MATH_QUIT) wxExit();
}
return 1;		// we did not really fix anything, but want no more warnings
}
*/

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
#include "bitmaps/open.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/efg.xpm"
#include "bitmaps/nfg.xpm"
wxBitmap *ToolbarHelpBitmap=new wxBitmap(help_xpm);
wxBitmap *ToolbarNfgBitmap=new wxBitmap(nfg_xpm);
wxBitmap *ToolbarEfgBitmap=new wxBitmap(efg_xpm);
wxBitmap *ToolbarOpenBitmap=new wxBitmap(open_xpm);
// Open | Efg,Nfg | Help
// Create the toolbar
SetMargins(2, 2);
#ifdef wx_msw
SetDefaultSize(33,30);
#endif
GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);
AddTool(FILE_LOAD, ToolbarOpenBitmap);
AddSeparator();
AddTool(FILE_NEW_EFG, ToolbarEfgBitmap);
AddTool(FILE_NEW_NFG, ToolbarNfgBitmap);
AddSeparator();
AddTool(GAMBIT_HELP_CONTENTS, ToolbarHelpBitmap);
Layout();
}

Bool GambitToolBar::OnLeftClick(int tool, Bool )
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
// First check if we have a current settings file (gambit.ini).  If not, exit!
int ver;
wxGetResource("Gambit","Gambit-Version",&ver,"gambit.ini");
if (ver!=GAMBIT_VERSION)
{
	wxMessageBox("Gambit is unable to locate a current configuration file.\nPlease make "
								"sure that the program was installed correctly","Config Error");
	return 0;
}
// Create the main frame window
GambitFrame *gambit_frame = new GambitFrame(NULL, "Gambit", 0, 0, 200,150,wxDEFAULT_FRAME);
// Give it an icon
wxIcon *frame_icon;
#ifdef wx_msw
	frame_icon = new wxIcon("gambit_icn");
#else
	#include "bitmaps/gambi.xpm"
	frame_icon = new wxIcon(gambi_xpm);
#endif
gambit_frame->SetIcon(frame_icon);

// Make a menubar
wxMenu *file_menu = new wxMenu;
	wxMenu *new_menu=new wxMenu;
		new_menu->Append(FILE_NEW_NFG,"Normal",						"Normal form game");
		new_menu->Append(FILE_NEW_EFG,"Extensive",					"Extensive form game");
	file_menu->Append(FILE_NEW,"&New",new_menu,						"Create a new game");
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
wxInitHelp("gambit","Gambit -- Graphics User Interface, Version 0.94\n\nDeveloped by Richard D. McKelvey (rdm@hss.caltech.edu)\nMain Programmer:  Theodore Turocy (magyar@hss.caltech.edu)\nFront End: Eugene Grayver (egrayver@hss.caltech.edu)\nCalifornia Institute of Technology, 1996.\nFunding provided by the National Science Foundation");
// Init the output (floating point) precision
int num_prec;
wxGetResource("Gambit","Output-Precision",&num_prec,"gambit.ini");
ToTextPrecision(num_prec);

gambit_frame->Show(TRUE);
// Set up the error handling functions.
#ifndef __BORLANDC__ // For some reason this does not work w/ BC++ (crash on exit)
signal(SIGFPE, (fptr)SigFPEHandler);
#endif

// Process command line arguments, if any
if (argc>1) gambit_frame->LoadFile(argv[1]);

// Return the main frame window
return gambit_frame;
}

int GambitApp::OnExit(void)
{
#ifndef _LINUX // there is no global wx_frame in wxxt(linux)
if (wx_frame) wx_frame->OnClose();
#endif
return TRUE;
}
// Define my frame constructor
GambitFrame::GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int ):
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
Enable(FALSE); // do not want to allow anything while the dialog is up
if (!s)
	s=wxFileSelector("Load data file", NULL, NULL, NULL, "*.?fg");
Enable(TRUE);
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
		{EfgGUI(0,s,0,this);return;}
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
		case FILE_QUIT: Close();	break;
		case FILE_LOAD:	LoadFile(); break;
#ifndef EFG_ONLY
		case FILE_NEW_NFG: NfgGUI(0,gText(),0,this);	break;
#endif
#ifndef NFG_ONLY
		case FILE_NEW_EFG: EfgGUI(0,gText(),0,this); break;
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
Show(FALSE);
#ifdef wx_x
	wxFlushResources();
#endif
wxKillHelp();
wout->OnClose();werr->OnClose();
return TRUE;
}
