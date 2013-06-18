//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/dlefglegend.cc
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

#include "dlefglegend.h"


//==========================================================================
//                 class gbtLegendDialog: Implementation
//==========================================================================

gbtLegendDialog::gbtLegendDialog(wxWindow *p_parent,
			   const gbtStyle &p_options)	
  : wxDialog(p_parent, -1, _("Labels"), wxDefaultPosition)
{
  wxStaticBoxSizer *nodeGroup = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, _("Node labeling")),
			 wxVERTICAL);

  wxString nodeLabelList[] = { _("no label"), 
			       _("the node's label"), 
			       _("the player's name"),
			       _("the information set's label"),
			       _("the information set's number"),
			       _("the realization probability"),
			       _("the belief probability"),
			       _("the payoff of reaching the node") };

  wxBoxSizer *nodeAboveSizer = new wxBoxSizer(wxHORIZONTAL);
  nodeAboveSizer->Add(new wxStaticText(this, wxID_STATIC,
				       _("Display")),
		      0, wxALL | wxALIGN_CENTER, 5);
  m_nodeAbove = new wxChoice(this, -1,
			     wxDefaultPosition, wxDefaultSize,
			     8, nodeLabelList);
  m_nodeAbove->SetSelection(p_options.NodeAboveLabel());
  nodeAboveSizer->Add(m_nodeAbove, 1, wxALL | wxALIGN_CENTER, 5);
  nodeAboveSizer->Add(new wxStaticText(this, wxID_STATIC,
				       _("above each node")),
		      0, wxALL | wxALIGN_CENTER, 5);
  nodeGroup->Add(nodeAboveSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *nodeBelowSizer = new wxBoxSizer(wxHORIZONTAL);
  nodeBelowSizer->Add(new wxStaticText(this, wxID_STATIC,
				       _("Display")),
		      0, wxALL | wxALIGN_CENTER, 5);
  m_nodeBelow = new wxChoice(this, -1,
			     wxDefaultPosition, wxDefaultSize,
			     8, nodeLabelList);
  m_nodeBelow->SetSelection(p_options.NodeBelowLabel());
  nodeBelowSizer->Add(m_nodeBelow, 1, wxALL | wxALIGN_CENTER, 5);
  nodeBelowSizer->Add(new wxStaticText(this, wxID_STATIC,
				       _("below each node")),
		      0, wxALL | wxALIGN_CENTER, 5);
  nodeGroup->Add(nodeBelowSizer, 0, wxALL | wxEXPAND, 5);


  wxStaticBoxSizer *actionGroup = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, _("Action labeling")),
			 wxVERTICAL);

  wxString actionLabelList[] = { _("no label"), 
				 _("the name of the action"),
				 _("the probability the action is played"), 
				 _("the value of the action") };

  wxBoxSizer *actionAboveSizer = new wxBoxSizer(wxHORIZONTAL);
  actionAboveSizer->Add(new wxStaticText(this, wxID_STATIC,
					 _("Display")),
		      0, wxALL | wxALIGN_CENTER, 5);
  m_actionAbove = new wxChoice(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       4, actionLabelList);
  m_actionAbove->SetSelection(p_options.BranchAboveLabel());
  actionAboveSizer->Add(m_actionAbove, 1, wxALL | wxALIGN_CENTER, 5);
  actionAboveSizer->Add(new wxStaticText(this, wxID_STATIC,
					 _("above each action")),
		      0, wxALL | wxALIGN_CENTER, 5);
  actionGroup->Add(actionAboveSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *actionBelowSizer = new wxBoxSizer(wxHORIZONTAL);
  actionBelowSizer->Add(new wxStaticText(this, wxID_STATIC,
					 _("Display")),
		      0, wxALL | wxALIGN_CENTER, 5);
  m_actionBelow = new wxChoice(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       4, actionLabelList);
  m_actionBelow->SetSelection(p_options.BranchBelowLabel());
  actionBelowSizer->Add(m_actionBelow, 1, wxALL | wxALIGN_CENTER, 5);
  actionBelowSizer->Add(new wxStaticText(this, wxID_STATIC,
					 _("below each action")),
		      0, wxALL | wxALIGN_CENTER, 5);
  actionGroup->Add(actionBelowSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(nodeGroup, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(actionGroup, 0, wxEXPAND | wxALL, 5);

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
