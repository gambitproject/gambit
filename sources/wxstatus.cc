#include "wx.h"
#include "wxstatus.h"

char tmp_str[200];
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
		sprintf(tmp_str, "%*.*lf", Width, Prec, x);
		(*twin)<<tmp_str;
		break;
	case 'e':
		sprintf(tmp_str, "%*.*le", Width, Prec, x);
		(*twin)<<tmp_str;
}
return *this;
}
gOutput &wxStatus::operator<<(const char *x)
{sprintf(tmp_str, "%s", x);(*twin)<<tmp_str;return *this;}
gOutput &wxStatus::operator<<(const void *x)
{sprintf(tmp_str, "%p", x);(*twin)<<tmp_str;return *this;}

