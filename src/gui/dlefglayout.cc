//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlefglayout.cc
// Declaration of dialog to set tree layout parameters
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
#include <wx/spinctrl.h>
#include "dlefglayout.h"

//==========================================================================
//                  class gbtLayoutDialog: Implementation
//==========================================================================

class gbtLayoutNodesPanel : public wxPanel {
private:
  wxChoice *m_chanceToken, *m_playerToken, *m_terminalToken;
  wxSpinCtrl *m_nodeSize, *m_terminalSpacing;

public:
  gbtLayoutNodesPanel(wxWindow *p_parent, const gbtStyle &p_settings);

  int NodeSize(void) const { return m_nodeSize->GetValue(); }
  int TerminalSpacing(void) const { return m_terminalSpacing->GetValue(); }
  
  int ChanceToken(void) const { return m_chanceToken->GetSelection(); }
  int PlayerToken(void) const { return m_playerToken->GetSelection(); }
  int TerminalToken(void) const { return m_terminalToken->GetSelection(); }
};


gbtLayoutNodesPanel::gbtLayoutNodesPanel(wxWindow *p_parent, 
					 const gbtStyle &p_settings)
  : wxPanel(p_parent, -1)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *nodeSizer = new wxStaticBoxSizer(wxVERTICAL, this,
						     _T("Drawing nodes"));

  wxFlexGridSizer *tokenSizer = new wxFlexGridSizer(2);

  wxString tokenChoices[] = { _("a line"), 
			      _("a box"), 
			      _("an unfilled circle"), 
			      _("a diamond"), 
			      _("a filled circle") };

  tokenSizer->Add(new wxStaticText(this, wxID_STATIC, 
				    _("Indicate chance nodes with")),
		  0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_chanceToken = new wxChoice(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       5, tokenChoices);
  m_chanceToken->SetSelection(p_settings.ChanceToken());
  tokenSizer->Add(m_chanceToken, 1, wxALL | wxEXPAND | wxALIGN_CENTER, 5);

  tokenSizer->Add(new wxStaticText(this, wxID_STATIC,
				   _("Indicate player nodes with")),
		  0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_playerToken = new wxChoice(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       5, tokenChoices);
  m_playerToken->SetSelection(p_settings.PlayerToken());
  tokenSizer->Add(m_playerToken, 1, wxALL | wxEXPAND | wxALIGN_CENTER, 5);

  tokenSizer->Add(new wxStaticText(this, wxID_STATIC,
				   _("Indicate terminal nodes with")),
		     0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_terminalToken = new wxChoice(this, -1,
				 wxDefaultPosition, wxDefaultSize,
				 5, tokenChoices);
  m_terminalToken->SetSelection(p_settings.TerminalToken());
  tokenSizer->Add(m_terminalToken, 1, wxALL | wxEXPAND | wxALIGN_CENTER, 5);

  nodeSizer->Add(tokenSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(nodeSizer, 0, wxALL | wxALIGN_CENTER, 5);


  wxStaticBoxSizer *sizeSizer = new wxStaticBoxSizer(wxVERTICAL, this,
						     _("Layout sizing"));

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
  gridSizer->AddGrowableCol(1);
		
  gridSizer->Add(new wxStaticText(this, -1, _("Horizontal size of nodes")),
		 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  const int NODE_LENGTH_MIN = 5;
  const int NODE_LENGTH_MAX = 100;

  m_nodeSize = new wxSpinCtrl(this, -1,
			      wxString::Format(_T("%d"), 
					       p_settings.NodeSize()),
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS,
			      NODE_LENGTH_MIN, NODE_LENGTH_MAX);
  gridSizer->Add(m_nodeSize, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, -1, 
				  _("Vertical spacing between terminal nodes")),
		 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  const int Y_SPACING_MIN = 15;
  const int Y_SPACING_MAX = 60;
  
  m_terminalSpacing =
    new wxSpinCtrl(this, -1,
		   wxString::Format(_T("%d"), p_settings.TerminalSpacing()),
		   wxDefaultPosition, wxDefaultSize,
		   wxSP_ARROW_KEYS, Y_SPACING_MIN, Y_SPACING_MAX);
  gridSizer->Add(m_terminalSpacing, 1, wxEXPAND | wxALL, 5);

  sizeSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(sizeSizer, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

class gbtLayoutBranchesPanel : public wxPanel {
private:
  wxChoice *m_branchStyle, *m_branchLabels;
  wxSpinCtrl *m_branchLength, *m_tineLength;

public:
  gbtLayoutBranchesPanel(wxWindow *p_parent, const gbtStyle &);

  int BranchLength(void) const { return m_branchLength->GetValue(); }
  int TineLength(void) const { return m_tineLength->GetValue(); }

  int BranchStyle(void) const { return m_branchStyle->GetSelection(); }
  int BranchLabels(void) const { return m_branchLabels->GetSelection(); }
};

gbtLayoutBranchesPanel::gbtLayoutBranchesPanel(wxWindow *p_parent,
			     const gbtStyle &p_settings)
  : wxPanel(p_parent, -1)
{
  const int BRANCH_LENGTH_MIN = 0;
  const int BRANCH_LENGTH_MAX = 100;

  const int TINE_LENGTH_MIN = 20;
  const int TINE_LENGTH_MAX = 100;

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *styleBoxSizer = 
    new wxStaticBoxSizer(wxHORIZONTAL, this, _("Drawing branches"));

  wxFlexGridSizer *styleSizer = new wxFlexGridSizer(2);

  styleSizer->Add(new wxStaticText(this, wxID_STATIC, _("Draw branches")),
		  0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxString styleChoices[] = { _("using straight lines between nodes"),
			      _("with a tine for branch labels") };
  m_branchStyle = new wxChoice(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       2, styleChoices);
  m_branchStyle->SetSelection(p_settings.BranchStyle());
  styleSizer->Add(m_branchStyle, 1, wxALL | wxEXPAND, 5);

  styleSizer->Add(new wxStaticText(this, wxID_STATIC, _("Draw labels")),
		  1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxString labelChoices[] = { _("horizontally"), 
			      _("rotated parallel to the branch") };
  m_branchLabels = new wxChoice(this, -1,
				wxDefaultPosition, wxDefaultSize,
				2, labelChoices);
  m_branchLabels->SetSelection(p_settings.BranchLabels());
  styleSizer->Add(m_branchLabels, 1, wxALL | wxEXPAND, 5);

  styleBoxSizer->Add(styleSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(styleBoxSizer, 0, wxALL | wxALIGN_CENTER, 5);

  wxStaticBoxSizer *lengthSizer = 
    new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Length of branches"));

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
  gridSizer->AddGrowableCol(1);

  gridSizer->Add(new wxStaticText(this, -1, _("Length of branch fork")),
		 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_branchLength = new wxSpinCtrl(this, -1,
				  wxString::Format(_T("%d"),
						   p_settings.BranchLength()),
				  wxDefaultPosition, wxDefaultSize,
				  wxSP_ARROW_KEYS,
				  BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX);
  gridSizer->Add(m_branchLength, 1, wxALL | wxEXPAND, 5);

  gridSizer->Add(new wxStaticText(this, -1, _("Length of branch tine")),
		 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_tineLength = new wxSpinCtrl(this, -1,
				wxString::Format(_T("%d"),
						 p_settings.TineLength()),
				wxDefaultPosition, wxDefaultSize,
				wxSP_ARROW_KEYS,
				TINE_LENGTH_MIN, TINE_LENGTH_MAX);
  gridSizer->Add(m_tineLength, 1, wxALL | wxEXPAND, 5);

  lengthSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(lengthSizer, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

class gbtLayoutInfosetsPanel : public wxPanel {
private:
  wxChoice *m_infosetConnect, *m_infosetJoin;

public:
  gbtLayoutInfosetsPanel(wxWindow *p_parent, const gbtStyle &);

  int InfosetConnect(void) const { return m_infosetConnect->GetSelection(); }
  int InfosetJoin(void) const { return m_infosetJoin->GetSelection(); }
};

gbtLayoutInfosetsPanel::gbtLayoutInfosetsPanel(wxWindow *p_parent,
			     const gbtStyle &p_settings)
  : wxPanel(p_parent, -1)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *infosetSizer = 
    new wxStaticBoxSizer(wxVERTICAL, this, _("Drawing information sets"));

  wxFlexGridSizer *styleSizer = new wxFlexGridSizer(2);

  styleSizer->Add(new wxStaticText(this, wxID_STATIC, 
				   _("Connect members of information sets")),
		  0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxString connectChoices[] = { _("invisibly (don't draw indicators)"), 
				_("only when on the same level"),
				_("regardless of level") };
  m_infosetConnect = new wxChoice(this, -1,
				  wxDefaultPosition, wxDefaultSize,
				  3, connectChoices);
  m_infosetConnect->SetSelection(p_settings.InfosetConnect());
  styleSizer->Add(m_infosetConnect, 0, wxALL, 5);

  styleSizer->Add(new wxStaticText(this, wxID_STATIC,
				   _("Draw information set connections")),
		  0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
  wxString joinChoices[] = { _("using lines"),
			     _("using bubbles") };
  m_infosetJoin = new wxChoice(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       2, joinChoices);
  m_infosetJoin->SetSelection(p_settings.InfosetJoin());
  styleSizer->Add(m_infosetJoin, 0, wxALL | wxEXPAND, 5);
  infosetSizer->Add(styleSizer, 0, wxALL | wxALIGN_CENTER, 5);
  topSizer->Add(infosetSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

gbtLayoutDialog::gbtLayoutDialog(wxWindow *p_parent, 
				 const gbtStyle &p_settings)
  : wxDialog(p_parent, -1, _("Layout options"), wxDefaultPosition),
    m_toDefaults(false)
{
  m_notebook = new wxNotebook(this, -1, wxDefaultPosition, wxDefaultSize);
  m_notebook->AddPage(new gbtLayoutNodesPanel(m_notebook, p_settings), _("Nodes"));
  m_notebook->AddPage(new gbtLayoutBranchesPanel(m_notebook, p_settings), _("Branches"));
  m_notebook->AddPage(new gbtLayoutInfosetsPanel(m_notebook, p_settings),
		      _("Information sets"));

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *defaultsButton = new wxButton(this, -1, _("Set to defaults"));
  Connect(defaultsButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtLayoutDialog::OnSetDefaults));
  buttonSizer->Add(defaultsButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_notebook, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
  CenterOnParent();
}

void gbtLayoutDialog::GetSettings(gbtStyle &p_settings)
{
  if (m_toDefaults) {
    p_settings.SetDefaults();
    return;
  }

  gbtLayoutNodesPanel *nodes = (gbtLayoutNodesPanel *) m_notebook->GetPage(0);
  p_settings.SetNodeSize(nodes->NodeSize());
  p_settings.SetTerminalSpacing(nodes->TerminalSpacing());
  p_settings.SetChanceToken(nodes->ChanceToken());
  p_settings.SetPlayerToken(nodes->PlayerToken());
  p_settings.SetTerminalToken(nodes->TerminalToken());

  gbtLayoutBranchesPanel *branches = (gbtLayoutBranchesPanel *) m_notebook->GetPage(1);
  p_settings.SetBranchLength(branches->BranchLength());
  p_settings.SetTineLength(branches->TineLength());
  p_settings.SetBranchStyle(branches->BranchStyle());
  p_settings.SetBranchLabels(branches->BranchLabels());

  gbtLayoutInfosetsPanel *infosets = (gbtLayoutInfosetsPanel *) m_notebook->GetPage(2);
  p_settings.SetInfosetConnect(infosets->InfosetConnect());
  p_settings.SetInfosetJoin(infosets->InfosetJoin());
}

void gbtLayoutDialog::OnSetDefaults(wxCommandEvent &)
{
  m_toDefaults = true;
  EndModal(wxID_OK);
}
