//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of application-level class for Gambit graphical interface
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


class efgGame;
class EfgShow;
class Nfg;
class NfgShow;

class Game;

class GambitApp : public wxApp {
private:
  wxString m_currentDir; /* Current position in directory tree. */
  wxFileHistory m_fileHistory;

  gBlock<Game *> m_gameList;

  bool OnInit(void);

public:
  GambitApp(void);
  virtual ~GambitApp();
  
  const wxString &CurrentDir(void)  { return m_currentDir; }
  void SetCurrentDir(const wxString &p_dir)  { m_currentDir = p_dir; }

  void AddGame(efgGame *, EfgShow *);
  void AddGame(Nfg *, NfgShow *);
  void AddGame(efgGame *, Nfg *, NfgShow *);
  void RemoveGame(efgGame *);
  void RemoveGame(Nfg *);
  void SetFilename(EfgShow *, const wxString &);
  void SetFilename(NfgShow *, const wxString &);

  EfgShow *GetWindow(const efgGame *);
  NfgShow *GetWindow(const Nfg *);

  void LoadFile(const wxString &);

  // Handlers for common menu items
  void OnFileNew(wxWindow *);
  void OnFileOpen(wxWindow *);
  void OnFileMRUFile(wxCommandEvent &);

  void OnHelpContents(void);
  void OnHelpIndex(void);
  void OnHelpAbout(wxWindow *);
};

DECLARE_APP(GambitApp)

const int wxID_HELP_INDEX = 1310;

#endif // GAMBIT_H

