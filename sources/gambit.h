//
// FILE: gambit.h -- declaration of the very top level of the gambit gui.
//
// $Id$
//

#ifndef GAMBIT_H
#define GAMBIT_H

#include "gtext.h"
#include "wx.h"
#include "wx_doc.h"

class GambitFrame;
extern GambitFrame *main_gambit_frame;
extern void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
                               long p_style = wxOK | wxCENTRE);


class GambitApp : public wxApp {
private:
  gText m_currentDir; /* Current position in directory tree. */
  gText m_resourceFile;  /* path to resource file */
  wxFileHistory *m_recentFiles;

public:
  wxFrame *OnInit(void);
  int OnExit(void);

  const gText &CurrentDir(void)   { return m_currentDir; }
  void SetCurrentDir(const gText &p_dir)  { m_currentDir = p_dir; }

  const gText &ResourceFile(void)  { return m_resourceFile; }

  void AddFileToHistory(const gText &p_name);
  gText GetHistoryFile(int i)
    { return m_recentFiles->GetHistoryFile(i); }
};

extern GambitApp gambitApp;


class GambitFrame : public wxFrame {
public:
  GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type);
  Bool OnClose(void);
  void OnMenuCommand(int id);
  void LoadFile(char *s = 0);
};


#define FILE_NEW                            1000
#define FILE_NEW_NFG                        1001
#define FILE_NEW_EFG                        1002
#define FILE_LOAD                           1010
#define FILE_QUIT                           1030

#define GAMBIT_HELP_CONTENTS                1700
#define GAMBIT_HELP_ABOUT                   1701

#define GAMBIT_GUI_HELP "Using Gambit GUI"

const int GAMBIT_VERSION = 96;

#endif // GAMBIT_H

