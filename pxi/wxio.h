// File: wxio.h--defines an input/output class based on gio for wxwindows
// This creates a text window that will accept output and provide input
// using the << and >> redirection operators.  One instance of this class
// will always be created and called gdebug, (see also gerror,gin,gout).
#ifndef 	WXIO_H
#define WXIO_H

#include <stdio.h>
#include <assert.h>
#include "wx.h"
#include "gambitio.h"

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

typedef enum {gIOWxInput,gIOWxOutput,gIOFileInput,gIOFileOutput,gIOInput,gIOOutput} gWxIOType; 

class gWxInput: public gInput
{
private:
	wxTextWindow	*f;
	gWxIOType			__type;
	char					line_buffer[100];
  int						ok;
	gWxInput(const gFileInput &);
	gWxInput &operator=(const gFileInput &);
public:
	// Constructors 
	gWxInput(void)
	{
	__type=gIOWxInput;
	wxFrame *frame=new wxFrame(0,"gWxInput",0,0,200,200);
	f=new wxTextWindow(frame,-1,-1,-1,-1,wxNATIVE_IMPL);
  frame->Show(TRUE);
	ok=(f) ? 1: 0;
	}

	gWxInput(const char *in)
	{
	__type=gIOWxInput;
	wxFrame *frame=new wxFrame(NULL,(char *)in,0,0,200,200);
	f=new wxTextWindow(frame,-1,-1,-1,-1,wxNATIVE_IMPL);
	frame->Show(TRUE);
	ok=(f) ? 1: 0;
	}

//
// Close the window pointed to, if any.
//
	~gWxInput()   { if (f) delete f->GetParent();}

// Load a text file into the input window (this is unique to WxInput)
	void Load(const char *file) {f->LoadFile((char *)file);}

//
// Input primitives for the builtin types.

		gInput& operator>>(int &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%d", &x);   return *this; }
		gInput& operator>>(unsigned int &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%d", &x);   return *this; }
		gInput& operator>>(long &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%ld", &x);   return *this; }
		gInput& operator>>(char &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%c", &x);   return *this; }
		gInput& operator>>(double &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%lf", &x);  return *this; }
		gInput& operator>>(float &x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines(),line_buffer); ok=sscanf(line_buffer, "%f", &x);   return *this; }
    gInput& operator>>(char *x)
			{ assert(f);  f->GetLineText(f->GetNumberOfLines()-1,line_buffer); ok=sscanf(line_buffer, "%s", x);
				return *this;
			}
//-grp

//
// Unget the character given.  Only one character of pushback is guaranteed.
//
		int get(char &c)	{return 1;}
		void unget(char c) {;}
		int eof(void) const {return 0;};

//
// Returns nonzero if the end-of-file marker has been reached.
//
		int IsValid(void) const	{return ok;}
		void seekp(long pos) const {;}
};

//
// This class is a (simple) implementation of an output stream.  It is built
// around the C stdio library in preference to the C++ iostream library
// since some compilers (notable GNU C++) suffer from executable bloat
// when iostreams are used.
//
class gWxOutput: public gOutput
{
  private:
	wxTextWindow	*f;
	gWxIOType			__type;
	char					buffer[100];
  int 					ok;
//
// The copy constructor and assignment operator are declared private to
// override the default meanings of these functions.  They are never
// implemented.
//+grp
		gWxOutput(const gFileOutput &);
    gWxOutput &operator=(const gFileOutput &);
//-grp

  public:
//
// The default constructor, initializing the instance to point to no stream
//
		gWxOutput(void)
		{
		__type=gIOWxOutput;
		wxFrame *frame=new wxFrame(NULL,"gWxOutput",0,0,200,200);
		f=new wxTextWindow(frame,-1,-1,300,200,wxREADONLY);
		frame->Show(TRUE);
		ok=(f) ? 1: 0;
		}
//
// Initialize the instance to point to the stream with a certain filename.
//
// <note> This is the preferred way to initialize an instance of this class.
//
		gWxOutput(const char *out)
		{
		__type=gIOWxOutput;
		wxFrame *frame=new wxFrame(NULL,(char *)out,0,0,200,200);
		f=new wxTextWindow(frame,-1,-1,300,200,wxREADONLY);
		frame->Show(TRUE);
		ok=(f) ? 1: 0;
		}
//
// Close the window pointed to, if any.
//
		~gWxOutput()   {if (f) delete f->GetParent();}



//
// Output primitives for the basic types

		gOutput& operator<<(int x)
			{ assert(f);  (*f)<<x; return *this; }
		gOutput& operator<<(unsigned int x)
			{ assert(f);  (*f)<<(int)x; return *this; }
		gOutput& operator<<(long x)
			{ assert(f);  (*f)<<x; return *this; }
		gOutput& operator<<(char x)
			{ assert(f);  (*f)<<x; return *this; }
		gOutput& operator<<(double x)
			{ assert(f);  (*f)<<x; return *this; }
		gOutput& operator<<(float x)
			{ assert(f);  (*f)<<x; return *this; }
		gOutput& operator<<(const char *x)
			{ assert(f);  (*f)<<(char *)x;  return *this; }
		gOutput& operator<<(const void *x)
			{ assert(f);  sprintf(buffer, "%p", x); (*f)<<buffer; return *this; }

		int IsValid(void) const {return ok;}

};
#endif   // WXIO_H
