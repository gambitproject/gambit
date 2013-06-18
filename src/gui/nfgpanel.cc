//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/nfgpanel.cc
// Implementation of normal form panel
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
#include <wx/colordlg.h>      // for picking player colors

#include "gamedoc.h"
#include "nfgpanel.h"
#include "nfgtable.h"
#include "menuconst.h"
#include "edittext.h"

#include "bitmaps/color.xpm"
#include "bitmaps/newrow.xpm"
#include "bitmaps/person.xpm"

//=========================================================================
//                      class gbtTablePlayerIcon
//=========================================================================

class gbtTablePlayerIcon : public wxStaticBitmap {
private:
  int m_player;

  // Event handlers
  void OnLeftClick(wxMouseEvent &);

public:
  gbtTablePlayerIcon(wxWindow *p_parent, int p_player);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtTablePlayerIcon, wxStaticBitmap)
  EVT_LEFT_DOWN(gbtTablePlayerIcon::OnLeftClick)
END_EVENT_TABLE()

gbtTablePlayerIcon::gbtTablePlayerIcon(wxWindow *p_parent, int p_player)
  : wxStaticBitmap(p_parent, -1, wxBitmap(person_xpm)), m_player(p_player)
{ }

void gbtTablePlayerIcon::OnLeftClick(wxMouseEvent &)
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



class gbtTablePlayerPanel : public wxPanel {
private:
  gbtNfgPanel *m_nfgPanel;
  gbtGameDocument *m_doc;
  int m_player;
  gbtEditableText *m_playerLabel;
  wxStaticText *m_payoff;

  /// @name Event handlers
  //@{
  /// The add strategy icon is clicked
  void OnNewStrategy(wxCommandEvent &);
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
  gbtTablePlayerPanel(wxWindow *, gbtNfgPanel *, 
		      gbtGameDocument *, int p_player);

  void OnUpdate(void);
  void PostPendingChanges(void);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtTablePlayerPanel, wxPanel)
  EVT_CHAR(gbtTablePlayerPanel::OnChar)
END_EVENT_TABLE()

gbtTablePlayerPanel::gbtTablePlayerPanel(wxWindow *p_parent,
					 gbtNfgPanel *p_nfgPanel,
					 gbtGameDocument *p_doc,
					 int p_player)
  : wxPanel(p_parent, -1), 
    m_nfgPanel(p_nfgPanel), m_doc(p_doc), m_player(p_player)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBitmap *playerIcon = new gbtTablePlayerIcon(this, m_player);
  labelSizer->Add(playerIcon, 0, wxALL | wxALIGN_CENTER, 0);

  if (!m_doc->IsTree()) {
    wxBitmapButton *addStrategyIcon = 
      new wxBitmapButton(this, -1, wxBitmap(newrow_xpm),
			 wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    addStrategyIcon->SetToolTip(_("Add a strategy for this player"));

    labelSizer->Add(addStrategyIcon, 0, wxALL | wxALIGN_CENTER, 0);
    Connect(addStrategyIcon->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	    wxCommandEventHandler(gbtTablePlayerPanel::OnNewStrategy));
  }

  wxBitmapButton *setColorIcon =
    new wxBitmapButton(this, -1, wxBitmap(color_xpm),
		       wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
  setColorIcon->SetToolTip(_("Change the color for this player"));

  labelSizer->Add(setColorIcon, 0, wxALL | wxALIGN_CENTER, 0);
  Connect(setColorIcon->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtTablePlayerPanel::OnSetColor));

  m_playerLabel = new gbtEditableText(this, -1, wxT(""),
				      wxDefaultPosition, wxSize(125, -1));
  m_playerLabel->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
  labelSizer->Add(m_playerLabel, 1, wxLEFT | wxEXPAND, 5);
  Connect(m_playerLabel->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtTablePlayerPanel::OnEditPlayerLabel));
  Connect(m_playerLabel->GetId(), wxEVT_COMMAND_TEXT_ENTER,
	  wxCommandEventHandler(gbtTablePlayerPanel::OnAcceptPlayerLabel));

  topSizer->Add(labelSizer, 0, wxALL, 0);

  m_payoff = new wxStaticText(this, wxID_STATIC, wxT("Payoff:"),
			      wxDefaultPosition, wxDefaultSize,
			      wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  m_payoff->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL));
  topSizer->Add(m_payoff, 0, wxALL | wxEXPAND, 0);
  topSizer->Show(m_payoff, false);

  SetSizer(topSizer);
  topSizer->SetSizeHints(this);
  topSizer->Fit(this);
  Layout();
  OnUpdate();
}

void gbtTablePlayerPanel::OnUpdate(void)
{
  wxColour color = m_doc->GetStyle().GetPlayerColor(m_player);

  m_playerLabel->SetForegroundColour(color);
  m_playerLabel->SetValue(wxString(m_doc->GetGame()->GetPlayer(m_player)->GetLabel().c_str(),
				   *wxConvCurrent));

  if (m_doc->GetCurrentProfile() > 0) {
    m_payoff->SetForegroundColour(color);

    std::string pay = m_doc->GetProfiles().GetPayoff(m_player);
    m_payoff->SetLabel(wxT("Payoff: ") + 
		       wxString(pay.c_str(), *wxConvCurrent));
    GetSizer()->Show(m_payoff, true);
  }
  else {
    GetSizer()->Show(m_payoff, false);
  }

  GetSizer()->Layout();
  GetSizer()->SetSizeHints(this);
  GetSizer()->Fit(this);
}

void gbtTablePlayerPanel::OnChar(wxKeyEvent &p_event)
{
  if (p_event.GetKeyCode() == WXK_ESCAPE) {
    m_playerLabel->EndEdit(false);
  }
  else {
    p_event.Skip();
  }
}

void gbtTablePlayerPanel::OnNewStrategy(wxCommandEvent &)
{
  m_doc->PostPendingChanges();
  m_doc->DoNewStrategy(m_doc->GetGame()->GetPlayer(m_player));
}

void gbtTablePlayerPanel::OnSetColor(wxCommandEvent &)
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

void gbtTablePlayerPanel::OnEditPlayerLabel(wxCommandEvent &)
{
  m_doc->PostPendingChanges();
  m_playerLabel->BeginEdit();
}

void gbtTablePlayerPanel::OnAcceptPlayerLabel(wxCommandEvent &)
{
  m_doc->DoSetPlayerLabel(m_doc->GetGame()->GetPlayer(m_player),
			  m_playerLabel->GetValue());
}

void gbtTablePlayerPanel::PostPendingChanges(void)
{
  if (m_playerLabel->IsEditing()) {
    m_playerLabel->EndEdit(true);
    m_doc->DoSetPlayerLabel(m_doc->GetGame()->GetPlayer(m_player),
			    m_playerLabel->GetValue());
  }
}

//=====================================================================
//                  class gbtTablePlayerToolbar
//=====================================================================

class gbtTablePlayerToolbar : public wxPanel, public gbtGameView {
private:
  gbtNfgPanel *m_nfgPanel;
  Gambit::Array<gbtTablePlayerPanel *> m_playerPanels;

public:
  gbtTablePlayerToolbar(gbtNfgPanel *p_parent, gbtGameDocument *p_doc);

  /// @name Implementation of gbtGameView members
  //@{
  void OnUpdate(void);
  void PostPendingChanges(void);
  //@}
};


gbtTablePlayerToolbar::gbtTablePlayerToolbar(gbtNfgPanel *p_parent, 
					     gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxSize(210, -1)), 
    gbtGameView(p_doc),
    m_nfgPanel(p_parent)
{ 
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  for (int pl = 1; pl <= m_doc->NumPlayers(); pl++) {
    m_playerPanels.Append(new gbtTablePlayerPanel(this, p_parent, m_doc, pl));
    topSizer->Add(m_playerPanels[pl], 0, wxALL | wxEXPAND, 5);
  }

  SetSizer(topSizer);
  Layout();
}

void gbtTablePlayerToolbar::OnUpdate(void)
{
  while (m_playerPanels.Length() < m_doc->NumPlayers()) {
    gbtTablePlayerPanel *panel = 
      new gbtTablePlayerPanel(this, m_nfgPanel, m_doc,
			      m_playerPanels.Length()+1);
    m_playerPanels.Append(panel);
    GetSizer()->Add(panel, 0, wxALL | wxEXPAND, 5);
  }

  while (m_playerPanels.Length() > m_doc->NumPlayers()) {
    gbtTablePlayerPanel *panel = m_playerPanels.Remove(m_playerPanels.Length());
    GetSizer()->Detach(panel);
    panel->Destroy();
  }
  
  for (int pl = 1; pl <= m_playerPanels.Length(); pl++) {
    m_playerPanels[pl]->OnUpdate();
  }

  GetSizer()->Layout();
}

void gbtTablePlayerToolbar::PostPendingChanges(void)
{
  for (int pl = 1; pl <= m_playerPanels.Length(); pl++) {
    m_playerPanels[pl]->PostPendingChanges();
  }
}


//=====================================================================
//               class gbtStrategyDominanceToolbar
//=====================================================================

class gbtStrategyDominanceToolbar : public wxPanel, public gbtGameView {
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

public:
  gbtStrategyDominanceToolbar(wxWindow *p_parent, gbtGameDocument *p_doc);
  virtual ~gbtStrategyDominanceToolbar() { }
};


#include "bitmaps/next.xpm"
#include "bitmaps/prev.xpm"
#include "bitmaps/tobegin.xpm"
#include "bitmaps/toend.xpm"

gbtStrategyDominanceToolbar::gbtStrategyDominanceToolbar(wxWindow *p_parent,
							 gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxT("Hide strategies which are ")),
		0, wxALL | wxALIGN_CENTER, 5);

  wxString domChoices[] = { wxT("strictly"), wxT("strictly or weakly") };
  wxChoice *choice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
				  2, domChoices);
  choice->SetSelection(0);
  Connect(choice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	  wxCommandEventHandler(gbtStrategyDominanceToolbar::OnStrength));
  topSizer->Add(choice, 0, wxALL | wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, wxID_STATIC, wxT("dominated:")),
		0, wxALL | wxALIGN_CENTER, 5);

  m_topButton = new wxBitmapButton(this, -1, wxBitmap(tobegin_xpm));
  m_topButton->SetToolTip(_("Show all strategies"));
  Connect(m_topButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtStrategyDominanceToolbar::OnTopLevel));
  topSizer->Add(m_topButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_prevButton = new wxBitmapButton(this, -1, wxBitmap(prev_xpm));
  m_prevButton->SetToolTip(_("Previous round of elimination"));
  Connect(m_prevButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtStrategyDominanceToolbar::OnPreviousLevel));
  topSizer->Add(m_prevButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_level = new wxStaticText(this, wxID_STATIC, 
			     wxT("All strategies shown"),
			     wxDefaultPosition, wxDefaultSize,
			     wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  topSizer->Add(m_level, 0, wxALL | wxALIGN_CENTER, 5);

  m_nextButton = new wxBitmapButton(this, -1, wxBitmap(next_xpm));
  m_nextButton->SetToolTip(_("Next round of elimination"));
  Connect(m_nextButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtStrategyDominanceToolbar::OnNextLevel));
  topSizer->Add(m_nextButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_allButton = new wxBitmapButton(this, -1, wxBitmap(toend_xpm));
  m_allButton->SetToolTip(_("Eliminate iteratively"));
  Connect(m_allButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtStrategyDominanceToolbar::OnLastLevel));
  topSizer->Add(m_allButton, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  Layout();
}

void gbtStrategyDominanceToolbar::OnStrength(wxCommandEvent &p_event)
{
  m_doc->SetStrategyElimStrength(p_event.GetSelection() == 0);
}

void gbtStrategyDominanceToolbar::OnTopLevel(wxCommandEvent &)
{
  m_doc->TopStrategyElimLevel();
}

void gbtStrategyDominanceToolbar::OnPreviousLevel(wxCommandEvent &)
{
  m_doc->PreviousStrategyElimLevel();
}

void gbtStrategyDominanceToolbar::OnNextLevel(wxCommandEvent &)
{
  m_doc->NextStrategyElimLevel();
}

void gbtStrategyDominanceToolbar::OnLastLevel(wxCommandEvent &)
{
  while (m_doc->NextStrategyElimLevel());
}

void gbtStrategyDominanceToolbar::OnUpdate(void)
{
  m_topButton->Enable(m_doc->GetStrategyElimLevel() > 1);
  m_prevButton->Enable(m_doc->GetStrategyElimLevel() > 1);
  m_nextButton->Enable(m_doc->CanStrategyElim());
  m_allButton->Enable(m_doc->CanStrategyElim());
  if (m_doc->GetStrategyElimLevel() == 1) {
    m_level->SetLabel(wxT("All strategies shown"));
  }
  else if (m_doc->GetStrategyElimLevel() == 2) {
    m_level->SetLabel(wxT("Eliminated 1 level")); 
  }
  else {
    m_level->SetLabel(wxString::Format(wxT("Eliminated %d levels"), 
				       m_doc->GetStrategyElimLevel()-1));
  }
  GetSizer()->Layout();
}

//======================================================================
//                   class gbtNfgPanel: Member functions
//======================================================================

BEGIN_EVENT_TABLE(gbtNfgPanel, wxPanel)
  EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, gbtNfgPanel::OnToolsDominance)
END_EVENT_TABLE()

gbtNfgPanel::gbtNfgPanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  m_dominanceToolbar = new gbtStrategyDominanceToolbar(this, m_doc);
  m_tableWidget = new gbtTableWidget(this, -1, m_doc);
  m_playerToolbar = new gbtTablePlayerToolbar(this, m_doc);

  wxBoxSizer *playerSizer = new wxBoxSizer(wxHORIZONTAL);
  playerSizer->Add(m_playerToolbar, 0, wxEXPAND, 0);
  playerSizer->Add(m_tableWidget, 1, wxEXPAND, 0);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_dominanceToolbar, 0, wxEXPAND, 0);
  topSizer->Show(m_dominanceToolbar, false);
  topSizer->Add(playerSizer, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  Layout();
  OnUpdate();
}

void gbtNfgPanel::OnToolsDominance(wxCommandEvent &p_event)
{
  GetSizer()->Show(m_dominanceToolbar, p_event.IsChecked(), true);
  GetSizer()->Layout();
  
  // Redraw the table with/without dominance markings
  m_tableWidget->OnUpdate();

  // This call is necessary on MSW to clear out a ghost of the
  // dominance toolbar's strength control, under certain circumstances.
  // (I think it is because of the empty upper-left panel in the
  // table widget, but I'm not sure.)
  Refresh();
}

void gbtNfgPanel::OnUpdate(void)
{ 
  m_playerToolbar->OnUpdate();
  m_tableWidget->OnUpdate();
  GetSizer()->Layout();
}

void gbtNfgPanel::PostPendingChanges(void)
{
  m_tableWidget->PostPendingChanges();
}

wxPrintout *gbtNfgPanel::GetPrintout(void)
{
  return m_tableWidget->GetPrintout();
}

bool gbtNfgPanel::GetBitmap(wxBitmap &p_bitmap, int p_marginX, int p_marginY)
{
  return m_tableWidget->GetBitmap(p_bitmap, p_marginX, p_marginY);
}

void gbtNfgPanel::GetSVG(const wxString &p_filename,
			 int p_marginX, int p_marginY)
{
  m_tableWidget->GetSVG(p_filename, p_marginX, p_marginY);
}

void gbtNfgPanel::RenderGame(wxDC &p_dc, int p_marginX, int p_marginY)
{
  m_tableWidget->RenderGame(p_dc, p_marginX, p_marginY);
}
