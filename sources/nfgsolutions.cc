//
// FILE: nfgsolutions.cc -- Normal form solutions display
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

#include "nfgframe.h"
#include "nfgsolutions.h"

const int idSOLUTION_WINDOW = 5001;

BEGIN_EVENT_TABLE(guiNfgSolutions, wxListCtrl)
  EVT_LIST_ITEM_SELECTED(idSOLUTION_WINDOW, OnSelected)
END_EVENT_TABLE()

guiNfgSolutions::guiNfgSolutions(guiNfgFrame *p_parent, wxWindow *p_window,
				 Nfg &p_nfg)
  : wxListCtrl(p_window, idSOLUTION_WINDOW,
	       wxPoint(-1, -1), wxDefaultSize, wxLC_REPORT),
    m_nfg(p_nfg), m_parent(p_parent)
{
  SetSingleStyle(wxLC_REPORT);
  InsertColumn(0, "Creator", wxLIST_FORMAT_LEFT, 100);
  InsertColumn(1, "Nash?", wxLIST_FORMAT_CENTRE, 50);
  InsertColumn(2, "Perfect?", wxLIST_FORMAT_CENTRE, 70);
  InsertColumn(3, "Liap", wxLIST_FORMAT_RIGHT, 100);
}

void guiNfgSolutions::AddSolutions(const gList<MixedSolution> &p_solutions)
{
  for (int i = 1; i <= p_solutions.Length(); i++) {
    int index = m_solutions.Length() + i - 1;
    InsertItem(index, (char *) NameNfgAlgType(p_solutions[i].Creator()));
    SetItem(index, 1, (char *) Name(p_solutions[i].IsNash()));
    SetItem(index, 2, (char *) Name(p_solutions[i].IsPerfect()));
    SetItem(index, 3, (char *) ToText(p_solutions[i].LiapValue()));
  }

  m_solutions += p_solutions;
}

void guiNfgSolutions::OnSelected(wxListEvent &p_event)
{
  m_parent->SetSolution(m_solutions[p_event.m_itemIndex + 1]);
}

/*
void guiNfgSolutions::OnChangeLabels(void)
{
  wxString labels[] = { "Creator", "Nash", "Perfect", "Liap" };

  for (int i = 0; i <= 3; i++) {
    SetLabelValue(wxHORIZONTAL, labels[i], i);
  }

  for (int i = 0; i < GetRows(); i++) {
    SetLabelValue(wxVERTICAL, (char *) ToText(i + 1), i);
  }
}

void guiNfgSolutions::OnSelectCell(int p_row, int)
{
  m_parent->SetSolution(p_row);
}

int guiNfgSolutions::CurrentSolution(void) const
{
  return GetCursorRow() + 1;
}
*/  
