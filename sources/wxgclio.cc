// File: wxgclio.cc -- Creates a GUI shell to simulate console IO for the Gambit
// command line.  The class gWxGclIo may come in useful anywhere we need to use
// console IO in a gui enviroment.  Note that you must define NO_GIO when compiling
// gambitio.cc to avoid duplicate symbols.
// $Id$
#include "wx.h"
#pragma hdr_stop
#include "wxgclio.h"

void gcl_main(void);

class gWxGclIoApp: public wxApp
{
	public:
		wxFrame *OnInit(void);
};

gWxGclIoApp GclIo;
gWxGclIo gclio;
wxFrame *gWxGclIoApp::OnInit(void)
{
wxFrame *frame=gclio.Create();
gclio<<"Gambit GCL / Windows interface Ready\n";
gcl_main();
gclio<<"*********** Gambit GCL Terminated. Close the window *************\n";
return frame;
}

gInput &gin=gclio;
gOutput &gout=gclio;
gOutput &gerr=gclio;


