#include "wx.h"
#include "wxstatus.h"

static char tmp_str[200];

wxStatus::wxStatus(wxFrame *frame,const char *title, int x, int y, int w, int h):
	wxDialogBox(frame,(char *)title,FALSE,-1,-1,300,250)
{
sig=0;Width=0;Prec=6;Represent='f';

gauge=new wxGauge(this,"",100,20,10,240,50,wxHORIZONTAL);
// Make a text window
twin = new wxMultiText(this, (wxFunction)0,"","",0,65,260,100,wxREADONLY);
wxButton *cancel = new wxButton(this, (wxFunction)&button_proc, "Cancel",100,170,100,30);
cancel->SetClientData((char *)this);

Show(TRUE);
}

void wxStatus::button_proc(wxButton& but, wxCommandEvent& event)
{((wxStatus *)but.GetClientData())->SetSignal();}

gOutput &wxStatus::operator<<(int x)
{sprintf(tmp_str, "%*d", Width,  x);twin->SetValue(tmp_str);return *this;}
gOutput &wxStatus::operator<<(unsigned int x)
{sprintf(tmp_str, "%*d", Width,  x);twin->SetValue(tmp_str);return *this;}
gOutput &wxStatus::operator<<(long x)
{sprintf(tmp_str, "%*ld", Width, x);twin->SetValue(tmp_str);return *this;}
gOutput &wxStatus::operator<<(char x)
{sprintf(tmp_str, "%c", x);twin->SetValue(tmp_str);return *this;}
gOutput &wxStatus::operator<<(double x)
{
	switch (Represent) {
	case 'f':
		sprintf(tmp_str, "%*.*lf", Width, Prec, x);
		twin->SetValue(tmp_str);
		break;
	case 'e':
		sprintf(tmp_str, "%*.*le", Width, Prec, x);
		twin->SetValue(tmp_str);
}
return *this;
}
gOutput &wxStatus::operator<<(float x)
{
switch (Represent) {
	case 'f':
		sprintf(tmp_str, "%*.*f", Width, Prec, x);
		twin->SetValue(tmp_str);
		break;
	case 'e':
		sprintf(tmp_str, "%*.*e", Width, Prec, x);
		twin->SetValue(tmp_str);
		break;
	}
return *this;
}
gOutput &wxStatus::operator<<(const char *x)
{sprintf(tmp_str, "%s", x);twin->SetValue(tmp_str);return *this;}
gOutput &wxStatus::operator<<(const void *x)
{sprintf(tmp_str, "%p", x);twin->SetValue(tmp_str);return *this;}

void wxStatus::SetProgress(double p)
{
gauge->SetValue((int)(p*100.0));
wxYield();
}
