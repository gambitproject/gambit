//
// FILE: wxmisc.h -- Defines some generic stuff that should be in wxWin
//
// $Id$
//

#ifndef WXMISC_H
#define WXMISC_H
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
public:
	MyForm *form;
	MyDialogBox(wxFrame *parent,char *title):wxDialogBox(parent,title,TRUE)
	{ form=new MyForm(wxFORM_BUTTON_OK | wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);}
  ~MyDialogBox(void){delete form;}
	int		Completed(void) {return form->Completed();}
};
#endif //WXMISC_H
