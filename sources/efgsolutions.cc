//
// FILE: efgsolutions.cc -- Extensive form solutions display
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "behavsol.h"

#include "efgframe.h"
#include "efgsolutions.h"

const int idSOLUTION_WINDOW = 5001;

BEGIN_EVENT_TABLE(guiEfgSolutions, wxListCtrl)
  EVT_LIST_ITEM_SELECTED(idSOLUTION_WINDOW, OnSelected)
END_EVENT_TABLE()

guiEfgSolutions::guiEfgSolutions(guiEfgFrame *p_parent, wxWindow *p_window,
				 Efg &p_efg)
  : wxListCtrl(p_window, idSOLUTION_WINDOW,
	       wxPoint(-1, -1), wxDefaultSize, wxLC_LIST),
    m_efg(p_efg), m_parent(p_parent)
{
  SetSingleStyle(wxLC_REPORT);
  InsertColumn(0, "Creator", wxLIST_FORMAT_LEFT, 100);
  InsertColumn(1, "Nash?", wxLIST_FORMAT_CENTRE, 50);
  InsertColumn(2, "Subgame?", wxLIST_FORMAT_CENTRE, 70);
  InsertColumn(3, "Liap", wxLIST_FORMAT_RIGHT, 100);
}

void guiEfgSolutions::AddSolutions(const gList<BehavSolution> &p_solutions)
{
  for (int i = 1; i <= p_solutions.Length(); i++) {
    int index = m_solutions.Length() + i - 1;
    InsertItem(index, (char *) NameEfgAlgType(p_solutions[i].Creator()));
    SetItem(index, 1, (char *) Name(p_solutions[i].IsNash()));
    SetItem(index, 2, (char *) Name(p_solutions[i].IsSubgamePerfect()));
    SetItem(index, 3, (char *) ToText(p_solutions[i].LiapValue()));
  }

  m_solutions += p_solutions;
}

void guiEfgSolutions::OnSelected(wxListEvent &p_event)
{
  //  m_parent->SetSolution(m_solutions[p_event.m_itemIndex + 1]);
}
