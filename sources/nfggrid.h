//
// FILE: nfggrid.h -- Normal form grid display window
//
// $Id$
//

#ifndef NFGGRID_H
#define NFGGRID_H

class guiNfgView;

class guiNfgGrid : public wxGrid {
private:
  Nfg &m_nfg;
  NFSupport m_support;
  guiNfgView *m_parent;
  int m_rowPlayer, m_colPlayer;
  gArray<int> m_profile;
  bool m_showProbs;
  const MixedSolution *m_solution;

public:
  guiNfgGrid(guiNfgView *m_parent, wxWindow *p_window, Nfg &p_nfg);
  virtual ~guiNfgGrid();

  void OnSelectCell(int row, int col);
  void OnChangeLabels(void);

  void SetPlayers(int p_rowPlayer, int p_colPlayer);
  void SetProfile(const gArray<int> &p_profile);
  const gArray<int> &GetProfile(void) const { return m_profile; }
  void SetOutcome(NFOutcome *);
  void SetSupport(NFSupport *);
  void SetSolution(const MixedSolution &);

  int RowPlayer(void) const { return m_rowPlayer; }
  int ColPlayer(void) const { return m_colPlayer; }

  void UpdateTable(void);

  void SetShowProbs(bool);

  void OnChar(wxKeyEvent &);
  DECLARE_EVENT_TABLE()
};

#endif  // NFGGRID_H
