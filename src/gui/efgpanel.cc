//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/efgpanel.cc
// Main viewing panel for extensive forms
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
#include <wx/dnd.h>           // for drag-and-drop features
#include <wx/image.h>         // for creating drag-and-drop cursor
#include <wx/print.h>         // for printing support
#include <wx/colordlg.h>      // for picking player colors
#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dcsvg.h>         // for SVG output
#else
#include "dcsvg.h"
#endif  /* wxCHECK_VERSION */

#include "efgpanel.h"
#include "efgdisplay.h"  // FIXME: communicate with tree window via events.
#include "menuconst.h"
#include "edittext.h"

//=====================================================================
//                class gbtBehavDominanceToolbar
//=====================================================================

//!
//! This panel serves as a toolbar for interactively viewing
//! dominance information on extensive forms.
//!
class gbtBehavDominanceToolbar : public wxPanel, public gbtGameView {
private:
  wxButton *m_topButton, *m_prevButton, *m_nextButton, *m_allButton;
  wxStaticText *m_level;

  // Overriding gbtGameView members
  void OnUpdate(void);

  // Event handlers
  void OnStrength(wxCommandEvent &);
  void OnTopLevel(wxCommandEvent &);
  void OnPreviousLevel(wxCommandEvent &);
  void OnNextLevel(wxCommandEvent &);
  void OnLastLevel(wxCommandEvent &);
  void OnShowReachable(wxCommandEvent &);

public:
  gbtBehavDominanceToolbar(wxWindow *p_parent, gbtGameDocument *p_doc);
  virtual ~gbtBehavDominanceToolbar() { }
};

#include "bitmaps/next.xpm"
#include "bitmaps/prev.xpm"
#include "bitmaps/tobegin.xpm"
#include "bitmaps/toend.xpm"

gbtBehavDominanceToolbar::gbtBehavDominanceToolbar(wxWindow *p_parent,
						   gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxT("Hide actions which are ")),
		0, wxALL | wxALIGN_CENTER, 5);

  wxString domChoices[] = { wxT("strictly"), wxT("strictly or weakly") };
  wxChoice *choice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
				  2, domChoices);
  choice->SetSelection(0);
  Connect(choice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnStrength));
  topSizer->Add(choice, 0, wxALL | wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, wxID_STATIC, wxT("dominated:")),
		0, wxALL | wxALIGN_CENTER, 5);

  m_topButton = new wxBitmapButton(this, -1, wxBitmap(tobegin_xpm));
  m_topButton->SetToolTip(_("Show all strategies"));
  Connect(m_topButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnTopLevel));
  topSizer->Add(m_topButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_prevButton = new wxBitmapButton(this, -1, wxBitmap(prev_xpm));
  m_prevButton->SetToolTip(_("Previous round of elimination"));
  Connect(m_prevButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnPreviousLevel));
  topSizer->Add(m_prevButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_level = new wxStaticText(this, wxID_STATIC, 
			     wxT("All actions shown"),
			     wxDefaultPosition, wxDefaultSize,
			     wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  topSizer->Add(m_level, 0, wxALL | wxALIGN_CENTER, 5);

  m_nextButton = new wxBitmapButton(this, -1, wxBitmap(next_xpm));
  m_nextButton->SetToolTip(_("Next round of elimination"));
  Connect(m_nextButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnNextLevel));
  topSizer->Add(m_nextButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_allButton = new wxBitmapButton(this, -1, wxBitmap(toend_xpm));
  m_allButton->SetToolTip(_("Eliminate iteratively"));
  Connect(m_allButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnLastLevel));
  topSizer->Add(m_allButton, 0, wxALL | wxALIGN_CENTER, 5);

  wxCheckBox *showReachable = new wxCheckBox(this, -1,
					     wxT("Show only reachable nodes"));
  showReachable->SetValue(m_doc->GetStyle().RootReachable());
  Connect(showReachable->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnShowReachable));
  topSizer->Add(showReachable, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  Layout();
}

void gbtBehavDominanceToolbar::OnStrength(wxCommandEvent &p_event)
{
  m_doc->SetBehavElimStrength(p_event.GetSelection() == 0);
}

void gbtBehavDominanceToolbar::OnTopLevel(wxCommandEvent &)
{
  m_doc->TopBehavElimLevel();
}

void gbtBehavDominanceToolbar::OnPreviousLevel(wxCommandEvent &)
{
  m_doc->PreviousBehavElimLevel();
}

void gbtBehavDominanceToolbar::OnNextLevel(wxCommandEvent &)
{
  m_doc->NextBehavElimLevel();
}

void gbtBehavDominanceToolbar::OnLastLevel(wxCommandEvent &)
{
  while (m_doc->NextBehavElimLevel());
}

void gbtBehavDominanceToolbar::OnShowReachable(wxCommandEvent &)
{
  gbtStyle style = m_doc->GetStyle();
  style.SetRootReachable(!style.RootReachable());
  m_doc->SetStyle(style);
}

void gbtBehavDominanceToolbar::OnUpdate(void)
{
  m_topButton->Enable(m_doc->GetBehavElimLevel() > 1);
  m_prevButton->Enable(m_doc->GetBehavElimLevel() > 1);
  m_nextButton->Enable(m_doc->CanBehavElim());
  m_allButton->Enable(m_doc->CanBehavElim());
  if (m_doc->GetBehavElimLevel() == 1) {
    m_level->SetLabel(wxT("All actions shown"));
  }
  else if (m_doc->GetBehavElimLevel() == 2) {
    m_level->SetLabel(wxT("Eliminated 1 level")); 
  }
  else {
    m_level->SetLabel(wxString::Format(wxT("Eliminated %d levels"), 
				       m_doc->GetBehavElimLevel()-1));
  }
  GetSizer()->Layout();
}

#include "bitmaps/color.xpm"
#include "bitmaps/person.xpm"

class gbtTreePlayerIcon : public wxStaticBitmap {
private:
  int m_player;

  // Event handlers
  void OnLeftClick(wxMouseEvent &);

public:
  gbtTreePlayerIcon(wxWindow *p_parent, int p_player);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtTreePlayerIcon, wxStaticBitmap)
  EVT_LEFT_DOWN(gbtTreePlayerIcon::OnLeftClick)
END_EVENT_TABLE()

gbtTreePlayerIcon::gbtTreePlayerIcon(wxWindow *p_parent, int p_player)
  : wxStaticBitmap(p_parent, -1, wxBitmap(person_xpm)), m_player(p_player)
{ }

void gbtTreePlayerIcon::OnLeftClick(wxMouseEvent &)
{
  wxBitmap bitmap(person_xpm);

#if defined( __WXMSW__) or defined(__WXMAC__)
  wxImage image = bitmap.ConvertToImage();
#else
  wxIcon image;
  image.CopyFromBitmap(bitmap);
#endif // _WXMSW__
      
  wxTextDataObject textData(wxString::Format(wxT("P%d"), m_player));
  wxDropSource source(textData, this, image, image, image);
  source.DoDragDrop(wxDrag_DefaultMove);
}

class gbtTreePlayerPanel : public wxPanel {
private:
  gbtGameDocument *m_doc;
  int m_player;
  gbtEditableText *m_playerLabel;
  wxStaticText *m_payoff, *m_nodeValue, *m_nodeProb;
  wxStaticText *m_infosetValue, *m_infosetProb, *m_belief;
  
  /// @name Event handlers
  //@{
  /// The set color icon is clicked
  void OnSetColor(wxCommandEvent &);
  /// Start the editing of the player label
  void OnEditPlayerLabel(wxCommandEvent &);
  /// End the editing of the player label after enter is pressed
  void OnAcceptPlayerLabel(wxCommandEvent &);
  /// Process a keypress event
  void OnChar(wxKeyEvent &);
  //@}

public:
  gbtTreePlayerPanel(wxWindow *, gbtGameDocument *, int p_player);

  void OnUpdate(void);
  void PostPendingChanges(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtTreePlayerPanel, wxPanel)
  EVT_CHAR(gbtTreePlayerPanel::OnChar)
END_EVENT_TABLE()

gbtTreePlayerPanel::gbtTreePlayerPanel(wxWindow *p_parent,
				       gbtGameDocument *p_doc,
				       int p_player)
  : wxPanel(p_parent, -1), m_doc(p_doc), m_player(p_player)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBitmap *playerIcon = new gbtTreePlayerIcon(this, m_player);
  labelSizer->Add(playerIcon, 0, wxALL | wxALIGN_CENTER, 0);

  wxBitmapButton *setColorIcon =
    new wxBitmapButton(this, -1, wxBitmap(color_xpm),
		       wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
  setColorIcon->SetToolTip(_("Change the color for this player"));

  labelSizer->Add(setColorIcon, 0, wxALL | wxALIGN_CENTER, 0);
  Connect(setColorIcon->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtTreePlayerPanel::OnSetColor));

  m_playerLabel = new gbtEditableText(this, -1, wxT(""),
				      wxDefaultPosition, wxSize(125, -1));
  m_playerLabel->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  labelSizer->Add(m_playerLabel, 1, wxLEFT | wxEXPAND, 10);
  Connect(m_playerLabel->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtTreePlayerPanel::OnEditPlayerLabel));
  Connect(m_playerLabel->GetId(), wxEVT_COMMAND_TEXT_ENTER,
	  wxCommandEventHandler(gbtTreePlayerPanel::OnAcceptPlayerLabel));

  topSizer->Add(labelSizer, 0, wxALL, 0);

  m_payoff = new wxStaticText(this, wxID_STATIC, wxT("Payoff:"),
			      wxDefaultPosition, wxDefaultSize,
			      wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  m_payoff->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  topSizer->Add(m_payoff, 0, wxALL | wxEXPAND, 0);
  topSizer->Show(m_payoff, false);

  m_nodeValue = new wxStaticText(this, wxID_STATIC, wxT("Node value:"),
				 wxDefaultPosition, wxDefaultSize,
				 wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  m_nodeValue->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  topSizer->Add(m_nodeValue, 0, wxALL | wxEXPAND, 0);
  topSizer->Show(m_nodeValue, false);

  m_nodeProb = new wxStaticText(this, wxID_STATIC, wxT("Node reached:"),
				wxDefaultPosition, wxDefaultSize,
				wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  m_nodeProb->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  topSizer->Add(m_nodeProb, 0, wxALL | wxEXPAND, 0);
  topSizer->Show(m_nodeProb, false);

  m_infosetValue = new wxStaticText(this, wxID_STATIC, wxT("Infoset value:"),
				    wxDefaultPosition, wxDefaultSize,
				    wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  m_infosetValue->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  topSizer->Add(m_infosetValue, 0, wxALL | wxEXPAND, 0);
  topSizer->Show(m_infosetValue, false);

  m_infosetProb = new wxStaticText(this, wxID_STATIC, wxT("Infoset reached:"),
				    wxDefaultPosition, wxDefaultSize,
				    wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  m_infosetProb->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  topSizer->Add(m_infosetProb, 0, wxALL | wxEXPAND, 0);
  topSizer->Show(m_infosetProb, false);

  m_belief = new wxStaticText(this, wxID_STATIC, wxT("Belief:"),
			      wxDefaultPosition, wxDefaultSize,
			      wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  m_belief->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  topSizer->Add(m_belief, 0, wxALL | wxEXPAND, 0);
  topSizer->Show(m_belief, false);

  SetSizer(topSizer);
  topSizer->SetSizeHints(this);
  topSizer->Fit(this);
  Layout();
  OnUpdate();
}

void gbtTreePlayerPanel::OnUpdate(void)
{
  if (!m_doc->IsTree())  return;

  wxColour color = m_doc->GetStyle().GetPlayerColor(m_player);

  m_playerLabel->SetForegroundColour(color);
  m_playerLabel->SetValue(wxString(m_doc->GetGame()->GetPlayer(m_player)->GetLabel().c_str(),
				   *wxConvCurrent));

  m_payoff->SetForegroundColour(color);
  if (m_doc->GetCurrentProfile() > 0) {
    std::string pay = m_doc->GetProfiles().GetPayoff(m_player);
    m_payoff->SetLabel(wxT("Payoff: ") + 
		       wxString(pay.c_str(), *wxConvCurrent));
    GetSizer()->Show(m_payoff, true);

    Gambit::GameNode node = m_doc->GetSelectNode();

    if (node) {
      m_nodeValue->SetForegroundColour(color);
      std::string value = m_doc->GetProfiles().GetNodeValue(node, m_player);
      m_nodeValue->SetLabel(wxT("Node value: ") +
			    wxString(value.c_str(), *wxConvCurrent));
      GetSizer()->Show(m_nodeValue, true);

      if (node->GetInfoset() && node->GetPlayer()->GetNumber() == m_player) {
	m_nodeProb->SetForegroundColour(color);
	std::string value = m_doc->GetProfiles().GetRealizProb(node);
	m_nodeProb->SetLabel(wxT("Node reached: ") +
			     wxString(value.c_str(), *wxConvCurrent));
	GetSizer()->Show(m_nodeProb, true);

	m_infosetValue->SetForegroundColour(color);
	value = m_doc->GetProfiles().GetInfosetValue(node);
	m_infosetValue->SetLabel(wxT("Infoset value: ") +
				 wxString(value.c_str(), *wxConvCurrent));
	GetSizer()->Show(m_infosetValue, true);

	m_infosetProb->SetForegroundColour(color);
	value = m_doc->GetProfiles().GetInfosetProb(node);
	m_infosetProb->SetLabel(wxT("Infoset reached: ") +
				wxString(value.c_str(), *wxConvCurrent));
	GetSizer()->Show(m_infosetProb, true);
	
	m_belief->SetForegroundColour(color);
	value = m_doc->GetProfiles().GetBeliefProb(node);
	m_belief->SetLabel(wxT("Belief: ") +
			   wxString(value.c_str(), *wxConvCurrent));
	GetSizer()->Show(m_belief, true);
      }
      else {
	GetSizer()->Show(m_nodeProb, false);
	GetSizer()->Show(m_infosetValue, false);
	GetSizer()->Show(m_infosetProb, false);
	GetSizer()->Show(m_belief, false);
      }
    }
    else {
      GetSizer()->Show(m_nodeValue, false);
    }
  }
  else {
    GetSizer()->Show(m_payoff, false);
    GetSizer()->Show(m_nodeValue, false);
    GetSizer()->Show(m_nodeProb, false);
    GetSizer()->Show(m_infosetValue, false);
    GetSizer()->Show(m_infosetProb, false);
    GetSizer()->Show(m_belief, false);
  }

  GetSizer()->Layout();
  GetSizer()->SetSizeHints(this);
  GetSizer()->Fit(this);
}

void gbtTreePlayerPanel::OnChar(wxKeyEvent &p_event)
{
  if (p_event.GetKeyCode() == WXK_ESCAPE) {
    m_playerLabel->EndEdit(false);
  }
  else {
    p_event.Skip();
  }
}

void gbtTreePlayerPanel::OnSetColor(wxCommandEvent &)
{
  wxColourData data;
  data.SetColour(m_doc->GetStyle().GetPlayerColor(m_player));
  wxColourDialog dialog(this, &data);
  dialog.SetTitle(wxString::Format(_("Choose color for player %d"),
				   m_player));

  if (dialog.ShowModal() == wxID_OK) {
    wxColour color = dialog.GetColourData().GetColour();
    gbtStyle style = m_doc->GetStyle();
    style.SetPlayerColor(m_player, color);
    m_doc->SetStyle(style);
  }
}

void gbtTreePlayerPanel::OnEditPlayerLabel(wxCommandEvent &)
{
  m_doc->PostPendingChanges();
  m_playerLabel->BeginEdit();
}

void gbtTreePlayerPanel::OnAcceptPlayerLabel(wxCommandEvent &)
{
  m_doc->DoSetPlayerLabel(m_doc->GetGame()->GetPlayer(m_player),
			  m_playerLabel->GetValue());
}

void gbtTreePlayerPanel::PostPendingChanges(void)
{
  if (m_playerLabel->IsEditing()) {
    m_playerLabel->EndEdit(true);
    m_doc->DoSetPlayerLabel(m_doc->GetGame()->GetPlayer(m_player), 
			    m_playerLabel->GetValue());
  }
}

#include "bitmaps/dice.xpm"

class gbtTreeChanceIcon : public wxStaticBitmap {
private:
  // Event handlers
  void OnLeftClick(wxMouseEvent &);

public:
  gbtTreeChanceIcon(wxWindow *p_parent);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtTreeChanceIcon, wxStaticBitmap)
  EVT_LEFT_DOWN(gbtTreeChanceIcon::OnLeftClick)
END_EVENT_TABLE()

gbtTreeChanceIcon::gbtTreeChanceIcon(wxWindow *p_parent)
  : wxStaticBitmap(p_parent, -1, wxBitmap(dice_xpm))
{ }

void gbtTreeChanceIcon::OnLeftClick(wxMouseEvent &)
{
  wxBitmap bitmap(dice_xpm);

#if defined( __WXMSW__) or defined(__WXMAC__)
  wxImage image = bitmap.ConvertToImage();
#else
  wxIcon image;
  image.CopyFromBitmap(bitmap);
#endif // _WXMSW__
      
  wxTextDataObject textData(wxT("P0"));
  wxDropSource source(textData, this, image, image, image);
  source.DoDragDrop(wxDrag_DefaultMove);
}

class gbtTreeChancePanel : public wxPanel, public gbtGameView {
private:
  wxStaticText *m_playerLabel;

  // Implementation of gbtGameView members
  void OnUpdate(void);

  /// @name Event handlers
  //@{
  /// The set color icon is clicked
  void OnSetColor(wxCommandEvent &);
  //@}

public:
  gbtTreeChancePanel(wxWindow *, gbtGameDocument *);
};

gbtTreeChancePanel::gbtTreeChancePanel(wxWindow *p_parent,
				       gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBitmap *playerIcon = new gbtTreeChanceIcon(this);
  labelSizer->Add(playerIcon, 0, wxALL | wxALIGN_CENTER, 0);

  wxBitmapButton *setColorIcon =
    new wxBitmapButton(this, -1, wxBitmap(color_xpm),
		       wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
  setColorIcon->SetToolTip(_("Change the color for this player"));

  labelSizer->Add(setColorIcon, 0, wxALL | wxALIGN_CENTER, 0);
  Connect(setColorIcon->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtTreeChancePanel::OnSetColor));

  m_playerLabel = new wxStaticText(this, wxID_STATIC, wxT("Chance"),
				   wxDefaultPosition, wxSize(125, -1),
				   wxALIGN_LEFT);
  m_playerLabel->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  labelSizer->Add(m_playerLabel, 1, wxLEFT | wxALIGN_CENTER, 10);

  topSizer->Add(labelSizer, 0, wxALL, 0);

  SetSizer(topSizer);
  topSizer->SetSizeHints(this);
  topSizer->Fit(this);
  Layout();
}

void gbtTreeChancePanel::OnUpdate(void)
{
  if (!m_doc->GetGame())  return;

  m_playerLabel->SetForegroundColour(m_doc->GetStyle().ChanceColor());
  GetSizer()->Layout();
}

void gbtTreeChancePanel::OnSetColor(wxCommandEvent &)
{
  wxColourData data;
  data.SetColour(m_doc->GetStyle().ChanceColor());
  wxColourDialog dialog(this, &data);
  dialog.SetTitle(wxT("Choose color for chance player"));

  if (dialog.ShowModal() == wxID_OK) {
    wxColour color = dialog.GetColourData().GetColour();
    gbtStyle style = m_doc->GetStyle();
    style.SetChanceColor(color);
    m_doc->SetStyle(style);
  }
}

//=====================================================================
//                  class gbtTreePlayerToolbar
//=====================================================================

class gbtTreePlayerToolbar : public wxPanel, public gbtGameView {
private:
  gbtTreeChancePanel *m_chancePanel;
  Gambit::Array<gbtTreePlayerPanel *> m_playerPanels;

  // @name Implementation of gbtGameView members
  //@{
  void OnUpdate(void);
  void PostPendingChanges(void);
  //@}

public:
  gbtTreePlayerToolbar(wxWindow *p_parent, gbtGameDocument *p_doc);
};


gbtTreePlayerToolbar::gbtTreePlayerToolbar(wxWindow *p_parent, 
					   gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxSize(210, -1)), 
    gbtGameView(p_doc)
{ 
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_chancePanel = new gbtTreeChancePanel(this, m_doc);
  topSizer->Add(m_chancePanel, 0, wxALL | wxEXPAND, 5);

  for (int pl = 1; pl <= m_doc->NumPlayers(); pl++) {
    m_playerPanels.Append(new gbtTreePlayerPanel(this, m_doc, pl));
    topSizer->Add(m_playerPanels[pl], 0, wxALL | wxEXPAND, 5);
  }

  SetSizer(topSizer);
  Layout();
}

void gbtTreePlayerToolbar::OnUpdate(void)
{
  while (m_playerPanels.Length() < m_doc->NumPlayers()) {
    gbtTreePlayerPanel *panel = new gbtTreePlayerPanel(this, m_doc,
						       m_playerPanels.Length()+1);
    m_playerPanels.Append(panel);
    GetSizer()->Add(panel, 0, wxALL | wxEXPAND, 5);
  }

  while (m_playerPanels.Length() > m_doc->NumPlayers()) {
    gbtTreePlayerPanel *panel = m_playerPanels.Remove(m_playerPanels.Length());
    GetSizer()->Detach(panel);
    panel->Destroy();
  }
  
  for (int pl = 1; pl <= m_playerPanels.Length(); pl++) {
    m_playerPanels[pl]->OnUpdate();
  }

  GetSizer()->Layout();
}

void gbtTreePlayerToolbar::PostPendingChanges(void)
{
  for (int pl = 1; pl <= m_playerPanels.Length(); pl++) {
    m_playerPanels[pl]->PostPendingChanges();
  }
}

//=====================================================================
//              Implementation of class gbtEfgPanel
//=====================================================================

BEGIN_EVENT_TABLE(gbtEfgPanel, wxPanel)
  EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, gbtEfgPanel::OnToolsDominance)
  EVT_MENU(GBT_MENU_VIEW_ZOOMIN, gbtEfgPanel::OnViewZoomIn)
  EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, gbtEfgPanel::OnViewZoomOut)
  EVT_MENU(GBT_MENU_VIEW_ZOOM100, gbtEfgPanel::OnViewZoom100)
  EVT_MENU(GBT_MENU_VIEW_ZOOMFIT, gbtEfgPanel::OnViewZoomFit)
END_EVENT_TABLE()

gbtEfgPanel::gbtEfgPanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  m_treeWindow = new gbtEfgDisplay(this, m_doc);
  m_playerToolbar = new gbtTreePlayerToolbar(this, m_doc);
  m_dominanceToolbar = new gbtBehavDominanceToolbar(this, m_doc);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_dominanceToolbar, 0, wxEXPAND, 0);
  topSizer->Show(m_dominanceToolbar, false);

  wxBoxSizer *treeSizer = new wxBoxSizer(wxHORIZONTAL);
  treeSizer->Add(m_playerToolbar, 0, wxEXPAND, 0);
  treeSizer->Add(m_treeWindow, 1, wxEXPAND, 0);
  
  topSizer->Add(treeSizer, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  Layout();
}

void gbtEfgPanel::OnToolsDominance(wxCommandEvent &p_event)
{
  GetSizer()->Show(m_dominanceToolbar, p_event.IsChecked(), true);
  GetSizer()->Layout();
}

void gbtEfgPanel::OnViewZoomIn(wxCommandEvent &)
{
  int zoom = m_treeWindow->GetZoom();
  if (zoom < 150)  zoom += 10;
  m_treeWindow->SetZoom(zoom);
}

void gbtEfgPanel::OnViewZoomOut(wxCommandEvent &)
{
  int zoom = m_treeWindow->GetZoom();
  if (zoom > 10)  zoom -= 10;
  m_treeWindow->SetZoom(zoom);
}

void gbtEfgPanel::OnViewZoom100(wxCommandEvent &)
{
  m_treeWindow->SetZoom(100);
}

void gbtEfgPanel::OnViewZoomFit(wxCommandEvent &)
{
  m_treeWindow->FitZoom();
}

class gbtEfgPrintout : public wxPrintout {
private:
  gbtEfgPanel *m_efgPanel;
    
public:
  gbtEfgPrintout(gbtEfgPanel *p_efgPanel, const wxString &p_label)
    : wxPrintout(p_label), m_efgPanel(p_efgPanel) { }
  virtual ~gbtEfgPrintout() { }

  bool OnPrintPage(int)
  { m_efgPanel->RenderGame(*GetDC(), 50, 50);  return true; }
  bool HasPage(int page) { return (page <= 1); }
  void GetPageInfo(int *minPage, int *maxPage,
		   int *selPageFrom, int *selPageTo)
  { *minPage = 1; *maxPage = 1; *selPageFrom = 1; *selPageTo = 1; }
};

wxPrintout *gbtEfgPanel::GetPrintout(void)
{
  return new gbtEfgPrintout(this,
			    wxString(m_doc->GetGame()->GetTitle().c_str(),
				     *wxConvCurrent));
}

bool gbtEfgPanel::GetBitmap(wxBitmap &p_bitmap, int p_marginX, int p_marginY)
{
  if (m_treeWindow->GetLayout().MaxX() > 65000 ||
      m_treeWindow->GetLayout().MaxY() > 65000) {
    // This is just too huge to export to graphics
    return false;
  }

  wxMemoryDC dc;
  p_bitmap = wxBitmap(m_treeWindow->GetLayout().MaxX() + 2 * p_marginX,
		      m_treeWindow->GetLayout().MaxY() + 2 * p_marginY);
  dc.SelectObject(p_bitmap);
  RenderGame(dc, p_marginX, p_marginY);
  return true;
}

void gbtEfgPanel::GetSVG(const wxString &p_filename,
			 int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  int maxX = m_treeWindow->GetLayout().MaxX();
  int maxY = m_treeWindow->GetLayout().MaxY();

  wxSVGFileDC dc(p_filename, maxX + 2*p_marginX, maxY + 2*p_marginY);
  // For some reason, this needs to be initialized
  dc.SetLogicalScale(1.0, 1.0);
  RenderGame(dc, p_marginX, p_marginY);
}

void gbtEfgPanel::RenderGame(wxDC &p_dc, int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  int maxX = m_treeWindow->GetLayout().MaxX();
  int maxY = m_treeWindow->GetLayout().MaxY();

  // Get the size of the DC in pixels
  wxCoord w, h;
  p_dc.GetSize(&w, &h);

  // Calculate a scaling factor
  double scaleX = (double) w / (double) (maxX + 2*p_marginX);
  double scaleY = (double) h / (double) (maxY + 2*p_marginY);
  double scale = (scaleX < scaleY) ? scaleX : scaleY;
  // Never zoom in
  if (scale > 1.0)  scale = 1.0;
  p_dc.SetUserScale(scale, scale);

  // Calculate the position on the DC to center the tree
  double posX = (double) ((w - (maxX * scale)) / 2.0);
  double posY = (double) ((h - (maxY * scale)) / 2.0);
  p_dc.SetDeviceOrigin((int) posX, (int) posY);

  printf("Drawing with scale %f\n", scale);

  // Draw!
  m_treeWindow->OnDraw(p_dc, scale);
}
