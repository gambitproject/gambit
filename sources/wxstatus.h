#include "wx.h"
#include "gstatus.h"

class wxStatus: public gStatus,public wxDialogBox
{
private:
	// Stuff for signal
	bool	signal;
	static void cancel_func(wxButton &ob,wxCommandEvent &ev);
	// stuff for progress
	wxGauge *gauge;
	int Width,Prec;
	char Represent;

public:
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
	{int c=fprintf(f, "%*d", Width,  x);  valid = (c == 1) ? 1 : 0;return *this;}
	gOutput &operator<<(unsigned int x)
	{int c=fprintf(f, "%*d", Width,  x);return *this;}
	gOutput &operator<<(long x)
	{int c=fprintf(f, "%*ld", Width, x);return *this;}
	gOutput &operator<<(char x)
	{int c=fprintf(f, "%c", x);return *this;}
	gOutput &operator<<(double x)
	{
		int c;
		switch (Represent) {
		case 'f':
			c=fprintf(f, "%*.*lf", Width, Prec, x);
			break;
		case 'e':
			c=fprintf(f, "%*.*le", Width, Prec, x);
	}
	return *this;
	}
	gOutput &operator<<(float x);
	{
	int c;
	switch (Represent) {
		case 'f':
			c=fprintf(f, "%*.*f", Width, Prec, x);
			break;
		case 'e':
			c=fprintf(f, "%*.*e", Width, Prec, x);
			break;
		}
	return *this;
	}
	gOutput &operator<<(const char *x);
	{int c=fprintf(f, "%s", x);return *this;}
	gOutput &operator<<(const void *x);
	{int c=fprintf(f, "%p", x);return *this;}

	bool IsValid(void) const {return true;}
	// functions for gProgress
	void	SetProgress(double p) {gauge->SetValue((int)(p*100));
	// functions for gSignal
	void	SetSignal() {signal=true;}
	bool Get(void) const {return signal;}
	void Reset(void) {signal=false;}
};

wxStatus::wxStatus(const char *name,wxFrame *parent):wxDialogBox(parent,(char *)name)
{
Reset();

Width=0;Prec=6;Represent='f';

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
//gauge->Centre();
twin->Centre(wxHORIZONTAL);
(*twin)<<"This is a TEST\n";
(*twin)<<1234<<'a';
gauge->SetValue(45);
Show(TRUE);
}

void cancel_func(wxButton &ob,wxCommandEvent &ev)
{
wxStatus *parent=(wxStatus *)ob.GetClientData();
parent->SetSignal();
}

