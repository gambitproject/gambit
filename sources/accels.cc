// File: accels.cc -- functions for working with accelerators
// $Id$
//

#include "wx.h"
#include "wxmisc.h"
#include "keynames.h"
#include "gmisc.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

#include "glist.imp"
#include "accels.h"
TEMPLATE class gList<Accel>;
TEMPLATE class gNode<Accel>;
#include "garray.imp"
TEMPLATE class gArray<AccelEvent>;
#ifdef __BORLANDC__
#pragma option -Jgx
#endif

gOutput &operator<<(gOutput &o,const AccelEvent &p)
{o<<p.name<<' '<<p.id<<'\n'; return o;}
gOutput &operator<<(gOutput &o,const Accel &p)
{o<<"Ctrl "<<p.ctrl<<" Shift "<<p.shift<<" Key "<<p.key<<" Id "<<p.id<<'\n'; return o;}

char *accel_state_str[3]={"Off", "On",  "??" };

gOutput &operator<<(gOutput &op,const gList<Accel> &p);
gOutput &operator<<(gOutput &op,const gArray<AccelEvent> &p);



//**************************************************************************
//*                          ACCELERATOR EDIT DIALOG                       *
//**************************************************************************

// Edit Dialog--a graphical way to add/remove accelerators
class EditAccelDialog: public wxDialogBox
{
private:
	// Panel items for reference within the program.
	wxText *key_text;
	wxChoice *key_choice;
	wxRadioBox *key_ctrl;
	wxRadioBox *key_shift;
	wxChoice *event_choice;
	// Actual data
	gList<Accel> &acc;
	const gArray<AccelEvent> &events;
	// Event handlers.  Low level
	static void key_text_func(wxText& ob, wxCommandEvent& ev);
	static void key_choice_func(wxChoice &ob,wxCommandEvent& ev);
	static void event_func(wxChoice& ob, wxCommandEvent& ev);
	static void add_func(wxButton& ob, wxCommandEvent& ev);
	static void delete_func(wxButton& ob, wxCommandEvent& ev);
	static void close_func(wxButton& ob, wxCommandEvent& ev);
	static void help_func(wxButton& ob, wxCommandEvent& ev);
	// Event handlers. High level
	void OnEventFunc(const gString &name);
	void OnAddFunc(void);
	void OnDeleteFunc(void);
	// Subsidiary functions
	int	 GetEventId(const gString &name);
	int	 RealKey(void);
public:
	// Constructor and destructor
	EditAccelDialog(gList<Accel> &acc_,const gArray<AccelEvent> &events_,wxFrame *parent=0, char *title="Accelerators");
	~EditAccelDialog(void);
};

EditAccelDialog::EditAccelDialog(gList<Accel> &acc_,const gArray<AccelEvent> &events_,wxFrame *parent, char *title):
	wxDialogBox(parent, title, TRUE),acc(acc_),events(events_)
{
// Create panel items
new wxGroupBox(this, "Key", 4, 2, 212, 218, 0, "key_box");
key_text = new wxText(this, (wxFunction)key_text_func, NULL, "", 21, 36, 28, 30, 0, "key_text");
key_choice = new wxChoice(this, (wxFunction)key_choice_func, NULL, 68, 36, 105, -1,NUM_KEYS, key_names, 0, "key_choice");

char **event_names=new char *[events.Length()];
for (int i=1;i<=events.Length();i++) event_names[i-1]=events[i].name;
SetLabelPosition(wxVERTICAL);
event_choice = new wxChoice(this, (wxFunction)event_func, "Event", 240, 96, -1, -1, events.Length(),event_names, 0, "event_choice");
SetLabelPosition(wxHORIZONTAL);
wxButton *add_button = new wxButton(this, (wxFunction)add_func, "Add", 23, 242, -1, -1, 0, "add_button");
wxButton *delete_button = new wxButton(this, (wxFunction)delete_func, "Delete", 114, 242, 80, -1, 0, "delete_button");
wxButton *close_button = new wxButton(this, (wxFunction)close_func, "Close", 214, 242, -1, -1, 0, "close_button");
                         new wxButton(this, (wxFunction)help_func, "Help", 294, 242, -1, -1, 0, "help_button");
key_ctrl = new wxRadioBox(this, NULL, "Ctrl", 23, 78, -1, -1, 3, accel_state_str, 3, wxHORIZONTAL, "ctrl_state");
key_shift = new wxRadioBox(this, NULL, "Shift", 22, 143, -1, -1, 3,accel_state_str, 3, wxHORIZONTAL, "shift_state");

add_button->SetClientData((char *)this);
delete_button->SetClientData((char *)this);
close_button->SetClientData((char *)this);
event_choice->SetClientData((char *)this);
key_text->SetClientData((char *)key_choice);
key_choice->SetClientData((char *)key_text);

key_choice->SetSelection(0);
// Fit panel to contents
Fit();
Show(TRUE);

}

EditAccelDialog::~EditAccelDialog(void)
{}

// Find the event's id
int EditAccelDialog::GetEventId(const gString &name)
{
int id=0;
for (int i=1;i<=events.Length() && !id;i++) if (events[i].name==name) id=events[i].id;
return id;
}

// Checks key_text and key_choice, and determines the real key id selected
int EditAccelDialog::RealKey(void)
{
if (key_choice->GetSelection()) return Name2Key(key_choice->GetStringSelection());
if (key_text->GetValue()) return (int)(key_text->GetValue()[0]);
return 0;
}

// The purpose of this function is to just limit the entry to 1 char and
// to set key_choice to selection 0
void EditAccelDialog::key_text_func(wxText& ob, wxCommandEvent& )
{
char *v=ob.GetValue();
if (v)
{
	if (strlen(v)>1)
	{char tmp_str[2];sprintf(tmp_str,"%c",v[0]);ob.SetValue(tmp_str);}
	if (strcmp(v,"")!=0)	// if key_text is non-blank
	{wxChoice *key_choice=(wxChoice *)ob.GetClientData();key_choice->SetSelection(0);}
}
}

// This function just sets the key_text to nothing
void EditAccelDialog::key_choice_func(wxChoice &ob,wxCommandEvent &)
{
wxText *key_text=(wxText *)ob.GetClientData();
key_text->SetValue("");
}

// Check to see if this event is already defined.  If so, fill in the fields
void EditAccelDialog::OnEventFunc(const gString &name)
{
int i,id=GetEventId(name);
// Now, check if it is defined
for (i=1;i<=acc.Length();i++)
	if (acc[i].id==id)
	{
		if (!Key2Name(acc[i].key))	// if this is not a 'special' key, its just ASCII
		{
			char tmp_str[2];
			sprintf(tmp_str,"%c",(char)acc[i].key);
			key_text->SetValue(tmp_str);
			key_choice->SetStringSelection(BLANK_STR);
		}
		else
		{
			key_text->SetValue("");
			key_choice->SetStringSelection(Key2Name(acc[i].key));
		}
		key_ctrl->SetStringSelection(accel_state_str[acc[i].ctrl]);
		key_shift->SetStringSelection(accel_state_str[acc[i].shift]);
		return;
	}
// if we got here, this key is not defined.  Clear all the settings
key_text->SetValue("");
key_choice->SetSelection(0);
key_ctrl->SetSelection(0);
key_shift->SetSelection(0);
}

void EditAccelDialog::event_func(wxChoice& ob, wxCommandEvent& )
{
EditAccelDialog *parent=(EditAccelDialog *)ob.GetClientData();
parent->OnEventFunc(ob.GetStringSelection());
}

void EditAccelDialog::OnAddFunc(void)
{
// First, check if this key combo is used elsewhere
int id=GetEventId(event_choice->GetStringSelection());
int acc_id=0;
for (int i=1;i<=acc.Length();i++)
	if (
		(acc[i].ctrl==key_ctrl->GetSelection() || key_ctrl->GetSelection()==aEither) &&
		(acc[i].shift==key_shift->GetSelection() || key_shift->GetSelection()==aEither) &&
		acc[i].key==RealKey() && acc[i].id!=id)
		{
			acc_id=i;
			wxMessageBox("This key combination is used elsewhere");
			return;
		}
// If we got here, this is fine.
Accel temp;
temp.ctrl=key_ctrl->GetSelection();temp.shift=key_shift->GetSelection();
temp.key=RealKey();temp.id=id;
if (acc_id) acc[acc_id]=temp; else acc+=temp;
}

void EditAccelDialog::add_func(wxButton& ob, wxCommandEvent& )
{
EditAccelDialog *parent=(EditAccelDialog *)ob.GetClientData();
parent->OnAddFunc();
}

void EditAccelDialog::OnDeleteFunc(void)
{
int id=GetEventId(event_choice->GetStringSelection());
for (int i=1;i<=acc.Length();i++)	if (acc[i].id==id) acc.Remove(i);
}

void EditAccelDialog::delete_func(wxButton& ob, wxCommandEvent& )
{
EditAccelDialog *parent=(EditAccelDialog *)ob.GetClientData();
parent->OnDeleteFunc();
}

void EditAccelDialog::close_func(wxButton& ob, wxCommandEvent& )
{
EditAccelDialog *parent=(EditAccelDialog *)ob.GetClientData();
parent->Show(FALSE);
}
void EditAccelDialog::help_func(wxButton& , wxCommandEvent& )
{wxHelpContents(ACCELERATORS_HELP);}

//**************************************************************************
//*                       FUNCTIONS FOR WORKING WITH ACCELERATORS          *
//**************************************************************************

// Reads in accelerators from a data file.  User can set both filename and section to read from
int ReadAccelerators(gList<Accel> &list,const char *section,const char *file_name)
{
char 	tmp_str[100],*tmp_str1=new char[100];
int		fields;

int num_accels=0;
list.Flush();
wxGetResource(section,"Num-Accells",&num_accels,file_name);
for (int i=1;i<=num_accels;i++)
{
	Accel tmp_acc;
	sprintf(tmp_str,"Accel%d",i);
	wxGetResource(section,tmp_str,&tmp_str1,file_name);
	fields=sscanf(tmp_str1,"%d %d %ld %ld",&tmp_acc.ctrl,&tmp_acc.shift,&tmp_acc.key,&tmp_acc.id);
	assert(fields==4 && "Accels::Invalid DataFormat");
	list+=tmp_acc;
}
delete [] tmp_str1;
return 1;
}
// Saves accelerators to a data file. User can set both filename and section to write to
int WriteAccelerators(const gList<Accel> &list,const char *section,const char *file_name)
{
char tmp_str[100],tmp_str1[100];

int num_accels=list.Length();
wxWriteResource(section,"Num-Accells",num_accels,file_name);
for (int i=1;i<=num_accels;i++)
{
	Accel tmp_acc=list[i];
	sprintf(tmp_str,"Accel%d",i);
	sprintf(tmp_str1,"%d %d %ld %ld",tmp_acc.ctrl,tmp_acc.shift,tmp_acc.key,tmp_acc.id);
	wxWriteResource(section,tmp_str,tmp_str1,file_name);
}
return 1;
}
// Checks if this keyevent triggers an accelerator.  If so, return the id of triggered event
long CheckAccelerators(gList<Accel> &list,wxKeyEvent &ev)
{
for (int i=1;i<=list.Length();i++) if (list[i]==ev) return list[i].id;
return 0;
}

// Starts up the dialog

void EditAccelerators(gList<Accel> &list,const gArray<AccelEvent> &events)
{
EditAccelDialog *a=new EditAccelDialog(list,events);
delete a;
}
