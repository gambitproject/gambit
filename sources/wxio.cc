//#
//# FILE: wxio.cc -- Implementation of more complicated I/O functions
//#
// $Id$
#include "wx.h"
#include "wxio.h"

static char	wxio_buffer[100];

class gWxIOFrame: public wxFrame
{
private:
	gWxOutput *parent;
public:
	wxTextWindow *f;
	gWxIOFrame(gWxOutput *parent,const char *label);
	Bool OnClose(void);
};

gWxIOFrame::gWxIOFrame(gWxOutput *parent_,const char *label)
							:wxFrame(NULL,(char *)label,0,0,200,200),parent(parent_)
{
f=new wxTextWindow(this,-1,-1,-1,-1,wxREADONLY);
Show(TRUE);
}

Bool gWxIOFrame::OnClose(void)
{
parent->OnClose();
return TRUE;
}
//************************************ G WX OUTPUT ****************************
gWxOutput::gWxOutput(const char *label_)
{
if (label_) label=copystring(label_); else label=0;
frame=0;
Width=0;
Prec=6;
Represent='f';
}

gWxOutput::~gWxOutput()
{
if (frame) delete frame;
if (label) delete [] label;
}

int gWxOutput::GetWidth(void)
{
	return Width;
}

gOutput &gWxOutput::SetWidth(int w)
{
	Width = w;
	return *this;
}

int gWxOutput::GetPrec(void)
{
	return Prec;
}

gOutput &gWxOutput::SetPrec(int p)
{
	Prec = p;
	return *this;
}

gOutput &gWxOutput::SetExpMode(void)
{
	Represent = 'e';
	return *this;
}

gOutput &gWxOutput::SetFloatMode(void)
{
	Represent = 'f';
	return *this;
}

char gWxOutput::GetRepMode(void)
{
	return Represent;
}


gOutput &gWxOutput::operator<<(int x)
{
	if (!frame) MakeFrame();
	sprintf(wxio_buffer, "%*d", Width,  x);
	*frame->f<<wxio_buffer; return *this;
}

gOutput &gWxOutput::operator<<(unsigned int x)
{
	if (!frame) MakeFrame();
	sprintf(wxio_buffer, "%*d", Width,  x);
	*frame->f<<wxio_buffer; return *this;
}

gOutput &gWxOutput::operator<<(long x)
{
	if (!frame) MakeFrame();
	sprintf(wxio_buffer, "%*ld", Width, x);
	*frame->f<<wxio_buffer; return *this;
}

gOutput &gWxOutput::operator<<(char x)
{
	if (!frame) MakeFrame();
	sprintf(wxio_buffer, "%c", x);
	*frame->f<<wxio_buffer; return *this;
}

gOutput &gWxOutput::operator<<(double x)
{
	if (!frame) MakeFrame();
	switch (Represent) {
		case 'f':
			sprintf(wxio_buffer, "%*.*f", Width, Prec, x);
			break;
		case 'e':
			sprintf(wxio_buffer, "%*.*e", Width, Prec, x);
			break;
		}

	*frame->f<<wxio_buffer; return *this;
}

gOutput &gWxOutput::operator<<(float x)
{
	if (!frame) MakeFrame();
	switch (Represent) {
		case 'f':
			sprintf(wxio_buffer, "%*.*f", Width, Prec, x);
			break;
		case 'e':
			sprintf(wxio_buffer, "%*.*e", Width, Prec, x);
			break;
		}
	*frame->f<<wxio_buffer; return *this;
}

gOutput &gWxOutput::operator<<(const char *x)
{
	if (!frame) MakeFrame();
	sprintf(wxio_buffer, "%s", x);
	*frame->f<<wxio_buffer; return *this;
}

gOutput &gWxOutput::operator<<(const void *x)
{
	if (!frame) MakeFrame();
	sprintf(wxio_buffer, "%p", x);
	*frame->f<<wxio_buffer; return *this;
}

bool gWxOutput::IsValid(void) const
{
	return true;
}

void gWxOutput::MakeFrame(void)
{
assert(!frame && "Frame already exists");
frame=new gWxIOFrame(this,(label) ? label : "WxIO");
#ifndef wx_msw
frame->SetSize(200,200); // does not resize automatically for some reason.
#endif
}

void gWxOutput::OnClose(void)
{
if (frame)
{
	delete frame;
	frame=0;
}
}

gWxOutput gwout("Default Output");
gWxOutput gwerr("Default Error");
gWxOutput *wout=&gwout;
gWxOutput *werr=&gwerr;
gOutput &gout=gwout;
gOutput &gerr=gwerr;


