#include <assert.h>
#include "wxstatus.h"
char tmp_str[200];
// Define my frame constructor
wxStatus::wxStatus(wxFrame *frame,const char *title, int x, int y, int w, int h):
	wxFrame(frame, (char *)title, x, y, w, h)
{
sig=0;Width=0;Prec=6;Represent='f';

// Make a panel
wxPanel *panel = new wxPanel(this, 0, 0, 1000, 500, 0);
panel->SetLabelPosition(wxVERTICAL);
wxPanel *p=new wxPanel(this);

gauge=new wxGauge(panel,"",100,-1,-1,-1,-1,wxHORIZONTAL);

wxLayoutConstraints *b1 = new wxLayoutConstraints;
b1->centreX.SameAs    (panel, wxCentreX);
b1->top.SameAs        (panel, wxTop, 5);
b1->width.PercentOf   (panel, wxWidth, 80);
b1->height.PercentOf  (panel, wxHeight, 80);
gauge->SetConstraints(b1);

// Make a text window
twin = new wxTextWindow(this, 0, 250, 400, 250, wxBORDER|wxREADONLY);
// Set constraints for panel subwindow
wxLayoutConstraints *c1 = new wxLayoutConstraints;
c1->left.SameAs       (this, wxLeft);
c1->top.SameAs        (this, wxTop);
c1->right.SameAs			(this,wxRight);
c1->height.PercentOf  (this, wxHeight, 35);
panel->SetConstraints(c1);

// Set constraints for text subwindow
wxLayoutConstraints *c3 = new wxLayoutConstraints;
c3->left.SameAs       (this, wxLeft);
c3->top.Below         (panel);
c3->right.SameAs      (this, wxRight);
c3->height.PercentOf  (this, wxHeight, 45);
twin->SetConstraints(c3);

wxLayoutConstraints *c4 = new wxLayoutConstraints;
c4->left.SameAs       (this, wxLeft);
c4->top.Below         (twin);
c4->right.SameAs      (this, wxRight);
c4->bottom.SameAs     (this, wxBottom);
p->SetConstraints(c4);

wxButton *btn2 = new wxButton(p, (wxFunction)&button_proc, "Cancel") ;
btn2->SetClientData((char *)this);
wxLayoutConstraints *b5 = new wxLayoutConstraints;
b5->centreX.SameAs    (p, wxCentreX);
b5->top.SameAs        (p, wxTop, 5);
b5->width.PercentOf   (p, wxWidth, 30);
b5->height.PercentOf  (p, wxHeight, 80);
btn2->SetConstraints(b5);
Show(TRUE);
}

gOutput &wxStatus::operator<<(int x)
{sprintf(tmp_str, "%*d", Width,  x);(*twin)<<tmp_str;return *this;}
gOutput &wxStatus::operator<<(unsigned int x)
{sprintf(tmp_str, "%*d", Width,  x);(*twin)<<tmp_str;return *this;}
gOutput &wxStatus::operator<<(long x)
{sprintf(tmp_str, "%*ld", Width, x);(*twin)<<tmp_str;return *this;}
gOutput &wxStatus::operator<<(char x)
{sprintf(tmp_str, "%c", x);(*twin)<<tmp_str;return *this;}
gOutput &wxStatus::operator<<(double x)
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
gOutput &wxStatus::operator<<(float x)
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
gOutput &wxStatus::operator<<(const char *x)
{sprintf(tmp_str, "%s", x);(*twin)<<tmp_str;return *this;}
gOutput &wxStatus::operator<<(const void *x)
{sprintf(tmp_str, "%p", x);(*twin)<<tmp_str;return *this;}

void wxStatus::SetProgress(double p)
{
assert(p>=0 && p<=1.0 && "wxProgress::Invalid Range");
gauge->SetValue((int)(p*100.0));
wxYield();
}

// Size the subwindows when the frame is resized
void wxStatus::OnSize(int w, int h)
{Layout();}

Bool wxStatus::OnClose(void)
{Show(FALSE);return TRUE;}

void wxStatus::button_proc(wxButton& ob, wxCommandEvent& event)
{
wxStatus *parent=(wxStatus *)ob.GetClientData();
parent->SetSignal();
wxMessageBox("SetSignal!", "Try me",wxOK | wxCANCEL) ;
}

