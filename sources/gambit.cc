//
// FILE: gambit.cc -- Main program for Gambit GUI
//
// $Id$
//


#include "wx.h"
#include "wx_form.h"
#include "extform.h"
#include "treewin.h"
#include "const.h"
#include "gambit.h"

GambitFrame   *frame = NULL;
wxList 		my_children;
wxCursor *arrow_cursor;
GambitApp gambitApp;
//---------------------------------------------------------------------
//                     GAMBITFRAME: CONSTRUCTOR
//---------------------------------------------------------------------
#ifdef wx_x
#include "aiai.xbm"
#endif

// The `main program' equivalent, creating the windows and returning the
// main frame
wxFrame *GambitApp::OnInit(void)
{
  // Create a red pen

	// Create a cursor

	arrow_cursor = new wxCursor(wxCURSOR_ARROW);
  // Create the main frame window
	frame = new GambitFrame(NULL, "Gambit", 0, 0, 200, 100,wxMDI_PARENT | wxDEFAULT_FRAME);

  // Give it an icon
wxIcon *test_icon;
#ifdef wx_msw
  test_icon = new wxIcon("aiai_icn");
#endif
#ifdef wx_x
  test_icon = new wxIcon(aiai_bits, aiai_width, aiai_height);
#endif

  frame->SetIcon(test_icon);

  // Make a menubar
	wxMenu *file_menu = new wxMenu;
	file_menu->Append(FILE_NEW,"&New",										"Create new file");
  file_menu->Append(FILE_LOAD,"&Open",									"Open a file");
	file_menu->Append(FILE_QUIT, "&Quit",                	"Quit program");

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, "&File");

  // Associate the menu bar with the frame
  frame->SetMenuBar(menu_bar);

//  wxCursor *cursor = new wxCursor(wxCURSOR_POINTER);
//	frame->SetCursor(cursor);


  // Return the main frame window
  return frame;
}

// Define my frame constructor
GambitFrame::GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type):
  wxFrame(frame, title, x, y, w, h)
{
}

//--------------------------------------------------------------------
//              GAMBITFRAME: EVENT-HANDLING HOOK MEMBERS
//--------------------------------------------------------------------

//********************************************************************
//                       FILE-LOAD MENU HANDLER
//********************************************************************

void GambitFrame::file_load(void)
{
	char *s=wxFileSelector("Load data file", NULL, NULL, NULL, "*.out");

/*
 * The following features aren't implemented in ExtForm yet...
 *

  if (fileOpen->completed())  {
    FILE *f = fopen(fileOpen->name(), "r");
    if (f != NULL)  {
      ExtensiveFrame *ef = new ExtensiveFrame(this,
			   new ExtForm(f, fileOpen->name()));
      ef->show();
    }
  }
*/
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
		case FILE_NEW:
		{
			ExtensiveFrame *ef = new ExtensiveFrame(frame,"NEW",50,50,500,300,wxDEFAULT_FRAME);
			break;
		}
	}
}

Bool GambitFrame::OnClose()
{
	return TRUE;
}
