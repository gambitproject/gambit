//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of extensive form support palette window
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgsupport.h"

const int idACTIONTREE = 8003;

class widgetActionTree : public wxTreeCtrl {
private:
  EfgSupportWindow *m_parent;
  wxMenu *m_menu;

  void OnRightClick(wxTreeEvent &);
  void OnMiddleClick(wxTreeEvent &);
  void OnKeypress(wxTreeEvent &);

public:
  widgetActionTree(EfgSupportWindow *p_parent);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(widgetActionTree, wxTreeCtrl)
  EVT_TREE_KEY_DOWN(idACTIONTREE, widgetActionTree::OnKeypress)
  EVT_TREE_ITEM_MIDDLE_CLICK(idACTIONTREE, widgetActionTree::OnMiddleClick)
  EVT_TREE_ITEM_RIGHT_CLICK(idACTIONTREE, widgetActionTree::OnRightClick)
END_EVENT_TABLE()

widgetActionTree::widgetActionTree(EfgSupportWindow *p_parent)
  : wxTreeCtrl(p_parent, idACTIONTREE), m_parent(p_parent)
{ 
  m_menu = new wxMenu;
  m_menu->Append(GBT_EFG_MENU_SUPPORT_DUPLICATE, "Duplicate support",
		 "Duplicate this support");
  m_menu->Append(GBT_EFG_MENU_SUPPORT_DELETE, "Delete support",
		 "Delete this support");
}

void widgetActionTree::OnRightClick(wxTreeEvent &p_event)
{
  // Cannot delete the "full support"
  m_menu->Enable(GBT_EFG_MENU_SUPPORT_DELETE, (m_parent->GetSupport() > 0));
  PopupMenu(m_menu, p_event.GetPoint());
}

void widgetActionTree::OnKeypress(wxTreeEvent &p_event)
{
  if (m_parent->GetSupport() == 0) {
    return;
  }
  if (p_event.GetCode() == WXK_SPACE) {
    m_parent->ToggleItem(GetSelection());
  }
}

void widgetActionTree::OnMiddleClick(wxTreeEvent &p_event)
{
  if (m_parent->GetSupport() == 0) {
    return;
  }
  m_parent->ToggleItem(p_event.GetItem());
}

//===========================================================================
//                       class EfgSupportWindow 
//===========================================================================

const int idSUPPORTLISTCHOICE = 8000;
const int idSUPPORTPREVBUTTON = 8001;
const int idSUPPORTNEXTBUTTON = 8002;

BEGIN_EVENT_TABLE(EfgSupportWindow, wxPanel)
  EVT_CHOICE(idSUPPORTLISTCHOICE, EfgSupportWindow::OnSupportList)
  EVT_BUTTON(idSUPPORTPREVBUTTON, EfgSupportWindow::OnSupportPrev)
  EVT_BUTTON(idSUPPORTNEXTBUTTON, EfgSupportWindow::OnSupportNext)
  EVT_TREE_ITEM_COLLAPSING(idACTIONTREE, EfgSupportWindow::OnTreeItemCollapse)
END_EVENT_TABLE()

EfgSupportWindow::EfgSupportWindow(gbtGameDocument *p_doc,
				   wxWindow *p_parent)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_doc(p_doc), m_map(gbtEfgAction())
{
  SetAutoLayout(true);

  m_supportList = new wxChoice(this, idSUPPORTLISTCHOICE,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0);
  m_prevButton = new wxButton(this, idSUPPORTPREVBUTTON, "<-",
			      wxDefaultPosition, wxSize(30, 30));
  m_nextButton = new wxButton(this, idSUPPORTNEXTBUTTON, "->",
			      wxDefaultPosition, wxSize(30, 30));
  m_actionTree = new widgetActionTree(this);
  UpdateValues();

  wxBoxSizer *selectSizer = new wxBoxSizer(wxHORIZONTAL);
  selectSizer->Add(m_prevButton, 0, wxALL, 5);
  selectSizer->Add(m_supportList, 1, wxALL | wxEXPAND, 5);
  selectSizer->Add(m_nextButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(selectSizer, 0, wxEXPAND, 5);
  topSizer->Add(m_actionTree, 1, wxEXPAND, 5);
  
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();

  Show(true);
}

void EfgSupportWindow::UpdateValues(void)
{
  m_supportList->Clear();

  const gList<EFSupport *> &supports = m_doc->m_efgShow->Supports();

  for (int i = 1; i <= supports.Length(); i++) {
    m_supportList->Append((char *)
			  (ToText(i) + ": " + supports[i]->GetName()));
  }

  int supportIndex = supports.Find(m_doc->m_efgShow->GetSupport());
  m_supportList->SetSelection(supportIndex - 1);
  m_prevButton->Enable((supportIndex > 1) ? true : false);
  m_nextButton->Enable((supportIndex < supports.Length()) ? true : false);

  m_actionTree->DeleteAllItems();

  m_actionTree->AddRoot((char *) m_doc->m_efgShow->GetSupport()->GetName());
  for (int pl = 1; pl <= m_doc->m_efgShow->GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = m_doc->m_efgShow->GetGame().GetPlayer(pl);

    wxTreeItemId id = m_actionTree->AppendItem(m_actionTree->GetRootItem(),
					       (char *) player.GetLabel());
    
    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      wxTreeItemId isetID = m_actionTree->AppendItem(id, 
						     (char *) infoset.GetLabel());
      for (int act = 1; act <= infoset.NumActions(); act++) {
	gbtEfgAction action = infoset.GetAction(act);
	wxTreeItemId actID = m_actionTree->AppendItem(isetID,
						      (char *) action.GetLabel());
	if (m_doc->m_efgShow->GetSupport()->Contains(action)) {
	  m_actionTree->SetItemTextColour(actID, *wxBLACK);
	}
	else {
	  m_actionTree->SetItemTextColour(actID, *wxLIGHT_GREY);
	}
	m_map.Define(actID, infoset.GetAction(act));
      }

      m_actionTree->Expand(isetID);
    }
    m_actionTree->Expand(id);
  }
  m_actionTree->Expand(m_actionTree->GetRootItem());

}

void EfgSupportWindow::OnSupportList(wxCommandEvent &p_event)
{
  m_doc->m_efgShow->SetSupportNumber(p_event.GetSelection() + 1);
}

void EfgSupportWindow::OnSupportPrev(wxCommandEvent &)
{
  m_doc->m_efgShow->SetSupportNumber(m_supportList->GetSelection());
}

void EfgSupportWindow::OnSupportNext(wxCommandEvent &)
{
  m_doc->m_efgShow->SetSupportNumber(m_supportList->GetSelection() + 2);

}

void EfgSupportWindow::OnTreeItemCollapse(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == m_actionTree->GetRootItem()) {
    p_event.Veto();
  }
}

void EfgSupportWindow::ToggleItem(wxTreeItemId p_id)
{
  gbtEfgAction action = m_map(p_id);
  if (action.IsNull()) {
    return;
  }

  if (m_doc->m_efgShow->GetSupport()->Contains(action) &&
      m_doc->m_efgShow->GetSupport()->NumActions(action.GetInfoset()) > 1) {
    m_doc->m_efgShow->GetSupport()->RemoveAction(action);
    m_actionTree->SetItemTextColour(p_id, *wxLIGHT_GREY);
  }
  else {
    m_doc->m_efgShow->GetSupport()->AddAction(action);
    m_actionTree->SetItemTextColour(p_id, *wxBLACK);
  }

  m_doc->m_efgShow->SetSupportNumber(m_supportList->GetSelection() + 1);
}


#include "base/gmap.imp"

static gOutput &operator<<(gOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gBaseMap<wxTreeItemId, gbtEfgAction>;
template class gOrdMap<wxTreeItemId, gbtEfgAction>;
