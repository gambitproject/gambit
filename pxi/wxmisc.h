//
// FILE: wxmisc.h -- Defines some generic stuff that should be in wxWin
//
// @(#)wxmisc.h	1.1 6/6/94
//

#ifndef WXMISC_H
#define WXMISC_H
#ifndef	wxRUNNING
#define	wxRUNNING 12345
#endif
#include <stdio.h>
#include "wx/wx.h"

//
// A generic standard dialog box featuring automatic layout, frame control
// handling, and standard button placement
//
class guiAutoDialog : public wxDialog {
protected:
  wxButton *m_okButton, *m_cancelButton, *m_helpButton;
  wxBoxSizer *m_buttonSizer;

  virtual const char *HelpString(void) const { return ""; }

  void OnHelp(void);

  void Go(void);

public:
  guiAutoDialog(wxWindow *p_parent, char *p_title);
  virtual ~guiAutoDialog();
};

int wxListFindString(wxList *l,char *s);
char *wxFontToString(wxFont *f);
wxFont *wxStringToFont(char *s);

void wxInitHelp(const char *file_name, const char *help_about_str = 0);
void wxHelpContents(const char *name);
void wxHelpAbout(const char *help_str = 0);
void wxKillHelp(void);

class wxIntegerItem : public wxTextCtrl {
private:
  int m_value;
  wxString m_data;

public:
  wxIntegerItem(wxPanel *p_parent, char *p_label, int p_default = 0,
		int p_x = -1, int p_y = -1, int p_w = -1, int p_h = -1);
  virtual ~wxIntegerItem() { }

  void SetInteger(int p_value);
  int GetInteger(void) const { return m_value; }
};

class wxNumberItem : public wxTextCtrl {
private:
  wxString m_data;
  double m_value; 

public:
  wxNumberItem(wxPanel *p_parent, char *p_label, const wxString &p_default,
	       const wxPoint &pos = wxDefaultPosition, 
	       const wxSize &size = wxDefaultSize);
  virtual ~wxNumberItem();

  void SetNumber(const double &p_value);
  double GetNumber(void);
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
  bool ExtraMedia(void) const;
  /** Returns either wysiwyg or fit to page if appropriate */
  wxOutputOption GetOption(void) const;
};

#endif //WXMISC_H



