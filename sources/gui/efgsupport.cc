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
#include "id.h"

//==========================================================================
//                     class gbtCmdAddAction
//==========================================================================

//
// Add an action to the current support
//
class gbtCmdAddAction : public gbtGameCommand {
private:
  gbtEfgAction m_action;

public:
  gbtCmdAddAction(gbtEfgAction p_action) : m_action(p_action) { }
  virtual ~gbtCmdAddAction() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdAddAction::Do(gbtGameDocument *p_doc)
{
  p_doc->GetEfgSupportList().GetCurrent().AddAction(m_action);
}

//==========================================================================
//                    class gbtCmdRemoveAction
//==========================================================================

//
// Remove an action from the current support
//
class gbtCmdRemoveAction : public gbtGameCommand {
private:
  gbtEfgAction m_action;

public:
  gbtCmdRemoveAction(gbtEfgAction p_action) : m_action(p_action) { }
  virtual ~gbtCmdRemoveAction() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdRemoveAction::Do(gbtGameDocument *p_doc)
{
  p_doc->GetEfgSupportList().GetCurrent().RemoveAction(m_action);
}

//==========================================================================
//                    class gbtCmdSetEfgSupport
//==========================================================================

//
// Make a support the currently-selected support
//
class gbtCmdSetEfgSupport : public gbtGameCommand {
private:
  int m_index;

public:
  gbtCmdSetEfgSupport(int p_index) : m_index(p_index) { }
  virtual ~gbtCmdSetEfgSupport() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdSetEfgSupport::Do(gbtGameDocument *p_doc)
{
  p_doc->GetEfgSupportList().SetCurrentIndex(m_index);
}

//==========================================================================
//                     class gbtEfgSupportWindow
//==========================================================================

const int GBT_CONTROL_ACTION_TREE = 8000;
const int GBT_CONTROL_SUPPORT_LIST = 8001;
const int GBT_CONTROL_SUPPORT_PREV = 8002;
const int GBT_CONTROL_SUPPORT_NEXT = 8003;

class gbtEfgSupportWindow : public wxPanel, public gbtGameView {
private:
  wxChoice *m_supportList;
  wxButton *m_prevButton, *m_nextButton;
  wxTreeCtrl *m_actionTree;
  gbtOrdMap<wxTreeItemId, gbtEfgAction> m_map;
  wxMenu *m_menu;

  void OnSupportList(wxCommandEvent &);
  void OnSupportPrev(wxCommandEvent &);
  void OnSupportNext(wxCommandEvent &);

  void OnTreeItemCollapse(wxTreeEvent &);

  void OnRightClick(wxMouseEvent &);

  bool IsEfgView(void) const { return true; }
  bool IsNfgView(void) const { return false; }

public:
  gbtEfgSupportWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtEfgSupportWindow() { }

  int GetSupport(void) const { return m_supportList->GetSelection(); }
  void ToggleItem(wxTreeItemId);

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

class gbtActionTreeCtrl : public wxTreeCtrl {
private:
  gbtEfgSupportWindow *m_parent;
  wxMenu *m_menu;

  void OnRightClick(wxTreeEvent &);
  void OnMiddleClick(wxTreeEvent &);
  void OnKeypress(wxTreeEvent &);

public:
  gbtActionTreeCtrl(gbtEfgSupportWindow *p_parent);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtActionTreeCtrl, wxTreeCtrl)
  EVT_TREE_KEY_DOWN(GBT_CONTROL_ACTION_TREE, gbtActionTreeCtrl::OnKeypress)
  EVT_TREE_ITEM_MIDDLE_CLICK(GBT_CONTROL_ACTION_TREE, 
			     gbtActionTreeCtrl::OnMiddleClick)
  EVT_TREE_ITEM_RIGHT_CLICK(GBT_CONTROL_ACTION_TREE,
			    gbtActionTreeCtrl::OnRightClick)
END_EVENT_TABLE()

gbtActionTreeCtrl::gbtActionTreeCtrl(gbtEfgSupportWindow *p_parent)
  : wxTreeCtrl(p_parent, GBT_CONTROL_ACTION_TREE), m_parent(p_parent)
{ 
  m_menu = new wxMenu;
  m_menu->Append(GBT_MENU_SUPPORTS_DUPLICATE, _("Duplicate support"),
		 _("Duplicate this support"));
  m_menu->Append(GBT_MENU_SUPPORTS_DELETE, _("Delete support"),
		 _("Delete this support"));
}

void gbtActionTreeCtrl::OnRightClick(wxTreeEvent &p_event)
{
  // Cannot delete the "full support"
  m_menu->Enable(GBT_MENU_SUPPORTS_DELETE, (m_parent->GetSupport() > 0));
  PopupMenu(m_menu, p_event.GetPoint());
}

void gbtActionTreeCtrl::OnKeypress(wxTreeEvent &p_event)
{
  if (m_parent->GetSupport() == 0) {
    return;
  }
  if (p_event.GetCode() == WXK_SPACE) {
    m_parent->ToggleItem(GetSelection());
  }
}

void gbtActionTreeCtrl::OnMiddleClick(wxTreeEvent &p_event)
{
  if (m_parent->GetSupport() == 0) {
    return;
  }
  m_parent->ToggleItem(p_event.GetItem());
}

//===========================================================================
//                       class gbtEfgSupportWindow 
//===========================================================================

BEGIN_EVENT_TABLE(gbtEfgSupportWindow, wxPanel)
  EVT_CHOICE(GBT_CONTROL_SUPPORT_LIST, gbtEfgSupportWindow::OnSupportList)
  EVT_BUTTON(GBT_CONTROL_SUPPORT_PREV, gbtEfgSupportWindow::OnSupportPrev)
  EVT_BUTTON(GBT_CONTROL_SUPPORT_NEXT, gbtEfgSupportWindow::OnSupportNext)
  EVT_TREE_ITEM_COLLAPSING(GBT_CONTROL_ACTION_TREE, 
			   gbtEfgSupportWindow::OnTreeItemCollapse)
END_EVENT_TABLE()

gbtEfgSupportWindow::gbtEfgSupportWindow(gbtGameDocument *p_doc,
				   wxWindow *p_parent)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    gbtGameView(p_doc), m_map(gbtEfgAction())
{
  SetAutoLayout(true);

  m_supportList = new wxChoice(this, GBT_CONTROL_SUPPORT_LIST,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0);
  m_prevButton = new wxButton(this, GBT_CONTROL_SUPPORT_PREV, wxT("<-"),
			      wxDefaultPosition, wxSize(30, 30));
  m_nextButton = new wxButton(this, GBT_CONTROL_SUPPORT_NEXT, wxT("->"),
			      wxDefaultPosition, wxSize(30, 30));
  m_actionTree = new gbtActionTreeCtrl(this);

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

void gbtEfgSupportWindow::OnUpdate(void)
{
  m_supportList->Clear();

  const gbtEfgSupportList &supports = m_doc->GetEfgSupportList();

  for (int i = 1; i <= supports.Length(); i++) {
    const gbtEfgSupport &support = supports.Get(i);
    m_supportList->Append(wxString::Format(wxT("%s"),
					   (char *)
					   (ToText(i) + ": " + support.GetLabel())));
  }

  int supportIndex = supports.GetCurrentIndex();
  const gbtEfgSupport &support = supports.GetCurrent();
  m_supportList->SetSelection(supportIndex - 1);
  m_prevButton->Enable((supportIndex > 1) ? true : false);
  m_nextButton->Enable((supportIndex < supports.Length()) ? true : false);

  m_actionTree->DeleteAllItems();

  m_actionTree->AddRoot(wxString::Format(wxT("%s"),
					 (char *) support.GetLabel()));
  for (int pl = 1; pl <= m_doc->GetEfg().NumPlayers(); pl++) {
    gbtEfgPlayer player = m_doc->GetEfg().GetPlayer(pl);

    wxTreeItemId id = m_actionTree->AppendItem(m_actionTree->GetRootItem(),
					       wxString::Format(wxT("%s"),
								(char *) player.GetLabel()));
    
    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      wxTreeItemId isetID = m_actionTree->AppendItem(id, 
						     wxString::Format(wxT("%s"), (char *) infoset.GetLabel()));
      for (int act = 1; act <= infoset.NumActions(); act++) {
	gbtEfgAction action = infoset.GetAction(act);
	wxTreeItemId actID = m_actionTree->AppendItem(isetID,
						      wxString::Format(wxT("%s"), (char *) action.GetLabel()));
	if (support.Contains(action)) {
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

void gbtEfgSupportWindow::OnSupportList(wxCommandEvent &p_event)
{
  m_doc->Submit(new gbtCmdSetEfgSupport(p_event.GetSelection() + 1));
}

void gbtEfgSupportWindow::OnSupportPrev(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdSetEfgSupport(m_supportList->GetSelection()));
}

void gbtEfgSupportWindow::OnSupportNext(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdSetEfgSupport(m_supportList->GetSelection() + 2));

}

void gbtEfgSupportWindow::OnTreeItemCollapse(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == m_actionTree->GetRootItem()) {
    p_event.Veto();
  }
}

void gbtEfgSupportWindow::ToggleItem(wxTreeItemId p_id)
{
  gbtEfgAction action = m_map(p_id);
  if (action.IsNull()) {
    return;
  }

  const gbtEfgSupport &support = m_doc->GetEfgSupportList().GetCurrent();
  if (support.Contains(action)) {
    if (support.NumActions(action.GetInfoset()) > 1) {
      m_doc->Submit(new gbtCmdRemoveAction(action));
    }
  }
  else {
    m_doc->Submit(new gbtCmdAddAction(action));
  }
}

//-------------------------------------------------------------------------
//                    class gbtEfgSupportFrame
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtEfgSupportFrame, wxFrame)
  EVT_MENU(wxID_CLOSE, gbtEfgSupportFrame::Close)
  EVT_CLOSE(gbtEfgSupportFrame::OnClose)
END_EVENT_TABLE()

gbtEfgSupportFrame::gbtEfgSupportFrame(gbtGameDocument *p_doc, 
				       wxWindow *p_parent)
  : wxFrame(p_parent, -1, wxT(""), wxDefaultPosition, wxSize(300, 300)),
    gbtGameView(p_doc)
{
  m_panel = new gbtEfgSupportWindow(p_doc, this);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));

  wxMenu *editMenu = new wxMenu;

  wxMenu *viewMenu = new wxMenu;

  wxMenu *formatMenu = new wxMenu;

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(editMenu, _("&Edit"));
  menuBar->Append(viewMenu, _("&View"));
  menuBar->Append(formatMenu, _("&Format"));
  SetMenuBar(menuBar);

  Show(false);
}

gbtEfgSupportFrame::~gbtEfgSupportFrame()
{ }

void gbtEfgSupportFrame::OnClose(wxCloseEvent &p_event)
{
  m_doc->SetShowEfgSupports(false);
  // Frame is now hidden; leave it that way, don't actually close
  p_event.Veto();
}

void gbtEfgSupportFrame::OnUpdate(gbtGameView *p_sender)
{
  if (m_doc->ShowEfgSupports()) {
    m_panel->OnUpdate();

    if (m_doc->GetFilename() != wxT("")) {
      SetTitle(wxString::Format(_("Gambit - Supports: [%s] %s"), 
				(const char *) m_doc->GetFilename().mb_str(), 
				(char *) m_doc->GetEfg().GetLabel()));
    }
    else {
      SetTitle(wxString::Format(_("Gambit - Supports: %s"),
				(char *) m_doc->GetEfg().GetLabel()));
    }
  }
  Show(m_doc->ShowEfgSupports());
}


#include "base/gmap.imp"

static gbtOutput &operator<<(gbtOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gbtBaseMap<wxTreeItemId, gbtEfgAction>;
template class gbtOrdMap<wxTreeItemId, gbtEfgAction>;
