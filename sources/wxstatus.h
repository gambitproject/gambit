// File: wxstatus.h -- definition of the class to implement progress
// indication/cancel feature for the gambit algorithms in the gui.
// $Id$
#ifndef WXSTATUS_H
#define WXSTATUS_H
#include "wx.h"
#include "gstatus.h"
class wxStatus: public wxDialogBox,public gStatus
{
protected:
	int Width,Prec;
	char Represent;
	wxGauge *gauge;
	wxMultiText *twin;
	bool sig;
	static void button_proc(wxButton& but, wxCommandEvent& event);
public:
	wxStatus(wxFrame *frame,const char *title="Progress");
	~wxStatus();
	// functions for gProgress::gOutput
	int GetWidth(void) {return Width;}
	gOutput &SetWidth(int w) {Width=w;return *this;}
	int GetPrec(void) {return Prec;}
	gOutput &SetPrec(int p) {Prec=p;return *this;}
	gOutput &SetExpMode(void) {Represent='e';return *this;}
	gOutput &SetFloatMode(void){Represent='f';return *this;}
	char GetRepMode(void){return Represent;}

	gOutput &operator<<(int x);
	gOutput &operator<<(unsigned int x);
	gOutput &operator<<(long x);
	gOutput &operator<<(char x);
	gOutput &operator<<(double x);
	gOutput &operator<<(float x);
	gOutput &operator<<(const char *x);
	gOutput &operator<<(const void *x);

	bool IsValid(void) const {return true;}
	// functions for gProgress
	virtual void SetProgress(double p);
	// functions for gSignal
	virtual void SetSignal(void) {sig=true;}
	virtual bool Get(void) const;
	virtual void Reset(void) {sig=false;}
};
#endif
