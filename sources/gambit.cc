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

#ifdef __BORLANDC__
#pragma hdrstop
#endif // __BORLANDC__

#include "wxio.h"
#include "gambit.h"
#include "wxmisc.h"
#include "nfggui.h"
#include "efggui.h"
#include "system.h"
#include <signal.h>
#include <math.h>

#ifdef _AIX
extern wxApp *wxTheApp = 1;
#endif


GambitApp   gambitApp;
GambitFrame *main_gambit_frame;

typedef void (*fptr)(int);


void SigFPEHandler(int type)
{
    signal(SIGFPE, (fptr)SigFPEHandler);  // Reinstall signal handler.
    wxMessageBox("A floating point error has occured!\n"
                 "The results returned may be invalid");
}


class MathErrHandl : public wxDialogBox
{
private:
    wxRadioBox *opt_box;
    int opt;
    static void ok_func(wxButton &ob, wxEvent &) 
    { ((MathErrHandl *)ob.GetClientData())->OnOk(); }
    void OnOk(void) { opt = opt_box->GetSelection(); Show(FALSE); }
    
public:
    MathErrHandl(const char *err):
        wxDialogBox(0, "Numerical Error", TRUE)
    {
        char *options[3] = {"Continue", "Ignore", "Quit"};
        (void) new wxMessage(this, (char *)err);
        this->NewLine();
        opt_box = new wxRadioBox(this, 0, "What now?", 
                                 -1, -1, -1, -1, 3, 
                                 options, 1, wxVERTICAL);
        this->NewLine();
        wxButton *ok = new wxButton(this, (wxFunction)ok_func, "OK");
        ok->SetClientData((char *)this);
        Fit();
        ok->Centre(wxHORIZONTAL);
        Show(TRUE);
    }
    
    int Option(void) { return opt; }
};


char *wxStrLwr(char *s)
{
    for (unsigned int i = 0; i < (unsigned int)strlen(s); i++) 
        s[i] = tolower(s[i]);
    
    return s;
}


//=====================================================================
//                       class GambitToolBar
//=====================================================================

class GambitToolBar
#ifdef wx_msw
  : public wxButtonBar {
#else
  : public wxToolBar {
#endif
private:
  wxFrame *parent;
    
public:
  GambitToolBar(wxFrame *frame);
  Bool OnLeftClick(int toolIndex, Bool toggled);
  void OnMouseEnter(int toolIndex);
};


GambitToolBar::GambitToolBar(wxFrame *frame)
#ifdef wx_msw
  : wxButtonBar(frame, 0, 0, -1, -1, 0, wxVERTICAL, 1), parent(frame)
#else
  : wxToolBar(frame, 0, 0, -1, -1, 0, wxHORIZONTAL, 30), parent(frame)
#endif
{
#ifdef wx_msw    
  wxBitmap *loadBitmap = new wxBitmap("OPEN_BITMAP");
  wxBitmap *efgBitmap = new wxBitmap("EFG_BITMAP");
  wxBitmap *nfgBitmap = new wxBitmap("NFG_BITMAP");
  wxBitmap *helpBitmap = new wxBitmap("HELP_BITMAP");
#else
#include "bitmaps/open.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/efg.xpm"
#include "bitmaps/nfg.xpm"
  wxBitmap *loadBitmap = new wxBitmap(open_xpm);
  wxBitmap *efgBitmap  = new wxBitmap(efg_xpm);
  wxBitmap *nfgBitmap  = new wxBitmap(nfg_xpm);
  wxBitmap *helpBitmap = new wxBitmap(help_xpm);
#endif  // wx_msw 
  
  SetMargins(2, 2);
#ifdef wx_msw
  SetDefaultSize(33, 31);
#endif // wx_msw
 
  AddTool(FILE_LOAD, loadBitmap);
  AddSeparator();
  AddTool(FILE_NEW_EFG, efgBitmap);
  AddTool(FILE_NEW_NFG, nfgBitmap);
  AddSeparator();
  AddTool(GAMBIT_HELP_CONTENTS, helpBitmap);

  GetDC()->SetBackground(wxLIGHT_GREY_BRUSH);

  CreateTools();
  Layout();
}

Bool GambitToolBar::OnLeftClick(int tool, Bool)
{
  parent->OnMenuCommand(tool);
  return TRUE;
}

void GambitToolBar::OnMouseEnter(int tool)
{
  parent->SetStatusText(parent->GetMenuBar()->GetHelpString(tool));
}


//---------------------------------------------------------------------
//                     GAMBITFRAME: CONSTRUCTOR
//---------------------------------------------------------------------

static gText ResolveVersion(void)
{
#ifdef __BORLANDC__
  gText resourceFile = "gambit.ini";
  gText firstPath = System::GetEnv("GAMBITHOME");

  if (firstPath != "") {
    int ver = 0;
    wxGetResource("Gambit", "Gambit-Version", &ver,
                  gText(firstPath) + "/" + resourceFile);

    if (ver == GAMBIT_VERSION)
      return gText(firstPath) + "/" + resourceFile;
  }

  char *workingDir = wxGetWorkingDirectory();
  if (workingDir)  {
    int ver = 0;
    wxGetResource("Gambit", "Gambit-Version", &ver,
                  gText(workingDir) + "/" + resourceFile);

    if (ver == GAMBIT_VERSION)
      return gText(workingDir) + "/" + resourceFile;
  }

  int ver = 0;
  wxGetResource("Gambit", "Gambit-Version", &ver, resourceFile);
  if (ver == GAMBIT_VERSION)
    return resourceFile;

  return "";

#else   // non-microsoft platforms
  int ver = 0;
  wxGetResource("Gambit", "Gambit-Version", &ver, "gambitrc");
  if (ver == GAMBIT_VERSION)
    return "gambitrc";
  else
    return "";
#endif
}

// The `main program' equivalent, creating the windows and returning the
// main frame

wxFrame *GambitApp::OnInit(void)
{
  // First check if we have a current settings file (gambit.ini).  If not, exit!
  m_resourceFile = ResolveVersion();
  
  if (m_resourceFile == "") {
    wxMessageBox("Gambit is unable to locate a current configuration file.\n"
		 "Please make sure that the program was installed correctly",
		 "Config Error");
    return NULL;
  }

  // Create the main frame window.
  GambitFrame *gambit_frame = new GambitFrame(NULL, "Gambit", 
					      0, 0, 200, 150, wxDEFAULT_FRAME);

  // Give it an icon.
  wxIcon *frame_icon;
    
#ifdef wx_msw
  frame_icon = new wxIcon("gambit_icn");
#else
#include "bitmaps/gambi.xpm"
  frame_icon = new wxIcon(gambi_xpm);
#endif
    
  gambit_frame->SetIcon(frame_icon);
    
  // Make a menubar.
  wxMenu *file_menu = new wxMenu;
  wxMenu *new_menu = new wxMenu;
  new_menu->Append(FILE_NEW_NFG, "Normal",               "Normal form game");
  new_menu->Append(FILE_NEW_EFG, "Extensive",            "Extensive form game");
  file_menu->Append(FILE_NEW,    "&New", new_menu,       "Create a new game");
  file_menu->Append(FILE_LOAD,   "&Open",                "Open a file");
  file_menu->Append(FILE_QUIT,   "&Quit",                "Quit program");

  m_recentFiles = new wxFileHistory(5);
  m_recentFiles->FileHistoryLoad(m_resourceFile, "Gambit");
  m_recentFiles->FileHistoryUseMenu(file_menu);

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(GAMBIT_HELP_CONTENTS, "&Contents",   "Table of contents");
  help_menu->Append(GAMBIT_HELP_ABOUT,    "&About",      "About this program");
  
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, "&File");
  menu_bar->Append(help_menu, "&Help");
  
  // Associate the menu bar with the frame.
  gambit_frame->SetMenuBar(menu_bar);
  gambit_frame->CreateStatusLine();
  (void) new GambitToolBar(gambit_frame);
  
  // Set up the help system.
  gText workingDir = wxGetWorkingDirectory();

  wxInitHelp(workingDir + "/gambit", "Gambit -- Graphics User Interface, Version 0.96\n\n"
	     "Developed by Richard D. McKelvey (rdm@hss.caltech.edu)\n"
	     "Main Programmer:  Theodore Turocy (arbiter@nwu.edu)\n"
	     "Front End: Eugene Grayver (egrayver@hss.caltech.edu)\n"
	     "California Institute of Technology, 1996-9.\n"
	     "Funding provided by the National Science Foundation");
  
  // Initialize the output (floating point) precision.
  int num_prec;
  wxGetResource("Gambit", "Output-Precision", &num_prec,
		gambitApp.ResourceFile());
  ToTextPrecision(num_prec);
  
  gambit_frame->Show(TRUE);
  
  // Set up the error handling functions.
#ifndef __BORLANDC__ // For some reason this does not work w/ BC++ (crash on exit)
  signal(SIGFPE, (fptr)SigFPEHandler);
#endif
    
  // Process command line arguments, if any.
  if (argc > 1) 
    gambit_frame->LoadFile(argv[1]);
  
  // Set current directory.

  gambitApp.SetCurrentDir(gText(wxGetWorkingDirectory()));
  
  // Return the main frame window.
  main_gambit_frame = gambit_frame;
  return gambit_frame;
}


int GambitApp::OnExit(void)
{
#ifndef LINUX_WXXT // there is no global wx_frame in wxxt(linux)
  if (wx_frame) 
    wx_frame->OnClose();
#endif

  m_recentFiles->FileHistorySave(ResourceFile(), "Gambit");
  return TRUE;
}


// Define my frame constructor.
GambitFrame::GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int )
  : wxFrame(frame, title, x, y, w, h)
{ }


//--------------------------------------------------------------------
//              GAMBITFRAME: EVENT-HANDLING MEMBERS
//--------------------------------------------------------------------

//********************************************************************
//                       FILE-LOAD MENU HANDLER
//********************************************************************

void GambitFrame::LoadFile(char *s)
{    
  if (!s) {
    Enable(FALSE); // Don't allow anything while the dialog is up.

    s = wxFileSelector("Load data file", gambitApp.CurrentDir(),
		       NULL, NULL, "*.?fg");

    Enable(TRUE);

    if (!s)
      return;

    // Save the current directory.
    // WARNING: since wxFileSelector returns the address of
    // a global buffer in wxxt, we have to copy s to a new
    // location.  This is probably also a memory leak.
    char *new_s = copystring(s);
    gText path(gPathOnly(s));
    gambitApp.SetCurrentDir(path);
    s = new_s;
  }
  
  if (strcmp(s, "") != 0) {
    gText filename(gFileNameFromPath(s));
    filename = filename.Dncase();

    if (strstr((const char *) filename, ".nfg")) {
      // This must be a normal form.
      NfgGUI(0, s, 0, this);
      return;
    }
    else if (strstr((const char *) filename, ".efg")) {
      // This must be an extensive form.
      EfgGUI(0, s, 0, this);
      return;
    }
    
    wxMessageBox("Unknown file type");
  }

  // RDM:  I don't think the following should be here.  
  //    delete [] s;
}


//*******************************************************************
//                    COMMAND EVENT HANDLER
//*******************************************************************

void GambitFrame::OnMenuCommand(int id)
{
  switch (id) {
  case FILE_QUIT:
    Close();
    break;
        
  case FILE_LOAD:
    LoadFile();
    break;
        
  case FILE_NEW_NFG: 
    NfgGUI(0, "", 0, this);
    break;

  case FILE_NEW_EFG: 
    EfgGUI(0, "", 0, this);
    break;
        
  case GAMBIT_HELP_ABOUT:
    wxHelpAbout(); 
    break;
        
  case GAMBIT_HELP_CONTENTS: 
    wxHelpContents(GAMBIT_GUI_HELP);
    break;
        
  default: 
    wxMessageBox("Error: Unknown Menu Selection"); 
    break;
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
  wout->OnClose(); werr->OnClose();
  return TRUE;
}


//
// A general-purpose dialog box to display the description of the exception
//
void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
            long p_style /*= wxOK | wxCENTRE*/)
{
  gText message = "An internal error occurred in Gambit:\n" + p_message;
  wxMessageBox(message, "Gambit Error", p_style, p_parent);
}


