#include "wx.h"
#include "gstatus.h"

char tmp_str[100];
class wxStatus: public gStatus,public wxDialogBox
{
private:
	// Stuff for signal
	bool	signal;
	static void cancel_func(wxButton &ob,wxCommandEvent &ev)
	{((wxStatus *)ob.GetClientData())->SetSignal();}
	// stuff for progress
	wxGauge *gauge;
	wxTextWindow *twin;
	int Width,Prec;
	char Represent;

public:
	// Constructor
	wxStatus(const char *label=0,wxFrame *parent=0);
	// functions for gProgress::gOutput
	int GetWidth(void) {return Width;}
	gOutput &SetWidth(int w) {Width=w;return *this;}
	int GetPrec(void) {return Prec;}
	gOutput &SetPrec(int p) {Prec=p;return *this;}
	gOutput &SetExpMode(void) {Represent='e';return *this;}
	gOutput &SetFloatMode(void){Represent='f';return *this;}
	char GetRepMode(void){return Represent;}

	gOutput &operator<<(int x)
	{sprintf(tmp_str, "%*d", Width,  x);(*twin)<<tmp_str;return *this;}
	gOutput &operator<<(unsigned int x)
	{sprintf(tmp_str, "%*d", Width,  x);(*twin)<<tmp_str;return *this;}
	gOutput &operator<<(long x)
	{sprintf(tmp_str, "%*ld", Width, x);(*twin)<<tmp_str;return *this;}
	gOutput &operator<<(char x)
	{sprintf(tmp_str, "%c", x);(*twin)<<tmp_str;return *this;}
	gOutput &operator<<(double x)
	{
		switch (Represent) {
		case 'f':
			sprintf(tmp_str, "%*.*lf", Width, Prec, x);
			(*twin)<<tmp_str;
			break;
		case 'e':
			sprintf(tmp_str, "%*.*le", Width, Prec, x);
			(*twin)<<tmp_str;
	}
	return *this;
	}
	gOutput &operator<<(float x)
	{
	switch (Represent) {
		case 'f':
			sprintf(tmp_str, "%*.*f", Width, Prec, x);
			(*twin)<<tmp_str;
			break;
		case 'e':
			sprintf(tmp_str, "%*.*e", Width, Prec, x);
			(*twin)<<tmp_str;
			break;
		}
	return *this;
	}
	gOutput &operator<<(const char *x)
	{sprintf(tmp_str, "%s", x);(*twin)<<tmp_str;return *this;}
	gOutput &operator<<(const void *x)
	{sprintf(tmp_str, "%p", x);(*twin)<<tmp_str;return *this;}

	bool IsValid(void) const {return true;}
	// functions for gProgress
	void	SetProgress(double p) {gauge->SetValue((int)(p*100));}
	// functions for gSignal
	void	SetSignal() {signal=true;}
	bool Get(void) const {return signal;}
	void Reset(void) {signal=false;}
};

wxStatus::wxStatus(const char *name,wxFrame *parent):wxDialogBox(parent,(char *)name)
{
Reset();

Width=0;Prec=6;Represent='f';
// Build the dialog box
gauge=new wxGauge(this,"",100,-1,-1,300,-1,wxVERTICAL);
NewLine();
int x,y;
GetCursor(&x,&y);
x=GetHorizontalSpacing();y*=GetVerticalSpacing();
wxTextWindow *twin=new wxTextWindow(this,x,y,300,100,wxREADONLY|wxBORDER);
NewLine();
wxButton *cancel=new wxButton(this,(wxFunction)cancel_func,"Cancel");
cancel->SetClientData((char *)this);
Fit();
cancel->Centre();
twin->Centre(wxHORIZONTAL);
Show(TRUE);
}
