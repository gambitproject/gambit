#include "wx.h"
#include "gstatus.h"

class wxStatus:public gStatus, public wxFrame
{
private:
	// Stuff for sig
	bool	sig;
	static void cancel_func(wxButton &ob,wxEvent &ev)
	{((wxStatus *)ob.GetClientData())->SetSignal();}
	// stuff for progress
	wxGauge *gauge;
	wxTextWindow *twin;
	wxButton *cancel;
	int Width,Prec;
	char Represent;
public:
	// Constructor
	wxStatus(const char *label=0,wxFrame *parent=0);
	// Destructor
	~wxStatus() {Show(FALSE);}
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
	void	SetProgress(double p) {gauge->SetValue((int)(p*100));wxYield();}
	// functions for gSignal
	void	SetSignal(void) {sig=true;}
	bool Get(void) const {return sig;}
	void Reset(void) {sig=false;}
	// Window event handlers
	void OnSize(int, int) {Layout();}
};


