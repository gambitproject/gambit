#include "gambdraw.h" 
#define WX_COLOR_LIST_LENGTH 11
gBlock<int> GambitDrawSettings::player_colors=gBlock<int>();
//-----------------------------Player Colors-----------------------//
// Unless SetPlayerColor has been used to define a color for a player,
// the color is just the player # modulus the number of colors.  This
// # is used to index into the wx_color_list array (wx_misc.h,.cc).
// Note that extform uses player numbers starting from -1, so, add 2

// UpdatePlayerColor: makes sure that the player_colors block contains
// this player.  If not, extends the block using default scheme
void GambitDrawSettings::UpdatePlayerColor(int pl)
{
if (player_colors.Length()<pl)
{
	int old_length=player_colors.Length();
	player_colors+=gBlock<int>(pl-old_length);
	for (int i=old_length+1;i<=pl;i++)
		player_colors[i]=(i%WX_COLOR_LIST_LENGTH)+1;
}
}
// SetPlayerColor: used to specify alternate coloring scheme for players
// If this is not called, default scheme is used (see above)
void GambitDrawSettings::SetPlayerColor(int pl,int cl)
{
pl+=2;
UpdatePlayerColor(pl);
player_colors[pl]=(cl%WX_COLOR_LIST_LENGTH);
}
// GetPlayerColor: used to get the players display color.
int GambitDrawSettings::GetPlayerColor(int pl)
{
pl+=2;
UpdatePlayerColor(pl);
return player_colors[pl];
}

