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
// Returns an wxStringList w/ string reps for integers 1..n
wxStringList* wxStringListInts(int num,wxStringList *l)
{
wxStringList *tmp=(l) ? l : new wxStringList;
char tmp_str[10];
for (int i=1;i<=num;i++) {sprintf(tmp_str,"%d",i);tmp->Add(tmp_str);}
return tmp;
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
FontDialogBox::FontDialogBox(wxWindow *parent,wxFont *def) : MyDialogBox(parent,"Font Selection")
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

//************************ OUTPUT DIALOG ***************************
wxOutputDialogBox::wxOutputDialogBox(wxStringList *extra_media,wxWindow *parent)
		:MyDialogBox(parent,"Output Media")
{
wxStringList media_list("Printer","PS File","Clipboard","Meta File",NULL);
if (extra_media)
	for (int i=0;i<extra_media->Number();i++)
		media_list.Add((const char *)(extra_media->Nth(i)->Data()));
media_box=new	wxRadioBox(this,NULL,"Media",-1,-1,-1,-1,media_list.Number(),
						 media_list.ListToArray(),(int)(media_list.Number()/2));
NewLine();
char *options[2]={"WYSIWYG","Fit to page"};
option_box=new wxRadioBox(this,NULL,NULL,-1,-1,-1,-1,2,options);
#ifdef wx_x	// Printer, Clipboard, and MetaFiles are not yet supp'ed
media_box->Enable(0,FALSE);media_box->Enable(2,FALSE);media_box->Enable(3,FALSE);
option_box->Enable(0,FALSE);option_box->Enable(1,FALSE);
#endif
Go();
}

int wxOutputDialogBox::GetSelection() {return media_box->GetSelection();}
int wxOutputDialogBox::GetOption() {return option_box->GetSelection();}

//***************************** BASIC KEYBOARD STUFF ****************
Bool	IsCursor(wxKeyEvent &ev)
{
long ch=ev.KeyCode();
return	 (ch==WXK_UP || ch==WXK_DOWN || ch==WXK_LEFT || ch==WXK_RIGHT ||
					ch==WXK_TAB || ch==WXK_RETURN);
}
Bool	IsEnter(wxKeyEvent &ev)
{return (ev.KeyCode()=='m' && ev.ControlDown());}
Bool	IsNumeric(wxKeyEvent &ev)
{long ch=ev.KeyCode(); return ((ch>'0' && ch<'9') || ch=='+' || ch=='-' ||	ch=='.');}
Bool	IsAlphaNum(wxKeyEvent &ev)
{return !(IsCursor(ev) || IsDelete(ev) || IsEnter(ev));}
Bool	IsDelete(wxKeyEvent &ev)
{
return ((ev.KeyCode()==WXK_DELETE && ev.ControlDown()) ||
				 (ev.KeyCode()==WXK_BACK && ev.ControlDown()));
}


// gDrawTextGetNum will scan the string, starting at position i,
// for a number.  It will stop at the first non-digit character.
#include <ctype.h>
#define GAMBIT_COLOR_LIST_LENGTH	11
extern char *gambit_color_list[GAMBIT_COLOR_LIST_LENGTH];
int gDrawTextGetNum(const gString &s,int *i)
{
gString tmp;
(*i)+=2;	// skip the opening {
while (isdigit(s[*i]) && *i<s.length() && s[*i]!='}')	{tmp+=s[*i];(*i)++;}
(*i)++;		// skip the closing }
return atoi((char *)tmp);
}
// gDrawText is an extension of the wxWindow's wxDC::DrawText function
// Besides providing the same features, it also supports imbedded codes
// to change the color of the output text.  The codes have the format
// of: "text[/C{#}]", where # is the number of the color to select
// from the gambit_color_list.  To print a \, use a \\.
void gDrawText(wxDC &dc,const gString &s,float x,float y)
{
int i=0,c;
float dx,dy;
gString tmp;
//gString old_foreground(wxTheColourDatabase->FindName(dc.GetTextForeground()));
gString old_foreground("BLACK");
while(i<s.length())
{
	tmp=gString();
	while (i<s.length() && s[i]!='\\') {tmp+=s[i];i++;}
	dc.DrawText((char *)tmp,x,y);
	dc.GetTextExtent((char *)tmp,&dx,&dy);
	x+=dx;
	if (s[i]=='\\')	// has to be a command
	{
		i++;
		switch (s[i])
		{
			case '\\':
				dc.DrawText("\\",x,y);
				dc.GetTextExtent("\\",&dx,&dy);
				x+=dx;i++;
				break;
			case 'C' :
				c=gDrawTextGetNum(s,&i);
				dc.SetTextForeground(wxTheColourDatabase->FindColour(gambit_color_list[c]));
				break;
			default:
				wxError("Unknown code in gDrawText");
				break;
		}
	}
}
dc.SetTextForeground(wxTheColourDatabase->FindColour(old_foreground));
}

