//
// FILE: gambdraw.h -- declaration of the generic display parameter class
// for gambit.  Currently only the player colors are set here.
//
// $Id$
//

#ifndef GAMBDRAW_H
#define GAMBDRAW_H

#include "gblock.h"
#include "gtext.h"

class GambitDrawSettings
{
private:
    void    UpdatePlayerColor(int pl);
    static void player_color_callback(wxListBox &ob, wxCommandEvent &ev);

    typedef struct 
    {
        GambitDrawSettings *parent;
        wxListBox *color_item;
    } player_color_struct;

protected:
    static gBlock<int> player_colors;

public:
    // Constructor
    GambitDrawSettings(void);

    // Controls the color each player gets
    void    SetPlayerColor(int pl, int cl);
    int     GetPlayerColor(int pl) const;

    // Allows to save/restore settings from file
    void    SaveOptions(char *f = 0) const;
    void    LoadOptions(char *f);

    // GUI Set colors
    void    PlayerColorDialog(gArray<gText> &);
};

#define INIFILE     "gambit.ini"

#endif  // GAMBDRAW_H

