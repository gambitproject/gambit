//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for setting labels for extensive form display
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

#include "dialog-tree-label.h"

//==========================================================================
//                 class gbtTreeLabelDialog: Implementation
//==========================================================================

gbtTreeLabelDialog::gbtTreeLabelDialog(wxWindow *p_parent,
				       const gbtTreeLayoutOptions &p_options)
  : wxDialog(p_parent, -1, _("Tree labels"), wxDefaultPosition)
{
  wxStaticBoxSizer *nodeGroup = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, 
					 _("Node labeling")),
			 wxHORIZONTAL);

  wxString nodeLabelList[] = { _("None"), _("Node label"), _("Player"),
			       _("Information set label"),
			       _("Information set number") };

  m_nodeAbove = new wxRadioBox(this, -1, _("Above node"),
			       wxDefaultPosition, wxDefaultSize,
			       5, nodeLabelList, 1, wxRA_SPECIFY_COLS);
  m_nodeAbove->SetSelection(p_options.GetNodeAboveLabel());
  nodeGroup->Add(m_nodeAbove, 0, wxALL, 5);

  m_nodeBelow = new wxRadioBox(this, -1, _("Below node"),
			       wxDefaultPosition, wxDefaultSize,
			       5, nodeLabelList, 1, wxRA_SPECIFY_COLS);
  m_nodeBelow->SetSelection(p_options.GetNodeBelowLabel());
  nodeGroup->Add(m_nodeBelow, 0, wxALL, 5);

  wxString nodeAfterList[] = { _("Payoffs"), _("Outcome name") };
  m_outcome = new wxRadioBox(this, -1, _("Outcome"),
			     wxDefaultPosition, wxDefaultSize,
			     2, nodeAfterList, 1, wxRA_SPECIFY_COLS);
  m_outcome->SetSelection(p_options.GetOutcomeLabel());
  nodeGroup->Add(m_outcome, 0, wxALL, 5);

  wxStaticBoxSizer *branchGroup =
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, 
					 _("Branch labeling")),
			 wxHORIZONTAL);

  wxString branchLabelList[] = { _("None"), _("Action label"),
				 _("Action probability") };
  m_branchAbove = new wxRadioBox(this, -1, _("Above branch"),
				 wxDefaultPosition, wxDefaultSize,
				 3, branchLabelList, 1, wxRA_SPECIFY_COLS);
  m_branchAbove->SetSelection(p_options.GetBranchAboveLabel());
  branchGroup->Add(m_branchAbove, 0, wxALL, 5);

  m_branchBelow = new wxRadioBox(this, -1, _("Below branch"),
				 wxDefaultPosition, wxDefaultSize,
				 3, branchLabelList, 1, wxRA_SPECIFY_COLS);
  m_branchBelow->SetSelection(p_options.GetBranchBelowLabel());
  branchGroup->Add(m_branchBelow, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(nodeGroup, 0, wxALIGN_CENTER | wxALL, 5);
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
