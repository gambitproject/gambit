//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of dialog to set tree layout parameters
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
#include "wx/spinctrl.h"
#include "dlefglayout.h"

//==========================================================================
//                  class dialogLayout: Implementation
//==========================================================================

class panelNodes : public wxPanel {
private:
  wxRadioBox *m_chanceToken, *m_playerToken, *m_terminalToken;
  wxSpinCtrl *m_nodeSize, *m_terminalSpacing;

public:
  panelNodes(wxWindow *p_parent, const TreeDrawSettings &p_settings);

  int NodeSize(void) const { return m_nodeSize->GetValue(); }
  int TerminalSpacing(void) const { return m_terminalSpacing->GetValue(); }
  
  int ChanceToken(void) const { return m_chanceToken->GetSelection(); }
  int PlayerToken(void) const { return m_playerToken->GetSelection(); }
  int TerminalToken(void) const { return m_terminalToken->GetSelection(); }
};


panelNodes::panelNodes(wxWindow *p_parent, const TreeDrawSettings &p_settings)
  : wxPanel(p_parent, -1)
{
  const int NODE_LENGTH_MIN = 20;
  const int NODE_LENGTH_MAX = 100;

  const int Y_SPACING_MIN = 15;
  const int Y_SPACING_MAX = 60;

  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *tokenSizer = new wxBoxSizer(wxHORIZONTAL);
  wxString tokenChoices[] = { "Line", "Box", "Circle", "Diamond" };
  m_chanceToken = new wxRadioBox(this, -1, "Chance nodes",
				 wxDefaultPosition, wxDefaultSize,
				 4, tokenChoices, 1, wxRA_SPECIFY_COLS);
  m_chanceToken->SetSelection(p_settings.ChanceToken());
  tokenSizer->Add(m_chanceToken, 0, wxALL, 5);

  m_playerToken = new wxRadioBox(this, -1, "Player nodes",
				 wxDefaultPosition, wxDefaultSize,
				 4, tokenChoices, 1, wxRA_SPECIFY_COLS);
  m_playerToken->SetSelection(p_settings.PlayerToken());
  tokenSizer->Add(m_playerToken, 0, wxALL, 5);

  m_terminalToken = new wxRadioBox(this, -1, "Terminal nodes",
				   wxDefaultPosition, wxDefaultSize,
				   4, tokenChoices, 1, wxRA_SPECIFY_COLS);
  m_terminalToken->SetSelection(p_settings.TerminalToken());
  tokenSizer->Add(m_terminalToken, 0, wxALL, 5);
  topSizer->Add(tokenSizer, 0, wxALL, 5);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
		
  gridSizer->Add(new wxStaticText(this, -1, "Size of nodes"),
		 0, wxCENTER | wxALL, 5);
  m_nodeSize = new wxSpinCtrl(this, -1,
			      wxString::Format("%ld", p_settings.NodeSize()),
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS,
			      NODE_LENGTH_MIN, NODE_LENGTH_MAX);
  gridSizer->Add(m_nodeSize, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, -1, "Terminal node spacing"),
		 0, wxCENTER | wxALL, 5);
  m_terminalSpacing =
    new wxSpinCtrl(this, -1,
		   wxString::Format("%ld", p_settings.TerminalSpacing()),
		   wxDefaultPosition, wxDefaultSize,
		   wxSP_ARROW_KEYS, Y_SPACING_MIN, Y_SPACING_MAX);
  gridSizer->Add(m_terminalSpacing, 1, wxEXPAND | wxALL, 5);

  topSizer->Add(gridSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

class panelBranches : public wxPanel {
private:
  wxRadioBox *m_branchStyle, *m_branchLabels;
  wxSpinCtrl *m_branchLength, *m_tineLength;

public:
  panelBranches(wxWindow *p_parent, const TreeDrawSettings &);

  int BranchLength(void) const { return m_branchLength->GetValue(); }
  int TineLength(void) const { return m_tineLength->GetValue(); }

  int BranchStyle(void) const { return m_branchStyle->GetSelection(); }
  int BranchLabels(void) const { return m_branchLabels->GetSelection(); }
};

panelBranches::panelBranches(wxWindow *p_parent,
			     const TreeDrawSettings &p_settings)
  : wxPanel(p_parent, -1)
{
  const int BRANCH_LENGTH_MIN = 0;
  const int BRANCH_LENGTH_MAX = 100;

  const int TINE_LENGTH_MIN = 20;
  const int TINE_LENGTH_MAX = 100;

  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *styleSizer = new wxBoxSizer(wxHORIZONTAL);
  wxString styleChoices[] = { "Straight line", "Fork-Tine" };
  m_branchStyle = new wxRadioBox(this, -1, "Branch style",
				 wxDefaultPosition, wxDefaultSize,
				 2, styleChoices, 1, wxRA_SPECIFY_COLS);
  m_branchStyle->SetSelection(p_settings.BranchStyle());
  styleSizer->Add(m_branchStyle, 0, wxALL, 5);

  wxString labelChoices[] = { "Horizontal", "Rotated" };
  m_branchLabels = new wxRadioBox(this, -1, "Branch labels",
				  wxDefaultPosition, wxDefaultSize,
				  2, labelChoices, 1, wxRA_SPECIFY_COLS);
  m_branchLabels->SetSelection(p_settings.BranchLabels());
  styleSizer->Add(m_branchLabels, 0, wxALL, 5);

  topSizer->Add(styleSizer, 0, wxALL | wxCENTER, 5);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
  gridSizer->Add(new wxStaticText(this, -1, "Branch length"),
		 0, wxCENTER | wxALL, 5);
  m_branchLength = new wxSpinCtrl(this, -1,
				  wxString::Format("%ld",
						   p_settings.BranchLength()),
				  wxDefaultPosition, wxDefaultSize,
				  wxSP_ARROW_KEYS,
				  BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX);
  gridSizer->Add(m_branchLength, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, -1, "Tine length"),
		 0, wxCENTER | wxALL, 5);
  m_tineLength = new wxSpinCtrl(this, -1,
				wxString::Format("%ld",
						 p_settings.TineLength()),
				wxDefaultPosition, wxDefaultSize,
				wxSP_ARROW_KEYS,
				TINE_LENGTH_MIN, TINE_LENGTH_MAX);
  gridSizer->Add(m_tineLength, 0, wxALL, 5);
  topSizer->Add(gridSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

class panelInfosets : public wxPanel {
private:
  wxRadioBox *m_infosetConnect, *m_infosetJoin;

public:
  panelInfosets(wxWindow *p_parent, const TreeDrawSettings &);

  int InfosetConnect(void) const { return m_infosetConnect->GetSelection(); }
  int InfosetJoin(void) const { return m_infosetJoin->GetSelection(); }
};

panelInfosets::panelInfosets(wxWindow *p_parent,
			     const TreeDrawSettings &p_settings)
  : wxPanel(p_parent, -1)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *styleSizer = new wxBoxSizer(wxHORIZONTAL);
  wxString connectChoices[] = { "Never", "Only on same level",
				"Across all levels" };
  m_infosetConnect = new wxRadioBox(this, -1,
				    "Connect information set members",
				    wxDefaultPosition, wxDefaultSize,
				    3, connectChoices, 1, wxRA_SPECIFY_COLS);
  m_infosetConnect->SetSelection(p_settings.InfosetConnect());
  styleSizer->Add(m_infosetConnect, 0, wxALL, 5);

  wxString joinChoices[] = { "Lines", "Circles" };
  m_infosetJoin = new wxRadioBox(this, -1, "Join style",
				 wxDefaultPosition, wxDefaultSize,
				 2, joinChoices, 1, wxRA_SPECIFY_COLS);
  m_infosetJoin->SetSelection(p_settings.InfosetJoin());
  styleSizer->Add(m_infosetJoin, 0, wxALL, 5);

  topSizer->Add(styleSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

class panelSubgames : public wxPanel {
private:
  wxRadioBox *m_subgameStyle;

public:
  panelSubgames(wxWindow *, const TreeDrawSettings &);

  int SubgameStyle(void) const { return m_subgameStyle->GetSelection(); }
};

panelSubgames::panelSubgames(wxWindow *p_parent, 
			     const TreeDrawSettings &p_settings)
  : wxPanel(p_parent, -1)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  wxString styleChoices[] = { "Do not show", "Use arcs" };
  m_subgameStyle = new wxRadioBox(this, -1, "Subgame display",
				  wxDefaultPosition, wxDefaultSize,
				  2, styleChoices, 1, wxRA_SPECIFY_COLS);
  m_subgameStyle->SetSelection(p_settings.SubgameStyle());
  topSizer->Add(m_subgameStyle, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}


dialogLayout::dialogLayout(wxWindow *p_parent, 
			   const TreeDrawSettings &p_settings)
  : wxDialog(p_parent, -1, "Layout options")
{
  SetAutoLayout(true);

  m_notebook = new wxNotebook(this, -1, wxDefaultPosition, wxDefaultSize);
  wxNotebookSizer *notebookSizer = new wxNotebookSizer(m_notebook);
  m_notebook->AddPage(new panelNodes(m_notebook, p_settings), "Nodes");
  m_notebook->AddPage(new panelBranches(m_notebook, p_settings), "Branches");
  m_notebook->AddPage(new panelInfosets(m_notebook, p_settings),
		      "Information sets");
  m_notebook->AddPage(new panelSubgames(m_notebook, p_settings), "Subgames");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(notebookSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

void dialogLayout::GetSettings(TreeDrawSettings &p_settings)
{
  panelNodes *nodes = (panelNodes *) m_notebook->GetPage(0);
  p_settings.SetNodeSize(nodes->NodeSize());
  p_settings.SetTerminalSpacing(nodes->TerminalSpacing());
  p_settings.SetChanceToken(nodes->ChanceToken());
  p_settings.SetPlayerToken(nodes->PlayerToken());
  p_settings.SetTerminalToken(nodes->TerminalToken());

  panelBranches *branches = (panelBranches *) m_notebook->GetPage(1);
  p_settings.SetBranchLength(branches->BranchLength());
  p_settings.SetTineLength(branches->TineLength());
  p_settings.SetBranchStyle(branches->BranchStyle());
  p_settings.SetBranchLabels(branches->BranchLabels());

  panelInfosets *infosets = (panelInfosets *) m_notebook->GetPage(2);
  p_settings.SetInfosetConnect(infosets->InfosetConnect());
  p_settings.SetInfosetJoin(infosets->InfosetJoin());

  panelSubgames *subgames = (panelSubgames *) m_notebook->GetPage(3);
  p_settings.SetSubgameStyle(subgames->SubgameStyle());
}

