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

#include "dlefglegend.h"


//==========================================================================
//                 class dialogLegend: Implementation
//==========================================================================

dialogLegend::dialogLegend(wxWindow *p_parent,
			     const TreeDrawSettings &p_options)	
  : wxDialog(p_parent, -1, "Legends")
{
  wxStaticBoxSizer *nodeGroup = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Node Labeling"),
			 wxHORIZONTAL);

  wxString nodeLabelList[] = { "Blank", "Node label", "Player",
			       "Information set label",
			       "Information set number",
			       "Outcome", "Realization probability", 
			       "Belief probability",
			       "Node value" };

  m_nodeAbove = new wxRadioBox(this, -1, "Above Node",
			       wxDefaultPosition, wxDefaultSize,
			       9, nodeLabelList, 1, wxRA_SPECIFY_COLS);
  m_nodeAbove->SetSelection(p_options.NodeAboveLabel());
  nodeGroup->Add(m_nodeAbove, 0, wxALL, 5);

  m_nodeBelow = new wxRadioBox(this, -1, "Below Node",
			       wxDefaultPosition, wxDefaultSize,
			       9, nodeLabelList, 1, wxRA_SPECIFY_COLS);
  m_nodeBelow->SetSelection(p_options.NodeBelowLabel());
  nodeGroup->Add(m_nodeBelow, 0, wxALL, 5);

  wxString nodeAfterList[] = { "Blank", "Payoffs", "Outcome name" };
  m_nodeAfter = new wxRadioBox(this, -1, "After Node",
			       wxDefaultPosition, wxDefaultSize,
			       3, nodeAfterList, 1, wxRA_SPECIFY_COLS);
  m_nodeAfter->SetSelection(p_options.NodeRightLabel());
  nodeGroup->Add(m_nodeAfter, 0, wxALL, 5);

  wxStaticBoxSizer *branchGroup =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Branch Labelling"),
			 wxHORIZONTAL);

  wxString branchLabelList[] = { "Blank", "Action label",
				 "Action probability", "Action value" };
  m_branchAbove = new wxRadioBox(this, -1, "Above Branch",
			       wxDefaultPosition, wxDefaultSize,
			       5, branchLabelList, 1, wxRA_SPECIFY_COLS);
  m_branchAbove->SetSelection(p_options.BranchAboveLabel());
  branchGroup->Add(m_branchAbove, 0, wxALL, 5);

  m_branchBelow = new wxRadioBox(this, -1, "Below Branch",
				 wxDefaultPosition, wxDefaultSize,
				 5, branchLabelList, 1, wxRA_SPECIFY_COLS);
  m_branchBelow->SetSelection(p_options.BranchBelowLabel());
  branchGroup->Add(m_branchBelow, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(nodeGroup, 0, wxCENTER | wxALL, 5);
  topSizer->Add(branchGroup, 0, wxCENTER | wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}
