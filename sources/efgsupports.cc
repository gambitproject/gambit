//
// FILE: efgsupports.cc -- Extensive form supports display
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

#include "efgview.h"
#include "efgsupports.h"

const int idSUPPORTS_WINDOW = 6001;

BEGIN_EVENT_TABLE(guiEfgSupports, wxListCtrl)
  EVT_LIST_ITEM_SELECTED(idSUPPORTS_WINDOW, OnSelected)
END_EVENT_TABLE()

guiEfgSupports::guiEfgSupports(guiEfgView *p_parent, Efg &p_efg)
  : wxListCtrl(p_parent, idSUPPORTS_WINDOW,
	       wxPoint(-1, -1), wxDefaultSize, wxLC_LIST),
    m_efg(p_efg), m_parent(p_parent)
{
  SetSingleStyle(wxLC_REPORT);
  InsertColumn(0, "Support Name", wxLIST_FORMAT_LEFT, 100);

  EFSupport *support = new EFSupport(p_efg);
  support->SetName("Full Support");
  AddSupport(support);
  m_currentSupport = 1;
}

guiEfgSupports::~guiEfgSupports()
{
  for (int i = 1; i <= m_supports.Length(); delete m_supports[i++]);
}

void guiEfgSupports::AddSupport(EFSupport *p_support)
{
  int index = m_supports.Length();
  InsertItem(index, (char *) p_support->GetName());

  m_supports += p_support;
}

void guiEfgSupports::SetCurrentSupport(int p_index)
{
  SetItemState(m_currentSupport - 1, 0, wxLIST_STATE_SELECTED);
  m_currentSupport = p_index;
  SetItemState(m_currentSupport - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void guiEfgSupports::EditCurrentSupport(const EFSupport &p_support)
{
  *m_supports[m_currentSupport] = p_support;
  SetItem(m_currentSupport - 1, 0, (char *) p_support.GetName());
}

void guiEfgSupports::DeleteSupport(int p_index)
{
  SetItemState(m_currentSupport - 1, 0, wxLIST_STATE_SELECTED);
  if (m_currentSupport == p_index) {
    m_currentSupport = 1;
  }
  else if (m_currentSupport > p_index) {
    m_currentSupport -= 1;
  }

  delete m_supports.Remove(p_index);
  DeleteItem(p_index - 1);
  SetItemState(m_currentSupport - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  m_parent->SetCurrentSupport(m_currentSupport, false);
}

void guiEfgSupports::OnSelected(wxListEvent &p_event)
{
  m_currentSupport = p_event.m_itemIndex + 1;
  m_parent->SetCurrentSupport(p_event.m_itemIndex + 1, false);
}

gText guiEfgSupports::UniqueSupportName(void) const
{
  int number = m_supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_supports.Length(); i++) {
      if (m_supports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}
