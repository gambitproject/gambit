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
#include "wx/wxhtml.h"    // for wxHtmlHelpController
#include "wx/listctrl.h"

extern void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
                               long p_style = wxOK | wxCENTRE);


class GambitApp : public wxApp {
private:
  gText m_currentDir; /* Current position in directory tree. */
  wxHtmlHelpController m_help;

  bool OnInit(void);

public:
  virtual ~GambitApp() { }
  
  const gText &CurrentDir(void)  { return m_currentDir; }
  void SetCurrentDir(const gText &p_dir)  { m_currentDir = p_dir; }

  wxHtmlHelpController &HelpController(void) { return m_help; }
};

DECLARE_APP(GambitApp)

namespace Efg {
  class Game;
}

class EfgShow;
class Nfg;
class NfgShow;
class Game;

const int wxID_HELP_INDEX = 1310;

class GambitFrame : public wxFrame {
private:
  wxFileHistory m_fileHistory;
  wxListCtrl *m_gameListCtrl;

  gBlock<Game *> m_gameList;

  // Menu event handlers
  void OnNew(wxCommandEvent &);
  void OnLoad(wxCommandEvent &);
  void OnMRUFile(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

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

  void AddGame(Efg::Game *, EfgShow *);
  void AddGame(Nfg *, NfgShow *);
  void AddGame(Efg::Game *, Nfg *, NfgShow *);
  void RemoveGame(Efg::Game *);
  void RemoveGame(Nfg *);

  EfgShow *GetWindow(const Efg::Game *);
  NfgShow *GetWindow(const Nfg *);

  void SetActiveWindow(EfgShow *);
  void SetActiveWindow(NfgShow *);

  DECLARE_EVENT_TABLE()
};

const int GAMBIT_VERSION = 97;

#endif // GAMBIT_H

