// WinEdit.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WinEdit.h"

#include "MainFrm.h"
#include "WinEditDoc.h"
#include "WinEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

mfcCmdLineInput::mfcCmdLineInput(int p_historyDepth)
  : gCmdLineInput(p_historyDepth)
{ }

char mfcCmdLineInput::GetNextChar(void)
{
  return ((CWinEditView*) ((CMainFrame*) AfxGetMainWnd())->GetActiveView())->GetChar();
}

/////////////////////////////////////////////////////////////////////////////
// CWinEditApp

BEGIN_MESSAGE_MAP(CWinEditApp, CWinApp)
	//{{AFX_MSG_MAP(CWinEditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinEditApp construction

extern void gclNewHandler(void);
extern gCmdLineInput &gcmdline;
extern GSM* _gsm;
extern char* _SourceDir;
extern char* _ExePath;

CWinEditApp::CWinEditApp()
: gcmdline(20), C(), P(&gcmdline, "Include[\"gclini.gcl\"]")
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance


  set_new_handler(gclNewHandler);
  int argc;
  char** argv;
  WinGetArgs( &argc, &argv );
  try {
    _ExePath = new char[strlen(argv[0]) + 2];
    // Apparently, Win95 surrounds the program name with explicit quotes;
    // if this occurs, special case code
    if (argv[0][0] == '"') {
      strncpy(_ExePath, argv[0] + 1, strlen(argv[0]) - 2);
    }
    else {
      strcpy(_ExePath, argv[0]);
    }
    const char SLASH = '\\';

    char *c = strrchr( _ExePath, SLASH );

    _SourceDir = new char[256];
    if (c != NULL)  {
      int len = strlen(_ExePath) - strlen(c);
      if (len >= 256)  len = 255;
      strncpy(_SourceDir, _ExePath, len);
    }
    else   {
      strcpy(_SourceDir, "");
    }
    _gsm = new GSM;
  }
  catch (gException &w)  {
    gout << "GCL EXCEPTION:" << w.Description() << "; Caught in gcl.cc, main()\n";
    return;
  }

}

BOOL CWinEditApp::OnIdle(LONG lcount)
{
  if(CWinApp::OnIdle(lcount)) return TRUE;
  try {
//    while (!P.eof()) {
    if (!P.eof()) {
      gText line = P.GetLine();
      gText fileName = P.GetFileName();
      int lineNumber = P.GetLineNumber();
      gText rawLine = P.GetRawLine();
      BeginWaitCursor();
      C.Parse(line, fileName, lineNumber, rawLine );
      EndWaitCursor();
    }
  //    delete[] _SourceDir;
  //    delete _gsm;
  }

  catch (gclQuitOccurred &E) {
    EndWaitCursor();
//    CWinApp::HideApplication();
//    CWinApp::CloseAllDocuments(TRUE);
    exit( ExitInstance() );
    return E.Value();
  }
  // The last line of defense for exceptions:
  catch (gException &w)  {
    gout << "GCL EXCEPTION:" << w.Description() << "; Caught in gcl.cc, main()\n";
    return 1;
  }
  return 1;
}


/////////////////////////////////////////////////////////////////////////////
// The one and only CWinEditApp object

CWinEditApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinEditApp initialization

BOOL CWinEditApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Gambit Project: Gambit Command Language"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWinEditDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWinEditView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

BOOL CWinEditApp::SaveAllModified()  { return 1; }

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CWinEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CWinEditApp commands

#include "gstream.h"

/*
int CWinEditApp::Run()
{
	// TODO: Add your specialized code here and/or call the base class


  int argc;
  char** argv;
  WinGetArgs( &argc, &argv );
  return gcl_main( argc, argv );
}
*/

void CWinEditApp::ProcessMessages( void )
{
  ASSERT_VALID(this);

/*
	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;

  // phase1: check to see if we can do idle work
	while (bIdle &&
		!::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE))
	{
		// call OnIdle while in bIdle state
		if (!OnIdle(lIdleCount++))
			bIdle = FALSE; // assume "no idle" state
	}
*/
	// phase2: pump messages while available
	do
	{
		// pump message, but quit on WM_QUIT
		if (!PumpMessage())
			exit( ExitInstance() );
			// reset "no idle" state after pumping "normal" message
/*
		if (IsIdleMessage(&m_msgCur))
		{
			bIdle = TRUE;
			lIdleCount = 0;
		}
*/
	} while (::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE));

}

void WinGetArgs(int *argc,char ***argv) // taken from the wx_win code
{
  CString m_lpCmdLine = GetCommandLine();

	// Split command line into tokens, as in usual main(argc, argv)
	char **command = (char **)malloc(sizeof(char *)*50);
	int count = 0;
	char *buf = (char *)malloc(sizeof(char)*(strlen(m_lpCmdLine) + 1));

	// Hangs around until end of app. in case
	// user carries pointers to the tokens

	/* Model independent strcpy */
	{
		int i;
		for (i = 0; (buf[i] = m_lpCmdLine[i]) != 0; i++)
		{
			/* loop */;
		}
	}

	/* Break-up string */
	{
		char *token;
		const char *IFS = " \t\r\n";
		if ((token = strtok(buf, IFS)) != NULL) {
			do {
				if (*token != '\0' && strchr(IFS, *token) == NULL)
					command[count++] = token;
			} while ((token = strtok(NULL, IFS)) != NULL);
		}
	}
	command[count] = NULL; /* argv[] is NULL terminated list! */
	*argv=command;
	*argc=count;
}


//
// Provide default input and output streams
//

#include "gwstream.h"

gFileInput _gin(stdin);
gInput &gin = _gin;

gWinOutput _gout;
gOutput &gout = _gout;

gWinOutput _gerr;
gOutput &gerr = _gerr;
