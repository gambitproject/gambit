// File: gambit.h -- declaration of the very top level of the gambit gui.
// $Id$

#ifndef GAMBIT_H
#define GAMBIT_H

#include "gtext.h"
#include "wx.h"

class GambitApp: public wxApp
{
public:
    wxFrame *OnInit(void);
    int OnExit(void);
};

class GambitFrame : public wxFrame
{
private:
    // ---------------------------------------
    // For gui logging:
    // ---------------------------------------
    
    // Class variables: 
    static gText GuiLogRootName;
    static int   GuiLogNameCount;
    
    // Instance variables:
    gText GuiLogName;
    
    // ---------------------------------------
    
public:
    GambitFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type);
    Bool OnClose(void);
    void OnMenuCommand(int id);
    void LoadFile(char *s=0);

    // ---------------------------------------
    // For gui logging:
    // ---------------------------------------

    const gText& get_log_name() { return GuiLogName; }

    // ---------------------------------------

    // Debugging.
    bool is_GambitFrame() const;
    void GambitFrame_hello() const;
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

