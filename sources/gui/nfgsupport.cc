//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of normal form support palette window
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
#include "base/gmap.h"
#include "nfgsupport.h"
#include "id.h"

//==========================================================================
//                     class gbtCmdAddStrategy
//==========================================================================

//
// Add a strategy to the current support
//
class gbtCmdAddStrategy : public gbtGameCommand {
private:
  gbtNfgAction m_strategy;

public:
  gbtCmdAddStrategy(gbtNfgAction p_strategy) : m_strategy(p_strategy) { }
  virtual ~gbtCmdAddStrategy() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdAddStrategy::Do(gbtGameDocument *p_doc)
{
  p_doc->GetNfgSupportList().GetCurrent().AddStrategy(m_strategy);
}

//==========================================================================
//                    class gbtCmdRemoveStrategy
//==========================================================================

//
// Remove a strategy from the current support
//
class gbtCmdRemoveStrategy : public gbtGameCommand {
private:
  gbtNfgAction m_strategy;

public:
  gbtCmdRemoveStrategy(gbtNfgAction p_strategy) : m_strategy(p_strategy) { }
  virtual ~gbtCmdRemoveStrategy() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdRemoveStrategy::Do(gbtGameDocument *p_doc)
{
  p_doc->GetNfgSupportList().GetCurrent().RemoveStrategy(m_strategy);
}

//==========================================================================
//                    class gbtCmdSetNfgSupport
//==========================================================================

//
// Make a support the currently-selected support
//
class gbtCmdSetNfgSupport : public gbtGameCommand {
private:
  int m_index;

public:
  gbtCmdSetNfgSupport(int p_index) : m_index(p_index) { }
  virtual ~gbtCmdSetNfgSupport() { }

  void Do(gbtGameDocument *);

  bool ModifiesGame(void) const { return false; }
  bool ModifiesPayoffs(void) const { return false; }
};

void gbtCmdSetNfgSupport::Do(gbtGameDocument *p_doc)
{
  p_doc->GetNfgSupportList().SetCurrentIndex(m_index);
}


//==========================================================================
//                    class gbtNfgSupportWidget
//==========================================================================

gbtNfgSupportWidget::gbtNfgSupportWidget(wxWindow *p_parent,
					 wxWindowID p_id)
  : wxTreeCtrl(p_parent, p_id), m_map(gbtNfgAction())
{
  Connect(p_id, wxEVT_COMMAND_TREE_ITEM_COLLAPSING,
	  (wxObjectEventFunction) (wxEventFunction)
	  (wxTreeEventFunction) &gbtNfgSupportWidget::OnTreeItemCollapse);
}

void gbtNfgSupportWidget::SetSupport(const gbtNfgSupport &p_support)
{
  DeleteAllItems();
  AddRoot(wxString::Format(wxT("%s"), (char *) p_support.GetLabel()));
  for (int pl = 1; pl <= p_support.GetGame().NumPlayers(); pl++) {
    gbtNfgPlayer player = p_support.GetGame().GetPlayer(pl);

    wxTreeItemId id = AppendItem(GetRootItem(),
				 wxString::Format(wxT("%s"),
						  (char *) player.GetLabel()));
    
    for (int st = 1; st <= player.NumStrategies(); st++) {
      gbtNfgAction strategy = player.GetStrategy(st);

      wxTreeItemId stratID = AppendItem(id, 
					wxString::Format(wxT("%s"),
							 (char *) strategy.GetLabel()));
      if (p_support.Contains(strategy)) {
	SetItemTextColour(stratID, *wxBLACK);
      }
      else {
	SetItemTextColour(stratID, *wxLIGHT_GREY);
      }
      m_map.Define(stratID, strategy);
    }
    Expand(id);
  }
  Expand(GetRootItem());
}

void gbtNfgSupportWidget::OnTreeItemCollapse(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == GetRootItem()) {
    p_event.Veto();
  }
}

//==========================================================================
//                    class gbtNfgSupportWindow
//==========================================================================

const int GBT_NFG_SUPPORT_LIST = 8000;
const int GBT_NFG_SUPPORT_PREV = 8001;
const int GBT_NFG_SUPPORT_NEXT = 8002;
const int GBT_NFG_STRATEGY_TREE = 8003;

class gbtNfgSupportWindow : public wxPanel, public gbtGameView {
private:
  wxChoice *m_supportList;
  wxButton *m_prevButton, *m_nextButton;
  gbtNfgSupportWidget *m_supportWidget;

  void OnSupportList(wxCommandEvent &);
  void OnSupportPrev(wxCommandEvent &);
  void OnSupportNext(wxCommandEvent &);

  void OnTreeKeypress(wxTreeEvent &);
  void OnTreeItemActivated(wxTreeEvent &);

  void ToggleStrategy(wxTreeItemId);

  // Overriding view members
  bool IsEfgView(void) const { return false; }
  bool IsNfgView(void) const { return true; }

public:
  gbtNfgSupportWindow(gbtGameDocument *p_doc, wxWindow *p_parent);
  virtual ~gbtNfgSupportWindow() { }

  void OnUpdate(gbtGameView *);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtNfgSupportWindow, wxPanel)
  EVT_CHOICE(GBT_NFG_SUPPORT_LIST, gbtNfgSupportWindow::OnSupportList)
  EVT_BUTTON(GBT_NFG_SUPPORT_PREV, gbtNfgSupportWindow::OnSupportPrev)
  EVT_BUTTON(GBT_NFG_SUPPORT_NEXT, gbtNfgSupportWindow::OnSupportNext)
  EVT_TREE_KEY_DOWN(GBT_NFG_STRATEGY_TREE,
		    gbtNfgSupportWindow::OnTreeKeypress)
  EVT_TREE_ITEM_ACTIVATED(GBT_NFG_STRATEGY_TREE,
			  gbtNfgSupportWindow::OnTreeItemActivated)
END_EVENT_TABLE()

gbtNfgSupportWindow::gbtNfgSupportWindow(gbtGameDocument *p_doc,
					 wxWindow *p_parent)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    gbtGameView(p_doc)
{
  SetAutoLayout(true);

  m_supportList = new wxChoice(this, GBT_NFG_SUPPORT_LIST,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0);
  m_prevButton = new wxButton(this, GBT_NFG_SUPPORT_PREV, wxT("<-"),
			      wxDefaultPosition, wxSize(30, 30));
  m_nextButton = new wxButton(this, GBT_NFG_SUPPORT_NEXT, wxT("->"),
			      wxDefaultPosition, wxSize(30, 30));
  m_supportWidget = new gbtNfgSupportWidget(this, GBT_NFG_STRATEGY_TREE);

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

void gbtNfgSupportWindow::OnUpdate(gbtGameView *)
{
  m_supportList->Clear();

  const gbtNfgSupportList &supports = m_doc->GetNfgSupportList();

  for (int i = 1; i <= supports.Length(); i++) {
    m_supportList->Append(wxString::Format(wxT("%d: %s"), i,
					   (char *) supports.Get(i).GetLabel()));
  }

  int supportIndex = m_doc->GetNfgSupportList().GetCurrentIndex();
  m_supportList->SetSelection(supportIndex - 1);
  m_prevButton->Enable((supportIndex > 1) ? true : false);
  m_nextButton->Enable((supportIndex < supports.Length()) ? true : false);
  m_supportWidget->SetSupport(m_doc->GetNfgSupportList().GetCurrent());

  Layout();
}

void gbtNfgSupportWindow::OnSupportList(wxCommandEvent &p_event)
{
  m_doc->Submit(new gbtCmdSetNfgSupport(p_event.GetSelection() + 1));
}

void gbtNfgSupportWindow::OnSupportPrev(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdSetNfgSupport(m_supportList->GetSelection()));
}

void gbtNfgSupportWindow::OnSupportNext(wxCommandEvent &)
{
  m_doc->Submit(new gbtCmdSetNfgSupport(m_supportList->GetSelection() + 2));
}

void gbtNfgSupportWindow::OnTreeKeypress(wxTreeEvent &p_event)
{
  if (p_event.GetKeyCode() == WXK_SPACE) {
    ToggleStrategy(m_supportWidget->GetSelection());
  }
}

void gbtNfgSupportWindow::OnTreeItemActivated(wxTreeEvent &p_event)
{
  ToggleStrategy(p_event.GetItem());
}

void gbtNfgSupportWindow::ToggleStrategy(wxTreeItemId p_id)
{
  gbtNfgAction strategy = m_supportWidget->GetStrategy(p_id);
  if (strategy.IsNull()) {
    return;
  }

  const gbtNfgSupport &support = m_doc->GetNfgSupportList().GetCurrent();
  if (support.Contains(strategy) &&
      support.NumStrats(strategy.GetPlayer()) > 1) {
    m_doc->Submit(new gbtCmdRemoveStrategy(strategy));
  }
  else {
    m_doc->Submit(new gbtCmdAddStrategy(strategy));
  }
}

//-------------------------------------------------------------------------
//                    class gbtNfgSupportFrame
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(gbtNfgSupportFrame, wxFrame)
  EVT_MENU(wxID_CLOSE, gbtNfgSupportFrame::Close)
  EVT_CLOSE(gbtNfgSupportFrame::OnClose)
END_EVENT_TABLE()

gbtNfgSupportFrame::gbtNfgSupportFrame(gbtGameDocument *p_doc, 
				       wxWindow *p_parent)
  : wxFrame(p_parent, -1, wxT(""), wxDefaultPosition, wxSize(300, 300)),
    gbtGameView(p_doc)
{
  m_panel = new gbtNfgSupportWindow(p_doc, this);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));

  wxMenu *editMenu = new wxMenu;
  editMenu->Append(GBT_MENU_SUPPORTS_DUPLICATE, _("Duplicate support"),
		   _("Duplicate this support"));
  editMenu->Append(GBT_MENU_SUPPORTS_DELETE, _("Delete support"),
		   _("Delete this support"));

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

gbtNfgSupportFrame::~gbtNfgSupportFrame()
{ }

void gbtNfgSupportFrame::OnClose(wxCloseEvent &p_event)
{
  m_doc->SetShowNfgSupports(false);
  // Frame is now hidden; leave it that way, don't actually close
  p_event.Veto();
}

void gbtNfgSupportFrame::OnUpdate(gbtGameView *p_sender)
{
  if (m_doc->ShowNfgSupports()) {
    m_panel->OnUpdate(p_sender);

    if (m_doc->GetFilename() != wxT("")) {
      SetTitle(wxString::Format(_("Gambit - Supports: [%s] %s"), 
				(const char *) m_doc->GetFilename().mb_str(), 
				(char *) m_doc->GetNfg().GetLabel()));
    }
    else {
      SetTitle(wxString::Format(_("Gambit - Supports: %s"),
				(char *) m_doc->GetNfg().GetLabel()));
    }
  }
  Show(m_doc->ShowNfgSupports());
}

#include "base/gmap.imp"

static gbtOutput &operator<<(gbtOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gbtBaseMap<wxTreeItemId, gbtNfgAction>;
template class gbtOrdMap<wxTreeItemId, gbtNfgAction>;
