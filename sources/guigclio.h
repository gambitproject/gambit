// File: wxgclio.h -- Defines a class that simulates the standard input/output
// as implemented in gin/gout of gambitio.cc implemented in a gui envoriment.
// $Id$

#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "gambitio.h"

class GuiGclIo: public gInput, public gOutput
{
private:
	GuiGclIo(const GuiGclIo &);
	char buffer[512];
	char sub_buffer[255];
	int pos,len;
	int prev_len,got_line;
	bool valid;
	int Width,Prec;
	char Represent;
	void GetNewLine(void)
	{
	gets(buffer);
	len=strlen(buffer)+1;
	buffer[len-1]=10; // CR
	buffer[len]=0; // EOL
	got_line=0;
	pos=0;
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
	GuiGclIo(void)
	{
	prev_len=0;
	got_line=0;
	len=0;pos=0;
	Width=0;
	Prec=6;
	Represent='f';
	}
	virtual ~GuiGclIo() {}
	// INPUT
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
	long getpos(void) const {assert(0);}
	void setpos(long x) const {assert(0);}
	// OUTPUT
	gOutput &operator<<(int x);
	gOutput &operator<<(unsigned int x);
	gOutput &operator<<(bool x);
	gOutput &operator<<(long x);
	gOutput &operator<<(char x);
	gOutput &operator<<(double x);
	gOutput &operator<<(float x);
	gOutput &operator<<(const char *x);
	gOutput &operator<<(const void *x);

	bool IsValid(void) const {return valid;}
	// Functions to control the appearance of the output
	int GetWidth(void) {return Width;}
	gOutput &SetWidth(int w) {Width=w;return (*this);}
	int GetPrec(void) {return Prec;}
	gOutput &SetPrec(int p) {Prec=p;return (*this);}
	gOutput &SetExpMode(void) {Represent = 'e';return *this;}
	gOutput &SetFloatMode(void){Represent = 'f';return *this;}
	char GetRepMode(void) {return Represent;}
	// Toggles the option to echo each line in the output window after CR is pressed
};


