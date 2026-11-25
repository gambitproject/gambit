//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#endif // WX_PRECOMP

#include "dlefglegend.h"
#include "dialogstyle.h"

namespace Gambit::GUI {
//==========================================================================
//                 class LegendDialog: Implementation
//==========================================================================

LegendDialog::LegendDialog(wxWindow *p_parent, const TreeStyle &p_options)
  : wxDialog(p_parent, wxID_ANY, _("Labels"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  const int gap = FromDIP(c_spaceRelated);

  // A sentence per row: "Display <choice> above each node"
  const auto addRow = [this](wxFlexGridSizer *p_grid, wxChoice *p_choice,
                             const wxString &p_where) {
    p_grid->Add(new wxStaticText(this, wxID_STATIC, _("Display")), 0, wxALIGN_CENTER_VERTICAL);
    p_grid->Add(p_choice, 1, wxEXPAND);
    p_grid->Add(new wxStaticText(this, wxID_STATIC, p_where), 0, wxALIGN_CENTER_VERTICAL);
  };
  const auto newGroup = [this, gap](const wxString &p_title, wxFlexGridSizer *&p_grid) {
    auto *group = new wxStaticBoxSizer(wxVERTICAL, this, p_title);
    p_grid = new wxFlexGridSizer(3, gap, gap);
    p_grid->AddGrowableCol(1, 1);
    group->Add(p_grid, 0, wxEXPAND | wxALL, gap);
    return group;
  };

  const wxString nodeLabelList[] = {_("no label"),
                                    _("the node's label"),
                                    _("the player's name"),
                                    _("the information set's label"),
                                    _("the information set's number"),
                                    _("the realization probability"),
                                    _("the belief probability"),
                                    _("the payoff of reaching the node")};

  wxFlexGridSizer *nodeGrid;
  auto *nodeGroup = newGroup(_("Node labeling"), nodeGrid);
  m_nodeAbove = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 8, nodeLabelList);
  m_nodeAbove->SetSelection(p_options.GetNodeAboveLabel());
  addRow(nodeGrid, m_nodeAbove, _("above each node"));
  m_nodeBelow = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 8, nodeLabelList);
  m_nodeBelow->SetSelection(p_options.GetNodeBelowLabel());
  addRow(nodeGrid, m_nodeBelow, _("below each node"));

  const wxString actionLabelList[] = {_("no label"), _("the name of the action"),
                                      _("the probability the action is played"),
                                      _("the value of the action")};

  wxFlexGridSizer *actionGrid;
  auto *actionGroup = newGroup(_("Action labeling"), actionGrid);
  m_actionAbove =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, actionLabelList);
  m_actionAbove->SetSelection(p_options.GetBranchAboveLabel());
  addRow(actionGrid, m_actionAbove, _("above each action"));
  m_actionBelow =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, actionLabelList);
  m_actionBelow->SetSelection(p_options.GetBranchBelowLabel());
  addRow(actionGrid, m_actionBelow, _("below each action"));

  auto *contentSizer = new wxBoxSizer(wxVERTICAL);
  contentSizer->Add(nodeGroup, 0, wxEXPAND | wxBOTTOM, FromDIP(c_spaceMargin));
  contentSizer->Add(actionGroup, 0, wxEXPAND);

  SetDialogLayout(this, contentSizer, wxOK | wxCANCEL);
}
} // namespace Gambit::GUI
