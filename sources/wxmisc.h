//
// FILE: wxmisc.h -- Some wxWindows extensions
//
// $Id$
//

#ifndef WXMISC_H
#define WXMISC_H

#ifndef wxRUNNING
#define wxRUNNING 12345
#endif

#include <stdio.h>
#include "wx_timer.h"
#include "wx_form.h"

#include "garray.h"

#define WX_COLOR_LIST_LENGTH 11

#ifndef WXMISC_C
extern const char *wx_color_list[WX_COLOR_LIST_LENGTH];
extern const char *wx_hilight_color_list[WX_COLOR_LIST_LENGTH];
#else
const char *wx_color_list[WX_COLOR_LIST_LENGTH] = 
{
    "GREEN", "RED", "BLUE", "YELLOW", "VIOLET RED", "SALMON", "ORCHID",
    "TURQUOISE", "BROWN", "PALE GREEN", "BLACK"
};

const char *wx_hilight_color_list[WX_COLOR_LIST_LENGTH] = 
{
    "PALE GREEN", "INDIAN RED", "MEDIUM BLUE", "YELLOW GREEN",
    "MEDIUM VIOLET RED", "TAN", "MEDIUM ORCHID", "MEDIUM TURQUOISE",
    "KHAKI", "SEA GREEN", "DARK GREY"
};
#endif


class MyDialogBox;

class MyForm: public wxForm
{
private:
    MyDialogBox *parent;

public:
    MyForm(MyDialogBox *p, Bool help = FALSE);
    void    OnOk(void);
    void    OnCancel(void);
    void    OnHelp(void);
};


class MyDialogBox: public wxDialogBox
{
private:
    MyForm *form;
    Bool completed;
    char *help_str;

public:
    // Constructor
    MyDialogBox(wxWindow *parent, char *title, const char *help_str = 0);

    // Destructor
    ~MyDialogBox(void);

    // What was the result? wxOK/wxCANCEL
    Bool        Completed(void) const;

    // Set the help string.
    void        SetHelpString(const char *help_str);

    // Access to private data, should be removed
    MyForm     *Form(void);

    // Simulate a form here, w/ some additional features
    wxFormItem *Add(wxFormItem *item, long id = -1) {form->Add(item, id); return item; }
    void        AssociatePanel(void)    {form->AssociatePanel(this); }

    // Choose go to start up the dialog right away (calls associate panel by itself)
    void        Go(void);

    // Choose Go1 if you have already called associate panel
    void           Go1(void);
    virtual void   OnOk(void);
    virtual void   OnCancel(void);
    virtual void   OnHelp(void);
    virtual Bool   OnClose(void);
};


class MyMessageBox: public MyDialogBox
{
public:
    MyMessageBox(const char *message, const char *caption = 0, 
                 const char *help_str = 0, wxWindow *parent = 0);
};


//
// A generic standard dialog box featuring automatic layout, frame control
// handling, and standard button placement
//
class guiAutoDialog : public wxDialogBox {
private:
  int m_completed;

  static void CallbackOk(wxButton &p_object, wxEvent &)
    { ((guiAutoDialog *) p_object.GetClientData())->OnOk(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((guiAutoDialog *) p_object.GetClientData())->OnClose(); }
  static void CallbackHelp(wxButton &p_object, wxEvent &)
    { ((guiAutoDialog *) p_object.GetClientData())->OnHelp(); }

  Bool OnClose(void);

protected:
  wxButton *m_okButton, *m_cancelButton, *m_helpButton;

  virtual const char *HelpString(void) const { return ""; }

  virtual void OnOk(void);
  void OnCancel(void);
  void OnHelp(void);

public:
  guiAutoDialog(wxWindow *p_parent, char *p_title);
  virtual ~guiAutoDialog() { }

  void Go(void);
  int Completed(void) const { return m_completed; }
};

/**
 * A "paged" dialog class (i.e., with "back" and "next" buttons)
 */
class guiPagedDialog : public guiAutoDialog {
private:
  static void CallbackNext(wxButton &p_object, wxEvent &)
    { ((guiPagedDialog *) p_object.GetClientData())->OnNext(); }
  static void CallbackBack(wxButton &p_object, wxEvent &)
    { ((guiPagedDialog *) p_object.GetClientData())->OnBack(); }

  void OnOk(void);
  void OnNext(void);
  void OnBack(void);

protected:
  int m_pageNumber, m_numFields;
  static int s_itemsPerPage;
  wxText **m_dataFields;
  wxButton *m_backButton, *m_nextButton;
  gArray<gText> m_dataValues;

public:
  guiPagedDialog(wxWindow *p_parent, char *p_title, int p_numItems);
  virtual ~guiPagedDialog();

  const gText &GetValue(int p_index) const { return m_dataValues[p_index]; }
  void SetValue(int p_index, const gText &p_value);
};

class FontDialogBox : public guiAutoDialog {
private:
  wxChoice *m_nameItem, *m_sizeItem;
  wxRadioBox *m_styleItem, *m_weightItem;
  wxCheckBox *m_underlineItem;

public:
  FontDialogBox(wxWindow *parent, wxFont *def = 0);
  virtual ~FontDialogBox() { }

  wxFont  *MakeFont(void);
};


typedef enum {
  wxWYSIWYG, wxFITTOPAGE
} wxOutputOption;

typedef enum {
  wxMEDIA_PRINTER = 0, wxMEDIA_PS, wxMEDIA_CLIPBOARD, 
  wxMEDIA_METAFILE, wxMEDIA_PREVIEW, wxMEDIA_NUM
} wxOutputMedia;

class wxOutputDialogBox : public guiAutoDialog {
private:
  wxRadioBox *m_mediaBox;
  wxCheckBox *m_fitBox;

public:
  wxOutputDialogBox(wxStringList *extra_media = 0, wxWindow *parent = 0);
  virtual ~wxOutputDialogBox() { }

  /** Returns one of the built-in media types */
  wxOutputMedia GetMedia(void) const;
  /** Returns the additional media type given in extra_media, if any */
  int GetExtraMedia(void) const;
  /** Returns true if the selection was an extra_media */
  Bool ExtraMedia(void) const;
  /** Returns either wysiwyg or fit to page if appropriate */
  wxOutputOption GetOption(void) const;
};


// Returns the position of s in the list l.  This is useful for finding
// the selection # in a String form item w/ a list constraint
int wxListFindString(wxList *l, char *s);

// Returns a wxStringList containing num strings representing integers
// from 1 to n (i.e. "1", "2", .."n").  If l is NULL, a new list is created
// otherwise, the nums are appended to l.  If "start" is supplied, it
// starts from that value.
wxStringList* wxStringListInts(int num, wxStringList *l = 0, int start = 1);

// Converts a font into a string format that can be used to save it in a data file
char *wxFontToString(wxFont *f);

// Converts an encoded string back into a font. see wxFontToString
wxFont *wxStringToFont(char *s);

// Some basic keyboard stuff...
Bool    IsCursor(wxKeyEvent &ev);
Bool    IsEnter(wxKeyEvent &ev);
Bool    IsNumeric(wxKeyEvent &ev);
Bool    IsAlphaNum(wxKeyEvent &ev);
Bool    IsDelete(wxKeyEvent &ev);

// gDrawText is an extension of the wxWindow's wxDC::DrawText function
// Besides providing the same features, it also supports imbedded codes
// to change the color of the output text.  The codes have the format
// of: "text[/C{#}]", where # is the number of the color to select
// from the gambit_color_list.  Also allows sup/superscripts
// Note: uses gString
#include "gtext.h"
void gDrawText(wxDC &dc, const gText &s, float x, float y);

// Calculates the size of the string when parsed as gDrawText
void gGetTextExtent(wxDC &dc, const gText &s0, float *x, float *y);

// Returns just the text portion of a gDrawText formated string
gText gPlainText(const gText &s);
gText gGetTextLine(const gText &s0 = "", wxFrame *parent = 0, int x = -1, int y = -1,
                   const char *title = "", bool titlebar = false);

// Returns the file name from a path.  The function FileNameFromPath()
// crashes when there is no path
gText gFileNameFromPath(const char *path);

// Returns the path only from a path name.  The function wxPathOnly()
// crashes when there is no path
gText gPathOnly(const char *name);

// FindFile: finds the specified file in the path.  User deletes the
// result
char *wxFindFile(const char *name);

// Functions to implement online help system.
void wxInitHelp(const char *file_name, const char *help_about_str = 0);
void wxHelpContents(const char *name);
void wxHelpAbout(const char *help_str = 0);
void wxKillHelp(void);


#define CREATE_DIALOG       0
#define DESTROY_DIALOG      1
#define UPDATE_DIALOG       2


//
// Some specialized panel items that implement constraints
//
class wxIntegerItem : public wxText {
private:
  int m_value;

  static void EventCallback(wxIntegerItem &p_object, wxCommandEvent &p_event);

public:
  wxIntegerItem(wxPanel *p_parent, char *p_label, int p_default = 0,
		int p_x = -1, int p_y = -1, int p_w = -1, int p_h = -1);
  virtual ~wxIntegerItem() { }

  void SetInteger(int p_value);
  int GetInteger(void) const { return m_value; }
};

#include "gnumber.h"

class wxNumberItem : public wxText {
private:
  gNumber m_value;

  static void EventCallback(wxNumberItem &p_object, wxCommandEvent &p_event);

public:
  wxNumberItem(wxPanel *p_parent, char *p_label, const gText &p_default,
	       int p_x = -1, int p_y = -1, int p_w = -1, int p_h = -1);
  virtual ~wxNumberItem() { }

  void SetNumber(const gNumber &p_value);
  gNumber GetNumber(void) const { return m_value; }
};

#endif // WXMISC_H

