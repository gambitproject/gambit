//
// FILE: wxmisc.h -- Defines some generic stuff that should be in wxWin
// Right now its a dialog box with a built in form, a font selector, and
// a wxListFindString function... More to come
//
// @(#)wxmisc.h	1.1 6/6/94
//

#ifndef WXMISC_H
#define WXMISC_H
#ifndef	wxRUNNING
#define	wxRUNNING 12345
#endif
#include <stdio.h>
#include "wx_timer.h"
#include "wx_form.h"

#define WX_COLOR_LIST_LENGTH 11
#ifndef WXMISC_C
extern const char *wx_color_list[WX_COLOR_LIST_LENGTH];
extern const char *wx_hilight_color_list[WX_COLOR_LIST_LENGTH];
#else
const char *wx_color_list[WX_COLOR_LIST_LENGTH]=
						{"GREEN","RED","BLUE","YELLOW","VIOLET RED","SALMON","ORCHID",
						 "TURQUOISE","BROWN","PALE GREEN","BLACK"};
const char *wx_hilight_color_list[WX_COLOR_LIST_LENGTH]=
						{"PALE GREEN","INDIAN RED","MEDIUM BLUE","YELLOW GREEN",
						 "MEDIUM VIOLET RED","TAN","MEDIUM ORCHID","MEDIUM TURQUOISE",
						 "KHAKI","SEA GREEN","DARK GREY"};
#endif

class MyDialogBox;
class MyForm: public wxForm
{
private:
	MyDialogBox *parent;
public:
	MyForm(MyDialogBox *p);
	void 	OnOk(void);
	void 	OnCancel(void);
};

class MyDialogBox: public wxDialogBox
{
private:
	MyForm *form;
	Bool completed;
public:
	// Constructor
	MyDialogBox(wxWindow *parent,char *title);
	// Destructor
	~MyDialogBox(void);
	// What was the result? wxOK/wxCANCEL
	Bool		Completed(void);
	// Access to private data, should be removed
	MyForm 	*Form(void);
	// Simulate a form here, w/ some additional features
	wxFormItem *Add(wxFormItem *item,long id=-1) {form->Add(item,id);return item;}
	void		AssociatePanel(void)	{form->AssociatePanel(this);}
	// Choose go to start up the dialog right away (calls associate panel by itself)
	void	  Go(void);
  // Choose Go1 if you have already called associate panel
	void		Go1(void);
	virtual void 		OnOk(void);
	virtual void 		OnCancel(void);
};

class FontDialogBox: public MyDialogBox
{
private:
	int		f_name;
	int		f_size;
	int		f_style;
	int		f_weight;
	Bool	f_under;
	int f_names[5]; 	//{wxSWISS,wxROMAN,wxDECORATIVE,wxMODERN,wxSCRIPT};
	int f_styles[3]; 	//{wxNORMAL,wxITALIC,wxSLANT};
	int f_weights[3];	//{wxNORMAL,wxLIGHT,wxBOLD};
	char * f_names_str[5]; 	//{wxSWISS,wxROMAN,wxDECORATIVE,wxMODERN,wxSCRIPT};
	char * f_styles_str[3]; 	//{wxNORMAL,wxITALIC,wxSLANT};
	char * f_weights_str[3];	//{wxNORMAL,wxLIGHT,wxBOLD};


public:
	FontDialogBox(wxWindow *parent,wxFont *def=0);
	~FontDialogBox(void);

	int 		FontName(void) {return f_name;}
	int			FontSize(void) {return f_size;}
	int			FontStyle(void) {return f_style;}
	int			FontWeight(void) {return f_weight;}
	int			FontUnder(void) {return f_under;}
	wxFont 	*MakeFont(void);
};

//**************************** Output Dialog ************************
// use the following constants to determine return times
#define wxMEDIA_PRINTER			0
#define wxMEDIA_PS					1
#define wxMEDIA_CLIPBOARD		2
#define wxMEDIA_METAFILE		3
typedef enum {wxWYSIWYG,wxFITTOPAGE} wxOutputOption;
class wxOutputDialogBox : public MyDialogBox
{
private:
	wxRadioBox *media_box;
	wxCheckBox *fit_box;
public:
	wxOutputDialogBox(wxStringList *extra_media=0,wxWindow *parent=0);
	int GetSelection();
	wxOutputOption GetOption();
};

// Returns the position of s in the list l.  This is useful for finding
// the selection # in a String form item w/ a list constraint
int wxListFindString(wxList *l,char *s);
// Returns a wxStringList containing num strings representing integers
// from 1 to n (i.e. "1","2",.."n").  If l is NULL, a new list is created
// otherwise, the nums are appended to l.
wxStringList* wxStringListInts(int num,wxStringList *l=0);
// Converts a font into a string format that can be used to save it in a data file
char *wxFontToString(wxFont *f);
// Converts an encoded string back into a font. see wxFontToString
wxFont *wxStringToFont(char *s);
// Some basic keyboard stuff...
Bool	IsCursor(wxKeyEvent &ev);
Bool	IsEnter(wxKeyEvent &ev);
Bool	IsNumeric(wxKeyEvent &ev);
Bool	IsAlphaNum(wxKeyEvent &ev);
Bool	IsDelete(wxKeyEvent &ev);
// gDrawText is an extension of the wxWindow's wxDC::DrawText function
// Besides providing the same features, it also supports imbedded codes
// to change the color of the output text.  The codes have the format
// of: "text[/C{#}]", where # is the number of the color to select
// from the gambit_color_list.  Note: uses gString
#include "gstring.h"
void gDrawText(wxDC &dc,const gString &s,float x,float y);
// Returns just the text portion of a gDrawText formated string
gString gPlainText(const gString &s);

// FindFile: finds the specified file in the path.  User deletes the
// result
char *wxFindFile(const char *name);

// OutputFile(base): returns a filename of the form baseXXX.out where XXX are consequtive
// numbers in this dir.
char *wxOutputFile(const char *name);
#endif //WXMISC_H
