//
// FILE: wxmisc.h -- Some wxWindows extensions
//
// $Id$
//

#ifndef WXMISC_H
#define WXMISC_H

#include "wx/grid.h"

#include <stdio.h>
#include "base/base.h"

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

public:
  guiAutoDialog(wxWindow *p_parent, char *p_title);
  virtual ~guiAutoDialog();

  DECLARE_EVENT_TABLE()
};

/**
 * A "paged" dialog class (i.e., with "back" and "next" buttons)
 */
class guiPagedDialog : public guiAutoDialog {
protected:
  wxGrid *m_grid;

public:
  guiPagedDialog(wxWindow *p_parent, char *p_title, int p_numItems);
  virtual ~guiPagedDialog();

  gText GetValue(int p_index) const;
  void SetValue(int p_index, const gText &p_value);

  void SetLabel(int p_index, const gText &p_label);
};

/**
 * A dialog box with just a slider, and OK/Cancel buttons
 */

class guiSliderDialog : public guiAutoDialog {
private:
  wxSlider *m_slider;

public:
  guiSliderDialog(wxWindow *p_parent, const gText &p_caption,
		  int p_min, int p_max, int p_default);
  virtual ~guiSliderDialog() { }

  int GetValue(void) const { return m_slider->GetValue(); }
};

class dialogTextWindow : public wxDialog {
public:
  dialogTextWindow(wxWindow *p_parent, const wxString &p_title,
		   const wxString &p_contents);
  virtual ~dialogTextWindow();
};

// gDrawText is an extension of the wxWindow's wxDC::DrawText function
// Besides providing the same features, it also supports imbedded codes
// to change the color of the output text.  The codes have the format
// of: "text[/C{#}]", where # is the number of the color to select
// from the gambit_color_list.  Also allows sup/superscripts
void gDrawText(wxDC &dc, const gText &s, float x, float y);

// in nfgshow.cc
const int idSTRATEGY_CHOICE = 2001;
const int idROWPLAYER_CHOICE = 2002;
const int idCOLPLAYER_CHOICE = 2003;

// in dlnfg.cc
const int idSTRATEGY_PLAYER_LISTBOX = 3011;
const int idSTRATEGY_STRATEGY_LISTBOX = 3012;

#endif // WXMISC_H
