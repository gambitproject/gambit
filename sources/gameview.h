//
// FILE: gameview.h -- Declaration of game viewing child frame
//
// $Id$
//

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include "glist.h"
#include "efg.h"
#include "nfg.h"
#include "mixedsol.h"

#include "wx/grid.h"
#include "wx/splitter.h"

class guiNfgInfoPanel;
class guiEfgInfoPanel;
class guiEfgTree;

class gambitGameView : public wxMDIChildFrame {
protected:
  wxSplitterWindow *m_solutionSplitter;

public:
  gambitGameView(wxMDIParentFrame *p_parent, Nfg *p_nfg,
		 const wxPoint &p_position, const wxSize &p_size);
  gambitGameView(wxMDIParentFrame *p_parent, Efg *p_efg,
		 const wxPoint &p_position, const wxSize &p_size);
  virtual ~gambitGameView();

  int CurrentSolution(void) const;

  void OnActivate(wxActivateEvent& event);

  void OnEditLabel(wxCommandEvent &);
  void OnEditPlayers(wxCommandEvent &);

  void OnClose(wxCommandEvent &);

  void SetSolution(int p_solution);

  DECLARE_EVENT_TABLE()
};

#endif // GAMEVIEW_H





