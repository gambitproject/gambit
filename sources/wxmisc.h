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
	MyDialogBox(wxFrame *parent,char *title):wxDialogBox(parent,title,TRUE)
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
	FontDialogBox(wxFrame *frame,wxFont *def=NULL);
	~FontDialogBox(void);

	int 		FontName(void) {return f_name;}
	int			FontSize(void) {return f_size;}
	int			FontStyle(void) {return f_style;}
	int			FontWeight(void) {return f_weight;}
	int			FontUnder(void) {return f_under;}
	wxFont 	*MakeFont(void);
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


int wxListFindString(wxList *l,char *s);
char *wxFontToString(wxFont *f);
wxFont *wxStringToFont(char *s);
#endif //WXMISC_H
