// File: gambit.h -- declaration of the very top level of the gambit gui.
// $Id$

#ifndef GAMBIT_H
#define GAMBIT_H

#include "gtext.h"
#include "wx.h"

// For gui logging ------------
#include "guiobj.h"
#include "guirec.h"
#include "guirecdb.h"
#include "guipb.h"
// ----------------------------

class GambitApp: public wxApp
{
public:
    wxFrame *OnInit(void);
    int OnExit(void);
};


class GambitFrame : public wxFrame, public GuiObject
{
public:
    GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type);
    Bool OnClose(void);
    void OnMenuCommand(int id);
    void LoadFile(char *s = 0);

    // Gui playback:
    void ExecuteLoggedCommand(const class gText &, const class gList<gText> &);
};


#define FILE_NEW                            1000
#define FILE_NEW_NFG                        1001
#define FILE_NEW_EFG                        1002
#define FILE_LOAD                           1010
#define FILE_QUIT                           1030

#define GAMBIT_HELP_CONTENTS                1700
#define GAMBIT_HELP_ABOUT                   1701

#define GAMBIT_GUI_HELP "Using Gambit GUI"

#define GAMBIT_VERSION                      93

#endif // GAMBIT_H

