#include "gblock.h"
class GambitDrawSettings
{
private:
	void	UpdatePlayerColor(int pl);
protected:
	static gBlock<int> player_colors;
public:
// Constructor
	NormalDrawSettings(void);
// Controls the color each player gets
	void	SetPlayerColor(int pl,int cl);
	int		GetPlayerColor(int pl);
};

