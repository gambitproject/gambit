//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Display configuration class for the extensive form
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
			   int p_nodeLength,
			   int p_forkLength, int p_ySpacing,
			   int p_infosetStyle)
  : guiAutoDialog(p_parent, "Layout Options")
{
  const int NODE_LENGTH_MIN = 20;
  const int NODE_LENGTH_MAX = 100;

  const int BRANCH_LENGTH_MIN = 0;
  const int BRANCH_LENGTH_MAX = 100;

  const int Y_SPACING_MIN = 15;
  const int Y_SPACING_MAX = 60;

  wxStaticBoxSizer *layoutSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Tree layout parameters"),
			 wxVERTICAL);
  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
		
  gridSizer->Add(new wxStaticText(this, -1, "Node length"),
		 0, wxCENTER | wxALL, 5);
  m_nodeLength = new wxSpinCtrl(this, -1, wxString::Format("%d", p_nodeLength),
				wxDefaultPosition, wxDefaultSize,
				wxSP_ARROW_KEYS,
				NODE_LENGTH_MIN, NODE_LENGTH_MAX);
  gridSizer->Add(m_nodeLength, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, -1, "Branch length"),
		 0, wxCENTER | wxALL, 5);
  m_branchLength = new wxSpinCtrl(this, -1,
				  wxString::Format("%d", p_forkLength),
				  wxDefaultPosition, wxDefaultSize,
				  wxSP_ARROW_KEYS,
				  BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX);
  gridSizer->Add(m_branchLength, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, -1, "Vertical spacing"),
		 0, wxCENTER | wxALL, 5);
  m_ySpacing = new wxSpinCtrl(this, -1, wxString::Format("%d", p_ySpacing),
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS,
			      Y_SPACING_MIN, Y_SPACING_MAX);
  gridSizer->Add(m_ySpacing, 1, wxEXPAND | wxALL, 5);

  layoutSizer->Add(gridSizer, 0, wxEXPAND | wxALL, 5);

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
  m_nodeAbove->SetSelection(p_options.LabelNodeAbove());
  nodeGroup->Add(m_nodeAbove, 0, wxALL, 5);

  m_nodeBelow = new wxRadioBox(this, -1, "Below Node",
			       wxDefaultPosition, wxDefaultSize,
			       9, nodeLabelList, 1, wxRA_SPECIFY_COLS);
  m_nodeBelow->SetSelection(p_options.LabelNodeBelow());
  nodeGroup->Add(m_nodeBelow, 0, wxALL, 5);

  wxString nodeAfterList[] = { "Blank", "Payoffs", "Outcome name" };
  m_nodeAfter = new wxRadioBox(this, -1, "After Node",
			       wxDefaultPosition, wxDefaultSize,
			       3, nodeAfterList, 1, wxRA_SPECIFY_COLS);
  m_nodeAfter->SetSelection(p_options.LabelNodeRight());
  nodeGroup->Add(m_nodeAfter, 0, wxALL, 5);

  wxStaticBoxSizer *branchGroup =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Branch Labelling"),
			 wxHORIZONTAL);

  wxString branchLabelList[] = { "Blank", "Action label",
				 "Action probability", "Action value" };
  m_branchAbove = new wxRadioBox(this, -1, "Above Branch",
			       wxDefaultPosition, wxDefaultSize,
			       5, branchLabelList, 1, wxRA_SPECIFY_COLS);
  m_branchAbove->SetSelection(p_options.LabelBranchAbove());
  branchGroup->Add(m_branchAbove, 0, wxALL, 5);

  m_branchBelow = new wxRadioBox(this, -1, "Below Branch",
				 wxDefaultPosition, wxDefaultSize,
				 5, branchLabelList, 1, wxRA_SPECIFY_COLS);
  m_branchBelow->SetSelection(p_options.LabelBranchBelow());
  branchGroup->Add(m_branchBelow, 0, wxALL, 5);

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
  config.Write("/TreeDisplay/BranchLength", (long) m_branchLength);
  config.Write("/TreeDisplay/OutcomeLength", (long) outcome_length);
  config.Write("/TreeDisplay/YSpacing", (long) y_spacing);

  config.Write("/TreeDisplay/ChanceColor", (long) chance_color);
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

  config.Write("/TreeDisplay/ColorOutcomes", (long) color_coded_outcomes);
  config.Write("/TreeDisplay/RootReachable", (long) root_reachable);
}

void TreeDrawSettings::LoadOptions(void)
{
  wxConfig config("Gambit");

  config.Read("/TreeDisplay/NodeLength", &node_length, 20);
  config.Read("/TreeDisplay/BranchLength", &m_branchLength, 60);
  config.Read("/TreeDisplay/OutcomeLength", &outcome_length, 60);
  config.Read("/TreeDisplay/YSpacing", &y_spacing, 45);

  config.Read("/TreeDisplay/DisplayPrecision", &num_prec, 2);

  config.Read("/TreeDisplay/ChanceColor", &chance_color, 0);
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

  config.Read("/TreeDisplay/ColorOutcomes", &color_coded_outcomes, 1);
  config.Read("/TreeDisplay/RootReachable", &root_reachable, 0);
}

