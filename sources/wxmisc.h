//
// FILE: wxmisc.h -- Defines some generic stuff that should be in wxWin
//
// @(#)wxmisc.h	1.1 6/6/94
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
private:
	MyForm *form;
public:
	MyDialogBox(wxFrame *parent,char *title):wxDialogBox(parent,title,TRUE)
	{ form=new MyForm(wxFORM_BUTTON_OK | wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);}
  ~MyDialogBox(void){delete form;}
	int			Completed(void) {return form->Completed();}
	MyForm 	*Form(void) {return form;}
	void	  Go(void)	{form->AssociatePanel(this);Fit();Show(TRUE);}
};
#endif //WXMISC_H
