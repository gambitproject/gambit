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

#include "userprefs.h"

extern void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
                               long p_style = wxOK | wxCENTRE);


class FullEfg;
class EfgShow;
class Nfg;
class NfgShow;

class Game;

class GambitApp : public wxApp {
private:
  wxString m_currentDir; /* Current position in directory tree. */
  wxHtmlHelpController m_help;
  wxFileHistory m_fileHistory;
  UserPreferences m_prefs;

  gBlock<Game *> m_gameList;

  bool OnInit(void);

public:
  GambitApp(void);
  virtual ~GambitApp();
  
  const wxString &CurrentDir(void)  { return m_currentDir; }
  void SetCurrentDir(const wxString &p_dir)  { m_currentDir = p_dir; }

  wxHtmlHelpController &HelpController(void) { return m_help; }

  UserPreferences &GetPreferences(void) { return m_prefs; }

  void AddGame(FullEfg *, EfgShow *);
  void AddGame(Nfg *, NfgShow *);
  void AddGame(FullEfg *, Nfg *, NfgShow *);
  void RemoveGame(FullEfg *);
  void RemoveGame(Nfg *);
  void SetFilename(EfgShow *, const wxString &);
  void SetFilename(NfgShow *, const wxString &);

  EfgShow *GetWindow(const FullEfg *);
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

const int GAMBIT_VERSION = 97;

#endif // GAMBIT_H

