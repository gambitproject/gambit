//
// FILE: gambit.h -- declaration of the very top level of the gambit gui.
//
// $Id$
//

#ifndef GAMBIT_H
#define GAMBIT_H

#include "gtext.h"
#include "wx/wx.h"
#include "wx/config.h"    // for wxConfig
#include "wx/docview.h"   // for wxFileHistory

extern void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
                               long p_style = wxOK | wxCENTRE);


class GambitApp : public wxApp {
private:
  gText m_currentDir; /* Current position in directory tree. */

  bool OnInit(void);

public:
  virtual ~GambitApp() { }
  
  const gText &CurrentDir(void)  { return m_currentDir; }
  void SetCurrentDir(const gText &p_dir)  { m_currentDir = p_dir; }
};

DECLARE_APP(GambitApp)

class GambitFrame : public wxFrame {
private:
  wxFileHistory m_fileHistory;

  // Menu event handlers
  void OnNewEfg(wxCommandEvent &);
  void OnNewNfg(wxCommandEvent &);
  void OnLoad(wxCommandEvent &);
  void OnMRUFile(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);

  int GetPlayers(void);
  int GetStrategies(gArray<int> &);

public:
  GambitFrame(wxFrame *p_parent, const wxString &p_title,
	      const wxPoint &p_position, const wxSize &p_size);
  virtual ~GambitFrame();

  void LoadFile(const gText &);

  DECLARE_EVENT_TABLE()
};


#define FILE_NEW                            1000
#define FILE_NEW_NFG                        1001
#define FILE_NEW_EFG                        1002
#define FILE_LOAD                           1010
#define FILE_QUIT                           1030

#define GAMBIT_HELP_CONTENTS                1700
#define GAMBIT_HELP_ABOUT                   1701

#define GAMBIT_GUI_HELP "GAMBIT GUI"

const int GAMBIT_VERSION = 96;

#endif // GAMBIT_H

