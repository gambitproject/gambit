//
// FILE: treedraw.cc -- Display configuration class for the extensive form
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "wxmisc.h"
#include "gambit.h"
#include "treedraw.h"
#include "legend.h"

#include "dllayout.h"
#include "dllegends.h"

TreeDrawSettings::TreeDrawSettings(void)
  : node_above_font(*wxNORMAL_FONT), node_below_font(*wxNORMAL_FONT),
    node_right_font(*wxNORMAL_FONT), branch_above_font(*wxNORMAL_FONT),
    branch_below_font(*wxNORMAL_FONT)
{
  LoadOptions();
}

dialogLayout::dialogLayout(wxWindow *p_parent,
			   int p_branchLength, int p_nodeLength,
			   int p_forkLength, int p_ySpacing,
			   int p_infosetStyle)
  : guiAutoDialog(p_parent, "Layout Options")
{
  wxStaticBoxSizer *layoutSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Tree layout parameters"),
			 wxVERTICAL);
			 
  wxBoxSizer *branchSizer = new wxBoxSizer(wxHORIZONTAL);
  branchSizer->Add(new wxStaticText(this, -1, "Branch length"),
		   0, wxCENTER | wxALL, 5);
  m_branchLength = new wxSlider(this, -1, p_branchLength, 
				BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX,
				wxDefaultPosition, wxDefaultSize,
				wxSL_HORIZONTAL | wxSL_LABELS);
  branchSizer->Add(m_branchLength, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *nodeSizer = new wxBoxSizer(wxHORIZONTAL);
  nodeSizer->Add(new wxStaticText(this, -1, "Node length"),
		 0, wxCENTER | wxALL, 5);
  m_nodeLength = new wxSlider(this, -1, p_nodeLength,
			      NODE_LENGTH_MIN, NODE_LENGTH_MAX,
			      wxDefaultPosition, wxDefaultSize,
			      wxSL_HORIZONTAL | wxSL_LABELS);
  nodeSizer->Add(m_nodeLength, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *forkSizer = new wxBoxSizer(wxHORIZONTAL);
  forkSizer->Add(new wxStaticText(this, -1, "Fork length"),
		 0, wxCENTER | wxALL, 5);
  m_forkLength = new wxSlider(this, -1, p_forkLength,
			      FORK_LENGTH_MIN, FORK_LENGTH_MAX,
			      wxDefaultPosition, wxDefaultSize,
			      wxSL_HORIZONTAL | wxSL_LABELS);
  forkSizer->Add(m_forkLength, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *spacingSizer = new wxBoxSizer(wxHORIZONTAL);
  spacingSizer->Add(new wxStaticText(this, -1, "Vertical spacing"),
		    0, wxCENTER | wxALL, 5);
  m_ySpacing = new wxSlider(this, -1, p_ySpacing,
			    Y_SPACING_MIN, Y_SPACING_MAX,
			    wxDefaultPosition, wxDefaultSize,
			    wxSL_HORIZONTAL | wxSL_LABELS);
  spacingSizer->Add(m_ySpacing, 1, wxEXPAND | wxALL, 5);

  layoutSizer->Add(branchSizer, 0, wxEXPAND | wxALL, 5);
  layoutSizer->Add(nodeSizer, 0, wxEXPAND | wxALL, 5);
  layoutSizer->Add(forkSizer, 0, wxEXPAND | wxALL, 5);
  layoutSizer->Add(spacingSizer, 0, wxEXPAND | wxALL, 5);

  wxString lineChoices[] = { "None", "Same Level", "All Levels" };
  m_infosetStyle = new wxRadioBox(this, -1, "Show Infoset Lines",
				  wxDefaultPosition, wxDefaultSize,
				  3, lineChoices);
  m_infosetStyle->SetSelection(p_infosetStyle);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(layoutSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(m_infosetStyle, 0, wxCENTER | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

dialogLegends::dialogLegends(wxWindow *p_parent,
			     const TreeDrawSettings &p_options)	
  : guiAutoDialog(p_parent, "Legends")
{
  wxStaticBoxSizer *nodeGroup = 
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Nodes"), wxHORIZONTAL);

  wxString nodeLabelList[] = { "Nothing", "Node Label", "Player",
			       "Infoset Label", "Infoset ID",
			       "Outcome", "Realiz Prob", "Belief Prob",
			       "Value" };

  wxBoxSizer *nodeAboveSizer = new wxBoxSizer(wxVERTICAL);
  nodeAboveSizer->Add(new wxStaticText(this, -1, "Above"),
		      0, wxCENTER | wxALL, 5);
  m_nodeAbove = new wxChoice(this, -1,
			     wxDefaultPosition, wxDefaultSize,
			     9, nodeLabelList);
  m_nodeAbove->SetSelection(p_options.LabelNodeAbove());
  nodeAboveSizer->Add(m_nodeAbove, 0, wxALL, 5);

  wxBoxSizer *nodeBelowSizer = new wxBoxSizer(wxVERTICAL);
  nodeBelowSizer->Add(new wxStaticText(this, -1, "Below"),
		      0, wxALL | wxCENTER, 5);
  m_nodeBelow = new wxChoice(this, -1,
			     wxDefaultPosition, wxDefaultSize,
			     9, nodeLabelList);
  m_nodeBelow->SetSelection(p_options.LabelNodeBelow());
  nodeBelowSizer->Add(m_nodeBelow, 0, wxALL, 5);

  wxBoxSizer *nodeAfterSizer = new wxBoxSizer(wxVERTICAL);
  nodeAfterSizer->Add(new wxStaticText(this, -1, "Right"),
		      0, wxALL | wxCENTER, 5);
  wxString nodeAfterList[] = { "Nothing", "Payoffs", "Name" };
  m_nodeAfter = new wxChoice(this, -1,
			     wxDefaultPosition, wxDefaultSize,
			     3, nodeAfterList);
  m_nodeAfter->SetSelection(p_options.LabelNodeRight());
  nodeAfterSizer->Add(m_nodeAfter, 0, wxALL, 5);

  nodeGroup->Add(nodeAboveSizer, 0, wxALL, 5);
  nodeGroup->Add(nodeBelowSizer, 0, wxALL, 5);
  nodeGroup->Add(nodeAfterSizer, 0, wxALL, 5);

  wxStaticBoxSizer *branchGroup =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Branches"), wxHORIZONTAL);

  wxBoxSizer *branchAboveSizer = new wxBoxSizer(wxVERTICAL);
  branchAboveSizer->Add(new wxStaticText(this, -1, "Above"),
			0, wxALL | wxCENTER, 5);  
  wxString branchLabelList[] = { "Nothing", "Label", "Player",
				 "Probs", "Value" };
  m_branchAbove = new wxChoice(this, -1, 
			       wxDefaultPosition, wxDefaultSize,
			       5, branchLabelList);
  m_branchAbove->SetSelection(p_options.LabelBranchAbove());
  branchAboveSizer->Add(m_branchAbove, 0, wxALL, 5);

  wxBoxSizer *branchBelowSizer = new wxBoxSizer(wxVERTICAL);
  branchBelowSizer->Add(new wxStaticText(this, -1, "Below"),
			0, wxALL | wxCENTER, 5);
  m_branchBelow = new wxChoice(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       5, branchLabelList);
  m_branchBelow->SetSelection(p_options.LabelBranchBelow());
  branchBelowSizer->Add(m_branchBelow, 0, wxALL, 5);

  branchGroup->Add(branchAboveSizer, 0, wxALL, 5);
  branchGroup->Add(branchBelowSizer, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(nodeGroup, 0, wxCENTER | wxALL, 5);
  topSizer->Add(branchGroup, 0, wxCENTER | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

void TreeDrawSettings::SaveOptions(void)
{
  wxConfig config("Gambit");
  config.Write("TreeDisplay/Branch-Length", (long) branch_length);
  config.Write("TreeDisplay/Fork-Length", (long) fork_length);
  config.Write("TreeDisplay/Outcome-Length", (long) outcome_length);
  config.Write("TreeDisplay/Y-Spacing", (long) y_spacing);

  config.Write("TreeDisplay/Chance-Color", (long) chance_color);
  config.Write("TreeDisplay/Cursor-Color", (long) cursor_color);
  config.Write("TreeDisplay/Show-Infosets", (long) show_infosets);
  config.Write("TreeDisplay/Node-Above-Label", (long) node_above_label);
  config.Write("TreeDisplay/Node-Below-Label", (long) node_below_label);
  config.Write("TreeDisplay/Node-Right-Label", (long) node_right_label);
  config.Write("TreeDisplay/Branch-Above-Label", (long) branch_above_label);
  config.Write("TreeDisplay/Branch-Below-Label", (long) branch_below_label);

  config.Write("TreeDisplay/Flashing-Cursor",  (long) flashing_cursor);
  config.Write("TreeDisplay/Color-Outcomes", (long) color_coded_outcomes);
  config.Write("TreeDisplay/Root-Reachable", (long) root_reachable);

  GambitDrawSettings::SaveOptions();
}

void TreeDrawSettings::LoadOptions(void)
{
  wxConfig config("Gambit");
  config.Read("TreeDisplay/Branch-Length", &branch_length, 60);
  config.Read("TreeDisplay/Node-Length", &node_length, 60);
  config.Read("TreeDisplay/Fork-Length", &fork_length, 60);
  config.Read("TreeDisplay/Outcome-Length", &outcome_length, 60);
  config.Read("TreeDisplay/Y-Spacing", &y_spacing, 30);

  config.Read("TreeDisplay/Display-Precision", &num_prec, 2);

  config.Read("TreeDisplay/Chance-Color", &chance_color, 0);
  config.Read("TreeDisplay/Cursor-Color", &cursor_color, 10);
  config.Read("TreeDisplay/Show-Infosets", &show_infosets, 2);
  config.Read("TreeDisplay/Node-Above-Label", &node_above_label, 1);
  config.Read("TreeDisplay/Node-Below-Label", &node_below_label, 4);
  config.Read("TreeDisplay/Branch-Above-Label", &branch_above_label, 1);
  config.Read("TreeDisplay/Branch-Below-Label", &branch_below_label, 3);
  config.Read("TreeDisplay/Node-Right-Label", &node_right_label, 1);
  
  config.Read("TreeDisplay/Flashing-Cursor", &flashing_cursor, 1);
  config.Read("TreeDisplay/Color-Outcomes", &color_coded_outcomes, 1);
  config.Read("TreeDisplay/Root-Reachable", &root_reachable, 0);

  GambitDrawSettings::LoadOptions();
}

