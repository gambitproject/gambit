// File: wxmisc.cc -- a few general purpose functions that rely on and enhance
// wxwin.
// $Id$
#include "wx.h"
#include "wx_form.h"
#include "wx_help.h"
#pragma hdr_stop
#define WXMISC_C
#include "wxmisc.h"
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
// Find In Array function--finds an integer in an array of integers, returns index
int FindIntArray(int *array,int num,int what)
{
for (int i=0;i<num;i++) if (array[i]==what) return i;
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

// FindFile
char *wxFindFile(const char *name)
{
wxPathList path_list;
path_list.AddEnvList("PATH");
char *file_name=path_list.FindValidPath((char *)name);
return  (file_name) ? copystring(file_name) : 0;
}

// OutputFile: takes the input filename, strips it of the path and extension and
// appends a ".pxi" extension.
char *wxOutputFile(const char *name)
{
static char	t_outfile[250];
strcpy(t_outfile,FileNameFromPath((char *)name)); // strip the path
char *period=strchr(t_outfile,'.'); // strip the extension
if (period) t_outfile[period-t_outfile]='\0';
strcat(t_outfile,".pxi"); // add a ".pxi" extension
return t_outfile;
}

// Help system functions.
wxHelpInstance *help_instance = 0;
void wxInitHelp(const char *name,const char *help_about_str)
{
if (!help_instance)
{
	help_instance = new wxHelpInstance(TRUE);
	help_instance->Initialize((char *)name);
}
if (help_about_str) wxHelpAbout(help_about_str);
}
void wxHelpContents(const char *section)
{
help_instance->LoadFile();
if (!section)
	help_instance->DisplayContents();
else
	help_instance->KeywordSearch((char *)section);
}
void wxHelpAbout(const char *helpstr)
{
static char *help_str="Product based on wxWin";
if (helpstr)	// init with a new string
	help_str=strdup(helpstr);
else
	wxMessageBox(help_str,"Help About");
}
void wxKillHelp(void)
{
if (help_instance) {help_instance->Quit();delete help_instance;}
}

//***************************************************************************
//                         SOME DIALOGS FOR WXWIN
// These are the commonly useful dialogs/forms that should be in the generic
// wxwin dialog code.
//***************************************************************************

// The basic dialog w/ a form
// The Form
MyForm::MyForm(MyDialogBox *p,Bool help) :
wxForm(wxFORM_BUTTON_OK|wxFORM_BUTTON_CANCEL|((help) ? wxFORM_BUTTON_HELP : 0),wxFORM_BUTTON_AT_BOTTOM), parent(p)
{ }
void MyForm::OnOk(void) {parent->OnOk();}
void MyForm::OnCancel(void) {parent->OnCancel();}
void MyForm::OnHelp(void) {parent->OnHelp();}

//The dialog
MyDialogBox::MyDialogBox(wxWindow *parent,char *title,const char *hlp_str) :
 wxDialogBox(parent,title,TRUE),help_str(0)
{
form=new MyForm(this,(hlp_str) ? TRUE : FALSE);
if (hlp_str) help_str=copystring(hlp_str);
}
MyDialogBox::~MyDialogBox(void)
{
// @@ delete form;
if (help_str) delete [] help_str;
}

Bool MyDialogBox::Completed(void) const {return completed;}
MyForm *MyDialogBox::Form(void) {return form;}
void MyDialogBox::Go(void) {form->AssociatePanel(this);Fit();Centre();Show(TRUE);}
void MyDialogBox::Go1(void) {Fit();Centre();Show(TRUE);}
void MyDialogBox::SetHelpString(const char *hlp_str)
{if (help_str) delete [] help_str; help_str=0; if (hlp_str) help_str=copystring(hlp_str);}
void MyDialogBox::OnOk(void) {completed=wxOK;Show(FALSE);}
void MyDialogBox::OnCancel(void) {completed=wxCANCEL;Show(FALSE);}
void MyDialogBox::OnHelp(void) {if (help_str) wxHelpContents(help_str);}

// Implementation for a message box with help capability
MyMessageBox::MyMessageBox(const char *message,const char *caption,const char *help_str,wxWindow *parent)
: MyDialogBox(parent,(char *)caption,help_str)
{
Add(wxMakeFormMessage((char *)message));
Go();
}
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


//************************ OUTPUT DIALOG ***************************
wxOutputDialogBox::wxOutputDialogBox(wxStringList *extra_media,wxWindow *parent)
		:MyDialogBox(parent,"Output Media")
{
wxStringList media_list("Printer","PS File","Clipboard","Meta File","PrintPreview",0);
if (extra_media)
	for (int i=0;i<extra_media->Number();i++)
		media_list.Add((const char *)(extra_media->Nth(i)->Data()));
media_box=new	wxRadioBox(this,NULL,"Media",-1,-1,-1,-1,media_list.Number(),
						 media_list.ListToArray(),(int)(media_list.Number()/2));
NewLine();
fit_box=new wxCheckBox(this,0,"Fit to page");
#ifdef wx_x	// Printer, Clipboard, and MetaFiles are not yet supp'ed
media_box->Enable(0,FALSE);media_box->Enable(2,FALSE);media_box->Enable(3,FALSE);
fit_box->Enable(FALSE);
#endif
Go();
}

wxOutputMedia wxOutputDialogBox::GetMedia(void)
{
switch (media_box->GetSelection())
{
case 0: return wxMEDIA_PRINTER;
case 1: return wxMEDIA_PS;
case 2: return wxMEDIA_CLIPBOARD;
case 3: return wxMEDIA_METAFILE;
case 4: return wxMEDIA_PREVIEW;
}
return wxMEDIA_NUM;
}

int wxOutputDialogBox::GetExtraMedia(void)
{return (media_box->GetSelection()>=wxMEDIA_NUM) ? media_box->GetSelection() : -1;}

wxOutputOption wxOutputDialogBox::GetOption(void)
{return (fit_box->GetValue()) ? wxFITTOPAGE : wxWYSIWYG;}

Bool wxOutputDialogBox::ExtraMedia(void)
{return (media_box->GetSelection()>=wxMEDIA_NUM);}

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
{long ch=ev.KeyCode(); return ((ch>='0' && ch<='9') || ch=='-' ||	ch=='.');}
Bool	IsAlphaNum(wxKeyEvent &ev)
{return !(IsCursor(ev) || IsDelete(ev) || IsEnter(ev));}
Bool	IsDelete(wxKeyEvent &ev)
{
return ((ev.KeyCode()==WXK_DELETE) ||
				 (ev.KeyCode()==WXK_BACK));
}


// gDrawTextGetNum will scan the string, starting at position i,
// for a number.  It will stop at the first non-digit character.
#include <ctype.h>
#include <stdlib.h>
int gDrawTextGetNum(const gString &s,int *i)
{
gString tmp;
(*i)+=2;	// skip the opening {
while (isdigit(s[*i]) && *i<s.length() && s[*i]!='}')	{tmp+=s[*i];(*i)++;}
(*i)++;		// skip the closing }
return atoi((char *)tmp);
}
// Hack to be able to parse gPolys (x^2 -> x\^2\~).  No error checking: a bad
// input will cause a crash.
gString gDrawTextPreParse(const gString &s)
{
if (((gString &)s).lastOccur('^')==0) return s;
gString tmp;
for (int n=0;n<s.length();n++)
	if (s[n]!='^')
   	tmp+=s[n];
   else
   {
		tmp+="\\^";
      while (isdigit(s[++n]) && n<s.length()) tmp+=s[n];
      tmp+="\\~";
      if (n<s.length()) tmp+=s[n];
   }
return tmp;
}

// gDrawText is an extension of the wxWindow's wxDC::DrawText function
// Besides providing the same features, it also supports imbedded codes
// to change the color of the output text.  The codes have the format
// of: "text[/C{#}]", where # is the number of the color to select
// from the wx_color_list.  To print a \, use a \\.
void gDrawText(wxDC &dc,const gString &s0,float x,float y)
{
int i=0,c;
float dx,dy;
gString s=gDrawTextPreParse(s0);
gString tmp;
//gString old_foreground(wxTheColourDatabase->FindName(dc.GetTextForeground()));
gString old_foreground("BLACK");
wxFont *old_font=0,*small_font=0;
float	old_y=0;int old_size=0;

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
			case 'C':
				c=(gDrawTextGetNum(s,&i)%WX_COLOR_LIST_LENGTH);
				dc.SetTextForeground(wxTheColourDatabase->FindColour(wx_color_list[c]));
				break;
			case '^':		// Start superscript
				if (!old_font) old_font=dc.GetFont();
				if (!old_size) old_size=old_font->GetPointSize();
				if (!small_font) small_font=wxTheFontList->FindOrCreateFont(old_size*2/3,old_font->GetFamily(),old_font->GetStyle(),old_font->GetWeight());
				dc.SetFont(small_font);
				old_y=y;y-=dy/4;
				i++;
				break;
			case '_':		// Start subscript
				if (!old_font) old_font=dc.GetFont();
				if (!old_size) old_size=old_font->GetPointSize();
				if (!small_font) small_font=wxTheFontList->FindOrCreateFont(old_size*2/3,old_font->GetFamily(),old_font->GetStyle(),old_font->GetWeight());
				dc.SetFont(small_font);
				old_y=y;y+=dy*2/3;
				i++;
				break;
			case '~':		// Stop sub/super script
				if (old_font) {dc.SetFont(old_font);y=old_y;i++;}
				break;
			default:
				wxError("Unknown code in gDrawText");
				break;
		}
	}
}
dc.SetTextForeground(wxTheColourDatabase->FindColour(old_foreground));
if (old_font) dc.SetFont(old_font);
}

gGetTextExtent(wxDC &dc,const gString &s0, float *x, float *y)
{
int i=0,c;
float dx,dy;
gString s=gDrawTextPreParse(s0);
gString tmp;
wxFont *old_font=0,*small_font=0;
float	old_y=0;int old_size=0;
*x=0;*y=0;

while(i<s.length())
{
	tmp=gString();
	while (i<s.length() && s[i]!='\\') {tmp+=s[i];i++;}
	dc.GetTextExtent((char *)tmp,&dx,&dy);
	*x+=dx;if (dy<*y) *y=dy;
	if (s[i]=='\\')	// has to be a command
	{
		i++;
		switch (s[i])
		{
			case '\\':
				dc.GetTextExtent("\\",&dx,&dy);
				*x+=dx;i++;
				break;
			case 'C':
				c=(gDrawTextGetNum(s,&i)%WX_COLOR_LIST_LENGTH);
				break;
			case '^':		// Start superscript
				if (!old_font) old_font=dc.GetFont();
				if (!old_size) old_size=old_font->GetPointSize();
				if (!small_font) small_font=wxTheFontList->FindOrCreateFont(old_size*2/3,old_font->GetFamily(),old_font->GetStyle(),old_font->GetWeight());
				dc.SetFont(small_font);
				old_y=*y;*y-=dy/4;
				i++;
				break;
			case '_':		// Start subscript
				if (!old_font) old_font=dc.GetFont();
				if (!old_size) old_size=old_font->GetPointSize();
				if (!small_font) small_font=wxTheFontList->FindOrCreateFont(old_size*2/3,old_font->GetFamily(),old_font->GetStyle(),old_font->GetWeight());
				dc.SetFont(small_font);
				old_y=*y;*y+=dy*2/3;
				i++;
				break;
			case '~':		// Stop sub/super script
				if (old_font) {dc.SetFont(old_font);*y=old_y;i++;}
				break;
			default:
				wxError("Unknown code in gDrawText");
				break;
		}
	}
}
if (old_font) dc.SetFont(old_font);
}

// Takes a string formated for gDrawText and returns just the text value of it.
gString gPlainText(const gString &s)
{
int i=0;
gString plain;
while(i<s.length())
{
	while (i<s.length() && s[i]!='\\') {plain+=s[i];i++;}
	if (s[i]=='\\')	// has to be a command
	{
		i++;
		switch (s[i])
		{
			case '\\':
				plain+="\\";
				break;
			case 'C' :
				gDrawTextGetNum(s,&i); // just absorb that info
				break;
         case '^' :
         case '_' :
         	plain+=s[i];
            break;
         case '~' :
         	break;
			default:
				wxError("Unknown code in gDrawText");
				break;
		}
	}
}
return plain;
}

// gGetTextLine is a fancy version of the wxGetTextFrom user.  This single line
// input allows for all of the capability of gDrawText, line editing using
// cursor keys and the mouse.
class gGetTextCanvas;
class gGetTextFrame : public wxFrame
{
private:
	gGetTextCanvas *d;
public:
	gGetTextFrame(const gString s0, wxFrame *frame, int x,int y, const char *title, bool titlebar);
	Bool OnClose(void);
   int  Completed(void) const;
   gString GetString(void) const;
};

class gGetTextCanvas: public wxCanvas
{
private:
	int completed;
	gGetTextFrame *parent;
	gString s;
   float w;
   int pos;
public:
	gGetTextCanvas(gGetTextFrame *p,const gString s0);
	void OnChar(wxKeyEvent &ev);
	void OnEvent(wxMouseEvent &ev);
	void OnPaint(void);
   void SetCompleted(int c);
   gString GetString(void) const;
   int Completed(void) const;
};



#define GETTEXT_FRAME_WIDTH	100
#define GETTEXT_FRAME_HEIGHT	30
#define GETTEXT_FRAME_WIDTH_M	300
#define TEXT_OFF				5

gGetTextFrame::gGetTextFrame(const gString s0, wxFrame *frame,int x, int y,
                             const char *title,bool titlebar):
	wxFrame(frame, title,x,y,GETTEXT_FRAME_WIDTH,GETTEXT_FRAME_HEIGHT,
    (titlebar) ? wxDEFAULT_FRAME : wxRESIZE_BORDER)
{
d=new gGetTextCanvas(this,s0);
}

Bool gGetTextFrame::OnClose()
{
d->SetCompleted(wxCANCEL);
Show(FALSE);
return FALSE;

}

int gGetTextFrame::Completed(void) const
{return d->Completed();}

gString gGetTextFrame::GetString(void) const
{return d->GetString();}

gGetTextCanvas::gGetTextCanvas(gGetTextFrame *parent_,const gString s0):
			wxCanvas(parent_),s(s0),parent(parent_)
{
SetBackground(wxTheBrushList->FindOrCreateBrush("YELLOW",wxSOLID));
SetFont(wxTheFontList->FindOrCreateFont(12,wxSWISS,wxNORMAL,wxNORMAL));
GetDC()->SetBackgroundMode(wxTRANSPARENT);
pos=s.length();
float h;
gGetTextExtent(*GetDC(),s,&w,&h);
parent->SetSize(-1,-1,w*3/2,-1,wxSIZE_USE_EXISTING);
completed=wxRUNNING;
}

void gGetTextCanvas::OnChar(wxKeyEvent &ev)
{
switch(ev.KeyCode())
{
case WXK_ESCAPE: completed=wxCANCEL; break;
case WXK_RETURN: completed=wxOK; break;
case WXK_LEFT:
{
	if (pos<1) break;
   pos--;
   OnPaint();
   break;
}
case WXK_RIGHT:
{
	if (pos>s.length()-1) break;
   pos++;
   OnPaint();
   break;
}

case WXK_DELETE:
case WXK_BACK:
{
	if (pos<1) break;
   pos--;
   s.remove(pos);
   float h;
   gGetTextExtent(*GetDC(),s,&w,&h);
	int cur_w,cur_h;
	GetSize(&cur_w,&cur_h);
   if (w<cur_w*2/3 && w>GETTEXT_FRAME_WIDTH)
   {
   	cur_w=w+3*TEXT_OFF;
      parent->SetSize(-1,-1,cur_w,-1,wxSIZE_USE_EXISTING);
   }
   else
	   OnPaint();
   break;
}
default:
{
	s.insert((char)ev.KeyCode(),pos);
   pos++;
   float h;
   gGetTextExtent(*GetDC(),s,&w,&h);
	int cur_w,cur_h;
	GetSize(&cur_w,&cur_h);
   if (w>cur_w && cur_w<GETTEXT_FRAME_WIDTH_M)
   {
   	cur_w=w*3/2+3*TEXT_OFF;
      parent->SetSize(-1,-1,cur_w,-1,wxSIZE_USE_EXISTING);
   }
   else
	   OnPaint();
   break;
}
}
}

void gGetTextCanvas::OnPaint(void)
{
Clear();
gDrawText(*GetDC(),s,TEXT_OFF,TEXT_OFF);
float h;
gGetTextExtent(*GetDC(),s.left(pos),&w,&h);
SetPen(wxGREEN_PEN);
DrawLine(w+TEXT_OFF*3/2,0,w+TEXT_OFF*3/2,GETTEXT_FRAME_HEIGHT);
}

void gGetTextCanvas::OnEvent(wxMouseEvent &ev)
{
if (ev.LeftDown())
{
	float tw,th,tw1=-1;
   // Find out where we clicked
   int npos=-1;
   for (int tpos=1;tpos<=s.length() && npos==-1;tpos++)
   {
	   gGetTextExtent(*GetDC(),s.left(tpos),&tw,&th);
   	if (ev.x>tw1 && ev.x<=tw) npos=tpos-1;
      tw1=tw;
   }
   if (npos==-1) npos=s.length();
   pos=npos;
   OnPaint();
}
}

int gGetTextCanvas::Completed(void) const
{return completed;}

gString gGetTextCanvas::GetString(void) const
{return s;}


void gGetTextCanvas::SetCompleted(int c)
{completed=c;}

gString gGetTextLine(const gString &s0,wxFrame *parent,int x,int y,
                     const char *title,bool titlebar)
{
gGetTextFrame *f=new gGetTextFrame(s0,parent,x,y,title,titlebar);
f->Show(TRUE);
while (f->Completed()==wxRUNNING) wxYield();
gString result_str;
if (f->Completed()==wxOK) result_str=f->GetString();
delete f;
return result_str;
}

