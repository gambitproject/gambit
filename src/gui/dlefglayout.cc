//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#endif // WX_PRECOMP
#include <wx/spinctrl.h>
#include "dlefglayout.h"
#include "dialogstyle.h"

namespace Gambit::GUI {
//==========================================================================
//                  class LayoutDialog: Implementation
//==========================================================================

namespace {

class LayoutNodesPanel final : public wxPanel {
  wxChoice *m_chanceToken, *m_playerToken, *m_terminalToken;
  wxSpinCtrl *m_nodeSize, *m_terminalSpacing;

public:
  LayoutNodesPanel(wxWindow *p_parent, const TreeStyle &p_settings);

  int NodeSize() const { return m_nodeSize->GetValue(); }
  int TerminalSpacing() const { return m_terminalSpacing->GetValue(); }

  NodeTokenStyle GetChanceToken() const
  {
    return static_cast<NodeTokenStyle>(m_chanceToken->GetSelection());
  }
  NodeTokenStyle GetPlayerToken() const
  {
    return static_cast<NodeTokenStyle>(m_playerToken->GetSelection());
  }
  NodeTokenStyle GetTerminalToken() const
  {
    return static_cast<NodeTokenStyle>(m_terminalToken->GetSelection());
  }
};

LayoutNodesPanel::LayoutNodesPanel(wxWindow *p_parent, const TreeStyle &p_settings)
  : wxPanel(p_parent, wxID_ANY)
{
  const int gap = FromDIP(c_spaceRelated);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *nodeSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Drawing nodes"));

  auto *tokenSizer = NewFormSizer(this);

  const wxString tokenChoices[] = {_("a line"), _("a box"), _("an unfilled circle"),
                                   _("a diamond"), _("a filled circle")};

  tokenSizer->Add(new wxStaticText(this, wxID_STATIC, _("Indicate chance nodes with")), 0,
                  wxALIGN_CENTER_VERTICAL);
  m_chanceToken = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, tokenChoices);
  m_chanceToken->SetSelection(p_settings.GetChanceToken());
  tokenSizer->Add(m_chanceToken, 1, wxEXPAND);

  tokenSizer->Add(new wxStaticText(this, wxID_STATIC, _("Indicate player nodes with")), 0,
                  wxALIGN_CENTER_VERTICAL);
  m_playerToken = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, tokenChoices);
  m_playerToken->SetSelection(p_settings.GetPlayerToken());
  tokenSizer->Add(m_playerToken, 1, wxEXPAND);

  tokenSizer->Add(new wxStaticText(this, wxID_STATIC, _("Indicate terminal nodes with")), 0,
                  wxALIGN_CENTER_VERTICAL);
  m_terminalToken =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, tokenChoices);
  m_terminalToken->SetSelection(p_settings.GetTerminalToken());
  tokenSizer->Add(m_terminalToken, 1, wxEXPAND);

  nodeSizer->Add(tokenSizer, 1, wxALL | wxEXPAND, gap);
  topSizer->Add(nodeSizer, 0, wxEXPAND | wxBOTTOM, FromDIP(c_spaceMargin));

  auto *sizeSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Layout sizing"));

  auto *gridSizer = NewFormSizer(this);

  gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Horizontal size of nodes")), 0,
                 wxALIGN_CENTER_VERTICAL);

  {
    constexpr int NODE_LENGTH_MIN = 5;
    constexpr int NODE_LENGTH_MAX = 100;
    wxString label;
    label << p_settings.GetNodeSize();
    m_nodeSize = new wxSpinCtrl(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                                wxSP_ARROW_KEYS, NODE_LENGTH_MIN, NODE_LENGTH_MAX);
    gridSizer->Add(m_nodeSize, 1, wxEXPAND);
  }

  gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Vertical spacing between terminal nodes")), 0,
                 wxALIGN_CENTER_VERTICAL);

  {
    constexpr int Y_SPACING_MIN = 15;
    constexpr int Y_SPACING_MAX = 60;
    wxString label;
    label << p_settings.GetTerminalSpacing();
    m_terminalSpacing = new wxSpinCtrl(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                                       wxSP_ARROW_KEYS, Y_SPACING_MIN, Y_SPACING_MAX);
    gridSizer->Add(m_terminalSpacing, 1, wxEXPAND);
  }

  sizeSizer->Add(gridSizer, 1, wxALL | wxEXPAND, gap);
  topSizer->Add(sizeSizer, 0, wxEXPAND);

  auto *panelSizer = new wxBoxSizer(wxVERTICAL);
  panelSizer->Add(topSizer, 1, wxEXPAND | wxALL, FromDIP(c_spaceMargin));
  SetSizerAndFit(panelSizer);
}

class LayoutBranchesPanel : public wxPanel {
  wxChoice *m_branchStyle, *m_branchLabels;
  wxSpinCtrl *m_branchLength, *m_tineLength;

public:
  LayoutBranchesPanel(wxWindow *p_parent, const TreeStyle &);

  int GetBranchLength() const { return m_branchLength->GetValue(); }
  int GetTineLength() const { return m_tineLength->GetValue(); }

  BranchLineStyle GetBranchStyle() const
  {
    return static_cast<BranchLineStyle>(m_branchStyle->GetSelection());
  }
  BranchLabelOrientationStyle GetBranchLabels() const
  {
    return static_cast<BranchLabelOrientationStyle>(m_branchLabels->GetSelection());
  }
};

LayoutBranchesPanel::LayoutBranchesPanel(wxWindow *p_parent, const TreeStyle &p_settings)
  : wxPanel(p_parent, wxID_ANY)
{
  const int gap = FromDIP(c_spaceRelated);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *styleBoxSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Drawing branches"));

  auto *styleSizer = NewFormSizer(this);

  styleSizer->Add(new wxStaticText(this, wxID_STATIC, _("Draw branches")), 0,
                  wxALIGN_CENTER_VERTICAL);
  wxString styleChoices[] = {_("using straight lines between nodes"),
                             _("with a tine for branch labels")};
  m_branchStyle = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, styleChoices);
  m_branchStyle->SetSelection(p_settings.GetBranchStyle());
  styleSizer->Add(m_branchStyle, 1, wxEXPAND);

  styleSizer->Add(new wxStaticText(this, wxID_STATIC, _("Draw labels")), 1,
                  wxALIGN_CENTER_VERTICAL);
  wxString labelChoices[] = {_("horizontally"), _("rotated parallel to the branch")};
  m_branchLabels = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, labelChoices);
  m_branchLabels->SetSelection(p_settings.GetBranchLabels());
  styleSizer->Add(m_branchLabels, 1, wxEXPAND);

  styleBoxSizer->Add(styleSizer, 1, wxALL | wxEXPAND, gap);
  topSizer->Add(styleBoxSizer, 0, wxEXPAND | wxBOTTOM, FromDIP(c_spaceMargin));

  auto *lengthSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Size of branches"));

  auto *gridSizer = NewFormSizer(this);

  {
    constexpr int BRANCH_LENGTH_MIN = 0;
    constexpr int BRANCH_LENGTH_MAX = 100;
    gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Size of branch fork")), 0,
                   wxALIGN_CENTER_VERTICAL);
    wxString label;
    label << p_settings.GetBranchLength();
    m_branchLength = new wxSpinCtrl(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                                    wxSP_ARROW_KEYS, BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX);
    gridSizer->Add(m_branchLength, 1, wxEXPAND);
  }

  {
    constexpr int TINE_LENGTH_MIN = 20;
    constexpr int TINE_LENGTH_MAX = 100;
    gridSizer->Add(new wxStaticText(this, wxID_ANY, _("Size of branch tine")), 1,
                   wxALIGN_CENTER_VERTICAL);
    wxString label;
    label << p_settings.GetTineLength();
    m_tineLength = new wxSpinCtrl(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                                  wxSP_ARROW_KEYS, TINE_LENGTH_MIN, TINE_LENGTH_MAX);
    gridSizer->Add(m_tineLength, 1, wxEXPAND);
  }

  lengthSizer->Add(gridSizer, 1, wxALL | wxEXPAND, gap);
  topSizer->Add(lengthSizer, 0, wxEXPAND);

  auto *panelSizer = new wxBoxSizer(wxVERTICAL);
  panelSizer->Add(topSizer, 1, wxEXPAND | wxALL, FromDIP(c_spaceMargin));
  SetSizerAndFit(panelSizer);
}

class LayoutInfosetsPanel final : public wxPanel {
  wxChoice *m_infosetJoin;

public:
  LayoutInfosetsPanel(wxWindow *p_parent, const TreeStyle &);

  InfosetJoinStyle GetInfosetJoin() const
  {
    return static_cast<InfosetJoinStyle>(m_infosetJoin->GetSelection());
  }
};

LayoutInfosetsPanel::LayoutInfosetsPanel(wxWindow *p_parent, const TreeStyle &p_settings)
  : wxPanel(p_parent, wxID_ANY)
{
  const int gap = FromDIP(c_spaceRelated);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *infosetSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Drawing information sets"));

  auto *styleSizer = NewFormSizer(this);

  styleSizer->Add(new wxStaticText(this, wxID_STATIC, _("Draw information set connections")), 0,
                  wxALIGN_CENTER_VERTICAL);
  wxString joinChoices[] = {_("using lines"), _("using bubbles")};
  m_infosetJoin = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, joinChoices);
  m_infosetJoin->SetSelection(p_settings.GetInfosetJoin());
  styleSizer->Add(m_infosetJoin, 0, wxEXPAND);
  infosetSizer->Add(styleSizer, 0, wxALL | wxEXPAND, gap);
  topSizer->Add(infosetSizer, 0, wxEXPAND);

  auto *panelSizer = new wxBoxSizer(wxVERTICAL);
  panelSizer->Add(topSizer, 1, wxEXPAND | wxALL, FromDIP(c_spaceMargin));
  SetSizerAndFit(panelSizer);
}

} // anonymous namespace

LayoutDialog::LayoutDialog(wxWindow *p_parent, const TreeStyle &p_settings)
  : wxDialog(p_parent, wxID_ANY, _("Layout options"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
  AddPages(p_settings);

  auto *defaultsButton = new wxButton(this, wxID_ANY, _("Set to defaults"));
  defaultsButton->Bind(wxEVT_BUTTON, &LayoutDialog::OnSetDefaults, this);
  auto *extraSizer = new wxBoxSizer(wxHORIZONTAL);
  extraSizer->Add(defaultsButton);

  auto *contentSizer = new wxBoxSizer(wxVERTICAL);
  contentSizer->Add(m_notebook, 1, wxEXPAND);
  SetDialogLayout(this, contentSizer, wxOK | wxCANCEL, extraSizer);
}

void LayoutDialog::AddPages(const TreeStyle &p_settings)
{
  m_notebook->AddPage(new LayoutNodesPanel(m_notebook, p_settings), _("Nodes"));
  m_notebook->AddPage(new LayoutBranchesPanel(m_notebook, p_settings), _("Branches"));
  m_notebook->AddPage(new LayoutInfosetsPanel(m_notebook, p_settings), _("Information sets"));
}

void LayoutDialog::GetSettings(TreeStyle &p_settings) const
{
  const auto *nodes = dynamic_cast<LayoutNodesPanel *>(m_notebook->GetPage(0));
  p_settings.SetNodeSize(nodes->NodeSize());
  p_settings.SetTerminalSpacing(nodes->TerminalSpacing());
  p_settings.SetChanceToken(nodes->GetChanceToken());
  p_settings.SetPlayerToken(nodes->GetPlayerToken());
  p_settings.SetTerminalToken(nodes->GetTerminalToken());

  const auto *branches = dynamic_cast<LayoutBranchesPanel *>(m_notebook->GetPage(1));
  p_settings.SetBranchLength(branches->GetBranchLength());
  p_settings.SetTineLength(branches->GetTineLength());
  p_settings.SetBranchStyle(branches->GetBranchStyle());
  p_settings.SetBranchLabels(branches->GetBranchLabels());

  const auto *infosets = dynamic_cast<LayoutInfosetsPanel *>(m_notebook->GetPage(2));
  p_settings.SetInfosetJoin(infosets->GetInfosetJoin());
}

void LayoutDialog::OnSetDefaults(wxCommandEvent &)
{
  // The defaults are staged in the controls, like any other setting, until OK
  const int page = m_notebook->GetSelection();
  m_notebook->DeleteAllPages();
  AddPages(TreeStyle());
  m_notebook->SetSelection(page);
}
} // namespace Gambit::GUI
