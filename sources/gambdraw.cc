//
// FILE: gambdraw.cc -- Draw settings implementation
//
// $Id$
//


#include "wx.h"
#include "wxmisc.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif  // __BORLANDC__

#include "gambit.h"
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
    char *file_name = (s) ? s : (char *) INIFILE;

    // Save the player color settings
    wxWriteResource("Gambit", "Num-Player-Colors", player_colors.Length(), file_name);
    char s_tempstr[40];

    for (int i = 1; i <= player_colors.Length(); i++)
    {
        sprintf(s_tempstr, "Player-Color-%d", i);
        wxWriteResource("Gambit", s_tempstr, player_colors[i], file_name);
    }
}

class dialogPlayerColor : public guiAutoDialog {
private:
  wxListBox *m_playerNames, *m_colorNames;
  gBlock<int> m_colors;

  static void CallbackPlayer(wxListBox &p_object, wxCommandEvent &)
    { ((dialogPlayerColor *) p_object.wxEvtHandler::GetClientData())->
	OnPlayer(); }
  static void CallbackColor(wxListBox &p_object, wxCommandEvent &)
    { ((dialogPlayerColor *) p_object.wxEvtHandler::GetClientData())->
	OnColor(); }
  

  void OnPlayer(void);
  void OnColor(void);

public:
  dialogPlayerColor(wxWindow *, const gArray<gText> &, const gBlock<int> &);
  virtual ~dialogPlayerColor() { }

  const gBlock<int> &GetColors(void) const { return m_colors; }
};

dialogPlayerColor::dialogPlayerColor(wxWindow *p_parent,
				     const gArray<gText> &p_playerNames,
				     const gBlock<int> &p_colors)
  : guiAutoDialog(p_parent, "Edit Player Colors"), m_colors(p_colors)
{
  m_playerNames = new wxListBox(this, (wxFunction) CallbackPlayer, "Players");
  for (int pl = 1; pl <= p_playerNames.Length(); pl++) {
    m_playerNames->Append(ToText(pl) + ": " + p_playerNames[pl]);
  }
  m_playerNames->wxEvtHandler::SetClientData((char *) this);
  m_playerNames->SetSelection(0);
  m_playerNames->SetConstraints(new wxLayoutConstraints);
  m_playerNames->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_playerNames->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_playerNames->GetConstraints()->width.AsIs();
  m_playerNames->GetConstraints()->height.AsIs();

  m_colorNames = new wxListBox(this, (wxFunction) CallbackColor, "Colors");
  for (int i = 0; i < WX_COLOR_LIST_LENGTH; i++) {
    m_colorNames->Append((char *) wx_color_list[i]);
  }
  m_colorNames->wxEvtHandler::SetClientData((char *) this);
  m_colorNames->SetConstraints(new wxLayoutConstraints);
  m_colorNames->GetConstraints()->top.SameAs(m_playerNames, wxTop);
  m_colorNames->GetConstraints()->left.SameAs(m_playerNames, wxRight, 10);
  m_colorNames->GetConstraints()->width.AsIs();
  m_colorNames->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_playerNames, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  OnPlayer();
  Go();
}

void dialogPlayerColor::OnPlayer(void)
{
  m_colorNames->SetSelection(m_colors[m_playerNames->GetSelection() + 1] - 1);
}

void dialogPlayerColor::OnColor(void)
{
  int player = m_playerNames->GetSelection() + 1;
  m_colors[player + 2] = m_colorNames->GetSelection();
}

// PlayerColorDialog: allows the user to change player colors graphically
void GambitDrawSettings::PlayerColorDialog(gArray<gText> &player_names)
{
  dialogPlayerColor dialog(0, player_names, player_colors);

  if (dialog.Completed() == wxOK) {
    player_colors = dialog.GetColors();
  }
}


