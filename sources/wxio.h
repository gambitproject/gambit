// File: wxio.h--defines an input/output class based on gio for wxwindows
// This creates a text window that will accept output and provide input
// using the << and >> redirection operators.  One instance of this class
// will always be created and called gdebug, (see also gerror,gin,gout).
// $Id$
#ifndef 	WXIO_H
#define WXIO_H

#include <stdio.h>
#include <assert.h>
#include "gambitio.h"

class gWxIOFrame;
class gWxOutput: public gOutput
{
private:
	gWxIOFrame		*frame;
	int Width,Prec;
	char Represent;
	char *label;
	void MakeFrame(void);
public:
	gWxOutput(const char *label=0);
	virtual ~gWxOutput();

// Output primitives for the basic types

	gOutput& operator<<(int x);
	gOutput& operator<<(unsigned int x);
	gOutput& operator<<(bool x);
	gOutput& operator<<(long x);
	gOutput& operator<<(char x);
	gOutput& operator<<(double x);
	gOutput& operator<<(float x);
	gOutput& operator<<(const char *x);
	gOutput& operator<<(const void *x);
	bool IsValid(void) const;

// Functions to control the appearance of the output
	int GetWidth(void);
	gOutput &SetWidth(int w);
	int GetPrec(void);
	gOutput &SetPrec(int p);
	gOutput &SetExpMode(void);
	gOutput &SetFloatMode(void);
	char GetRepMode(void);

// Iterface to the display window
	void OnClose(void);
};

#define gWXOUT		"wout"
#define gWXERR		"werr"
#define gWXIN			"win"

extern gWxOutput *wout,*werr;
#endif   // WXIO_H
