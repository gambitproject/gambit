//
// FILE: efgframe.h -- Frame class for extensive form
//
// $Id$
//

#ifndef EFGFRAME_H
#define EFGFRAME_H

#include "gameview.h"

class guiEfgView;
class guiEfgSolutions;

class guiEfgFrame : public gambitGameView {
private:
  guiEfgView *m_efgView;
  guiEfgSolutions *m_solutionView;
  int m_currentSupport;
  Node *m_copyNode;
  EFOutcome *m_copyOutcome;

  void OnViewSolutions(wxCommandEvent &);

  void OnSolveCustomNfgEnumMixed(wxCommandEvent &);

public:
  guiEfgFrame(wxMDIParentFrame *p_parent, Efg *p_efg,
	      const wxPoint &p_position, const wxSize &p_size);
  virtual ~guiEfgFrame() { }

  void OnActivate(wxActivateEvent& event);

  void OnSolveEfgStandard(wxCommandEvent &);
  void OnSolveEfgCustom(wxCommandEvent &);
  void OnSolveEfgNfg(wxCommandEvent &);

  void OnEditCopyEfg(wxCommandEvent &);
  void OnEditPasteEfg(wxCommandEvent &);
  void OnEditDeleteEfg(wxCommandEvent &);
  void OnEditLabel(wxCommandEvent &);
  void OnEditPlayers(wxCommandEvent &);
  void OnEditPropertiesEfg(wxCommandEvent &);

  void OnViewZoomIn(wxCommandEvent &);
  void OnViewZoomOut(wxCommandEvent &);

  void OnClose(wxCommandEvent &);

  void SetSolution(int p_solution);

  DECLARE_EVENT_TABLE()
};

#endif // EFGFRAME_H
