//
// FILE: treedraw.cc -- Display configuration class for the extensive form
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "guishare/wxmisc.h"
#include "gambit.h"
#include "treedraw.h"

#include "dllayout.h"
#include "dllegends.h"

TreeDrawSettings::TreeDrawSettings(void)
  : m_nodeAboveFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_nodeBelowFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_nodeRightFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_branchAboveFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_branchBelowFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL))
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

void TreeDrawSettings::SaveFont(const wxString &p_prefix, 
				wxConfig &p_config, const wxFont &p_font)
{
  p_config.Write(p_prefix + "Size", (long) p_font.GetPointSize());
  p_config.Write(p_prefix + "Family", (long) p_font.GetFamily());
  p_config.Write(p_prefix + "Face", p_font.GetFaceName());
  p_config.Write(p_prefix + "Style", (long) p_font.GetStyle());
  p_config.Write(p_prefix + "Weight", (long) p_font.GetWeight());
}

void TreeDrawSettings::LoadFont(const wxString &p_prefix,
				const wxConfig &p_config, wxFont &p_font)
{
  int size, family, style, weight;
  wxString face;
  p_config.Read(p_prefix + "Size", &size, 10);
  p_config.Read(p_prefix + "Family", &family, wxMODERN);
  p_config.Read(p_prefix + "Face", &face, "");
  p_config.Read(p_prefix + "Style", &style, wxNORMAL);
  p_config.Read(p_prefix + "Weight", &weight, wxNORMAL);

  p_font = *wxTheFontList->FindOrCreateFont(size, family, style, weight,
					    false, face);
}

void TreeDrawSettings::SaveOptions(void)
{
  wxConfig config("Gambit");
  config.Write("/TreeDisplay/BranchLength", (long) branch_length);
  config.Write("/TreeDisplay/ForkLength", (long) fork_length);
  config.Write("/TreeDisplay/OutcomeLength", (long) outcome_length);
  config.Write("/TreeDisplay/YSpacing", (long) y_spacing);

  config.Write("/TreeDisplay/ChanceColor", (long) chance_color);
  config.Write("/TreeDisplay/CursorColor", (long) cursor_color);
  config.Write("/TreeDisplay/ShowInfosets", (long) show_infosets);
  config.Write("/TreeDisplay/NodeAboveLabel", (long) node_above_label);
  config.Write("/TreeDisplay/NodeBelowLabel", (long) node_below_label);
  config.Write("/TreeDisplay/NodeRightLabel", (long) node_right_label);
  config.Write("/TreeDisplay/BranchAboveLabel", (long) branch_above_label);
  config.Write("/TreeDisplay/BranchBelowLabel", (long) branch_below_label);

  SaveFont("/TreeDisplay/NodeAboveFont", config, m_nodeAboveFont);
  SaveFont("/TreeDisplay/NodeBelowFont", config, m_nodeBelowFont);
  SaveFont("/TreeDisplay/NodeRightFont", config, m_nodeRightFont);
  SaveFont("/TreeDisplay/BranchAboveFont", config, m_branchAboveFont);
  SaveFont("/TreeDisplay/BranchBelowFont", config, m_branchBelowFont);

  config.Write("/TreeDisplay/FlashingCursor",  (long) flashing_cursor);
  config.Write("/TreeDisplay/ColorOutcomes", (long) color_coded_outcomes);
  config.Write("/TreeDisplay/RootReachable", (long) root_reachable);
}

void TreeDrawSettings::LoadOptions(void)
{
  wxConfig config("Gambit");

  config.Read("/TreeDisplay/BranchLength", &branch_length, 60);
  config.Read("/TreeDisplay/NodeLength", &node_length, 60);
  config.Read("/TreeDisplay/ForkLength", &fork_length, 60);
  config.Read("/TreeDisplay/OutcomeLength", &outcome_length, 60);
  config.Read("/TreeDisplay/YSpacing", &y_spacing, 30);

  config.Read("/TreeDisplay/DisplayPrecision", &num_prec, 2);

  config.Read("/TreeDisplay/ChanceColor", &chance_color, 0);
  config.Read("/TreeDisplay/CursorColor", &cursor_color, 10);
  config.Read("/TreeDisplay/ShowInfosets", &show_infosets, 2);
  config.Read("/TreeDisplay/NodeAboveLabel", &node_above_label, 1);
  config.Read("/TreeDisplay/NodeBelowLabel", &node_below_label, 4);
  config.Read("/TreeDisplay/BranchAboveLabel", &branch_above_label, 1);
  config.Read("/TreeDisplay/BranchBelowLabel", &branch_below_label, 3);
  config.Read("/TreeDisplay/NodeRightLabel", &node_right_label, 1);
  
  LoadFont("/TreeDisplay/NodeAboveFont", config, m_nodeAboveFont);
  LoadFont("/TreeDisplay/NodeBelowFont", config, m_nodeBelowFont);
  LoadFont("/TreeDisplay/NodeRightFont", config, m_nodeRightFont);
  LoadFont("/TreeDisplay/BranchAboveFont", config, m_branchAboveFont);
  LoadFont("/TreeDisplay/BranchBelowFont", config, m_branchBelowFont);

  config.Read("/TreeDisplay/FlashingCursor", &flashing_cursor, 1);
  config.Read("/TreeDisplay/ColorOutcomes", &color_coded_outcomes, 1);
  config.Read("/TreeDisplay/RootReachable", &root_reachable, 0);
}

