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

// Returns the file name from a path.  The function FileNameFromPath()
// crashes when there is no path
gText gFileNameFromPath(const char *path);

// Returns the path only from a path name.  The function wxPathOnly()
// crashes when there is no path
gText gPathOnly(const char *name);

// in efgsolvd.cc, nfgsolvd.cc
const int idTYPE_RADIOBOX = 501;
const int idNUM_RADIOBOX = 502;

// in dlefg.cc
const int idMOVE_PLAYER_LISTBOX = 1011;
const int idMOVE_INFOSET_LISTBOX = 1012;
const int idINFOSET_EDIT_BUTTON = 1021;
const int idINFOSET_PLAYER_LISTBOX = 1022;
const int idINFOSET_INFOSET_LISTBOX = 1023;
const int idINFOSET_NEW_BUTTON = 1024;
const int idINFOSET_REMOVE_BUTTON = 1025;
const int idEFPLAYERS_NEW_BUTTON = 1031;
const int idEFPLAYERS_EDIT_BUTTON = 1032;
const int idEFSUPPORT_PLAYER_LISTBOX = 1041;
const int idEFSUPPORT_INFOSET_LISTBOX = 1042;
const int idEFSUPPORT_ACTION_LISTBOX = 1043;
const int idEFG_BROWSE_BUTTON = 1051;

// in dlsupport.cc
const int idMETHOD_BOX = 1200;

// in nfgshow.cc
const int idSTRATEGY_CHOICE = 2001;
const int idROWPLAYER_CHOICE = 2002;
const int idCOLPLAYER_CHOICE = 2003;

// in algdlgs.cc
const int idTRACE_CHOICE = 2400;
const int idALL_CHECKBOX = 2500;
const int idTRACE_BUTTON = 2501;
const int idDEPTH_CHOICE = 2502;
const int idRUNPXI_BOX = 2300;

// in dlnfg.cc
const int idNFPLAYERS_EDIT_BUTTON = 3001;
const int idSTRATEGY_PLAYER_LISTBOX = 3011;
const int idSTRATEGY_STRATEGY_LISTBOX = 3012;
const int idNFSUPPORT_PLAYER_LISTBOX = 3021;
const int idNFSUPPORT_STRATEGY_LISTBOX = 3022;
const int idNFG_BROWSE_BUTTON = 3031;

// in accels.cc
const int idACCELS_EVENT_LISTBOX = 4001;
const int idACCELS_KEY_LISTBOX = 4002;

// in efgsolutions.cc, nfgsolutions.cc
const int idSOLUTION_WINDOW = 5001;

#endif // WXMISC_H
