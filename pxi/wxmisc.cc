#include "wx.h"
#include "wx_form.h"
#pragma hdr_stop
#include "wxmisc.h"
#include "general.h"
//***************************************************************************
//                       RANDOM USEFUL FUNCTIONS
// These will hopefully be a part of wxwin some day.  These all belong in
// different files/classes.
//***************************************************************************
// List Find String function--finds the index of a string in a wxList
int wxListFindString(wxList *l,char *s)
{
for (int i=0;i<l->Number();i++)
	if (strcmp((char *)l->Nth(i)->Data(),s)==0) return i;
return -1;
}
// Font To String--writes all the font data to a string.  Use with String To Font
// Note, returns a static buffer, copy it if needed
char *wxFontToString(wxFont *f)
{
static char fts_buffer[200];
// write the size,family,style,weight,underlining
sprintf(fts_buffer,"%d %d %d %d %d",f->GetPointSize(),f->GetFamily(),
																		f->GetStyle(),f->GetWeight(),f->GetUnderlined());
return fts_buffer;
}
// String To Font--converts the data in a string created by Font To String
// into a font.  Note that this allocated the required memory.  Delete if
// necessary.
wxFont *wxStringToFont(char *s)
{
int si,f,st,w,u;
sscanf(s,"%d %d %d %d %d",&si,&f,&st,&w,&u);
return (new wxFont(si,f,st,w,u));
}

//***************************************************************************
//                         SOME DIALOGS FOR WXWIN
// These are the commonly useful dialogs/forms that should be in the generic
// wxwin dialog code.
//***************************************************************************

// Implementation for a font selector
FontDialogBox::FontDialogBox(wxFrame *frame,wxFont *def) : MyDialogBox(frame,"Font Selection")
{
// wxWindows fonts...
f_names[0]=wxSWISS;f_names[1]=wxROMAN;f_names[2]=wxDECORATIVE;f_names[3]=wxMODERN;f_names[4]=wxSCRIPT;
f_styles[0]=wxNORMAL;f_styles[1]=wxITALIC;f_styles[2]=wxSLANT;
f_weights[0]=wxNORMAL;f_weights[1]=wxLIGHT;f_weights[2]=wxBOLD;
f_names_str[0]="Swiss";f_names_str[1]="Roman";f_names_str[2]="Decorative";f_names_str[3]="Modern";f_names_str[4]="Script";
f_styles_str[0]="Normal";f_styles_str[1]="Italic";f_styles_str[2]="Slant";
f_weights_str[0]="Normal";f_weights_str[1]="Light";f_weights_str[2]="Bold";

// Init some vars
int	 i;
char *f_name_str=new char[20];
char *f_size_str=new char[20];
char *f_weight_str=new char[20];
char *f_style_str=new char[20];
char *f_tempstr=new char[20];
Bool temp_f_under;
f_name=f_size=f_style=f_weight=-1;f_under=FALSE;
if (def==NULL)		// if creating font from scratch--no default
{
	strcpy(f_name_str,"Swiss");
	strcpy(f_size_str,"12");
	strcpy(f_weight_str,"Normal");
	strcpy(f_style_str,"Normal");
	temp_f_under=FALSE;
}
else	// if modifying an existing font--default exists
{
	strcpy(f_name_str,f_names_str[FindIntArray(f_names,5,def->GetFamily())]);
  sprintf(f_tempstr,"%d",def->GetPointSize());
	strcpy(f_size_str,f_tempstr);
	strcpy(f_weight_str,f_weights_str[FindIntArray(f_weights,3,def->GetWeight())]);
	strcpy(f_style_str,f_styles_str[FindIntArray(f_styles,3,def->GetStyle())]);
	temp_f_under=def->GetUnderlined();
}
// Create the string list for name
wxStringList *f_name_list=new wxStringList;
for (i=0;i<5;i++) f_name_list->Add(f_names_str[i]);
// Create the string list for point size
wxStringList *f_size_list=new wxStringList;
for (i=8;i<30;i++)
{
	sprintf(f_tempstr,"%d",i);
	f_size_list->Add(f_tempstr);
}
// Create the string list for style
wxStringList *f_style_list=new wxStringList;
for (i=0;i<3;i++) f_style_list->Add(f_styles_str[i]);
// Create the string list for weight
wxStringList *f_weight_list=new wxStringList;
for (i=0;i<3;i++) f_weight_list->Add(f_weights_str[i]);
// Build the dialog
Form()->Add(wxMakeFormString("Name",&f_name_str,wxFORM_CHOICE,
			 new wxList(wxMakeConstraintStrings(f_name_list), 0),NULL,wxVERTICAL));
Form()->Add(wxMakeFormString("Size",&f_size_str,wxFORM_CHOICE,
			 new wxList(wxMakeConstraintStrings(f_size_list), 0),NULL,wxVERTICAL));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormString("Style",&f_style_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(f_style_list), 0),NULL,wxVERTICAL));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormString("Weight",&f_weight_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(f_weight_list), 0),NULL,wxVERTICAL));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormBool("Underline",&temp_f_under));
Go();
// Process results of the dialog
if (Completed()==wxOK)
{
	f_name=wxListFindString(f_name_list,f_name_str);
	f_size=wxListFindString(f_size_list,f_size_str)+8;
	f_style=wxListFindString(f_style_list,f_style_str);
	f_weight=wxListFindString(f_weight_list,f_weight_str);
  f_under=temp_f_under;
}
delete [] f_name_str;
delete [] f_size_str;
delete [] f_style_str;
delete [] f_weight_str;
delete [] f_tempstr;
}

wxFont *FontDialogBox::MakeFont(void)
{
if (f_name!=-1)
	return (new wxFont(f_size,f_names[f_name],f_styles[f_style],
											f_weights[f_weight],f_under));
return NULL;
}

FontDialogBox::~FontDialogBox(void)
{}

wxProgressIndicator::wxProgressIndicator(long total_time)
{
cur_value=0;
dialog=new wxDialogBox(NULL,"Progress");
wxFormItem *slider_item=wxMakeFormShort("Done %",&dummy,wxFORM_SLIDER,
	 new wxList(wxMakeConstraintRange(0.0,100.0), 0),NULL,wxHORIZONTAL,140);
form=new wxForm(wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);
form->Add(slider_item);
form->SetEditable(FALSE);
form->AssociatePanel(dialog);
slider=(wxSlider *)slider_item->GetPanelItem();
done=FALSE;
Start(total_time/100);
dialog->Fit();
dialog->Centre();
dialog->Show(TRUE);
}

void wxProgressIndicator::Notify(void)
{
cur_value++;
if (cur_value>=100)
{
	if (done==FALSE)
  {
		done=TRUE;
		Stop();
		dialog->Show(FALSE);
		delete dialog;
		delete form;
  }
}
else
	slider->SetValue(cur_value);

}

wxProgressIndicator::~wxProgressIndicator(void)
{
if (done==FALSE)
{
	done=TRUE;
	Stop();
	dialog->Show(FALSE);
	delete dialog;
	delete form;
}
}

//************************** PROGRESS INDICATOR 1 *************************
wxProgressIndicator1::wxProgressIndicator1(void)
{
cur_value=0;
dummy=0;
dialog=new wxDialogBox(NULL,"Progress");
wxFormItem *slider_item=wxMakeFormShort("Done %",&dummy,wxFORM_SLIDER,
	 new wxList(wxMakeConstraintRange(0.0,100.0), 0),NULL,wxHORIZONTAL,140);
form=new wxForm(wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);
form->Add(slider_item);
form->SetEditable(FALSE);
form->AssociatePanel(dialog);
slider=(wxSlider *)slider_item->GetPanelItem();
done=FALSE;
dialog->Fit();
dialog->Centre();
dialog->Show(TRUE);
}

void wxProgressIndicator1::SetMax(int total)
{
	max=total;
}
void wxProgressIndicator1::Update(void)
{
cur_value++;
if (cur_value>=max)
{
	if (done==FALSE)
  {
		done=TRUE;
		dialog->Show(FALSE);
		delete dialog;
		delete form;
	}
}
else
	slider->SetValue((int)((float)cur_value/(float)max*100.0));

}

wxProgressIndicator1::~wxProgressIndicator1(void)
{
if (done==FALSE)
{
	done=TRUE;
	dialog->Show(FALSE);
	delete dialog;
	delete form;
}
}