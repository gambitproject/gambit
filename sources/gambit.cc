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
#include <signal.h>
#include <math.h>

// Bitmaps.

#include "bitmaps/open.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/efg.xpm"
#include "bitmaps/nfg.xpm"

#ifndef wx_msw
#include "bitmaps/gambi.xpm"
#endif

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


class GambitToolBar:    // No reason to have yet another .h file for just this.
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
    parent = frame;
    
    // Load palette bitmaps
    
    wxBitmap *ToolbarHelpBitmap = new wxBitmap(help_xpm);
    wxBitmap *ToolbarNfgBitmap  = new wxBitmap(nfg_xpm);
    wxBitmap *ToolbarEfgBitmap  = new wxBitmap(efg_xpm);
    wxBitmap *ToolbarOpenBitmap = new wxBitmap(open_xpm);
    
    // Open | Efg, Nfg | Help
    // Create the toolbar
    
    SetMargins(2, 2);
    
#ifdef wx_msw
    SetDefaultSize(33, 30);
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

// The `main program' equivalent, creating the windows and returning the
// main frame

wxFrame *GambitApp::OnInit(void)
{
    // First check if we have a current settings file (gambit.ini).  If not, exit!
    
    int ver;
    wxGetResource("Gambit", "Gambit-Version", &ver, "gambit.ini");
    
    if (ver != GAMBIT_VERSION)
    {
        wxMessageBox("Gambit is unable to locate a current configuration file.\n"
                     "Please make sure that the program was installed correctly",
                     "Config Error");
        return NULL;
    }

	//
	// Initialize GUI record/playback globals.
	//

	gui_recorder    = new GuiRecorder;
	gui_recorder_db = new GuiRecorderDatabase;
	gui_playback    = new GuiPlayback;

    // ----------------------------------------------------------------------
    // Check command-line options.  These are used in the GUI record/playback
    // system.  If relevant options are found, they are removed from the 
    // argc/argv list.
    // ----------------------------------------------------------------------

    char usage[256];
    sprintf(usage, "Usage: %s [-record <log-file>] [-playback <log-file>]\n" 
            "(only one of -record or -playback may be selected)", wxApp::argv[0]);

    gText playback_filename;
    bool  playback_requested = false;
    bool  record_requested   = false;
    bool  options_found      = false;

    int rpindex=0;  // index into argc/argv list for record/playback options.

    for (int i = 0; i < wxApp::argc; i++)
    {
        if (strcmp(argv[i], "-record") == 0)
        {
            if (i == (wxApp::argc - 1)) // No log file in argument list.
            {
                wxMessageBox(usage);
                return NULL;
            }

            rpindex = i;
            i++;

            gui_recorder->openFile(argv[i]);

            record_requested = true;
            options_found = true;
        }
        else if (strcmp(argv[i], "-playback") == 0)
        {
            if (i == (wxApp::argc - 1)) // No log file in argument list.
            {
                wxMessageBox(usage);
                return NULL;
            }

            rpindex = i;
            i++;

            playback_filename  = wxApp::argv[i];
            playback_requested = true;
            options_found = true;
        }

        // Ignore other command-line options here.
    }

    // Signal an error if both playback and record were requested.

    if (playback_requested && record_requested)
    {
        wxMessageBox("Error! Record and playback options cannot both be selected!");
        return NULL;
    }

    // If options have been found they have to be removed from the 
    // argc/argv list.

    if (options_found)
    {
        assert(rpindex >= 1);

        for (int j = rpindex; j < wxApp::argc - 2; j++)
        {
            wxApp::argv[j] = wxApp::argv[j + 2];
        }

        wxApp::argc -= 2;
    }

    // ----------------------------------------------------------------------
    // End of command-line options-handling code.
    // ----------------------------------------------------------------------

    // Create the main frame window.
    GambitFrame *gambit_frame = new GambitFrame(NULL, "Gambit", 
                                                0, 0, 200, 150, wxDEFAULT_FRAME);

    // Give it an icon.
    wxIcon *frame_icon;
    
#ifdef wx_msw
    frame_icon = new wxIcon("gambit_icn");
#else
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
    wxInitHelp("gambit", "Gambit -- Graphics User Interface, Version 0.94\n\n"
               "Developed by Richard D. McKelvey (rdm@hss.caltech.edu)\n"
               "Main Programmer:  Theodore Turocy (magyar@hss.caltech.edu)\n"
               "Front End: Eugene Grayver (egrayver@hss.caltech.edu)\n"
               "California Institute of Technology, 1996.\n"
               "Funding provided by the National Science Foundation");
    
    // Initialize the output (floating point) precision.
    int num_prec;
    wxGetResource("Gambit", "Output-Precision", &num_prec, "gambit.ini");
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

    // If playing back a log file, read in the log file and
    // execute the log file commands one by one.

    if (playback_requested)
    {
        try
        {
            gui_playback->Playback(playback_filename);
        }
        catch (gException &e)
        {
            gout << "EXCEPTION: " << e.Description() << '\n';
        }
    }

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
    return TRUE;
}


// Define my frame constructor.
GambitFrame::GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int )
    : wxFrame(frame, title, x, y, w, h), GuiObject(gText("GambitFrame"))
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
    if (GUI_PLAYBACK) {
      gText arg = GUI_READ_ARG("GambitFrame::LoadFile", 1);
      s = copystring((char *) arg);
    }
    else {
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
      
      GUI_RECORD_ARG("GambitFrame::LoadFile", 1, s);
    }
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
    switch (id)
    {
    case FILE_QUIT:
        GUI_RECORD("FILE:QUIT");
        GUI_RECORDER_CLOSE
        Close();    
        break;
        
    case FILE_LOAD:
        GUI_RECORD("FILE:LOAD");
        LoadFile(); 
        break;
        
    case FILE_NEW_NFG: 
        GUI_RECORD("FILE:NEW_NFG");
        NfgGUI(0, gText(), 0, this);   
        break;

    case FILE_NEW_EFG: 
        GUI_RECORD("FILE:NEW_EFG");
        EfgGUI(0, gText(), 0, this); 
        break;
        
    case GAMBIT_HELP_ABOUT:
        // No logging for help system.
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


// Gui playback code:

void GambitFrame::ExecuteLoggedCommand(const gText& command,
                                       const gList<gText>& arglist)
{
#ifdef GUIPB_DEBUG
    printf("in GambitFrame::ExecuteLoggedCommand...\n");
    printf("command: %s\n", (char *)command);

    for (int i = 1; i <= arglist.Length(); i++)
        printf("arglist[%d] = %s\n", i, (char *)arglist[i]);
#endif

    // FIXME! add more commands.

    if (command == "FILE:QUIT")
    {
        Close();
    }
    else if (command == "FILE:LOAD")
    {
        LoadFile();
    }
    else
    {
        throw InvalidCommand();
    }
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


