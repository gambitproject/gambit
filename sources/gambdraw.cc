// File: gambdraw.cc 

#include "wx.h"
#include "wxmisc.h"
#pragma hdrstop
#include "gambdraw.h"

gBlock<int> GambitDrawSettings::player_colors = gBlock<int>();

// -------------------------- Player Colors ------------------------------
// Unless SetPlayerColor has been used to define a color for a player, the
// color is just the player # modulus the number of colors.  This # is used
// to index into the wx_color_list array (wx_misc.h,.cc).  Note that
// extform uses player numbers starting from -1, so, add 2

// Constructor.  All this does right now is read in the defaults stored
// in the gambit.ini file.
GambitDrawSettings::GambitDrawSettings(void)
{
    // if the colors array is empty, try to read it in from config file
    if (player_colors.Length() == 0) 
        LoadOptions(INIFILE);
}


// UpdatePlayerColor: makes sure that the player_colors block contains
// this player.  If not, extends the block using default scheme
void GambitDrawSettings::UpdatePlayerColor(int pl)
{
    if (player_colors.Length() < pl)
    {
        int old_length = player_colors.Length();
        player_colors += gBlock<int>(pl - old_length);

        for (int i = old_length + 1; i <= pl; i++)
            player_colors[i] = (i%WX_COLOR_LIST_LENGTH) + 1;
    }
}


// SetPlayerColor: used to specify alternate coloring scheme for players
// If this is not called, default scheme is used (see above)
void GambitDrawSettings::SetPlayerColor(int pl, int cl)
{
    pl += 2;
    UpdatePlayerColor(pl);
    player_colors[pl] = (cl%WX_COLOR_LIST_LENGTH);
}


// GetPlayerColor: used to get the players display color.
int GambitDrawSettings::GetPlayerColor(int pl)  const
{
    pl += 2;  // +2 is not used anymore...remnant of old player numbering
    //UpdatePlayerColor(pl);

    if (player_colors.Length() < pl)
        return (pl%WX_COLOR_LIST_LENGTH + 1);
    else
        return player_colors[pl];
}


// LoadOptions: used to get the player colors from a config file
void GambitDrawSettings::LoadOptions(char *file_name)
{
    // Load the player color settings
    char l_tempstr[40];
    int num_player_colors = 0;
    wxGetResource("Gambit", "Num-Player-Colors", &num_player_colors, file_name);
    player_colors = gBlock<int>(num_player_colors);

    for (int i = 1; i <= num_player_colors; i++)
    {
        sprintf(l_tempstr, "Player-Color-%d", i);
        wxGetResource("Gambit", l_tempstr, &(player_colors[i]), file_name);
    }
}


// SaveOptions: used to store the player colors to a config file
void GambitDrawSettings::SaveOptions(char *s) const
{
    char *file_name = (s) ? s : INIFILE;

    // Save the player color settings
    wxWriteResource("Gambit", "Num-Player-Colors", player_colors.Length(), file_name);
    char s_tempstr[40];

    for (int i = 1; i <= player_colors.Length(); i++)
    {
        sprintf(s_tempstr, "Player-Color-%d", i);
        wxWriteResource("Gambit", s_tempstr, player_colors[i], file_name);
    }
}


// PlayerColorDialog: allows the user to change player colors graphically
void GambitDrawSettings::PlayerColorDialog(gArray<gText> &player_names)
{
    MyDialogBox *player_color_dialog = new MyDialogBox(0, "Edit Player Colors");

    // list of player names
    wxStringList *name_list = new wxStringList;
    name_list->Add("Chance");
    int i;
    for (i = 1; i <= player_names.Length(); i++) 
        name_list->Add((char *)player_names[i]);

    // list of color names
    wxStringList *color_list = new wxStringList;
    for (i = 0; i < WX_COLOR_LIST_LENGTH; i++) 
        color_list->Add(wx_color_list[i]);

    // Create listboxes.
    char *name = new char[100];
    wxFormItem *name_item = wxMakeFormString("Player", &name, wxFORM_SINGLE_LIST,
                                             new wxList(wxMakeConstraintStrings(name_list), 0), 
                                             NULL, wxVERTICAL);
    player_color_dialog->Form()->Add(name_item);
    char *color = new char[100];
    wxFormItem *color_item = wxMakeFormString("Color", &color, wxFORM_SINGLE_LIST,
                                              new wxList(wxMakeConstraintStrings(color_list), 0), 
                                              NULL, wxVERTICAL);
    player_color_dialog->Form()->Add(color_item);
    player_color_dialog->Form()->Add(wxMakeFormNewLine());
    Bool save_now = FALSE;
    player_color_dialog->Form()->Add(wxMakeFormBool("Save now", &save_now));

    // Associate the panel.
    player_color_dialog->Form()->AssociatePanel(player_color_dialog);
    wxListBox *name_listbox = (wxListBox *)name_item->GetPanelItem();
    wxListBox *color_listbox = (wxListBox *)color_item->GetPanelItem();
    name_listbox->Callback((wxFunction)player_color_callback);

    player_color_struct pcs =
    {
        this, color_listbox
    };

    name_listbox->wxWindow::SetClientData((char *)&pcs);

    // Init the first entry.
    wxCommandEvent ev(wxEVENT_TYPE_CHOICE_COMMAND);
    ev.commandInt = 0;
    name_listbox->Command(ev);
    player_color_dialog->Go1();

    if (player_color_dialog->Completed() == wxOK)
    {
        SetPlayerColor(name_listbox->GetSelection(), color_listbox->GetSelection());

        if (save_now) 
            SaveOptions();
    }

    // Reset the first entry.
    ev.commandInt = -1;
    name_listbox->Command(ev);

    delete [] name;
    delete [] color;
}


void GambitDrawSettings::player_color_callback(wxListBox &ob, wxCommandEvent &ev)
{
    static int prev_pos = -1;
    int new_pos = ev.commandInt;

    if (new_pos >= 0)
    {
        player_color_struct *pcs = 
            (player_color_struct *)ob.wxWindow::GetClientData();

        if (prev_pos >= 0)
            pcs->parent->SetPlayerColor(prev_pos, pcs->color_item->GetSelection());

        int color = pcs->parent->GetPlayerColor(new_pos);
        pcs->color_item->SetSelection(color);
    }

    prev_pos = new_pos;
}

