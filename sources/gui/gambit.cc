//
// FILE: gambit.cc -- Main program for Gambit GUI
//
// $Id$
//

#include <string.h>
#include <ctype.h>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/wizard.h"
#include "wx/image.h"

#include "gambit.h"
#include "guishare/wxmisc.h"
#include "guishare/dlabout.h"
#include "splash.h"
#include "efgshow.h"
#include "nfgshow.h"
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

class Game {
public:
  Efg::Game *m_efg;
  EfgShow *m_efgShow;
  Nfg *m_nfg;
  NfgShow *m_nfgShow;
  gText m_fileName;

  Game(Efg::Game *p_efg) : m_efg(p_efg), m_efgShow(0), m_nfg(0), m_nfgShow(0) { }
  Game(Nfg *p_nfg) : m_efg(0), m_efgShow(0), m_nfg(p_nfg), m_nfgShow(0) { }
};

GambitApp::GambitApp(void)
  : m_fileHistory(5)
{ }

bool GambitApp::OnInit(void)
{
  wxConfig config("Gambit");
  m_fileHistory.Load(config);

  Splash *splash = new Splash(2);
  splash->Show(true);
  while (splash->IsShown()) {
    wxYield();
  }

  const long c_defaultFrameWidth = 400;
  const long c_defaultFrameHeight = 300;
  long frameWidth, frameHeight;
  config.Read("/Sizes/MainFrameWidth", &frameWidth,
	      c_defaultFrameWidth);
  config.Read("/Sizes/MainFrameHeight", &frameHeight,
	      c_defaultFrameHeight);
  
  FullEfg *efg = new FullEfg;
  efg->NewPlayer()->SetName("Player 1");
  efg->NewPlayer()->SetName("Player 2");
  efg->SetTitle("Untitled Extensive Form Game");
  EfgShow *efgShow = new EfgShow(*efg, 0);
  efgShow->SetFilename("");
  AddGame(efg, efgShow);

  // Set up the help system.
  //  m_help.SetTempDir(".");
  wxInitAllImageHandlers();
  // m_help.AddBook("help/guiman.hhp");
  // m_help.AddBook("help/gclman.hhp");

  // Set up the error handling functions.
  // For some reason this does not work w/ BC++ (crash on exit)
#ifndef __BORLANDC__ 
  signal(SIGFPE, (fptr) SigFPEHandler);
#endif
    
  // Process command line arguments, if any.
#ifdef UNUSED
  if (argc > 1) { 
    gambitFrame->LoadFile(argv[1]);
  }
#endif  // UNUSED

  // Set current directory.
  SetCurrentDir(wxGetWorkingDirectory());

  return true;
}

GambitApp::~GambitApp()
{
  wxConfig config("Gambit");
  m_fileHistory.Save(config);

  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfg != 0) {
      delete m_gameList[i]->m_nfg;
    }
    if (m_gameList[i]->m_efg != 0) {
      delete m_gameList[i]->m_efg;
    }
    delete m_gameList[i];
  }
}

class NewGameTypePage : public wxWizardPage {
private:
  wxRadioBox *m_gameType;
  wxWizardPage *m_efgPage, *m_nfgPage;

public:
  NewGameTypePage(wxWizard *, wxWizardPage *, wxWizardPage *);

  bool CreateEfg(void) const { return (m_gameType->GetSelection() == 0); }

  wxWizardPage *GetPrev(void) const { return 0; }
  wxWizardPage *GetNext(void) const
    { return (CreateEfg()) ? m_efgPage : m_nfgPage; }
};

NewGameTypePage::NewGameTypePage(wxWizard *p_parent,
				 wxWizardPage *p_efgPage,
				 wxWizardPage *p_nfgPage)
  : wxWizardPage(p_parent), m_efgPage(p_efgPage), m_nfgPage(p_nfgPage)
{
  SetAutoLayout(true);

  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(this, -1,
			      "Step 1: Select the representation to work with"),
	     0, wxALL | wxCENTER, 10);

  wxString typeChoices[] = { "Extensive form", "Normal form" };
  m_gameType = new wxRadioBox(this, -1, "Representation",
			      wxDefaultPosition, wxDefaultSize,
			      2, typeChoices);
  sizer->Add(m_gameType, 0, wxALL | wxCENTER, 10);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);

  Layout();
}

const int idADDPLAYER_BUTTON = 2001;
const int idDELETEPLAYER_BUTTON = 2002;

class NfgPlayersPage : public wxWizardPageSimple {
private:
  wxButton *m_addButton, *m_deleteButton;
  wxGrid *m_nameGrid;

  // Event handlers
  void OnAddPlayer(wxCommandEvent &);
  void OnDeletePlayer(wxCommandEvent &);

public:
  NfgPlayersPage(wxWizard *);

  int NumPlayers(void) const { return m_nameGrid->GetRows(); }
  gText GetName(int) const;
  gArray<int> NumStrats(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(NfgPlayersPage, wxWizardPageSimple)
  EVT_BUTTON(idADDPLAYER_BUTTON, NfgPlayersPage::OnAddPlayer)
  EVT_BUTTON(idDELETEPLAYER_BUTTON, NfgPlayersPage::OnDeletePlayer)
END_EVENT_TABLE()

NfgPlayersPage::NfgPlayersPage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{
  SetAutoLayout(true);
  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_addButton = new wxButton(this, idADDPLAYER_BUTTON, "Add player");
  buttonSizer->Add(m_addButton, 0, wxALL, 5);
  m_deleteButton = new wxButton(this, idDELETEPLAYER_BUTTON, "Delete player");
  m_deleteButton->Enable(false);
  buttonSizer->Add(m_deleteButton, 0, wxALL, 5);

  m_nameGrid = new wxGrid(this, -1, wxDefaultPosition, wxSize(250, 200));
  m_nameGrid->CreateGrid(2, 2);
  m_nameGrid->SetLabelValue(wxHORIZONTAL, "Label", 0);
  m_nameGrid->SetLabelValue(wxHORIZONTAL, "Strategies", 1);
  m_nameGrid->SetLabelAlignment(wxVERTICAL, wxCENTRE);
  m_nameGrid->SetCellValue("Player1", 0, 0);
  m_nameGrid->SetCellValue("Player2", 1, 0);
  m_nameGrid->SetCellValue("2", 0, 1);
  m_nameGrid->SetCellValue("2", 1, 1);
  m_nameGrid->DisableDragRowSize();
  m_nameGrid->DisableDragColSize();

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(new wxStaticText(this, -1,
				 "Step 2: Define the players for the game"),
		0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(m_nameGrid, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

void NfgPlayersPage::OnAddPlayer(wxCommandEvent &)
{
  m_nameGrid->AppendRows();
  m_nameGrid->AdjustScrollbars();
  m_nameGrid->SetCellValue((char *) ("Player" + ToText(m_nameGrid->GetRows())),
			   m_nameGrid->GetRows() - 1, 0);
  m_nameGrid->SetCellValue("2", m_nameGrid->GetRows() - 1, 1);
  m_deleteButton->Enable(true);
}

void NfgPlayersPage::OnDeletePlayer(wxCommandEvent &)
{
  m_nameGrid->DeleteRows(m_nameGrid->GetCursorRow());
  m_nameGrid->AdjustScrollbars();
  m_deleteButton->Enable(m_nameGrid->GetRows() > 2);
}

gText NfgPlayersPage::GetName(int p_player) const
{
  return m_nameGrid->GetCellValue(p_player - 1, 0).c_str();
}

gArray<int> NfgPlayersPage::NumStrats(void) const
{
  gArray<int> numStrats(m_nameGrid->GetRows());
  for (int pl = 1; pl <= numStrats.Length(); pl++) {
    numStrats[pl] = atoi(m_nameGrid->GetCellValue(pl - 1, 1));
  }

  return numStrats;
}

class EfgPlayersPage : public wxWizardPageSimple {
private:
  wxButton *m_addButton, *m_deleteButton;
  wxGrid *m_nameGrid;

  // Event handlers
  void OnAddPlayer(wxCommandEvent &);
  void OnDeletePlayer(wxCommandEvent &);

public:
  EfgPlayersPage(wxWizard *);

  int NumPlayers(void) const { return m_nameGrid->GetRows(); }
  gText GetName(int) const;
  gArray<int> NumStrats(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(EfgPlayersPage, wxWizardPageSimple)
  EVT_BUTTON(idADDPLAYER_BUTTON, EfgPlayersPage::OnAddPlayer)
  EVT_BUTTON(idDELETEPLAYER_BUTTON, EfgPlayersPage::OnDeletePlayer)
END_EVENT_TABLE()

EfgPlayersPage::EfgPlayersPage(wxWizard *p_parent)
  : wxWizardPageSimple(p_parent)
{
  SetAutoLayout(true);
  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_addButton = new wxButton(this, idADDPLAYER_BUTTON, "Add player");
  buttonSizer->Add(m_addButton, 0, wxALL, 5);
  m_deleteButton = new wxButton(this, idDELETEPLAYER_BUTTON, "Delete player");
  m_deleteButton->Enable(false);
  buttonSizer->Add(m_deleteButton, 0, wxALL, 5);

  m_nameGrid = new wxGrid(this, -1, wxDefaultPosition, wxSize(250, 200));
  m_nameGrid->CreateGrid(2, 1);
  m_nameGrid->SetLabelValue(wxHORIZONTAL, "Label", 0);
  m_nameGrid->SetLabelAlignment(wxVERTICAL, wxCENTRE);
  m_nameGrid->SetCellValue("Player1", 0, 0);
  m_nameGrid->SetCellValue("Player2", 1, 0);
  m_nameGrid->DisableDragRowSize();
  m_nameGrid->DisableDragColSize();

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(new wxStaticText(this, -1,
				 "Step 2: Define the players for the game"),
		0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(m_nameGrid, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

void EfgPlayersPage::OnAddPlayer(wxCommandEvent &)
{
  m_nameGrid->AppendRows();
  m_nameGrid->AdjustScrollbars();
  m_nameGrid->SetCellValue((char *) ("Player" + ToText(m_nameGrid->GetRows())),
			   m_nameGrid->GetRows() - 1, 0);
  m_deleteButton->Enable(true);
}

void EfgPlayersPage::OnDeletePlayer(wxCommandEvent &)
{
  m_nameGrid->DeleteRows(m_nameGrid->GetCursorRow());
  m_nameGrid->AdjustScrollbars();
  m_deleteButton->Enable(m_nameGrid->GetRows() > 2);
}

gText EfgPlayersPage::GetName(int p_player) const
{
  return m_nameGrid->GetCellValue(p_player - 1, 0).c_str();
}

void GambitApp::OnFileNew(wxWindow *p_parent)
{
  wxWizard *wizard = wxWizard::Create(p_parent, -1, "Creating a new game");
  EfgPlayersPage *efgPage = new EfgPlayersPage(wizard);
  NfgPlayersPage *nfgPage = new NfgPlayersPage(wizard);
  NewGameTypePage *page1 = new NewGameTypePage(wizard, efgPage, nfgPage);
  efgPage->SetPrev(page1);
  nfgPage->SetPrev(page1);

  if (wizard->RunWizard(page1)) {
    if (page1->CreateEfg()) {
      FullEfg *efg = new FullEfg;
      for (int pl = 1; pl <= efgPage->NumPlayers(); pl++) {
	efg->NewPlayer()->SetName(efgPage->GetName(pl));
      }
      efg->SetTitle("Untitled Extensive Form Game");
      EfgShow *efgShow = new EfgShow(*efg, 0);
      efgShow->SetFilename("");
      AddGame(efg, efgShow);
    }
    else {
      Nfg *nfg = new Nfg(nfgPage->NumStrats());
      for (int pl = 1; pl <= nfgPage->NumPlayers(); pl++) {
	nfg->Players()[pl]->SetName(nfgPage->GetName(pl));
      }
      nfg->SetTitle("Untitled Normal Form Game");
      NfgShow *nfgShow = new NfgShow(*nfg, 0);
      nfgShow->SetFilename("");
      AddGame(nfg, nfgShow);
    }
  }

  wizard->Destroy();
}

void GambitApp::OnFileOpen(wxWindow *p_parent)
{
  wxFileDialog dialog(p_parent, "Choose file", "", "", "*.?fg");

  if (dialog.ShowModal() == wxID_OK) {
    SetCurrentDir(wxPathOnly(dialog.GetPath()));
    LoadFile(dialog.GetPath());
  }
}

void GambitApp::OnFileMRUFile(wxCommandEvent &p_event)
{
  LoadFile(m_fileHistory.GetHistoryFile(p_event.GetId() - wxID_FILE1));
}

void GambitApp::OnHelpContents(void)
{
  //  HelpController().DisplaySection("Main page");
}

void GambitApp::OnHelpIndex(void)
{
  //  HelpController().DisplayContents();
}

void GambitApp::OnHelpAbout(wxWindow *p_parent)
{
  dialogAbout dialog(p_parent, "About Gambit...",
		     "Gambit Graphical User Interface",
		     "Version 0.97.0.0 (special for CBMS conference)");
  dialog.ShowModal();
}

void GambitApp::LoadFile(const wxString &p_filename)
{    
  try {
    gFileInput infile(p_filename);
    Nfg *nfg = 0;

    ReadNfgFile(infile, nfg);

    if (nfg) {
      m_fileHistory.AddFileToHistory(p_filename);
      NfgShow *nfgShow = new NfgShow(*nfg, 0);
      nfgShow->SetFilename(p_filename);
      AddGame(nfg, nfgShow);
      SetFilename(nfgShow, p_filename);
      return;
    }
  }
  catch (gFileInput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open '%s' for reading",
				  p_filename.c_str()),
		 "Error", wxOK, 0);
    return;
  }

  try {
    gFileInput infile(p_filename);
    FullEfg *efg = ReadEfgFile(infile);
                
    if (!efg) {
      wxMessageBox(wxString::Format("'%s' is not in a recognized format.",
				    p_filename.c_str()),
		   "Error", wxOK, 0);
      return;
    }

    m_fileHistory.AddFileToHistory(p_filename);
    EfgShow *efgShow = new EfgShow(*efg, 0);
    efgShow->SetFilename(p_filename);
    AddGame(efg, efgShow);
    SetFilename(efgShow, p_filename);
  }
  catch (gFileInput::OpenFailed &) { 
    wxMessageBox(wxString::Format("Could not open '%s' for reading",
				  p_filename.c_str()),
		 "Error", wxOK, 0);
    return;
  }
}


IMPLEMENT_APP(GambitApp)

void GambitApp::AddGame(Efg::Game *p_efg, EfgShow *p_efgShow)
{
  Game *game = new Game(p_efg);
  game->m_efgShow = p_efgShow;
  m_gameList.Append(game);
  m_fileHistory.UseMenu(p_efgShow->GetMenuBar()->GetMenu(0));
  m_fileHistory.AddFilesToMenu(p_efgShow->GetMenuBar()->GetMenu(0));
}

void GambitApp::AddGame(Nfg *p_nfg, NfgShow *p_nfgShow)
{
  Game *game = new Game(p_nfg);
  game->m_nfgShow = p_nfgShow;
  m_gameList.Append(game);
  m_fileHistory.UseMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
  m_fileHistory.AddFilesToMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
}

void GambitApp::AddGame(Efg::Game *p_efg, Nfg *p_nfg, NfgShow *p_nfgShow)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efg == p_efg) {
      m_gameList[i]->m_nfg = p_nfg;
      m_gameList[i]->m_nfgShow = p_nfgShow;
      break;
    }
  }
  m_fileHistory.UseMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
  m_fileHistory.AddFilesToMenu(p_nfgShow->GetMenuBar()->GetMenu(0));
}

void GambitApp::RemoveGame(Efg::Game *p_efg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efg == p_efg) {
      m_fileHistory.RemoveMenu(m_gameList[i]->m_efgShow->GetMenuBar()->GetMenu(0));
      if (m_gameList[i]->m_nfg) {
	m_fileHistory.RemoveMenu(m_gameList[i]->m_nfgShow->GetMenuBar()->GetMenu(0));
	m_gameList[i]->m_nfgShow->Close();
	delete m_gameList[i]->m_nfg;
      }
      delete m_gameList.Remove(i);
      delete p_efg;
      break;
    }
  }
}

void GambitApp::RemoveGame(Nfg *p_nfg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfg == p_nfg) {
      m_fileHistory.RemoveMenu(m_gameList[i]->m_nfgShow->GetMenuBar()->GetMenu(0));
      if (m_gameList[i]->m_efg == 0) {
	delete m_gameList.Remove(i);
      }
      else {
	m_gameList[i]->m_nfg = 0;
	m_gameList[i]->m_nfgShow = 0;
      }
      delete p_nfg;
    }
  }
}

EfgShow *GambitApp::GetWindow(const Efg::Game *p_efg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efg == p_efg) {
      return m_gameList[i]->m_efgShow;
    }
  }
  return 0;
}

NfgShow *GambitApp::GetWindow(const Nfg *p_nfg)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfg == p_nfg) {
      return m_gameList[i]->m_nfgShow;
    }
  }
  return 0;
}

void GambitApp::SetFilename(EfgShow *p_efgShow, const wxString &p_file)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_efgShow == p_efgShow) {
      m_gameList[i]->m_fileName = p_file;
      return;
    }
  }
}

void GambitApp::SetFilename(NfgShow *p_nfgShow, const wxString &p_file)
{
  for (int i = 1; i <= m_gameList.Length(); i++) {
    if (m_gameList[i]->m_nfgShow == p_nfgShow) {
      m_gameList[i]->m_fileName = p_file;
      return;
    }
  }
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

#include "base/garray.imp"
#include "base/gblock.imp"

template class gArray<Game *>;
template class gBlock<Game *>;
