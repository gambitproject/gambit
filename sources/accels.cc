//
// FILE: accels.cc -- functions for working with accelerators
//
// $Id$
//

#include <ctype.h>

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

char *accel_state_str[3] = {"Off", "On",  "??" };

gOutput &operator<<(gOutput &op, const gList<Accel> &p);
gOutput &operator<<(gOutput &op, const gArray<AccelEvent> &p);


class KeyInfo {
public:
  char *m_name;
  long m_key;
  AccelState m_control;
};

KeyInfo keyInfo[] = 
{
  { "(none)", 0, accelEither },
  { "BACK", WXK_BACK, accelEither },
  { "TAB", WXK_TAB, accelEither },
  { "RETURN", WXK_RETURN, accelEither },
  { "ESCAPE", WXK_ESCAPE, accelEither },
  { "SPACE", WXK_SPACE, accelEither },
  { "DELETE", WXK_DELETE, accelEither },
  { "END", WXK_END, accelEither },
  { "HOME" , WXK_HOME, accelEither },
  { "LEFT", WXK_LEFT, accelEither },
  { "UP", WXK_UP, accelEither },
  { "RIGHT", WXK_RIGHT, accelEither },
  { "DOWN", WXK_DOWN, accelEither },
  { "F1", WXK_F1, accelEither },
  { "F2", WXK_F2, accelEither },
  { "F3", WXK_F3, accelEither },
  { "F4", WXK_F4, accelEither },
  { "F5", WXK_F5, accelEither },
  { "F6", WXK_F6, accelEither },
  { "F7", WXK_F7, accelEither },
  { "F8", WXK_F8, accelEither },
  { "F9", WXK_F9, accelEither },
  { "F10", WXK_F10, accelEither },
  { "F11", WXK_F11, accelEither },
  { "F12", WXK_F12, accelEither },
  { "CTRL-A", 'A', accelOn },
  { "CTRL-B", 'B', accelOn },
  { "CTRL-C", 'C', accelOn },
  { "CTRL-D", 'D', accelOn },
  { "CTRL-E", 'E', accelOn },
  { "CTRL-F", 'F', accelOn },
  { "CTRL-G", 'G', accelOn },
  { "CTRL-H", 'H', accelOn },
  { "CTRL-I", 'I', accelOn },
  { "CTRL-J", 'J', accelOn },
  { "CTRL-K", 'K', accelOn },
  { "CTRL-L", 'L', accelOn },
  { "CTRL-M", 'M', accelOn },
  { "CTRL-N", 'N', accelOn },
  { "CTRL-O", 'O', accelOn },
  { "CTRL-P", 'P', accelOn },
  { "CTRL-Q", 'Q', accelOn },
  { "CTRL-R", 'R', accelOn },
  { "CTRL-S", 'S', accelOn },
  { "CTRL-T", 'T', accelOn },
  { "CTRL-U", 'U', accelOn },
  { "CTRL-V", 'V', accelOn },
  { "CTRL-W", 'W', accelOn },
  { "CTRL-X", 'X', accelOn },
  { "CTRL-Y", 'Y', accelOn },
  { "CTRL-Z", 'Z', accelOn },
  { 0, 0 }
};

//**************************************************************************
//*                          ACCELERATOR EDIT DIALOG                       *
//**************************************************************************

class EditAccelDialog : public guiAutoDialog {
private:
  wxListBox *m_eventBox, *m_keyBox;

  gList<Accel> m_accels;
  const gArray<AccelEvent> &m_events;
    
  static void CallbackEvent(wxListBox &p_object, wxCommandEvent &)
    { ((EditAccelDialog *) p_object.wxEvtHandler::GetClientData())->
	OnEventBox(); }
  static void CallbackKey(wxListBox &p_object, wxCommandEvent &)
    { ((EditAccelDialog *) p_object.wxEvtHandler::GetClientData())->
	OnKeyBox(); }
    
  void OnEventBox(void);
  void OnKeyBox(void);

  int GetAccelKey(int);
  int GetKeyIndex(int);

  const char *HelpString(void) const { return "Accelerators Dialog"; }
    
public:
  EditAccelDialog(wxWindow *, const gList<Accel> &,
		  const gArray<AccelEvent> &events); 
  virtual ~EditAccelDialog() { }

  const gList<Accel> &GetAccels(void) const { return m_accels; }
};


EditAccelDialog::EditAccelDialog(wxWindow *p_parent,
				 const gList<Accel> &p_accels,
				 const gArray<AccelEvent> &p_events) 
  : guiAutoDialog(p_parent, "Accelerators"),
    m_accels(p_accels), m_events(p_events)
{
  char **eventNames = new char *[m_events.Length()];
  for (int i = 1; i <= m_events.Length(); i++) 
    eventNames[i-1] = m_events[i].name;
    
  SetLabelPosition(wxVERTICAL);
  m_eventBox = new wxListBox(this, (wxFunction) CallbackEvent,
			     "Event", wxSINGLE,
			     1, 1, -1, -1, m_events.Length(), 
			     eventNames);
  m_eventBox->wxEvtHandler::SetClientData((char *) this);
  m_eventBox->SetSelection(0);
  m_eventBox->SetConstraints(new wxLayoutConstraints);
  m_eventBox->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_eventBox->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_eventBox->GetConstraints()->width.AsIs();
  m_eventBox->GetConstraints()->height.AsIs();

  m_keyBox = new wxListBox(this, (wxFunction) CallbackKey, "Key");
  for (int i = 0; keyInfo[i].m_name; i++) {
    m_keyBox->Append(keyInfo[i].m_name);
  }
  m_keyBox->wxEvtHandler::SetClientData((char *) this);
  m_keyBox->SetConstraints(new wxLayoutConstraints);
  m_keyBox->GetConstraints()->top.SameAs(m_eventBox, wxTop);
  m_keyBox->GetConstraints()->left.SameAs(m_eventBox, wxRight, 10);
  m_keyBox->GetConstraints()->width.AsIs();
  m_keyBox->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_eventBox, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  OnEventBox();
  Go();
}

int EditAccelDialog::GetAccelKey(int p_id)
{
  for (int i = 1; i <= m_accels.Length(); i++) {
    if (m_accels[i].Id() == p_id) {
      for (int j = 1; keyInfo[j].m_name; j++) {
	if (m_accels[i].Key() == keyInfo[j].m_key) {
	  return j;
	}
      }
    }
  }
    
  return 0;
}

int EditAccelDialog::GetKeyIndex(int p_key)
{
  for (int i = 1; keyInfo[i].m_name; i++) {
    if (keyInfo[i].m_key == p_key) {
      return i;
    }
  }
  return 0;
}

void EditAccelDialog::OnEventBox(void)
{
  int id = m_events[m_eventBox->GetSelection()+1].id;

  for (int i = 1; i <= m_accels.Length(); i++) {
    if (m_accels[i].Id() == id) {
      int keyIndex = GetKeyIndex(m_accels[i].Key());
      m_keyBox->SetSelection(keyIndex);
      return;
    }
  }
    
  m_keyBox->SetSelection(0);
}

void EditAccelDialog::OnKeyBox(void)
{
  int id = m_events[m_eventBox->GetSelection()+1].id;

  if (m_keyBox->GetSelection() > 0) {
    long key = keyInfo[m_keyBox->GetSelection()].m_key;
    int accelIndex = 0;

    for (int i = 1; i <= m_accels.Length(); i++) {
      if (m_accels[i].Id() == id) {
	accelIndex = i;
      }
      else if (m_accels[i].Key() == key) {
	for (int j = 1; ; j++) {
	  if (m_events[j].id == m_accels[i].Id()) {
	    wxMessageBox(gText(keyInfo[m_keyBox->GetSelection()].m_name) + 
			 " is already bound to " +
			 m_events[j].name, "Error");
	    m_keyBox->SetSelection(GetAccelKey(id));
	    return;
	  }
	}
      }
    }
    
    Accel temp(keyInfo[m_keyBox->GetSelection()].m_control,
	       accelEither, key, id);

    if (accelIndex) 
      m_accels[accelIndex] = temp;
    else 
      m_accels += temp;
  }
  else {
    // clicked on "none"... delete accelerator
    for (int i = 1; i <= m_accels.Length(); i++) {
      if (m_accels[i].Id() == id) {
	m_accels.Remove(i);
	return;
      }
    }
  }
}

//=======================================================================
//                    class Accel: Member functions
//=======================================================================

Accel::Accel(void)
  : m_control(accelOff), m_shift(accelOff), m_key(0), m_id(0)
{ }

Accel::Accel(AccelState p_control, AccelState p_shift, long p_key, long p_id)
  : m_control(p_control), m_shift(p_shift), m_key(p_key), m_id(p_id)
{ }

bool Accel::operator==(const Accel &p_accel) const
{
  return (m_control == p_accel.m_control &&
	  m_shift == p_accel.m_shift &&
	  m_key == p_accel.m_key);
}

bool Accel::operator==(wxKeyEvent &p_event) const 
{
  if (isalpha(p_event.KeyCode())) {
    return (m_key == toupper(p_event.KeyCode()) &&
	    (m_control == p_event.ControlDown() || m_control == accelEither));
  }
  else {
    return ((m_control == p_event.ControlDown() || m_control == accelEither) && 
	    (m_shift == p_event.ShiftDown() || m_shift == accelEither) && 
	    m_key == p_event.KeyCode());
  }
}

gOutput &operator<<(gOutput &p_file, const Accel &p_accel)
{
  p_file << "Ctrl " << p_accel.Control()
	 << " Shift " << p_accel.Shift() 
	 << " Key " << p_accel.Key()
	 << " Id " << p_accel.Id() << '\n';
  return p_file;
}

//**************************************************************************
//*                       FUNCTIONS FOR WORKING WITH ACCELERATORS          *
//**************************************************************************

// Reads in accelerators from a data file.  User can set both filename and section to read from
int ReadAccelerators(gList<Accel> &list, 
		     const char *section, const char *file_name)
{
  char *tmp_str1 = new char[100];

  int num_accels = 0;
  list.Flush();
  wxGetResource(section, "Num-Accels", &num_accels, file_name);
  
  for (int i = 1; i <= num_accels; i++) {
    int control, shift;
    long key, id;
    gText tmpStr = "Accel" + ToText(i);
    wxGetResource(section, tmpStr, &tmp_str1, file_name);
    sscanf(tmp_str1, "%d %d %ld %ld",
	   &control, &shift, &key, &id);
    list += Accel((AccelState) control, (AccelState) shift, key, id);
  }
    
  delete [] tmp_str1;
  return 1;
}

// Saves accelerators to a data file. User can set both filename and section to write to
int WriteAccelerators(const gList<Accel> &list, 
                      const char *section, 
                      const char *file_name)
{
  char tmp_str1[100];
    
  int num_accels = list.Length();
  wxWriteResource(section, "Num-Accels", num_accels, file_name);
    
  for (int i = 1; i <= num_accels; i++) {
    gText tmpStr = "Accel" + ToText(i);
    sprintf(tmp_str1, "%d %d %ld %ld",
	    list[i].Control(), list[i].Shift(), list[i].Key(), list[i].Id());
    wxWriteResource(section, tmpStr, tmp_str1, file_name);
  }
    
  return 1;
}

// Checks if this keyevent triggers an accelerator.  If so, return the id of triggered event
long CheckAccelerators(gList<Accel> &list, wxKeyEvent &ev)
{
  for (int i = 1; i <= list.Length(); i++) {
    if (list[i] == ev) {
      return list[i].Id();
    }
  }
    
  return 0;
}

void EditAccelerators(gList<Accel> &list, const gArray<AccelEvent> &events)
{
  EditAccelDialog dialog(NULL, list, events);

  if (dialog.Completed() == wxOK) {
    list = dialog.GetAccels();
  }
}
