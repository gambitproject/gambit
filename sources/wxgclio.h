// File: wxgclio.h -- Defines a class that simulates the standard input/output
// as implemented in gin/gout of gambitio.cc implemented in a gui envoriment.
// $Id$

#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "gambitio.h"

class gWxGclIoFrame: public wxFrame
{
private:
	wxText *&in;
public:
	gWxGclIoFrame(wxText *&in_): wxFrame(0,"IO"), in(in)
	{ }
	Bool OnClose(void) {return TRUE;}
	void OnSetFocus(void) {in->SetFocus();}
};

class gWxGclIo: public gInput, public gOutput
{
private:
	gWxGclIo(const gWxGclIo &);
	wxTextWindow	*out;
	wxText *in;
	char *buffer;
	char sub_buffer[255];
	int pos,len;
	int prev_len,got_line;
	bool echo;
	int Width,Prec;
	char Represent;
	static void in_func(wxText &ob,wxEvent &ev)
	{((gWxGclIo *)ob.GetClientData())->OnIn();}
	void OnIn(void)
	{
	int l=strlen(in->GetValue());
	if (l!=0 && prev_len==l)
		{prev_len=0;got_line=1;}
	else
		prev_len=l;
	}
	void GetNewLine(void)
	{
	while (!got_line) wxYield();
	if (buffer) delete [] buffer;
	len=strlen(in->GetValue())+1;
	buffer=new char[len+1];
	strcpy(buffer,in->GetValue());
	buffer[len-1]=10; // CR
	buffer[len]=0; // EOL
	in->SetValue("");
	if (echo) (*this)<<buffer;
	got_line=0;
	pos=0;
	in->SetFocus();
	}
	char *sub_string(void)
	{
	  char a;
	  int p=0;
	  do  {
		 a=buffer[pos++];
	  }  while (isspace(a) && pos<len);

	  if (a == '\"'){
		 a=buffer[pos++];
		 while  (a != '\"' && pos<len)  {
			sub_buffer[p++]=a;
			a=buffer[pos++];
		 }
	  }
	  else  {
		 do  {
			sub_buffer[p++]=a;
			a=buffer[pos++];
		 }  while (!isspace(a) && pos<=len);
	  }
	sub_buffer[p]=0;
	return sub_buffer;
	}

public:
	gWxGclIo(void):in(0),out(0)
	{ }
	wxFrame *Create(void)
	{
	wxFrame *frame=new gWxGclIoFrame(in);
	// Figure out the size of an 80x25 window
	float w,h;
	frame->GetTextExtent("0Ayi",&w,&h);w/=4;
	out=new wxTextWindow(frame,0,0,81*w,25*h,wxREADONLY);
	wxPanel *p=new wxPanel(frame,0,25*h+1,81*w,44);
	in=new wxText(p,(wxFunction)in_func,0,"",-1,-1,77*w,-1,wxPROCESS_ENTER);
	prev_len=0;
	buffer=0;
	got_line=0;
	echo=true;
	len=0;pos=0;
	int x,y;
	in->GetSize(&x,&y);
	in->SetClientData((char *)this);
	frame->SetSize(0,0,80*w,26*h+55);
	Width=0;
	Prec=6;
	Represent='f';
	in->SetFocus();
	frame->Show(TRUE);
	return frame;
	}
	virtual ~gWxGclIo() {if (buffer) delete [] buffer;}
	gInput& operator>>(int &x)
	{if (pos>=len) GetNewLine(); x=atoi(sub_string()); return *this;}
	gInput& operator>>(unsigned int &x)
	{if (pos>=len) GetNewLine(); x=atoi(sub_string()); return *this;}
	gInput& operator>>(long &x)
	{if (pos>=len) GetNewLine(); x=atol(sub_string()); return *this;}
	gInput& operator>>(char &x)
	{if (pos>=len) GetNewLine(); x=buffer[pos++]; return *this;}
	gInput& operator>>(double &x)
	{if (pos>=len) GetNewLine(); x=atof(sub_string()); return *this;}
	gInput& operator>>(float &x)
	{if (pos>=len) GetNewLine(); x=atof(sub_string()); return *this;}
	gInput& operator>>(char *x)
	{if (pos>=len) GetNewLine(); strcpy(x,sub_string()); return *this;}
	int get(char &c)
	{if (pos>=len) GetNewLine(); c=buffer[pos++];return true;}
	void unget(char c)
	{if (pos>0) pos--;}
	bool eof(void) const {return false;}
	void seekp(long x) const {assert(0);}

	gOutput& operator<<(int x)
	{ assert(out); (*out)<<x; in->SetFocus(); return *this; }
	gOutput& operator<<(unsigned int x)
	{ assert(out); (*out)<<(int)x; in->SetFocus();return *this; }
	gOutput& operator<<(long x)
	{ assert(out); (*out)<<x; in->SetFocus();return *this; }
	gOutput& operator<<(char x)
	{ assert(out); (*out)<<x; in->SetFocus();return *this; }
	gOutput& operator<<(double x)
	{ assert(out); (*out)<<x; in->SetFocus();return *this; }
	gOutput& operator<<(float x)
	{ assert(out); (*out)<<x; in->SetFocus();return *this; }
	gOutput& operator<<(const char *x)
	{ assert(out); (*out)<<(char *)x; in->SetFocus(); return *this; }
	gOutput& operator<<(const void *x)
	{ assert(out); sprintf(buffer, "%p", x); (*out)<<buffer; in->SetFocus();return *this; }
	bool IsValid(void) const {return true;}
	// Functions to control the appearance of the output
	int GetWidth(void) {return Width;}
	gOutput &SetWidth(int w) {Width=w;return (*this);}
	int GetPrec(void) {return Prec;}
	gOutput &SetPrec(int p) {Prec=p;return (*this);}
	gOutput &SetExpMode(void) {Represent = 'e';return *this;}
	gOutput &SetFloatMode(void){Represent = 'f';return *this;}
	char GetRepMode(void) {return Represent;}
	// Toggles the option to echo each line in the output window after CR is pressed
	void SetEcho(bool e) {echo=e;}
};


