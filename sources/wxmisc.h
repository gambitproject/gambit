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


class MyForm: public wxForm
{
private:
	int _completed;
public:
	MyForm(int use_buttons,int place_buttons):wxForm(use_buttons,place_buttons) {};
	void 	OnOk(void) {_completed=wxOK;wx_form_panel->Show(FALSE);}
	void 	OnCancel(void) {_completed=wxCANCEL;wx_form_panel->Show(FALSE);}
	int		Completed(void) {return _completed;}
};

class MyDialogBox: public wxDialogBox
{
private:
	MyForm *form;
public:
	MyDialogBox(wxWindow *parent,char *title):wxDialogBox(parent,title,TRUE)
	{ form=new MyForm(wxFORM_BUTTON_OK | wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);}
  ~MyDialogBox(void){delete form;}
	int			Completed(void) {return form->Completed();}
	MyForm 	*Form(void) {return form;}
	void	  Go(void)	{form->AssociatePanel(this);Fit();Centre();Show(TRUE);}
	void		Go1(void)	{Fit();Centre();Show(TRUE);}
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
	FontDialogBox(wxWindow *parent,wxFont *def=NULL);
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
class wxOutputDialogBox : public MyDialogBox
{
private:
	wxRadioBox *media_box;
	wxRadioBox *option_box;
public:
	wxOutputDialogBox(wxStringList *extra_media=0,wxWindow *parent=0);
	int GetSelection();
	int GetOption();
};


//***************************** Progress Indicator ***********************
class wxProgressIndicator:public wxTimer
{
private:
	wxDialogBox *dialog;
	wxForm *form;
	wxSlider *slider;
	int			cur_value;
	int 		dummy;
  Bool		done;
public:
	wxProgressIndicator(long total_time);
	~wxProgressIndicator(void);
	void Notify(void);
};

//***************************** Progress Indicator  1 *********************
class wxProgressIndicator1
{
private:
	wxDialogBox *dialog;
	wxForm *form;
	wxSlider *slider;
	int			cur_value,max;
	int 		dummy;
  Bool		done;
public:
	wxProgressIndicator1(void);
	~wxProgressIndicator1(void);
	void	SetMax(int total);
	void	Update(void);
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

// FindFile: finds the specified file in the path.  User deletes the
// result
char *wxFindFile(const char *name);
#endif //WXMISC_H
