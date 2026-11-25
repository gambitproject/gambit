//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/style.cc
// Display configuration class for the extensive form
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
// aint with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <algorithm>
#include <map>
#include <sstream>
#include <utility>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "games/gametree.h"
#include "style.h"
#include "gamedoc.h" // for AnalysisWorkspace's complete type -- only needed here, not in style.h

namespace Gambit::GUI {
namespace {

wxString EmptyLabel(const GameNode &, const AnalysisWorkspace &) { return {}; }

wxString NodeLabelText(const GameNode &n, const AnalysisWorkspace &)
{
  return wxString::FromUTF8(n->GetLabel());
}

wxString PlayerLabel(const GameNode &n, const AnalysisWorkspace &)
{
  if (!n->GetPlayer()) {
    return {};
  }
  return wxString::FromUTF8(n->GetPlayer()->GetLabel());
}

wxString InfosetLabel(const GameNode &n, const AnalysisWorkspace &)
{
  if (!n->GetInfoset()) {
    return {};
  }
  return wxString::FromUTF8(n->GetInfoset()->GetLabel());
}

wxString InfosetIdLabel(const GameNode &n, const AnalysisWorkspace &)
{
  if (!n->GetInfoset()) {
    return {};
  }
  wxString label;
  if (n->GetInfoset()->IsChanceInfoset()) {
    label << "C:" << n->GetInfoset()->GetNumber();
  }
  else {
    label << n->GetPlayer()->GetNumber() << ":" << n->GetInfoset()->GetNumber();
  }
  return label;
}

wxString RealizProbLabel(const GameNode &n, const AnalysisWorkspace &p_workspace)
{
  return wxString::FromUTF8(p_workspace.GetProfiles().GetRealizProb(n));
}

wxString BeliefProbLabel(const GameNode &n, const AnalysisWorkspace &p_workspace)
{
  return wxString::FromUTF8(p_workspace.GetProfiles().GetBeliefProb(n));
}

wxString NodeValueLabel(const GameNode &n, const AnalysisWorkspace &p_workspace)
{
  if (!n->GetInfoset() || n->GetPlayer()->GetNumber() <= 0) {
    return {};
  }
  return {p_workspace.GetProfiles().GetNodeValue(n, n->GetPlayer()->GetNumber()).c_str(),
          *wxConvCurrent};
}

using NodeLabelFunction = wxString (*)(const GameNode &, const AnalysisWorkspace &);

const std::map<NodeLabelStyle, NodeLabelFunction> s_nodeLabelFunctions = {
    {GBT_NODE_LABEL_NOTHING, &EmptyLabel},         {GBT_NODE_LABEL_LABEL, &NodeLabelText},
    {GBT_NODE_LABEL_PLAYER, &PlayerLabel},         {GBT_NODE_LABEL_ISETLABEL, &InfosetLabel},
    {GBT_NODE_LABEL_ISETID, &InfosetIdLabel},      {GBT_NODE_LABEL_REALIZPROB, &RealizProbLabel},
    {GBT_NODE_LABEL_BELIEFPROB, &BeliefProbLabel}, {GBT_NODE_LABEL_VALUE, &NodeValueLabel},
};

wxString BranchLabelText(const GameNode &n, const AnalysisWorkspace &)
{
  const GameNode parent = n->GetParent();
  const int childNumber = n->GetPriorAction()->GetNumber();
  return wxString::FromUTF8(parent->GetInfoset()->GetAction(childNumber)->GetLabel());
}

wxString BranchProbLabel(const GameNode &n, const AnalysisWorkspace &p_workspace)
{
  const GameNode parent = n->GetParent();
  const int childNumber = n->GetPriorAction()->GetNumber();
  if (parent->GetPlayer() && parent->GetPlayer()->IsChance()) {
    return {static_cast<std::string>(
                parent->GetInfoset()->GetActionProb(parent->GetInfoset()->GetAction(childNumber)))
                .c_str(),
            *wxConvCurrent};
  }
  if (p_workspace.NumProfileLists() == 0) {
    return {};
  }
  return wxString::FromUTF8(p_workspace.GetProfiles().GetActionProb(parent, childNumber));
}

wxString BranchValueLabel(const GameNode &n, const AnalysisWorkspace &p_workspace)
{
  if (p_workspace.NumProfileLists() == 0) {
    return {};
  }
  const GameNode parent = n->GetParent();
  const int childNumber = n->GetPriorAction()->GetNumber();
  return wxString::FromUTF8(p_workspace.GetProfiles().GetActionValue(parent, childNumber));
}

using BranchLabelFunction = wxString (*)(const GameNode &, const AnalysisWorkspace &);

const std::map<BranchLabelStyle, BranchLabelFunction> s_branchLabelFunctions = {
    {GBT_BRANCH_LABEL_NOTHING, &EmptyLabel},
    {GBT_BRANCH_LABEL_LABEL, &BranchLabelText},
    {GBT_BRANCH_LABEL_PROBS, &BranchProbLabel},
    {GBT_BRANCH_LABEL_VALUE, &BranchValueLabel},
};

} // namespace

LabelGenerator GetNodeLabelGenerator(NodeLabelStyle p_which, const AnalysisWorkspace &p_workspace)
{
  const auto iter = s_nodeLabelFunctions.find(p_which);
  const NodeLabelFunction fn = (iter != s_nodeLabelFunctions.end()) ? iter->second : &EmptyLabel;
  return [fn, &p_workspace](const GameNode &n) { return fn(n, p_workspace); };
}

LabelGenerator GetBranchLabelGenerator(BranchLabelStyle p_which,
                                       const AnalysisWorkspace &p_workspace)
{
  const auto iter = s_branchLabelFunctions.find(p_which);
  const BranchLabelFunction fn =
      (iter != s_branchLabelFunctions.end()) ? iter->second : &EmptyLabel;
  return [fn, &p_workspace](const GameNode &n) { return fn(n, p_workspace); };
}

wxFont GetLabelFont(const TreeStyle &p_style)
{
  const auto &font = p_style.GetFont();
  if (!font) {
    return {10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD};
  }
  return {font->size,
          static_cast<wxFontFamily>(font->family),
          static_cast<wxFontStyle>(font->style),
          static_cast<wxFontWeight>(font->weight),
          false,
          wxString::FromUTF8(font->face)};
}

TreeFont ToTreeFont(const wxFont &p_font)
{
  return {.size = p_font.GetPointSize(),
          .family = static_cast<int>(p_font.GetFamily()),
          .face = p_font.GetFaceName().ToStdString(wxConvUTF8),
          .style = static_cast<int>(p_font.GetStyle()),
          .weight = static_cast<int>(p_font.GetWeight())};
}

wxBitmap MakeColorSwatch(const wxColour &p_color, int p_size)
{
  wxBitmap bitmap(p_size, p_size);
  wxMemoryDC dc(bitmap);
  dc.SetBackground(wxBrush(p_color));
  dc.Clear();
  dc.SelectObject(wxNullBitmap);
  return bitmap;
}

} // namespace Gambit::GUI
