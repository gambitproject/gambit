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

class guiNfgView : public wxWindow {
private:
  guiNfgGrid *m_grid;
  guiNfgInfoPanel *m_infoPanel;

  Nfg *m_nfg;
  int m_currentSupport;
  gList<NFSupport *> m_supports;

public:
  guiNfgView(Nfg *p_nfg, wxWindow *p_solutionSplitter);

  Nfg *GetNfg(void) const { return m_nfg; }
  wxWindow *GridWindow(void) const;
  wxWindow *InfoPanel(void) const;
  void ShowWindows(bool);

  void SetPlayers(int p_rowPlayer, int p_colPlayer);
  void SetProfile(const gArray<int> &p_profile);
  void SetOutcome(NFOutcome *);
  void SetPayoffs(void);
  void SetSolution(const MixedSolution &);
  void SetShowProbs(bool);

  int RowPlayer(void) const;
  int ColPlayer(void) const;

  const gList<NFSupport *> &Supports(void) const
    { return m_supports; }
  int NumSupports(void) const { return m_supports.Length(); }
  NFSupport *CurrentSupport(void) const 
    { return m_supports[m_currentSupport]; }
  int CurrentSupportIndex(void) const { return m_currentSupport; }
  gText UniqueSupportName(void) const;

  void AddSupport(NFSupport *);
  void DeleteSupport(int p_support);
  void SetCurrentSupport(int p_support);
  void EditCurrentSupport(const NFSupport &);

  void OnSize(wxSizeEvent &);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGVIEW_H
