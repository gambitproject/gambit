#include "wx.h"
#include "gstatus.h"

char tmp_str[100];
class wxStatus:public gStatus, public wxFrame
{
private:
	// Stuff for signal
	bool	signal;
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
	void	SetProgress(double p) {gauge->SetValue((int)(p*100));wxYield();}
	// functions for gSignal
	void	SetSignal(void) {signal=true;}
	bool Get(void) const {return signal;}
	void Reset(void) {signal=false;}
	// Window event handlers
	void OnSize(int, int) {Layout();}
};

wxStatus::wxStatus(const char *name,wxFrame *parent):wxFrame(parent,(char *)name,200,200,300,300)
{
Reset();
Width=0;Prec=6;Represent='f';
// Build the dialog box
wxPanel *gauge_panel=new wxPanel(this);
gauge=new wxGauge(gauge_panel,"",100,-1,-1,-1,-1,wxHORIZONTAL);
wxLayoutConstraints *b0 = new wxLayoutConstraints;
b0->centreX.SameAs  (gauge_panel, wxCentreX);
b0->centreY.SameAs  (gauge_panel, wxCentreY);
b0->width.PercentOf (gauge_panel, wxWidth, 80);
b0->height.PercentOf(gauge_panel, wxHeight, 80);
gauge->SetConstraints(b0);

wxLayoutConstraints *b1 = new wxLayoutConstraints;
b1->top.SameAs     (this, wxTop, 5);
b1->left.SameAs    (this, wxLeft,5);
b1->right.SameAs    (this, wxRight,5);
b1->height.PercentOf  (this, wxHeight, 25);
gauge_panel->SetConstraints(b1);

twin=new wxTextWindow(this,-1,-1,-1,-1,wxREADONLY|wxBORDER);
wxLayoutConstraints *b2 = new wxLayoutConstraints;
b2->top.SameAs     (gauge_panel, wxBottom, 5);
b2->left.SameAs    (this, wxLeft,5);
b2->right.SameAs   (this, wxRight,5);
b2->height.PercentOf(this, wxHeight, 50);
twin->SetConstraints(b2);

wxPanel *cancel_panel=new wxPanel(this);
wxLayoutConstraints *b3 = new wxLayoutConstraints;
b3->top.SameAs     (twin, wxBottom, 5);
b3->left.SameAs    (this, wxLeft,5);
b3->right.SameAs   (this, wxRight,5);
b3->bottom.SameAs  (this, wxBottom, 5);
cancel_panel->SetConstraints(b3);

cancel=new wxButton(cancel_panel,(wxFunction)wxStatus::cancel_func,"Cancel");
cancel->SetClientData((char *)this);
wxLayoutConstraints *b4 = new wxLayoutConstraints;
b4->top.SameAs     (cancel_panel, wxTop, 5);
b4->centreX.SameAs (cancel_panel, wxCentreX,5);
b4->bottom.SameAs  (cancel_panel, wxBottom, 5);
b4->width.PercentOf(cancel_panel, wxWidth, 30);
cancel->SetConstraints(b4);

Show(TRUE);
}

