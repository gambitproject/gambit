//
// FILE: wxgcl.cc -- Experimental wxWindows front-end for GCL
//
// $Id$
//

#include "wx/wx.h"
#include "wx/splitter.h"

#include "gsm.h"
#include "gcmdline.h"
#include "gpreproc.h"
#include "gcompile.h"

GSM* _gsm;
char* _SourceDir = NULL;
char* _ExePath = NULL;

class wxCommandLine : public gclCommandLine {
protected:
  virtual char GetNextChar(void) { return '\n'; }

public:
  wxCommandLine(int p_historyDepth) : gclCommandLine(p_historyDepth) { }
  virtual ~wxCommandLine() { }

  void SetPrompt(bool) { }

  virtual bool eof(void) const { return true; }
  virtual gInput &operator>>(int &) { return *this; }
  virtual gInput &operator>>(unsigned int &) { return *this; }
  virtual gInput &operator>>(long int &) { return *this; }
  virtual gInput &operator>>(char &) { return *this; }
  virtual gInput &operator>>(double &) { return *this; }
  virtual gInput &operator>>(float &) { return *this; }
  virtual gInput &operator>>(char *) { return *this; }
  virtual int get(char &) { return 0; }
  virtual void unget(char) { }
  virtual void seekp(long int) const { }
  virtual long getpos(void) const { return 0L; }
  virtual void setpos(long) const { }
};

class wxOutputWindowStream : public gOutput {
private:
  wxTextCtrl *m_window;

public:
  wxOutputWindowStream(wxTextCtrl *p_window);
  virtual ~wxOutputWindowStream() { }

  virtual gOutput &operator<<(int x);
  virtual gOutput &operator<<(unsigned int x);
  virtual gOutput &operator<<(bool x);
  virtual gOutput &operator<<(long x);
  virtual gOutput &operator<<(char x);
  virtual gOutput &operator<<(double x);
  virtual gOutput &operator<<(float x);
  virtual gOutput &operator<<(const char *x);
  virtual gOutput &operator<<(const void *x);

  virtual int GetWidth(void) const { return 2; }
  virtual gOutput &SetWidth(int w) { return *this; }
  virtual int GetPrec(void) const { return 2; }
  virtual gOutput &SetPrec(int p) { return *this; } 
  virtual gOutput &SetExpMode(void) { return *this; }
  virtual gOutput &SetFloatMode(void) { return *this; }
  virtual char GetRepMode(void) const { return 'f'; }
};

wxOutputWindowStream::wxOutputWindowStream(wxTextCtrl *p_window)
  : m_window(p_window)
{ }

gOutput &wxOutputWindowStream::operator<<(int x)
{
  m_window->AppendText((char *) ToText(x));
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(unsigned int x)
{
  m_window->AppendText((char *) ToText((int) x));
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(bool x)
{
  m_window->AppendText((x) ? "True" : "False");
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(long x)
{
  m_window->AppendText((char *) ToText(x));
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(char x)
{
  m_window->AppendText(x);
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(double x)
{
  m_window->AppendText((char *) ToText(x));
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(float x)
{
  m_window->AppendText((char *) ToText(x));
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(const char *x)
{
  m_window->AppendText(x);
  return *this;
}

gOutput &wxOutputWindowStream::operator<<(const void *x)
{
  char buf[10];
  sprintf(buf, "%p", x);
  m_window->AppendText(buf);
  return *this;
}

class GclApp : public wxApp {
private:
  bool OnInit(void);

public:
  virtual ~GclApp() { }
};

DECLARE_APP(GclApp)

IMPLEMENT_APP(GclApp)

class GclFrame : public wxFrame {
private:
  wxTextCtrl *m_outputWindow;
  wxTextCtrl *m_inputWindow;
  wxOutputWindowStream *m_outputStream;

  GCLCompiler m_compiler;

  // Menu event handlers
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);

  void OnTextEnter(wxCommandEvent &);

public:
  GclFrame(wxFrame *p_parent, const wxString &p_title,
	   const wxPoint &p_position, const wxSize &p_size);
  virtual ~GclFrame();

  DECLARE_EVENT_TABLE()
};

bool GclApp::OnInit(void)
{
  (void) new GclFrame(0, "Gambit Command Language",
		      wxPoint(0, 0), wxSize(400, 400));
  return true;
}

const int idINPUT_WINDOW = 1001;

GclFrame::GclFrame(wxFrame *p_parent, const wxString &p_title,
		   const wxPoint &p_position, const wxSize &p_size)
  : wxFrame(p_parent, -1, p_title, p_position, p_size)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_EXIT, "&Quit", "Quit program");
  
  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, "&About", "About this program");
  helpMenu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(helpMenu, "&Help");
  SetMenuBar(menuBar);

  CreateStatusBar();

  wxSplitterWindow *splitter = new wxSplitterWindow(this);
  m_outputWindow = new wxTextCtrl(splitter, -1, "",
				  wxDefaultPosition, wxDefaultSize,
				  wxTE_MULTILINE | wxTE_READONLY);
  m_outputStream = new wxOutputWindowStream(m_outputWindow);
  m_inputWindow = new wxTextCtrl(splitter, idINPUT_WINDOW, "",
				 wxDefaultPosition, wxDefaultSize,
				 wxTE_MULTILINE | wxTE_PROCESS_ENTER);
  splitter->SplitHorizontally(m_outputWindow, m_inputWindow, 300);

  Show(true);

  _gsm = new GSM(gin, *m_outputStream, *m_outputStream);
  wxCommandLine cmdline(20);
  gPreprocessor preproc(&cmdline, "Include[\"gclini.gcl\"]");
  try {
    while (!preproc.eof()) {
      gText line = preproc.GetLine();
      gText fileName = preproc.GetFileName();
      int lineNumber = preproc.GetLineNumber();
      gText rawLine = preproc.GetRawLine();
      m_compiler.Parse(line, fileName, lineNumber, rawLine);
    }
  }
  catch (gclQuitOccurred &) {

  }
  catch (gException &w)  {
    _gsm->OutputStream() << "GCL EXCEPTION:" << w.Description() << "; Caught in GclFrame::GclFrame()\n";
  }
}

GclFrame::~GclFrame()
{ }

BEGIN_EVENT_TABLE(GclFrame, wxFrame)
  EVT_MENU(wxID_EXIT, wxWindow::Close)
  EVT_MENU(wxID_ABOUT, GclFrame::OnHelpAbout)
  EVT_MENU(wxID_HELP_CONTENTS, GclFrame::OnHelpContents)
  EVT_TEXT_ENTER(idINPUT_WINDOW, GclFrame::OnTextEnter)
  EVT_CLOSE(GclFrame::OnCloseWindow)
END_EVENT_TABLE()

void GclFrame::OnHelpAbout(wxCommandEvent &)
{
  wxMessageBox("Gambit Command Language, Version 0.96.3\n"
	       "Built with " wxVERSION_STRING "\n\n"
	       "Part of the Gambit Project\n"
	       "www.hss.caltech.edu/~gambit/Gambit.html\n"
	       "gambit@hss.caltech.edu\n\n"
	       "Copyright (C) 1999-2000\n"
	       "California Institute of Technology\n"
	       "Funding provided by the National Science Foundation",
	       "About...");
}

void GclFrame::OnHelpContents(wxCommandEvent &)
{ }

void GclFrame::OnCloseWindow(wxCloseEvent &)
{
  Destroy();
}

void GclFrame::OnTextEnter(wxCommandEvent &)
{
  m_outputWindow->AppendText(m_inputWindow->GetValue());
  m_outputWindow->AppendText("\n");

  wxCommandLine cmdline(20);
  gPreprocessor preproc(&cmdline,
			gText(m_inputWindow->GetValue().c_str()) + "\n");
  try {
    while (!preproc.eof()) {
      gText line = preproc.GetLine();
      gText fileName = preproc.GetFileName();
      int lineNumber = preproc.GetLineNumber();
      gText rawLine = preproc.GetRawLine();
      m_compiler.Parse(line, fileName, lineNumber, rawLine);
    }
  }
  catch (gclQuitOccurred &) {

  }
  catch (gException &w)  {
    _gsm->OutputStream() << "GCL EXCEPTION:" << w.Description() << "; Caught in GclFrame::OnTextEnter()\n";
  }
  m_inputWindow->Clear();
  m_outputWindow->AppendText("\n");
}

/*
gFileInput _gin(stdin);
gInput &gin = _gin;
*/
