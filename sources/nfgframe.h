//
// FILE: nfgframe.h -- Frame class for normal form games
//
// $Id$
//

#ifndef NFGFRAME_H
#define NFGFRAME_H

#include "gameview.h"

class guiNfgView;
class guiNfgSolutions;

class guiNfgFrame : public gambitGameView {
private:
  guiNfgView *m_nfgView;
  guiNfgSolutions *m_solutionView;

public:
  guiNfgFrame(wxMDIParentFrame *p_parent, Nfg *p_nfg,
	     const wxPoint &p_position, const wxSize &p_size);
  virtual ~guiNfgFrame();

  int CurrentSolution(void) const;
  void SetSolution(const MixedSolution &);

  void OnActivate(wxActivateEvent& event);
  void OnSupportsNfgUndominated(wxCommandEvent &);
  void OnSupportsNfgNew(wxCommandEvent &);
  void OnSupportsNfgEdit(wxCommandEvent &);
  void OnSupportsNfgDelete(wxCommandEvent &);
  void OnSupportsNfgSelect(wxCommandEvent &);
  void OnSolveNfgStandard(wxCommandEvent &);
  void OnSolveNfgCustomEnumPure(wxCommandEvent &);
  void OnSolveNfgCustomEnumMixed(wxCommandEvent &);
  void OnSolveNfgCustomLp(wxCommandEvent &);
  void OnSolveNfgCustomLcp(wxCommandEvent &);
  void OnSolveNfgCustomLiap(wxCommandEvent &);
  void OnSolveNfgCustomPolEnum(wxCommandEvent &);
  void OnSolveNfgCustomSimpdiv(wxCommandEvent &);
  void OnSolveNfgCustomQre(wxCommandEvent &);
  void OnSolveNfgCustomQreGrid(wxCommandEvent &);

  void OnEditLabel(wxCommandEvent &);
  void OnEditPlayers(wxCommandEvent &);

  void OnViewSolutions(wxCommandEvent &);

  void OnClose(wxCommandEvent &);

  DECLARE_EVENT_TABLE()
};

#endif // NFGFRAME_H

