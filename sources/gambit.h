//
// FILE: gambit.h -- declaration of the very top level of the gambit gui.
//
// $Id$
//

#ifndef GAMBIT_H
#define GAMBIT_H

#include "base/base.h"
#include "wx/wx.h"
#include "wx/config.h"    // for wxConfig
#include "wx/docview.h"   // for wxFileHistory
#include "wx/listctrl.h"

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

class Efg;
class EfgShow;
class Nfg;
class NfgShow;
class Game;

class GambitFrame : public wxFrame {
private:
  wxFileHistory m_fileHistory;
  wxListCtrl *m_gameListCtrl;
  
  gBlock<Game *> m_gameList;

  // Menu event handlers
  void OnNew(wxCommandEvent &);
  void OnLoad(wxCommandEvent &);
  void OnMRUFile(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);

  // Other event handlers
  void OnCloseWindow(wxCloseEvent &);
  void OnGameSelected(wxListEvent &);

  void MakeToolbar(void);
  void UpdateGameList(void);

public:
  GambitFrame(wxFrame *p_parent, const wxString &p_title,
	      const wxPoint &p_position, const wxSize &p_size);
  virtual ~GambitFrame();

  void LoadFile(const gText &);

  void AddGame(Efg *, EfgShow *);
  void AddGame(Nfg *, NfgShow *);
  void AddGame(Efg *, Nfg *, NfgShow *);
  void RemoveGame(Efg *);
  void RemoveGame(Nfg *);

  EfgShow *GetWindow(const Efg *);
  NfgShow *GetWindow(const Nfg *);

  void SetActiveWindow(EfgShow *);
  void SetActiveWindow(NfgShow *);

  DECLARE_EVENT_TABLE()
};

#define GAMBIT_GUI_HELP "GAMBIT GUI"

const int GAMBIT_VERSION = 96;

#endif // GAMBIT_H

