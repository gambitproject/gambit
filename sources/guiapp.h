//
// FILE: guiapp.h -- Declaration of main GambitApp class
//
// $Id$
//

#ifndef GUIAPP_H
#define GUIAPP_H

#include <wx/toolbar.h>

class gambitFrame;

class gambitApp : public wxApp {
private:
  gambitFrame *m_frame;

public:
  bool OnInit(void);
};

class gambitFrame : public wxMDIParentFrame {
private:


public:    
  gambitFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const long style);

  void OnSize(wxSizeEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnNewEfg(wxCommandEvent &);
  void OnNewNfg(wxCommandEvent &);
  void OnQuit(wxCommandEvent& event);
  void OnOpenEfg(wxCommandEvent &);
  void OnOpenNfg(wxCommandEvent &);
  DECLARE_EVENT_TABLE()
};

#define GAMBIT_QUIT        1
#define GAMBIT_NEW_EFG     2
#define GAMBIT_NEW_NFG     3
#define GAMBIT_NEW         4
#define GAMBIT_REFRESH     5
#define GAMBIT_CLOSE       6
#define GAMBIT_ABOUT       7
#define GAMBIT_OPEN_EFG    8
#define GAMBIT_OPEN_NFG    9
#define GAMBIT_OPEN        10
#define GAME_EDIT_LABEL    2000

#endif  // GUIAPP_H
