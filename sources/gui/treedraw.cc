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
#include "wx/notebook.h"
#include "wx/spinctrl.h"

#include "guishare/wxmisc.h"
#include "gambit.h"
#include "treedraw.h"

#include "dllayout.h"
#include "dllegends.h"

//===========================================================================
//                 class TreeDrawSettings: Implementation
//===========================================================================

TreeDrawSettings::TreeDrawSettings(void)
  : m_nodeAboveFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_nodeBelowFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_nodeRightFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_branchAboveFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL)),
    m_branchBelowFont(*wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxNORMAL))
{
  LoadOptions();
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

void TreeDrawSettings::SaveOptions(void) const
{
  wxConfig config("Gambit");

  config.Write("/TreeDisplay/NodeSize", (long) m_nodeSize);
  config.Write("/TreeDisplay/TerminalSpacing", (long) m_terminalSpacing);
  config.Write("/TreeDisplay/ChanceToken", (long) m_chanceToken);
  config.Write("/TreeDisplay/PlayerToken", (long) m_playerToken);
  config.Write("/TreeDisplay/TerminalToken", (long) m_terminalToken);
  config.Write("/TreeDisplay/RootReachable", (long) m_rootReachable);

  config.Write("/TreeDisplay/BranchLength", (long) m_branchLength);
  config.Write("/TreeDisplay/TineLength", (long) m_tineLength);
  config.Write("/TreeDisplay/BranchStyle", (long) m_branchStyle);
  config.Write("/TreeDisplay/BranchLabels", (long) m_branchLabels);

  config.Write("/TreeDisplay/InfosetConnect", (long) m_infosetConnect);
  config.Write("/TreeDisplay/InfosetJoin", (long) m_infosetJoin);

  config.Write("/TreeDisplay/SubgameStyle", (long) m_subgameStyle);

  config.Write("/TreeDisplay/NodeAboveLabel", (long) m_nodeAboveLabel);
  config.Write("/TreeDisplay/NodeBelowLabel", (long) m_nodeBelowLabel);
  config.Write("/TreeDisplay/NodeRightLabel", (long) m_nodeRightLabel);
  config.Write("/TreeDisplay/BranchAboveLabel", (long) m_branchAboveLabel);
  config.Write("/TreeDisplay/BranchBelowLabel", (long) m_branchBelowLabel);

  SaveFont("/TreeDisplay/NodeAboveFont", config, m_nodeAboveFont);
  SaveFont("/TreeDisplay/NodeBelowFont", config, m_nodeBelowFont);
  SaveFont("/TreeDisplay/NodeRightFont", config, m_nodeRightFont);
  SaveFont("/TreeDisplay/BranchAboveFont", config, m_branchAboveFont);
  SaveFont("/TreeDisplay/BranchBelowFont", config, m_branchBelowFont);

  config.Write("/TreeDisplay/NumDecimals", (long) m_numDecimals);
}

void TreeDrawSettings::LoadOptions(void)
{
  wxConfig config("Gambit");
  config.Read("/TreeDisplay/NodeSize", &m_nodeSize, 20);
  config.Read("/TreeDisplay/TerminalSpacing", &m_terminalSpacing, 45);
  config.Read("/TreeDisplay/ChanceToken", &m_chanceToken, 2);
  config.Read("/TreeDisplay/PlayerToken", &m_playerToken, 2);
  config.Read("/TreeDisplay/TerminalToken", &m_terminalToken, 2);
  config.Read("/TreeDisplay/RootReachable", &m_rootReachable, 0);

  config.Read("/TreeDisplay/BranchLength", &m_branchLength, 60);
  config.Read("/TreeDisplay/TineLength", &m_tineLength, 20);
  config.Read("/TreeDisplay/BranchStyle", &m_branchStyle, 0);
  config.Read("/TreeDisplay/BranchLabels", &m_branchLabels, 0);

  config.Read("/TreeDisplay/InfosetConnect", &m_infosetConnect, 2);
  config.Read("/TreeDisplay/InfosetJoin", &m_infosetJoin, 1);

  config.Read("/TreeDisplay/SubgameStyle", &m_subgameStyle, 1);

  config.Read("/TreeDisplay/NodeAboveLabel", &m_nodeAboveLabel, 1);
  config.Read("/TreeDisplay/NodeBelowLabel", &m_nodeBelowLabel, 4);
  config.Read("/TreeDisplay/NodeRightLabel", &m_nodeRightLabel, 1);
  config.Read("/TreeDisplay/BranchAboveLabel", &m_branchAboveLabel, 1);
  config.Read("/TreeDisplay/BranchBelowLabel", &m_branchBelowLabel, 2);

  LoadFont("/TreeDisplay/NodeAboveFont", config, m_nodeAboveFont);
  LoadFont("/TreeDisplay/NodeBelowFont", config, m_nodeBelowFont);
  LoadFont("/TreeDisplay/NodeRightFont", config, m_nodeRightFont);
  LoadFont("/TreeDisplay/BranchAboveFont", config, m_branchAboveFont);
  LoadFont("/TreeDisplay/BranchBelowFont", config, m_branchBelowFont);

  config.Read("/TreeDisplay/NumDecimals", &m_numDecimals, 2);
}

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
  wxString tokenChoices[] = { "Line", "Box", "Circle", "Rounded" };
  m_chanceToken = new wxRadioBox(this, -1, "Chance nodes",
				 wxDefaultPosition, wxDefaultSize,
				 4, tokenChoices);
  m_chanceToken->SetSelection(p_settings.ChanceToken());
  tokenSizer->Add(m_chanceToken, 0, wxALL, 5);

  m_playerToken = new wxRadioBox(this, -1, "Player nodes",
				 wxDefaultPosition, wxDefaultSize,
				 4, tokenChoices);
  m_playerToken->SetSelection(p_settings.PlayerToken());
  tokenSizer->Add(m_playerToken, 0, wxALL, 5);

  m_terminalToken = new wxRadioBox(this, -1, "Terminal nodes",
				   wxDefaultPosition, wxDefaultSize,
				   4, tokenChoices);
  m_terminalToken->SetSelection(p_settings.TerminalToken());
  tokenSizer->Add(m_terminalToken, 0, wxALL, 5);
  topSizer->Add(tokenSizer, 0, wxALL, 5);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
		
  gridSizer->Add(new wxStaticText(this, -1, "Size of nodes"),
		 0, wxCENTER | wxALL, 5);
  m_nodeSize = new wxSpinCtrl(this, -1,
			      wxString::Format("%d", p_settings.NodeSize()),
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS,
			      NODE_LENGTH_MIN, NODE_LENGTH_MAX);
  gridSizer->Add(m_nodeSize, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, -1, "Terminal node spacing"),
		 0, wxCENTER | wxALL, 5);
  m_terminalSpacing =
    new wxSpinCtrl(this, -1,
		   wxString::Format("%d", p_settings.TerminalSpacing()),
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
				 2, styleChoices);
  m_branchStyle->SetSelection(p_settings.BranchStyle());
  styleSizer->Add(m_branchStyle, 0, wxALL, 5);

  wxString labelChoices[] = { "Horizontal", "Rotated" };
  m_branchLabels = new wxRadioBox(this, -1, "Branch labels",
				  wxDefaultPosition, wxDefaultSize,
				  2, labelChoices);
  m_branchLabels->SetSelection(p_settings.BranchLabels());
  styleSizer->Add(m_branchLabels, 0, wxALL, 5);

  topSizer->Add(styleSizer, 0, wxALL | wxCENTER, 5);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(2);
  gridSizer->Add(new wxStaticText(this, -1, "Branch length"),
		 0, wxCENTER | wxALL, 5);
  m_branchLength = new wxSpinCtrl(this, -1,
				  wxString::Format("%d",
						   p_settings.BranchLength()),
				  wxDefaultPosition, wxDefaultSize,
				  wxSP_ARROW_KEYS,
				  BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX);
  gridSizer->Add(m_branchLength, 1, wxEXPAND | wxALL, 5);

  gridSizer->Add(new wxStaticText(this, -1, "Tine length"),
		 0, wxCENTER | wxALL, 5);
  m_tineLength = new wxSpinCtrl(this, -1,
				wxString::Format("%d",
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
				    3, connectChoices);
  m_infosetConnect->SetSelection(p_settings.InfosetConnect());
  styleSizer->Add(m_infosetConnect, 0, wxALL, 5);

  wxString joinChoices[] = { "Lines", "Circles" };
  m_infosetJoin = new wxRadioBox(this, -1, "Join style",
				 wxDefaultPosition, wxDefaultSize,
				 2, joinChoices);
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
				  2, styleChoices);
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
  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(notebookSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
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

//==========================================================================
//                 class dialogLegends: Implementation
//==========================================================================

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
  topSizer->Add(m_buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}


