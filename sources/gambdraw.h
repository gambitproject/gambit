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

class GambitDrawSettings {
private:
  void UpdatePlayerColor(int pl);

protected:
  static gBlock<int> player_colors;

public:
  GambitDrawSettings(void);
  virtual ~GambitDrawSettings() { }

  void SetPlayerColor(int pl, int color);
  int GetPlayerColor(int pl) const;

  // Allows to save/restore settings from file
  void SaveOptions(char *f = 0) const;
  void LoadOptions(char *f);

  // GUI Set colors
  void PlayerColorDialog(gArray<gText> &);
};

#define INIFILE     (gambitApp.ResourceFile())

#endif  // GAMBDRAW_H

