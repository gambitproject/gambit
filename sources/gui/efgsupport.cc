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

//===========================================================================
//                    class gbtEfgSetSupportCommand
//===========================================================================

gbtEfgSetSupportCommand::gbtEfgSetSupportCommand(gbtGameDocument *p_game,
						 int p_index)
  : gbtGameCommand(false, "Set support"),
    m_game(p_game), m_index(p_index)
{ }

bool gbtEfgSetSupportCommand::Do(void)
{
  m_game->m_currentEfgSupport = m_game->m_efgSupports[m_index];
  m_game->UpdateAllViews();
  return false;
}

//===========================================================================
//                 class gbtEfgDuplicateSupportCommand
//===========================================================================

gbtEfgDuplicateSupportCommand::
gbtEfgDuplicateSupportCommand(gbtGameDocument *p_game, int p_index)
  : gbtGameCommand(false, "Duplicate support"),
    m_game(p_game), m_index(p_index)
{ }

bool gbtEfgDuplicateSupportCommand::Do(void)
{
  EFSupport *newSupport = new EFSupport(*m_game->m_currentEfgSupport);
  //  newSupport->SetName(m_game->UniqueSupportName());
  m_game->m_efgSupports.Append(newSupport);
  m_game->m_currentEfgSupport = newSupport;
  m_game->UpdateAllViews();
  return false;
}

//===========================================================================
//                  class gbtEfgDeleteSupportCommand
//===========================================================================

gbtEfgDeleteSupportCommand::gbtEfgDeleteSupportCommand(gbtGameDocument *p_game)
  : gbtGameCommand(false, "Delete support"),
    m_game(p_game)
{ }

bool gbtEfgDeleteSupportCommand::Do(void)
{
  delete m_game->m_efgSupports.Remove(m_game->m_efgSupports.Find(m_game->m_currentEfgSupport));
  m_game->m_currentEfgSupport = m_game->m_efgSupports[1];
  m_game->UpdateAllViews();
  return false;
}

//===========================================================================
//                       class widgetActionTree
//===========================================================================

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
  m_menu->Append(efgmenuSUPPORT_DUPLICATE, "Duplicate support",
		 "Duplicate this support");
  m_menu->Append(efgmenuSUPPORT_DELETE, "Delete support",
		 "Delete this support");
}

void widgetActionTree::OnRightClick(wxTreeEvent &p_event)
{
  // Cannot delete the "full support"
  m_menu->Enable(efgmenuSUPPORT_DELETE, (m_parent->GetSupport() > 0));
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

EfgSupportWindow::EfgSupportWindow(gbtGameDocument *p_game,
				   EfgShow *p_efgShow, wxWindow *p_parent)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    gbtGameView(p_game),
    m_parent(p_efgShow), m_map((Action *) 0)
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

void EfgSupportWindow::OnUpdate(gbtGameView *)
{
  m_supportList->Clear();

  const gList<EFSupport *> &supports = m_game->m_efgSupports;

  for (int i = 1; i <= supports.Length(); i++) {
    m_supportList->Append((char *)
			  (ToText(i) + ": " + supports[i]->GetName()));
  }

  int supportIndex = supports.Find(m_game->GetEfgSupport());
  m_supportList->SetSelection(supportIndex - 1);
  m_prevButton->Enable((supportIndex > 1) ? true : false);
  m_nextButton->Enable((supportIndex < supports.Length()) ? true : false);

  m_actionTree->DeleteAllItems();

  m_actionTree->AddRoot((char *) m_game->GetEfgSupport()->GetName());
  for (int pl = 1; pl <= m_game->m_efg->NumPlayers(); pl++) {
    EFPlayer *player = m_game->m_efg->Players()[pl];

    wxTreeItemId id = m_actionTree->AppendItem(m_actionTree->GetRootItem(),
					       (char *) player->GetName());
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Infoset *infoset = player->Infosets()[iset];
      wxTreeItemId isetID = m_actionTree->AppendItem(id, 
						     (char *) infoset->GetName());
      for (int act = 1; act <= infoset->NumActions(); act++) {
	Action *action = infoset->Actions()[act];
	wxTreeItemId actID = m_actionTree->AppendItem(isetID,
						      (char *) action->GetName());
	if (m_game->GetEfgSupport()->Contains(action)) {
	  m_actionTree->SetItemTextColour(actID, *wxBLACK);
	}
	else {
	  m_actionTree->SetItemTextColour(actID, *wxLIGHT_GREY);
	}
	m_map.Define(actID, infoset->Actions()[act]);
      }

      m_actionTree->Expand(isetID);
    }
    m_actionTree->Expand(id);
  }
  m_actionTree->Expand(m_actionTree->GetRootItem());

}

void EfgSupportWindow::OnSupportList(wxCommandEvent &p_event)
{
  gbtEfgSetSupportCommand *command =
    new gbtEfgSetSupportCommand(m_game, p_event.GetSelection() + 1);
  m_game->Submit(command);
}

void EfgSupportWindow::OnSupportPrev(wxCommandEvent &)
{
  gbtEfgSetSupportCommand *command =
    new gbtEfgSetSupportCommand(m_game, m_supportList->GetSelection());
  m_game->Submit(command);
}

void EfgSupportWindow::OnSupportNext(wxCommandEvent &)
{
  gbtEfgSetSupportCommand *command =
    new gbtEfgSetSupportCommand(m_game, m_supportList->GetSelection() + 2);
  m_game->Submit(command);
}

void EfgSupportWindow::OnTreeItemCollapse(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == m_actionTree->GetRootItem()) {
    p_event.Veto();
  }
}

void EfgSupportWindow::ToggleItem(wxTreeItemId p_id)
{
  Action *action = m_map(p_id);
  if (!action) {
    return;
  }

  if (m_game->GetEfgSupport()->Contains(action) &&
      m_game->GetEfgSupport()->NumActions(action->BelongsTo()) > 1) {
    m_game->GetEfgSupport()->RemoveAction(action);
    m_actionTree->SetItemTextColour(p_id, *wxLIGHT_GREY);
  }
  else {
    m_game->GetEfgSupport()->AddAction(action);
    m_actionTree->SetItemTextColour(p_id, *wxBLACK);
  }
}


#include "base/gmap.imp"

static gOutput &operator<<(gOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gBaseMap<wxTreeItemId, Action *>;
template class gOrdMap<wxTreeItemId, Action *>;
