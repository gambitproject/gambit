//
// FILE: dlsubgames.h -- Dialog for subgame viewing and marking
//
// $Id$
//

#ifndef DLSUBGAMES_H
#define DLSUBGAMES_H

class dialogSubgames : public guiAutoDialog {
private:
  FullEfg &m_efg;
  wxListCtrl *m_subgameList;

public:
  dialogSubgames(wxWindow *, FullEfg &);
  virtual ~dialogSubgames() { }
};

#endif   // DLSUBGAMES_H
