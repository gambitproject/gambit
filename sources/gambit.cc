//
// FILE: gambit.cc -- Main program for Gambit GUI
//
// $Id$
//

#include <string.h>
#include <ctype.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif // __BORLANDC__

#include "gambit.h"
#include "wxmisc.h"
#include "efgshow.h"
#include "nfgshow.h"
#include "system.h"
#include <signal.h>
#include <math.h>

typedef void (*fptr)(int);


void SigFPEHandler(int type)
{
  signal(SIGFPE, (fptr)SigFPEHandler);  // Reinstall signal handler.
  wxMessageBox("A floating point error has occured!\n"
	       "The results returned may be invalid");
}

//
// FIXME: Figure out how to handle math errors!
//
#ifdef UNUSED
class MathErrHandl : public wxDialog {
private:
  wxRadioBox *opt_box;
  int opt;
  static void ok_func(wxButton &ob, wxEvent &) 
    { ((MathErrHandl *)ob.GetClientData())->OnOk(); }
  void OnOk(void) { opt = opt_box->GetSelection(); Show(FALSE); }
    
public:
  MathErrHandl(const char *err);
  int Option(void) { return opt; }
};

MathErrHandl::MathErrHandl(const char *err)
  : wxDialog(0, 0, "Numerical Error")
{
  char *options[3] = {"Continue", "Ignore", "Quit"};
  /*
  wxMessage(this, (char *)err);
  this->NewLine();
  opt_box = new wxRadioBox(this, 0, "What now?", 
			   -1, -1, -1, -1, 3, 
			   options, 1, wxVERTICAL);
  this->NewLine();
  wxButton *ok = new wxButton(this, (wxFunction)ok_func, "OK");
  ok->SetClientData((char *)this);
  Fit();
  ok->Centre(wxHORIZONTAL);
  */
  ShowModal();
}
    
#ifdef __WXMSW__ // this handler is defined differently under win/unix
const int MATH_CONTINUE = 0;
const int MATH_IGNORE = 1;
const int MATH_QUIT = 2;
int _RTLENTRY _matherr (struct exception *e)
#else
#ifdef _LINUX
struct exception { char *name; double arg1, arg2; int type; };
#endif
int matherr(struct exception *e)
#endif
{
  static char *whyS[] = { "argument domain error",
                          "argument singularity ",
			  "overflow range error ",
			  "underflow range error",
			  "total loss of significance",
			  "partial loss of significance" };

  static int option = MATH_CONTINUE;
  char errMsg[80];
  if (option != MATH_IGNORE)   {
  sprintf (errMsg, "%s (%8g,%8g): %s\n",
	   e->name, e->arg1, e->arg2, whyS [e->type - 1]);
  MathErrHandl E(errMsg);
  option = E.Option();
  if (option==MATH_QUIT)   {
    wxExit();
  }
  // we did not really fix anything, but want no more warnings
  return 1;	
}
#endif  // UNUSED

bool GambitApp::OnInit(void)
{
  wxConfig config("Gambit");

  // Create the main frame window.
  GambitFrame *gambitFrame = new GambitFrame(0, "Gambit", 
					      wxPoint(0, 0), wxSize(200, 150));

  // Set up the help system.
  wxString helpDir = wxGetWorkingDirectory();
  config.Read("Help-Directory", &helpDir);

  wxInitHelp(gText(helpDir.c_str()) + "/gambit", 
	     "Gambit Graphics User Interface, Version 0.96.3\n"
	     "Built with " wxVERSION_STRING "\n\n"
	     "Part of the Gambit Project\n"
	     "www.hss.caltech.edu/~gambit/Gambit.html\n"
	     "gambit@hss.caltech.edu\n\n"
	     "Copyright (C) 1999-2000\n"
	     "California Institute of Technology\n"
	     "Funding provided by the National Science Foundation");
  
  gambitFrame->Show(true);

  // Set up the error handling functions.
  // For some reason this does not work w/ BC++ (crash on exit)
#ifndef __BORLANDC__ 
  signal(SIGFPE, (fptr) SigFPEHandler);
#endif
    
  // Process command line arguments, if any.
  if (argc > 1) { 
    gambitFrame->LoadFile(argv[1]);
  }

  // Set current directory.
  wxGetApp().SetCurrentDir(gText(wxGetWorkingDirectory()));

  return true;
}

IMPLEMENT_APP(GambitApp)

//=====================================================================
//                       class GambitToolbar
//=====================================================================

const int GAMBIT_TOOLBAR_ID = 101;

class GambitToolbar : public wxToolBar {
private:
  wxFrame *m_parent;

  // Event handlers
  void OnMouseEnter(wxCommandEvent &);
    
public:
  GambitToolbar(wxFrame *, const wxPoint &, const wxSize &);
  virtual ~GambitToolbar() { }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GambitToolbar, wxToolBar)
  EVT_TOOL_ENTER(GAMBIT_TOOLBAR_ID, GambitToolbar::OnMouseEnter)
END_EVENT_TABLE()

GambitToolbar::GambitToolbar(wxFrame *p_parent, const wxPoint &p_position,
			     const wxSize &p_size)
  : wxToolBar(p_parent, GAMBIT_TOOLBAR_ID, p_position, p_size),
    m_parent(p_parent)
{
#ifdef __WXMSW__
  wxBitmap loadBitmap("OPEN_BITMAP");
  wxBitmap efgBitmap("EFG_BITMAP");
  wxBitmap nfgBitmap("NFG_BITMAP");
  wxBitmap helpBitmap("HELP_BITMAP");
#else
#include "bitmaps/open.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/efg.xpm"
#include "bitmaps/nfg.xpm"
  wxBitmap loadBitmap(open_xpm);
  wxBitmap efgBitmap(efg_xpm);
  wxBitmap nfgBitmap(nfg_xpm);
  wxBitmap helpBitmap(help_xpm);
#endif  // __WXMSW__
  
  SetMargins(2, 2);
#ifdef __WXMSW__
  SetToolBitmapSize(wxSize(33, 30));
#endif  // __WXMSW__
  AddTool(FILE_OPEN, loadBitmap);
  AddSeparator();
  AddTool(FILE_NEW_EFG, efgBitmap);
  AddTool(FILE_NEW_NFG, nfgBitmap);
  AddSeparator();
  AddTool(GAMBIT_HELP_CONTENTS, helpBitmap);

  Realize();
}

void GambitToolbar::OnMouseEnter(wxCommandEvent &p_event)
{
  if (p_event.GetSelection() > 0) {
    m_parent->SetStatusText(m_parent->GetMenuBar()->GetHelpString(p_event.GetSelection()));
  }
  else {
    m_parent->SetStatusText("");
  }
}

//=====================================================================
//                       class GambitFrame
//=====================================================================

GambitFrame::GambitFrame(wxFrame *p_parent, const wxString &p_title,
			 const wxPoint &p_position, const wxSize &p_size)
  : wxFrame(p_parent, -1, p_title, p_position, p_size),
    m_fileHistory(5)
{
#ifdef __WXMSW__
  SetIcon(wxIcon("gambit_icn"));
#else
#include "bitmaps/gambi.xpm"
  SetIcon(wxIcon(gambi_xpm));
#endif
    
  wxMenu *fileMenu = new wxMenu;
  wxMenu *newMenu = new wxMenu;
  newMenu->Append(FILE_NEW_NFG, "&Normal", "Create a new normal form game");
  newMenu->Append(FILE_NEW_EFG, "&Extensive",
		  "Create a new extensive form game");
  fileMenu->Append(FILE_NEW, "&New", newMenu, "Create a new game");
  fileMenu->Append(FILE_OPEN, "&Open\tCtrl-O", "Open a saved game");
  fileMenu->AppendSeparator();
  fileMenu->Append(FILE_QUIT, "&Quit\tCtrl-X", "Quit Gambit");

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(GAMBIT_HELP_CONTENTS, "&Contents\tF1", "Table of contents");
  helpMenu->Append(GAMBIT_HELP_ABOUT, "&About", "About Gambit");
  
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(helpMenu, "&Help");
  SetMenuBar(menuBar);

 wxAcceleratorEntry entries[3];
  entries[0].Set(wxACCEL_CTRL, (int) 'O', FILE_OPEN);
  entries[1].Set(wxACCEL_CTRL, (int) 'X', FILE_QUIT);
  entries[2].Set(wxACCEL_NORMAL, WXK_F1, GAMBIT_HELP_CONTENTS);
  wxAcceleratorTable accel(3, entries);
  SetAcceleratorTable(accel);

  wxConfig config("Gambit");
  m_fileHistory.Load(config);
  m_fileHistory.UseMenu(fileMenu);
  m_fileHistory.AddFilesToMenu();

  CreateStatusBar();

  (void) new GambitToolbar(this, wxPoint(0, 0), wxSize(200, 40));
}

GambitFrame::~GambitFrame()
{
  wxConfig config("Gambit");
  m_fileHistory.Save(config);
}

//--------------------------------------------------------------------
//              GambitFrame: Event-handling members
//--------------------------------------------------------------------

BEGIN_EVENT_TABLE(GambitFrame, wxFrame)
  EVT_MENU(FILE_NEW_EFG, GambitFrame::OnNewEfg)
  EVT_MENU(FILE_NEW_NFG, GambitFrame::OnNewNfg)
  EVT_MENU(FILE_OPEN, GambitFrame::OnLoad)
  EVT_MENU(FILE_QUIT, wxWindow::Close)
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, GambitFrame::OnMRUFile)
  EVT_MENU(GAMBIT_HELP_CONTENTS, GambitFrame::OnHelpContents)
  EVT_MENU(GAMBIT_HELP_ABOUT, GambitFrame::OnHelpAbout)
  EVT_CLOSE(GambitFrame::OnCloseWindow)
END_EVENT_TABLE()

void GambitFrame::OnNewNfg(wxCommandEvent &)
{
  int numPlayers = GetPlayers();
  if (numPlayers >= 2) {
    gArray<int> dimensionality(numPlayers);
    if (GetStrategies(dimensionality)) {
      Nfg *nfg = new Nfg(dimensionality);
      NfgShow *nfgShow = new NfgShow(*nfg, 0, this);
      nfgShow->SetFileName("");
    }
  }
}

void GambitFrame::OnNewEfg(wxCommandEvent &)
{
  EfgShow *efgShow = new EfgShow(*(new FullEfg), 0, this);
  efgShow->SetFileName("");
}

void GambitFrame::OnLoad(wxCommandEvent &)
{
  Enable(false); // Don't allow anything while the dialog is up.

  gText filename = wxFileSelector("Load data file", wxGetApp().CurrentDir(),
				  NULL, NULL, "*.?fg").c_str();
  Enable(true);

  if (filename == "") {
    return;
  }

  wxGetApp().SetCurrentDir(gPathOnly(filename));

  LoadFile(filename);
}

void GambitFrame::OnMRUFile(wxCommandEvent &p_event)
{
  LoadFile(m_fileHistory.GetHistoryFile(p_event.GetSelection() - wxID_FILE1).c_str());
}

void GambitFrame::OnHelpAbout(wxCommandEvent &)
{
  wxHelpAbout(); 
}

void GambitFrame::OnHelpContents(wxCommandEvent &)
{
  wxHelpContents(GAMBIT_GUI_HELP);
}

class dialogDimensionality : public guiPagedDialog {
public:
  dialogDimensionality(wxWindow *p_parent, int p_numPlayers);
  virtual ~dialogDimensionality() { }
};

dialogDimensionality::dialogDimensionality(wxWindow *p_parent,
					   int p_numPlayers)
  : guiPagedDialog(p_parent, "Number of Strategies", p_numPlayers)
{
  for (int pl = 1; pl <= p_numPlayers; pl++) {
    SetValue(pl, "2");
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_grid, 0, wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

int GambitFrame::GetPlayers(void)
{
  int numPlayers = 2;

  const char *label = wxGetTextFromUser("Number of players",
					"Create new normal form",
					(char *) ToText(numPlayers));
  if (label) {
    numPlayers = ToNumber(label);
    if (numPlayers < 1) {
      return 0;
    }
    else {
      return numPlayers;
    }
  }
  else {
    return 0;
  }
}
 
int GambitFrame::GetStrategies(gArray<int> &p_dimensionality)
{
  dialogDimensionality dialog(this, p_dimensionality.Length());
  
  if (dialog.ShowModal() == wxID_OK) {
    for (int pl = 1; pl <= p_dimensionality.Length(); pl++) {
      p_dimensionality[pl] = ToNumber(dialog.GetValue(pl));
    }
    return 1;
  }
  else {
    return 0;
  }
}

void GambitFrame::LoadFile(const gText &p_filename)
{    
  gText filename(gFileNameFromPath(p_filename));
  filename = filename.Dncase();

  if (strstr((const char *) filename, ".nfg")) {
    // This must be a normal form.
    try {
      gFileInput infile(p_filename);
      Nfg *nfg = 0;

      ReadNfgFile(infile, nfg);

      if (!nfg) {
	wxMessageBox((char *) (p_filename + " is not a valid .nfg file"));
      }
      else {
	m_fileHistory.AddFileToHistory((char *) p_filename);
      }
      NfgShow *nfgShow = new NfgShow(*nfg, 0, this);
      nfgShow->SetFileName(p_filename);
      return;
    }
    catch (gFileInput::OpenFailed &) {
      wxMessageBox((char *) ("Could not open " + p_filename + " for reading"));
      return;
    }
  }
  else if (strstr((const char *) filename, ".efg")) {
    // This must be an extensive form.
    try {
      gFileInput infile(p_filename);
      FullEfg *efg = ReadEfgFile(infile);
                
      if (!efg) {
	wxMessageBox((char *) (filename + " is not a valid .efg file"));
      }
      else {
	m_fileHistory.AddFileToHistory((char *) p_filename);
      }

      EfgShow *efgShow = new EfgShow(*efg, 0, this);
      efgShow->SetFileName(filename);
      return;
    }
    catch (gFileInput::OpenFailed &) { 
      wxMessageBox((char *) ("Could not open " + filename + " for reading"));
      return;
    }
  }

  wxMessageBox("Unknown file type");
}

void GambitFrame::OnCloseWindow(wxCloseEvent &)
{
  wxKillHelp();
  //  wout->OnClose(); werr->OnClose();
  Destroy();
}


//
// A general-purpose dialog box to display the description of the exception
//
void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
            long p_style /*= wxOK | wxCENTRE*/)
{
  gText message = "An internal error occurred in Gambit:\n" + p_message;
  wxMessageBox((char *) message, "Gambit Error", p_style, p_parent);
}


