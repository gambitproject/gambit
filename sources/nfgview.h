//
// FILE: nfgview.h -- Declaration of normal form viewer
//
// $Id$
//

#ifndef NFGVIEW_H
#define NFGVIEW_H

#include "gameview.h"

class guiNfgGrid;
class guiNfgInfoPanel;
class guiNfgFrame;

class guiNfgView {
private:
  guiNfgGrid *m_grid;
  guiNfgInfoPanel *m_infoPanel;
  guiNfgFrame *m_parent;

  Nfg *m_nfg;
  int m_currentSupport;
  gList<NFSupport *> m_supports;

public:
  guiNfgView(guiNfgFrame *p_parent, Nfg *p_nfg,
	     wxSplitterWindow *p_solutionSplitter,
	     wxSplitterWindow *p_infoSplitter);

  Nfg *GetNfg(void) const { return m_nfg; }
  guiNfgGrid *GridWindow(void) const { return m_grid; }

  void SetPlayers(int p_rowPlayer, int p_colPlayer);
  void SetProfile(const gArray<int> &p_profile);
  void SetOutcome(NFOutcome *);
  void SetPayoffs(void);
  void SetSolution(const MixedSolution &);

  int RowPlayer(void) const;
  int ColPlayer(void) const;

  const gList<NFSupport *> &Supports(void) const
    { return m_supports; }
  int NumSupports(void) const { return m_supports.Length(); }
  NFSupport *CurrentSupport(void) const 
    { return m_supports[m_currentSupport]; }
  int CurrentSupportIndex(void) const { return m_currentSupport; }

  void AddSupport(NFSupport *);
  void DeleteSupport(int p_support);
  void SetCurrentSupport(int p_support);
  void EditCurrentSupport(const NFSupport &);
};

#endif  // NFGVIEW_H
