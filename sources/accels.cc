//
// FILE: accels.cc -- functions for working with accelerators
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"
#include "gmisc.h"

#include "glist.imp"
#include "accels.h"
template class gList<Accel>;
#include "garray.imp"
template class gArray<AccelEvent>;

gOutput &operator<<(gOutput &o, const AccelEvent &p)
{
    o << p.name << ' ' << p.id << '\n';
    return o;
}

gOutput &operator<<(gOutput &o, const Accel &p)
{
    o << "Ctrl " << p.ctrl << " Shift " << p.shift << " Key " << p.key << " Id " << p.id << '\n';
    return o;
}

char *accel_state_str[3] = {"Off", "On",  "??" };

gOutput &operator<<(gOutput &op, const gList<Accel> &p);
gOutput &operator<<(gOutput &op, const gArray<AccelEvent> &p);


#define NUM_KEYS	74		// total # of keys defined here
#define BLANK_STR "      " 	// just a blank string of 7 spaces--maps to 0 key
int key_values[NUM_KEYS]=
{
0		,
8   ,
9   ,
13  ,
27  ,
32  ,
127 ,
300	,
301	,
302	,
303	,
304	,
305	,
306	,
307	,
308	,
309	,
310	,
311	,
312	,
313	,
314	,
315	,
316	,
317	,
318	,
319	,
320	,
321	,
322	,
323	,
324	,
325	,
326	,
327	,
328	,
329	,
330	,
331	,
332	,
333	,
334	,
335	,
336	,
337	,
338	,
339	,
340	,
341	,
342	,
343	,
344	,
345	,
346	,
347	,
348	,
349	,
350	,
351	,
352	,
353	,
354	,
355	,
356	,
357	,
358	,
359	,
360	,
361	,
362	,
363	,
364	,
365	,
366
};

char *key_names[NUM_KEYS]=
{
BLANK_STR,
"BACK",
"TAB",
"RETURN",
"ESCAPE",
"SPACE",
"DELETE",
"START", /*=300*/
"LBUTTON",
"RBUTTON",
"CANCEL",
"MBUTTON",
"CLEAR",
"SHIFT",
"CONTROL",
"MENU",
"PAUSE",
"CAPITAL",
"PRIOR",
"NEXT",
"END",
"HOME",
"LEFT",
"UP",
"RIGHT",
"DOWN",
"SELECT",
"PRINT",
"EXECUTE",
"SNAPSHOT",
"INSERT",
"HELP",
"NUMPAD0",
"NUMPAD1",
"NUMPAD2",
"NUMPAD3",
"NUMPAD4",
"NUMPAD5",
"NUMPAD6",
"NUMPAD7",
"NUMPAD8",
"NUMPAD9",
"MULTIPLY",
"ADD",
"SEPARATOR",
"SUBTRACT",
"DECIMAL",
"DIVIDE",
"F1",
"F2",
"F3",
"F4",
"F5",
"F6",
"F7",
"F8",
"F9",
"F10",
"F11",
"F12",
"F13",
"F14",
"F15",
"F16",
"F17",
"F18",
"F19",
"F20",
"F21",
"F22",
"F23",
"F24",
"NUMLOCK",
"SCROLL"
};

static int Name2Key(const char *name)
{
  for (int i=0;i<NUM_KEYS;i++) if (strcmp(name,key_names[i])==0) return key_values[i];
  return 0;
}

static char *Key2Name(int key)  
{
  for (int i=0;i<NUM_KEYS;i++) if (key_values[i]==key) return key_names[i];
  return 0;
}

//**************************************************************************
//*                          ACCELERATOR EDIT DIALOG                       *
//**************************************************************************

class EditAccelDialog : public guiAutoDialog {
private:
  wxText *m_keyText;
  wxChoice *m_keyChoice, *m_eventChoice;
  wxRadioBox *m_keyCtrl, *m_keyShift;

  gList<Accel> &acc;
  const gArray<AccelEvent> &events;
    
  static void CallbackKeyText(wxText &p_object, wxCommandEvent &p_event);
  static void CallbackKeyChoice(wxChoice &p_object, wxCommandEvent &)
    { ((wxText *) p_object.GetClientData())->SetValue(""); }
  static void CallbackEvent(wxChoice &p_object, wxCommandEvent &)
    { ((EditAccelDialog *) p_object.GetClientData())->
	OnChangeEvent(p_object.GetStringSelection()); }
  static void CallbackAdd(wxButton &p_object, wxCommandEvent &)
    { ((EditAccelDialog *) p_object.GetClientData())->OnAdd(); }
  static void CallbackDelete(wxButton &p_object, wxCommandEvent &)
    { ((EditAccelDialog *) p_object.GetClientData())->OnDelete(); }
    
  void OnChangeEvent(const gText &name);
  void OnAdd(void);
  void OnDelete(void);
    
  int GetEventId(const gText &name);
  int RealKey(void);

  const char *HelpString(void) const { return "Accelerator Keys"; }
    
public:
  // Constructor and destructor
  EditAccelDialog(gList<Accel> &acc_, const gArray<AccelEvent> &events_, 
		  wxFrame *parent = 0, char *title = "Accelerators");
  virtual ~EditAccelDialog() { }
};


EditAccelDialog::EditAccelDialog(gList<Accel> &acc_,
				 const gArray<AccelEvent> &events_, 
                                 wxFrame *parent, char *title)
  : guiAutoDialog(parent, title), acc(acc_), events(events_)
{
  wxGroupBox *keyGroup = new wxGroupBox(this, "Key");

  m_keyText = new wxText(this, (wxFunction) CallbackKeyText, NULL, "     ");
  m_keyText->SetConstraints(new wxLayoutConstraints);
  m_keyText->GetConstraints()->top.SameAs(keyGroup, wxTop, 20);
  m_keyText->GetConstraints()->left.SameAs(keyGroup, wxLeft, 10);
  m_keyText->GetConstraints()->height.AsIs();
  m_keyText->GetConstraints()->width.AsIs();

  m_keyChoice = new wxChoice(this, (wxFunction) CallbackKeyChoice, NULL,
			     1, 1, -1, -1, NUM_KEYS, key_names, 0);
  m_keyChoice->SetConstraints(new wxLayoutConstraints);
  m_keyChoice->GetConstraints()->top.SameAs(m_keyText, wxTop);
  m_keyChoice->GetConstraints()->left.SameAs(m_keyText, wxRight, 10);
  m_keyChoice->GetConstraints()->height.AsIs();
  m_keyChoice->GetConstraints()->width.AsIs();
    
  m_keyText->SetClientData((char *) m_keyChoice);
  m_keyChoice->SetClientData((char *) m_keyText);
  m_keyChoice->SetSelection(0);

  m_keyCtrl  = new wxRadioBox(this, 0, "Ctrl", 
			      1, 1, -1, -1, 3, accel_state_str,
			      3, wxHORIZONTAL); 
  m_keyCtrl->SetConstraints(new wxLayoutConstraints);
  m_keyCtrl->GetConstraints()->top.SameAs(m_keyText, wxBottom, 10);
  m_keyCtrl->GetConstraints()->left.SameAs(m_keyText, wxLeft);
  m_keyCtrl->GetConstraints()->width.AsIs();
  m_keyCtrl->GetConstraints()->height.AsIs();

  m_keyShift = new wxRadioBox(this, 0, "Shift", 
			      1, 1, -1, -1, 3, accel_state_str,
			      3, wxHORIZONTAL); 
  m_keyShift->SetConstraints(new wxLayoutConstraints);
  m_keyShift->GetConstraints()->top.SameAs(m_keyCtrl, wxBottom, 10);
  m_keyShift->GetConstraints()->left.SameAs(m_keyCtrl, wxLeft);
  m_keyShift->GetConstraints()->width.AsIs();
  m_keyShift->GetConstraints()->height.AsIs();
  
  keyGroup->SetConstraints(new wxLayoutConstraints);
  keyGroup->GetConstraints()->top.SameAs(this, wxTop, 10);
  keyGroup->GetConstraints()->left.SameAs(this, wxLeft, 10);
  keyGroup->GetConstraints()->right.SameAs(m_keyChoice, wxRight, -10);
  keyGroup->GetConstraints()->bottom.SameAs(m_keyShift, wxBottom, -10);
 
  char **eventNames = new char *[events.Length()];
  for (int i = 1; i <= events.Length(); i++) 
    eventNames[i-1] = events[i].name;
    
  SetLabelPosition(wxVERTICAL);
  m_eventChoice = new wxChoice(this, (wxFunction) CallbackEvent,
			      "Event", 1, 1, -1, -1, events.Length(), 
			      eventNames, 0);
  m_eventChoice->SetClientData((char *)this);
  m_eventChoice->SetConstraints(new wxLayoutConstraints);
  m_eventChoice->GetConstraints()->top.SameAs(keyGroup, wxTop);
  m_eventChoice->GetConstraints()->left.SameAs(keyGroup, wxRight, 10);
  m_eventChoice->GetConstraints()->width.AsIs();
  m_eventChoice->GetConstraints()->height.AsIs();

  wxButton *addButton = new wxButton(this, (wxFunction) CallbackAdd, "Add"); 
  addButton->SetClientData((char *)this);

  wxButton *deleteButton = new wxButton(this, (wxFunction) CallbackDelete,
					"Delete");
  deleteButton->SetClientData((char *)this);
    
  addButton->SetConstraints(new wxLayoutConstraints);
  addButton->GetConstraints()->top.SameAs(keyGroup, wxBottom, 10);
  addButton->GetConstraints()->right.SameAs(this, wxCentreX, -5);
  addButton->GetConstraints()->width.SameAs(deleteButton, wxWidth);
  addButton->GetConstraints()->height.AsIs();

  deleteButton->SetConstraints(new wxLayoutConstraints);
  deleteButton->GetConstraints()->top.SameAs(addButton, wxTop);
  deleteButton->GetConstraints()->left.SameAs(addButton, wxRight, 10);
  deleteButton->GetConstraints()->width.AsIs();
  deleteButton->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(deleteButton, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(this, wxCentreX, -5);
  m_okButton->GetConstraints()->width.SameAs(deleteButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->Show(FALSE);
  m_cancelButton->GetConstraints()->top.SameAs(this, wxTop);
  m_cancelButton->GetConstraints()->left.SameAs(this, wxLeft);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(deleteButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

// Find the event's id
int EditAccelDialog::GetEventId(const gText &name)
{
  int id = 0;
    
  for (int i = 1; i <= events.Length() && !id; i++) {
    if (events[i].name == name) 
      id = events[i].id;
  }
    
  return id;
}

// Checks m_keyText and key_choice, and determines the real key id selected
int EditAccelDialog::RealKey(void)
{   
  if (m_keyChoice->GetSelection()) 
    return Name2Key(m_keyChoice->GetStringSelection());
    
  if (m_keyText->GetValue()) 
    return (int)(m_keyText->GetValue()[0]);
    
  return 0;
}

// The purpose of this function is to just limit the entry to 1 char and
// to set keychoice to selection 0
void EditAccelDialog::CallbackKeyText(wxText &p_object, wxCommandEvent &)
{
  char *value = p_object.GetValue();
    
  if (value) {
    if (strlen(value) > 1) {
      char tmp_str[2];
      sprintf(tmp_str, "%c", value[0]);
      p_object.SetValue(tmp_str);
    }
        
    if (strcmp(value, "") != 0)  {  // if keytext is non-blank
      ((wxChoice *) p_object.GetClientData())->SetSelection(0);
    }
  }
}

// Check to see if this event is already defined.  If so, fill in the fields
void EditAccelDialog::OnChangeEvent(const gText &name)
{
  int id = GetEventId(name);
    
  // Now, check if it is defined
  for (int i = 1; i <= acc.Length(); i++) {
    if (acc[i].id == id) {
      if (!Key2Name(acc[i].key)) {
	// if this is not a 'special' key, its just ASCII
	char tmp_str[2];
	sprintf(tmp_str, "%c", (char)acc[i].key);
	m_keyText->SetValue(tmp_str);
	m_keyChoice->SetStringSelection(BLANK_STR);
      }
      else {
	m_keyText->SetValue("");
	m_keyChoice->SetStringSelection(Key2Name(acc[i].key));
      }
            
      m_keyCtrl->SetStringSelection(accel_state_str[acc[i].ctrl]);
      m_keyShift->SetStringSelection(accel_state_str[acc[i].shift]);
      return;
    }
  }
    
  // if we got here, this key is not defined.  Clear all the settings
  m_keyText->SetValue("");
  m_keyChoice->SetSelection(0);
  m_keyCtrl->SetSelection(0);
  m_keyShift->SetSelection(0);
}

void EditAccelDialog::OnAdd(void)
{
  // First, check if this key combo is used elsewhere
  int id = GetEventId(m_eventChoice->GetStringSelection());
  int acc_id = 0;
    
  for (int i = 1; i <= acc.Length(); i++) {
    if ((acc[i].ctrl == m_keyCtrl->GetSelection() || 
	 m_keyCtrl->GetSelection() == aEither) &&
	(acc[i].shift == m_keyShift->GetSelection() || 
	 m_keyShift->GetSelection() == aEither) &&
	acc[i].key == RealKey() && acc[i].id != id) {
      wxMessageBox("This key combination is used elsewhere");
      return;
    }
  }
    
  // If we got here, this is fine.
  Accel temp;
  temp.ctrl = (AccelState) m_keyCtrl->GetSelection();
  temp.shift = (AccelState) m_keyShift->GetSelection();
  temp.key = RealKey();
  temp.id = id;
    
  if (acc_id) 
    acc[acc_id] = temp;
  else 
    acc += temp;
}

void EditAccelDialog::OnDelete(void)
{
  int id = GetEventId(m_eventChoice->GetStringSelection());
  for (int i = 1; i <= acc.Length(); i++)  {
    if (acc[i].id == id)  {
      acc.Remove(i);
    }
  }
}


//**************************************************************************
//*                       FUNCTIONS FOR WORKING WITH ACCELERATORS          *
//**************************************************************************

// Reads in accelerators from a data file.  User can set both filename and section to read from
int ReadAccelerators(gList<Accel> &list, const char *section, const char *file_name)
{
    char    tmp_str[100], *tmp_str1 = new char[100];
    int     fields;
    
    int num_accels = 0;
    list.Flush();
    wxGetResource(section, "Num-Accells", &num_accels, file_name);
    
    for (int i = 1; i <= num_accels; i++)
    {
        Accel tmp_acc;
        sprintf(tmp_str, "Accel%d", i);
        wxGetResource(section, tmp_str, &tmp_str1, file_name);
        fields = sscanf(tmp_str1, "%d %d %ld %ld",
                        (int *) &tmp_acc.ctrl, (int *)&tmp_acc.shift,
                        &tmp_acc.key, &tmp_acc.id);
        assert(fields == 4 && "Accels::Invalid DataFormat");
        list += tmp_acc;
    }
    
    delete [] tmp_str1;
    return 1;
}

// Saves accelerators to a data file. User can set both filename and section to write to
int WriteAccelerators(const gList<Accel> &list, 
                      const char *section, 
                      const char *file_name)
{
    char tmp_str[100], tmp_str1[100];
    
    int num_accels = list.Length();
    wxWriteResource(section, "Num-Accells", num_accels, file_name);
    
    for (int i = 1; i <= num_accels; i++)
    {
        Accel tmp_acc = list[i];
        sprintf(tmp_str, "Accel%d", i);
        sprintf(tmp_str1, "%d %d %ld %ld", tmp_acc.ctrl, tmp_acc.shift, 
                tmp_acc.key, tmp_acc.id);
        wxWriteResource(section, tmp_str, tmp_str1, file_name);
    }
    
    return 1;
}

// Checks if this keyevent triggers an accelerator.  If so, return the id of triggered event
long CheckAccelerators(gList<Accel> &list, wxKeyEvent &ev)
{
    for (int i = 1; i <= list.Length(); i++) 
    {
        if (list[i] == ev) 
            return list[i].id;
    }
    
    return 0;
}

// Starts up the dialog
void EditAccelerators(gList<Accel> &list, const gArray<AccelEvent> &events)
{
  EditAccelDialog dialog(list, events);
}
