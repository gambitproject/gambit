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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
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
  gbtGameAction m_action;

public:
  gbtCmdAddAction(gbtGameAction p_action) : m_action(p_action) { }
  virtual ~gbtCmdAddAction() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdAddAction::Do(gbtGameDocument *p_doc)
{
  p_doc->GetEfgSupportList().GetCurrent()->AddAction(m_action);
}

//==========================================================================
//                    class gbtCmdRemoveAction
//==========================================================================

//
// Remove an action from the current support
//
class gbtCmdRemoveAction : public gbtGameCommand {
private:
  gbtGameAction m_action;

public:
  gbtCmdRemoveAction(gbtGameAction p_action) : m_action(p_action) { }
  virtual ~gbtCmdRemoveAction() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdRemoveAction::Do(gbtGameDocument *p_doc)
{
  p_doc->GetEfgSupportList().GetCurrent()->RemoveAction(m_action);
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
//                    class gbtEfgSupportWidget
//==========================================================================

gbtEfgSupportWidget::gbtEfgSupportWidget(wxWindow *p_parent,
					 wxWindowID p_id)
  : wxTreeCtrl(p_parent, p_id), m_map(gbtGameAction())
{ 
  Connect(p_id, wxEVT_COMMAND_TREE_ITEM_COLLAPSING,
	  (wxObjectEventFunction) (wxEventFunction)
	  (wxTreeEventFunction) &gbtEfgSupportWidget::OnTreeItemCollapse);
}

void gbtEfgSupportWidget::SetSupport(const gbtEfgSupport &p_support)
{
  DeleteAllItems();
  AddRoot(wxString::Format(wxT("%s"), (char *) p_support->GetLabel()));
  for (int pl = 1; pl <= p_support->NumPlayers(); pl++) {
    gbtGamePlayer player = p_support->GetPlayer(pl);

    wxTreeItemId id = AppendItem(GetRootItem(),
				 wxString::Format(wxT("%s"),
						  (char *) player->GetLabel()));
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      gbtGameInfoset infoset = player->GetInfoset(iset);
      wxTreeItemId isetID = AppendItem(id, 
				       wxString::Format(wxT("%s"), (char *) infoset->GetLabel()));
      for (int act = 1; act <= infoset->NumActions(); act++) {
	gbtGameAction action = infoset->GetAction(act);
	wxTreeItemId actID = AppendItem(isetID,
					wxString::Format(wxT("%s"), (char *) action->GetLabel()));
	if (p_support->Contains(action)) {
	  SetItemTextColour(actID, *wxBLACK);
	}
	else {
	  SetItemTextColour(actID, *wxLIGHT_GREY);
	}
	m_map.Define(actID, infoset->GetAction(act));
      }

      Expand(isetID);
    }
    Expand(id);
  }
  Expand(GetRootItem());
}

void gbtEfgSupportWidget::OnTreeItemCollapse(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == GetRootItem()) {
    p_event.Veto();
  }
}

//==========================================================================
//                     class gbtEfgSupportWindow
//==========================================================================

const int GBT_EFG_SUPPORT_LIST = 8000;
const int GBT_EFG_SUPPORT_PREV = 8001;
const int GBT_EFG_SUPPORT_NEXT = 8002;
const int GBT_EFG_ACTION_TREE = 8003;

class gbtEfgSupportWindow : public wxPanel, public gbtGameView {
private:
  wxChoice *m_supportList;
  wxButton *m_prevButton, *m_nextButton;
  gbtEfgSupportWidget *m_supportWidget;

  void OnSupportList(wxCommandEvent &);
  void OnSupportPrev(wxCommandEvent &);
  void OnSupportNext(wxCommandEvent &);

  void OnTreeKeypress(wxTreeEvent &);
  void OnTreeItemActivated(wxTreeEvent &);

  void ToggleAction(wxTreeItemId);

  // Overriding view members
  bool IsEfgView(void) const { return true; }
  bool IsNfgView(void) const { return false; }

public:
  gbtEfgSupportWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtEfgSupportWindow() { }

  void OnUpdate(void);

  DECLARE_EVENT_TABLE()
};

//===========================================================================
//                       class gbtEfgSupportWindow 
//===========================================================================

BEGIN_EVENT_TABLE(gbtEfgSupportWindow, wxPanel)
  EVT_CHOICE(GBT_EFG_SUPPORT_LIST, gbtEfgSupportWindow::OnSupportList)
  EVT_BUTTON(GBT_EFG_SUPPORT_PREV, gbtEfgSupportWindow::OnSupportPrev)
  EVT_BUTTON(GBT_EFG_SUPPORT_NEXT, gbtEfgSupportWindow::OnSupportNext)
  EVT_TREE_KEY_DOWN(GBT_EFG_ACTION_TREE,
		    gbtEfgSupportWindow::OnTreeKeypress)
  EVT_TREE_ITEM_ACTIVATED(GBT_EFG_ACTION_TREE,
			  gbtEfgSupportWindow::OnTreeItemActivated)
END_EVENT_TABLE()

gbtEfgSupportWindow::gbtEfgSupportWindow(gbtGameDocument *p_doc,
				   wxWindow *p_parent)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    gbtGameView(p_doc)
{
  SetAutoLayout(true);

  m_supportList = new wxChoice(this, GBT_EFG_SUPPORT_LIST,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0);
  m_prevButton = new wxButton(this, GBT_EFG_SUPPORT_PREV, wxT("<-"),
			      wxDefaultPosition, wxSize(30, 30));
  m_nextButton = new wxButton(this, GBT_EFG_SUPPORT_NEXT, wxT("->"),
			      wxDefaultPosition, wxSize(30, 30));
  m_supportWidget = new gbtEfgSupportWidget(this, GBT_EFG_ACTION_TREE);

  wxBoxSizer *selectSizer = new wxBoxSizer(wxHORIZONTAL);
  selectSizer->Add(m_prevButton, 0, wxALL, 5);
  selectSizer->Add(m_supportList, 1, wxALL | wxEXPAND, 5);
  selectSizer->Add(m_nextButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(selectSizer, 0, wxEXPAND, 5);
  topSizer->Add(m_supportWidget, 1, wxEXPAND, 5);
  
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
					   (ToText(i) + ": " + support->GetLabel())));
  }

  int supportIndex = supports.GetCurrentIndex();
  m_supportList->SetSelection(supportIndex - 1);
  m_prevButton->Enable((supportIndex > 1) ? true : false);
  m_nextButton->Enable((supportIndex < supports.Length()) ? true : false);
  m_supportWidget->SetSupport(m_doc->GetEfgSupportList().GetCurrent());

  Layout();
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

void gbtEfgSupportWindow::OnTreeKeypress(wxTreeEvent &p_event)
{
  if (p_event.GetKeyCode() == WXK_SPACE) {
    ToggleAction(m_supportWidget->GetSelection());
  }
}

void gbtEfgSupportWindow::OnTreeItemActivated(wxTreeEvent &p_event)
{
  ToggleAction(p_event.GetItem());
}

void gbtEfgSupportWindow::ToggleAction(wxTreeItemId p_id)
{
  gbtGameAction action = m_supportWidget->GetAction(p_id);
  if (action.IsNull()) {
    return;
  }

  const gbtEfgSupport &support = m_doc->GetEfgSupportList().GetCurrent();
  if (support->Contains(action) &&
      action->GetInfoset()->NumActions() > 1) {
    m_doc->Submit(new gbtCmdRemoveAction(action));
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
				(char *) m_doc->GetGame()->GetLabel()));
    }
    else {
      SetTitle(wxString::Format(_("Gambit - Supports: %s"),
				(char *) m_doc->GetGame()->GetLabel()));
    }
  }
  Show(m_doc->ShowEfgSupports());
}


#include "base/gmap.imp"

static gbtOutput &operator<<(gbtOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gbtBaseMap<wxTreeItemId, gbtGameAction>;
template class gbtOrdMap<wxTreeItemId, gbtGameAction>;
