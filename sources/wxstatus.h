// File: wxstatus.h -- definition of the class to implement progress 
// indication/cancel feature for the gambit algorithms in the gui.
// $Id$
#include "wx.h"
#include "gstatus.h"
class wxStatus: public wxFrame,public gStatus
{
private:
	int Width,Prec;
	char Represent;
	wxGauge *gauge;
	wxTextWindow *twin;
	bool sig;
	static void button_proc(wxButton& but, wxCommandEvent& event);
public:
	wxStatus(wxFrame *frame,const char *title="Progress", int x=300, int y=300, int w=300, int h=250);
	// functions for gProgress::gOutput
	int GetWidth(void) {return Width;}
	gOutput &SetWidth(int w) {Width=w;return *this;}
	int GetPrec(void) {return Prec;}
	gOutput &SetPrec(int p) {Prec=p;return *this;}
	gOutput &SetExpMode(void) {Represent='e';return *this;}
	gOutput &SetFloatMode(void){Represent='f';return *this;}
	char GetRepMode(void){return Represent;}

	gOutput &operator<<(int x);
	gOutput &operator<<(unsigned int x);
	gOutput &operator<<(long x);
	gOutput &operator<<(char x);
	gOutput &operator<<(double x);
	gOutput &operator<<(float x);
	gOutput &operator<<(const char *x);
	gOutput &operator<<(const void *x);

	bool IsValid(void) const {return true;}
	// functions for gProgress
	void	SetProgress(double p);
	// functions for gSignal
	void	SetSignal(void) {sig=true;}
	bool Get(void) const {return sig;}
	void Reset(void) {sig=false;}
	// Window event handlers
	void OnSize(int w, int h);
	Bool OnClose(void);
	void OnActivate(Bool) {}
};

