//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for setting layout parameters for extensive form display
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#include "dialog-tree-layout.h"


const int GBT_RADIO_BRANCH_STYLE = 1000;

BEGIN_EVENT_TABLE(gbtTreeLayoutDialog, wxDialog)
  EVT_RADIOBOX(GBT_RADIO_BRANCH_STYLE, gbtTreeLayoutDialog::OnBranchStyle)
END_EVENT_TABLE()


//==========================================================================
//                class gbtTreeLayoutDialog: Implementation
//=========================================================================

gbtTreeLayoutDialog::gbtTreeLayoutDialog(wxWindow *p_parent,
					 const gbtTreeLayoutOptions &p_options)
  : wxDialog(p_parent, -1, _("Tree layout"), wxDefaultPosition)
{
  const int NODE_LENGTH_MIN = 5;
  const int NODE_LENGTH_MAX = 150;
  const int Y_SPACING_MIN = 15;
  const int Y_SPACING_MAX = 100;
  const int BRANCH_LENGTH_MIN = 5;
  const int BRANCH_LENGTH_MAX = 100;
  const int TINE_LENGTH_MIN = 5;
  const int TINE_LENGTH_MAX = 100;

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *tokenSizer = new wxBoxSizer(wxHORIZONTAL);
  wxString tokenChoices[] = { _("Line"), _("Box"), 
			      _("Circle"), _("Dot"), _("Diamond") };
  m_chanceToken = new wxRadioBox(this, -1, _("Chance nodes"),
				 wxDefaultPosition, wxDefaultSize,
				 5, tokenChoices, 1, wxRA_SPECIFY_COLS);
  m_chanceToken->SetSelection(p_options.GetChanceToken());
  tokenSizer->Add(m_chanceToken, 0, wxALL, 5);

  m_playerToken = new wxRadioBox(this, -1, _("Player nodes"),
				 wxDefaultPosition, wxDefaultSize,
				 5, tokenChoices, 1, wxRA_SPECIFY_COLS);
  m_playerToken->SetSelection(p_options.GetPlayerToken());
  tokenSizer->Add(m_playerToken, 0, wxALL, 5);

  m_terminalToken = new wxRadioBox(this, -1, _("Terminal nodes"),
				   wxDefaultPosition, wxDefaultSize,
				   5, tokenChoices, 1, wxRA_SPECIFY_COLS);
  m_terminalToken->SetSelection(p_options.GetTerminalToken());
  tokenSizer->Add(m_terminalToken, 0, wxALL, 5);
  topSizer->Add(tokenSizer, 0, wxALIGN_CENTER | wxALL, 5);

  wxStaticBoxSizer *nodeGroup = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 _("Node placement")),
			 wxHORIZONTAL);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Size of nodes")),
		 0, wxALIGN_CENTER | wxALL, 5);
  m_nodeSize = new wxSpinCtrl(this, -1,
			      wxString::Format(_T("%d"), 
					       p_options.GetNodeSize()),
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS,
			      NODE_LENGTH_MIN, NODE_LENGTH_MAX);
  gridSizer->Add(m_nodeSize, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, wxID_STATIC, 
				  _("Vertical spacing of terminal nodes")),
		 0, wxALIGN_CENTER | wxALL, 5);
  m_terminalSpacing =
    new wxSpinCtrl(this, -1,
		   wxString::Format(_T("%d"), p_options.GetTerminalSpacing()),
		   wxDefaultPosition, wxDefaultSize,
		   wxSP_ARROW_KEYS, Y_SPACING_MIN, Y_SPACING_MAX);
  gridSizer->Add(m_terminalSpacing, 1, wxEXPAND | wxALL, 5);
  nodeGroup->Add(gridSizer, 0, wxALIGN_CENTER | wxALL, 0);
  topSizer->Add(nodeGroup, 0, wxALIGN_CENTER | wxALL, 5);

  wxStaticBoxSizer *branchGroup = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 _("Branch rendering")),
			 wxHORIZONTAL);

  wxString branchChoices[] = { _("Fork-tine (classic)"), 
			       _("Horizontal labels"),
			       _("Rotated labels") };
  m_branchStyle = new wxRadioBox(this, GBT_RADIO_BRANCH_STYLE, 
				 _("Branch style"),
				 wxDefaultPosition, wxDefaultSize,
				 3, branchChoices, 1, wxRA_SPECIFY_COLS);
  m_branchStyle->SetSelection(p_options.GetBranchStyle());
  branchGroup->Add(m_branchStyle, 0, wxALL | wxALIGN_CENTER, 5);

  wxFlexGridSizer *branchGridSizer = new wxFlexGridSizer(2);

  branchGridSizer->Add(new wxStaticText(this, wxID_STATIC, 
					_("Length of branches")),
		 0, wxALIGN_CENTER | wxALL, 5);
  m_branchLength = new wxSpinCtrl(this, -1,
				  wxString::Format(_T("%d"), 
						   p_options.GetBranchLength()),
				  wxDefaultPosition, wxDefaultSize,
				  wxSP_ARROW_KEYS,
				  BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX);
  branchGridSizer->Add(m_branchLength, 1, wxEXPAND | wxALL, 5);

  branchGridSizer->Add(new wxStaticText(this, wxID_STATIC, 
					_("Length of tines")),
		 0, wxALIGN_CENTER | wxALL, 5);
  m_tineLength = new wxSpinCtrl(this, -1,
				wxString::Format(_T("%d"), 
						 p_options.GetTineLength()),
				wxDefaultPosition, wxDefaultSize,
				wxSP_ARROW_KEYS,
				TINE_LENGTH_MIN, TINE_LENGTH_MAX);
  m_tineLength->Enable(p_options.GetBranchStyle() == 0);
  branchGridSizer->Add(m_tineLength, 1, wxEXPAND | wxALL, 5);

  branchGroup->Add(branchGridSizer, 0, wxALIGN_CENTER | wxALL, 0);
  topSizer->Add(branchGroup, 0, wxALIGN_CENTER | wxALL, 5);
  
  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}

void gbtTreeLayoutDialog::OnBranchStyle(wxCommandEvent &)
{
  m_tineLength->Enable(m_branchStyle->GetSelection() == 0);
}
