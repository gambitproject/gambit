//
// FILE: dlefgplayer.h -- (Extensive form) player selection dialog
//
// $Id$
//

#ifndef DLEFGPLAYER_H
#define DLEFGPLAYER_H

class dialogEfgSelectPlayer : public guiAutoDialog {
private:
  const Efg &m_efg;
  bool m_chance;
  wxListBox *m_playerNameList;

  const char *HelpString(void) const { return "Infoset Menu"; }

public:
  dialogEfgSelectPlayer(const Efg &, bool p_chance = true, wxWindow * = 0);
  virtual ~dialogEfgSelectPlayer();

  EFPlayer *GetPlayer(void);
};

#endif  // DLEFGPLAYER_H
