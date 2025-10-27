//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efglayout.cc
// Implementation of tree layout representation
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

#include <cmath>
#include <algorithm> // for std::min, std::max
#include <numeric>   // for std::partial_sum

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "gambit.h"
#include "efgdisplay.h"

#include "layout.h"

namespace Gambit::GUI {
namespace {

NodeTokenStyle GetTokenForNode(const TreeRenderConfig &p_style, const GameNode &p_node)
{
  if (p_node->IsTerminal()) {
    return p_style.GetTerminalToken();
  }
  if (p_node->GetPlayer()->IsChance()) {
    return p_style.GetChanceToken();
  }
  return p_style.GetPlayerToken();
}

} // namespace

//-----------------------------------------------------------------------
//                   class NodeEntry: Member functions
//-----------------------------------------------------------------------

//
// Draws the node token itself, as well as the incoming branch
// (if not the root node)
//
void TreeLayout::DrawNode(wxDC &p_dc, const std::shared_ptr<NodeEntry> &p_entry,
                          const GameNode &p_selection, bool p_noHints) const
{
  if (p_entry->m_node->GetParent() && p_entry->m_inSupport) {
    DrawIncomingBranch(p_dc, p_entry);
  }
  else {
    p_entry->m_branchAboveRect = wxRect();
    p_entry->m_branchBelowRect = wxRect();
  }

  const auto color = m_doc->GetStyle().GetPlayerColor(p_entry->m_node->GetPlayer());
  p_dc.SetPen(*wxThePenList->FindOrCreatePen(color, (p_selection == p_entry->m_node) ? 6 : 3,
                                             wxPENSTYLE_SOLID));
  p_dc.SetTextForeground(color);

  if (GetTokenForNode(m_doc->GetStyle(), p_entry->m_node) == GBT_NODE_TOKEN_LINE) {
    p_dc.DrawLine(p_entry->m_x, p_entry->m_y, p_entry->m_x + p_entry->m_size, p_entry->m_y);
    if (m_doc->GetStyle().GetBranchStyle() == GBT_BRANCH_STYLE_FORKTINE) {
      // "classic" Gambit style: draw a small 'token' to separate
      // the fork from the node
      p_dc.DrawEllipse(p_entry->m_x - 1, p_entry->m_y - 1, 3, 3);
    }
  }
  else if (GetTokenForNode(m_doc->GetStyle(), p_entry->m_node) == GBT_NODE_TOKEN_BOX) {
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawRectangle(p_entry->m_x, p_entry->m_y - p_entry->m_size / 2, p_entry->m_size,
                       p_entry->m_size);
  }
  else if (GetTokenForNode(m_doc->GetStyle(), p_entry->m_node) == GBT_NODE_TOKEN_DIAMOND) {
    wxPoint points[4] = {
        wxPoint(p_entry->m_x + p_entry->m_size / 2, p_entry->m_y - p_entry->m_size / 2),
        wxPoint(p_entry->m_x, p_entry->m_y),
        wxPoint(p_entry->m_x + p_entry->m_size / 2, p_entry->m_y + p_entry->m_size / 2),
        wxPoint(p_entry->m_x + p_entry->m_size, p_entry->m_y)};
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawPolygon(4, points);
  }
  else if (GetTokenForNode(m_doc->GetStyle(), p_entry->m_node) == GBT_NODE_TOKEN_DOT) {
    p_dc.SetBrush(wxBrush(m_doc->GetStyle().GetPlayerColor(p_entry->m_node->GetPlayer()),
                          wxBRUSHSTYLE_SOLID));
    p_dc.DrawEllipse(p_entry->m_x, p_entry->m_y - p_entry->m_size / 2, p_entry->m_size,
                     p_entry->m_size);
  }
  else {
    // Default: draw circles
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawEllipse(p_entry->m_x, p_entry->m_y - p_entry->m_size / 2, p_entry->m_size,
                     p_entry->m_size);
  }

  int textWidth, textHeight;
  p_dc.SetFont(m_doc->GetStyle().GetFont());
  p_dc.GetTextExtent(p_entry->m_nodeAboveLabel, &textWidth, &textHeight);
  p_dc.DrawText(p_entry->m_nodeAboveLabel, p_entry->m_x + (p_entry->m_size - textWidth) / 2,
                p_entry->m_y - textHeight - 9);
  p_dc.SetFont(m_doc->GetStyle().GetFont());
  p_dc.GetTextExtent(p_entry->m_nodeBelowLabel, &textWidth, &textHeight);
  p_dc.DrawText(p_entry->m_nodeBelowLabel, p_entry->m_x + (p_entry->m_size - textWidth) / 2,
                p_entry->m_y + 9);

  p_dc.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  DrawOutcome(p_dc, p_entry, p_noHints);
}

void TreeLayout::DrawIncomingBranch(wxDC &p_dc, const std::shared_ptr<NodeEntry> &p_entry) const
{
  const int xStart = p_entry->m_parent->m_x + p_entry->m_parent->m_size;
  const int xEnd = p_entry->m_x;
  const int yStart = p_entry->m_parent->m_y;
  const int yEnd = p_entry->m_y;

  const auto color = m_doc->GetStyle().GetPlayerColor(p_entry->m_node->GetParent()->GetPlayer());
  p_dc.SetPen(*wxThePenList->FindOrCreatePen(color, 4, wxPENSTYLE_SOLID));
  p_dc.SetTextForeground(color);

  if (m_doc->GetStyle().GetBranchStyle() == GBT_BRANCH_STYLE_LINE) {
    p_dc.DrawLine(xStart, yStart, xEnd, yEnd);

    // Draw in the highlight indicating action probabilities
    if (p_entry->m_actionProb >= 0.0) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 4, wxPENSTYLE_SOLID));
      p_dc.DrawLine(xStart, yStart, xStart + (xEnd - xStart) * p_entry->m_actionProb,
                    yStart + (yEnd - yStart) * p_entry->m_actionProb);
    }

    int textWidth, textHeight;
    p_dc.SetFont(m_doc->GetStyle().GetFont());
    p_dc.GetTextExtent(p_entry->m_branchAboveLabel, &textWidth, &textHeight);

    // The angle of the branch
    const double theta =
        -std::atan(static_cast<double>(yEnd - yStart) / static_cast<double>(xEnd - xStart));
    // The "centerpoint" of the branch
    const int xbar = (xStart + xEnd) / 2;
    const int ybar = (yStart + yEnd) / 2;

    if (m_doc->GetStyle().GetBranchLabels() == GBT_BRANCH_LABEL_ORIENT_HORIZONTAL) {
      if (yStart >= yEnd) {
        p_dc.DrawText(p_entry->m_branchAboveLabel, xbar - textWidth / 2,
                      ybar - textHeight + textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
        p_entry->m_branchAboveRect =
            wxRect(xbar - textWidth / 2,
                   ybar - textHeight + textWidth / 2 * (yEnd - yStart) / (xEnd - xStart),
                   textWidth, textHeight);
      }
      else {
        p_dc.DrawText(p_entry->m_branchAboveLabel, xbar - textWidth / 2,
                      ybar - textHeight - textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
        p_entry->m_branchAboveRect =
            wxRect(xbar - textWidth / 2,
                   ybar - textHeight - textWidth / 2 * (yEnd - yStart) / (xEnd - xStart),
                   textWidth, textHeight);
      }
    }
    else {
      // Draw the text rotated appropriately
      p_dc.DrawRotatedText(
          p_entry->m_branchAboveLabel,
          (xbar - textHeight * std::sin(theta) - textWidth * std::cos(theta) / 2.0),
          (ybar - textHeight * std::cos(theta) + textWidth * std::sin(theta) / 2.0),
          theta * 180.0 / 3.14159);
      p_entry->m_branchAboveRect = wxRect();
    }

    p_dc.SetFont(m_doc->GetStyle().GetFont());
    p_dc.GetTextExtent(p_entry->m_branchBelowLabel, &textWidth, &textHeight);

    if (m_doc->GetStyle().GetBranchLabels() == GBT_BRANCH_LABEL_ORIENT_HORIZONTAL) {
      if (yStart >= yEnd) {
        p_dc.DrawText(p_entry->m_branchBelowLabel, xbar - textWidth / 2,
                      ybar - textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
        p_entry->m_branchBelowRect =
            wxRect(xbar - textWidth / 2, ybar - textWidth / 2 * (yEnd - yStart) / (xEnd - xStart),
                   textWidth, textHeight);
      }
      else {
        p_dc.DrawText(p_entry->m_branchBelowLabel, xbar - textWidth / 2,
                      ybar + textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
        p_entry->m_branchBelowRect =
            wxRect(xbar - textWidth / 2, ybar + textWidth / 2 * (yEnd - yStart) / (xEnd - xStart),
                   textWidth, textHeight);
      }
    }
    else {
      // Draw the text rotated appropriately
      p_dc.DrawRotatedText(p_entry->m_branchBelowLabel, (xbar - textWidth * std::cos(theta) / 2.0),
                           (ybar + textWidth * std::sin(theta) / 2.0), theta * 180.0 / 3.14159);
      p_entry->m_branchBelowRect = wxRect();
    }
  }
  else {
    // Old style fork-tine
    p_dc.DrawLine(xStart, yStart, xStart + p_entry->m_branchLength, yEnd);
    p_dc.DrawLine(xStart + p_entry->m_branchLength, yEnd, xEnd, yEnd);

    // Draw in the highlight indicating action probabilities
    if (p_entry->m_actionProb >= 0.0) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxPENSTYLE_SOLID));
      p_dc.DrawLine(xStart, yStart, xStart + p_entry->m_branchLength * p_entry->m_actionProb,
                    yStart + (yEnd - yStart) * p_entry->m_actionProb);
    }

    int textWidth, textHeight;
    p_dc.SetFont(m_doc->GetStyle().GetFont());
    p_dc.GetTextExtent(p_entry->m_branchAboveLabel, &textWidth, &textHeight);
    p_dc.DrawText(p_entry->m_branchAboveLabel, xStart + p_entry->m_branchLength + 3,
                  yEnd - textHeight - 3);
    p_entry->m_branchAboveRect =
        wxRect(xStart + p_entry->m_branchLength + 3, yEnd - textHeight - 3, textWidth, textHeight);

    p_dc.SetFont(m_doc->GetStyle().GetFont());
    p_dc.GetTextExtent(p_entry->m_branchBelowLabel, &textWidth, &textHeight);
    p_dc.DrawText(p_entry->m_branchBelowLabel, xStart + p_entry->m_branchLength + 3, yEnd + 3);
    p_entry->m_branchBelowRect =
        wxRect(xStart + p_entry->m_branchLength + 3, yEnd + +3, textWidth, textHeight);
  }
}

static wxPoint DrawFraction(wxDC &p_dc, wxPoint p_point, const Rational &p_value)
{
  p_dc.SetFont(wxFont(7, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

  int numWidth, numHeight;
  const wxString num =
      wxString(lexical_cast<std::string>(p_value.numerator()).c_str(), *wxConvCurrent);
  p_dc.GetTextExtent(num, &numWidth, &numHeight);

  int denWidth, denHeight;
  const wxString den =
      wxString(lexical_cast<std::string>(p_value.denominator()).c_str(), *wxConvCurrent);
  p_dc.GetTextExtent(den, &denWidth, &denHeight);

  const int width = ((numWidth > denWidth) ? numWidth : denWidth);

  p_dc.DrawLine(p_point.x, p_point.y, p_point.x + width + 4, p_point.y);
  p_dc.DrawText(num, p_point.x + 2 + (width - numWidth) / 2, p_point.y - 2 - numHeight);
  p_dc.DrawText(den, p_point.x + 2 + (width - denWidth) / 2, p_point.y + 2);

  p_point.x += width + 14;
  return p_point;
}

void TreeLayout::DrawOutcome(wxDC &p_dc, const std::shared_ptr<NodeEntry> &p_entry,
                             bool p_noHints) const
{
  wxPoint point(p_entry->m_x + p_entry->m_size + 20, p_entry->m_y);

  const GameOutcome outcome = p_entry->m_node->GetOutcome();
  if (!outcome) {
    if (p_noHints) {
      return;
    }

    int width, height;
    p_dc.SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD));
    p_dc.SetTextForeground(*wxLIGHT_GREY);
    p_dc.GetTextExtent(wxT("(u)"), &width, &height);
    p_dc.DrawText(wxT("(u)"), point.x, point.y - height / 2);
    p_entry->m_outcomeRect = wxRect(point.x, point.y - height / 2, width, height);
    p_entry->m_payoffRect = Array<wxRect>();
    return;
  }

  int width, height = 25;
  p_entry->m_payoffRect = Array<wxRect>();
  for (const auto &player : p_entry->m_node->GetGame()->GetPlayers()) {
    p_dc.SetTextForeground(m_doc->GetStyle().GetPlayerColor(player));

    const auto &payoff = outcome->GetPayoff<std::string>(player);

    if (payoff.find('/') != std::string::npos) {
      p_dc.SetPen(wxPen(m_doc->GetStyle().GetPlayerColor(player), 1, wxPENSTYLE_SOLID));
      const int oldX = point.x;
      point = DrawFraction(p_dc, point, outcome->GetPayoff<Rational>(player));
      p_entry->m_payoffRect.push_back(
          wxRect(oldX - 5, point.y - height / 2, point.x - oldX + 10, height));
    }
    else {
      const wxString label = wxString(payoff.c_str(), *wxConvCurrent);
      p_dc.SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
      p_dc.GetTextExtent(label, &width, &height);
      p_dc.DrawText(label, point.x, point.y - height / 2);
      p_entry->m_payoffRect.push_back(
          wxRect(point.x - 5, point.y - height / 2, width + 10, height));
      point.x += width + 10;
    }
  }

  if (height == 0) {
    // Happens if all payoffs are fractional
    height = 25;
  }

  p_entry->m_outcomeRect = wxRect(p_entry->m_x + p_entry->m_size + 20, p_entry->m_y - height / 2,
                                  point.x - (p_entry->m_x + p_entry->m_size + 20), height);
}

bool TreeLayout::NodeHitTest(const std::shared_ptr<NodeEntry> &p_entry, const int p_x,
                             const int p_y) const
{
  if (p_x < p_entry->m_x || p_x >= p_entry->m_x + p_entry->m_size) {
    return false;
  }

  if (GetTokenForNode(m_doc->GetStyle(), p_entry->m_node) == GBT_NODE_TOKEN_LINE) {
    constexpr int DELTA = 8; // a fudge factor for "almost" hitting the node
    return (p_y >= p_entry->m_y - DELTA && p_y <= p_entry->m_y + DELTA);
  }
  return (p_y >= p_entry->m_y - p_entry->m_size / 2 && p_y <= p_entry->m_y + p_entry->m_size / 2);
}

//-----------------------------------------------------------------------
//                class TreeLayout: Member functions
//-----------------------------------------------------------------------

GameNode TreeLayout::NodeHitTest(int p_x, int p_y) const
{
  const auto hit =
      std::find_if(m_nodeList.begin(), m_nodeList.end(),
                   [this, p_x, p_y](const std::shared_ptr<NodeEntry> &p_entry) -> bool {
                     return NodeHitTest(p_entry, p_x, p_y);
                   });
  return (hit != m_nodeList.end()) ? (*hit)->GetNode() : nullptr;
}

GameNode TreeLayout::OutcomeHitTest(int p_x, int p_y) const
{
  const auto hit =
      std::find_if(m_nodeList.begin(), m_nodeList.end(),
                   [this, p_x, p_y](const std::shared_ptr<NodeEntry> &p_entry) -> bool {
                     return p_entry->OutcomeHitTest(p_x, p_y);
                   });
  return (hit != m_nodeList.end()) ? (*hit)->GetNode() : nullptr;
}

GameNode TreeLayout::BranchAboveHitTest(int p_x, int p_y) const
{
  const auto hit =
      std::find_if(m_nodeList.begin(), m_nodeList.end(),
                   [this, p_x, p_y](const std::shared_ptr<NodeEntry> &p_entry) -> bool {
                     return p_entry->BranchAboveHitTest(p_x, p_y);
                   });
  return (hit != m_nodeList.end()) ? (*hit)->GetNode()->GetParent() : nullptr;
}

GameNode TreeLayout::BranchBelowHitTest(int p_x, int p_y) const
{
  const auto hit =
      std::find_if(m_nodeList.begin(), m_nodeList.end(),
                   [this, p_x, p_y](const std::shared_ptr<NodeEntry> &p_entry) -> bool {
                     return p_entry->BranchBelowHitTest(p_x, p_y);
                   });
  return (hit != m_nodeList.end()) ? (*hit)->GetNode()->GetParent() : nullptr;
}

bool TreeLayout::InfosetHitTest(const std::shared_ptr<NodeEntry> &p_entry, const int p_x,
                                const int p_y) const
{
  if (p_entry->GetNextMember() && p_entry->GetNode()->GetInfoset()) {
    if (p_x > p_entry->m_x + p_entry->GetSublevel() * m_infosetSpacing - 2 &&
        p_x < p_entry->m_x + p_entry->GetSublevel() * m_infosetSpacing + 2) {
      if (p_y > p_entry->m_y && p_y < p_entry->GetNextMember()->m_y) {
        // next infoset is below this one
        return true;
      }
      if (p_y > p_entry->GetNextMember()->m_y && p_y < p_entry->m_y) {
        // next infoset is above this one
        return true;
      }
    }
  }
  return false;
}

GameNode TreeLayout::InfosetHitTest(int p_x, int p_y) const
{
  const auto hit =
      std::find_if(m_nodeList.begin(), m_nodeList.end(),
                   [this, p_x, p_y](const std::shared_ptr<NodeEntry> &p_entry) -> bool {
                     return InfosetHitTest(p_entry, p_x, p_y);
                   });
  return (hit != m_nodeList.end()) ? (*hit)->GetNode() : nullptr;
}

wxString TreeLayout::CreateNodeLabel(const std::shared_ptr<NodeEntry> &p_entry, int p_which) const
{
  const GameNode n = p_entry->GetNode();

  try {
    switch (p_which) {
    case GBT_NODE_LABEL_NOTHING:
      return wxT("");
    case GBT_NODE_LABEL_LABEL:
      return {n->GetLabel().c_str(), *wxConvCurrent};
    case GBT_NODE_LABEL_PLAYER:
      if (n->GetPlayer()) {
        return {n->GetPlayer()->GetLabel().c_str(), *wxConvCurrent};
      }
      else {
        return wxT("");
      }
    case GBT_NODE_LABEL_ISETLABEL:
      if (n->GetInfoset()) {
        return {n->GetInfoset()->GetLabel().c_str(), *wxConvCurrent};
      }
      else {
        return wxT("");
      }
    case GBT_NODE_LABEL_ISETID:
      if (n->GetInfoset()) {
        if (n->GetInfoset()->IsChanceInfoset()) {
          return wxString::Format(wxT("C:%d"), n->GetInfoset()->GetNumber());
        }
        else {
          return wxString::Format(wxT("%d:%d"), n->GetPlayer()->GetNumber(),
                                  n->GetInfoset()->GetNumber());
        }
      }
      else {
        return _T("");
      }
    case GBT_NODE_LABEL_REALIZPROB:
      return {m_doc->GetProfiles().GetRealizProb(n).c_str(), *wxConvCurrent};
    case GBT_NODE_LABEL_BELIEFPROB:
      return {m_doc->GetProfiles().GetBeliefProb(n).c_str(), *wxConvCurrent};
    case GBT_NODE_LABEL_VALUE:
      if (n->GetInfoset() && n->GetPlayer()->GetNumber() > 0) {
        return {m_doc->GetProfiles().GetNodeValue(n, n->GetPlayer()->GetNumber()).c_str(),
                *wxConvCurrent};
      }
      else {
        return wxT("");
      }
    default:
      return wxT("");
    }
  }
  catch (...) {
    return wxT("");
  }
}

wxString TreeLayout::CreateBranchLabel(const std::shared_ptr<NodeEntry> &p_entry,
                                       int p_which) const
{
  const GameNode parent = p_entry->GetParent()->GetNode();

  try {
    switch (p_which) {
    case GBT_BRANCH_LABEL_NOTHING:
      return wxT("");
    case GBT_BRANCH_LABEL_LABEL:
      return {parent->GetInfoset()->GetAction(p_entry->GetChildNumber())->GetLabel().c_str(),
              *wxConvCurrent};
    case GBT_BRANCH_LABEL_PROBS:
      if (parent->GetPlayer() && parent->GetPlayer()->IsChance()) {
        return {static_cast<std::string>(
                    parent->GetInfoset()->GetActionProb(
                        parent->GetInfoset()->GetAction(p_entry->GetChildNumber())))
                    .c_str(),
                *wxConvCurrent};
      }
      else if (m_doc->NumProfileLists() == 0) {
        return wxT("");
      }
      else {
        return {m_doc->GetProfiles().GetActionProb(parent, p_entry->GetChildNumber()).c_str(),
                *wxConvCurrent};
      }
    case GBT_BRANCH_LABEL_VALUE:
      if (m_doc->NumProfileLists() == 0) {
        return wxT("");
      }
      else {
        return {m_doc->GetProfiles().GetActionValue(parent, p_entry->GetChildNumber()).c_str(),
                *wxConvCurrent};
      }
    default:
      return wxT("");
    }
  }
  catch (...) {
    return wxT("");
  }
}

std::shared_ptr<NodeEntry> TreeLayout::GetRenderedAncestor(const GameNode &p_node) const
{
  auto n = p_node;
  auto entry = GetNodeEntry(n->GetParent());
  while (!entry) {
    n = n->GetParent();
    entry = GetNodeEntry(n->GetParent());
  }
  return entry;
}

std::shared_ptr<NodeEntry> TreeLayout::GetRenderedDescendant(const GameNode &p_node) const
{
  for (const auto &child : p_node->GetChildren()) {
    auto n = GetNodeEntry(child);
    if (n) {
      return n;
    }
    n = GetRenderedDescendant(child);
    if (n) {
      return n;
    }
  }
  return nullptr;
}

GameNode TreeLayout::PriorSameLevel(const GameNode &p_node) const
{
  if (auto entry = GetNodeEntry(p_node)) {
    auto e = std::next(std::find(m_nodeList.rbegin(), m_nodeList.rend(), entry));
    while (e != m_nodeList.rend()) {
      if ((*e)->GetLevel() == entry->GetLevel()) {
        return (*e)->GetNode();
      }
      --e;
    }
  }
  return nullptr;
}

GameNode TreeLayout::NextSameLevel(const GameNode &p_node) const
{
  if (auto entry = GetNodeEntry(p_node)) {
    auto e = std::next(std::find(m_nodeList.begin(), m_nodeList.end(), entry));
    while (e != m_nodeList.end()) {
      if ((*e)->GetLevel() == entry->GetLevel()) {
        return (*e)->GetNode();
      }
      ++e;
    }
  }
  return nullptr;
}

void TreeLayout::ComputeOffsets(const GameNode &p_node, const BehaviorSupportProfile &p_support,
                                int &p_ycoord)
{
  const auto &settings = m_doc->GetStyle();

  const auto entry = GetNodeEntry(p_node);
  if (m_doc->GetStyle().RootReachable() && p_node->GetInfoset() &&
      !p_node->GetInfoset()->GetPlayer()->IsChance()) {
    const auto actions = p_support.GetActions(p_node->GetInfoset());
    for (const auto &action : actions) {
      ComputeOffsets(p_node->GetChild(action), p_support, p_ycoord);
    }
    entry->m_y = (GetNodeEntry(p_node->GetChild(actions.front()))->m_y +
                  GetNodeEntry(p_node->GetChild(actions.back()))->m_y) /
                 2;
  }
  else if (!p_node->IsTerminal()) {
    const auto actions = p_node->GetInfoset()->GetActions();
    for (const auto &action : actions) {
      const auto child = p_node->GetChild(action);
      ComputeOffsets(child, p_support, p_ycoord);
      if (!p_node->GetPlayer()->IsChance() && !p_support.Contains(action)) {
        GetNodeEntry(child)->m_inSupport = false;
      }
    }
    entry->m_y = (GetNodeEntry(p_node->GetChild(actions.front()))->m_y +
                  GetNodeEntry(p_node->GetChild(actions.back()))->m_y) /
                 2;
  }
  else {
    entry->m_y = p_ycoord;
    p_ycoord += settings.TerminalSpacing();
  }
}

std::shared_ptr<NodeEntry>
TreeLayout::ComputeNextInInfoset(const std::shared_ptr<NodeEntry> &p_entry)
{
  const auto infoset = p_entry->m_node->GetInfoset();
  if (!infoset) {
    // For terminal nodes, the next in information set is always the next terminal node,
    // irrespective of draw settings
    auto entry = std::next(std::find(m_nodeList.begin(), m_nodeList.end(), p_entry));
    while (entry != m_nodeList.end()) {
      if ((*entry)->m_node->IsTerminal()) {
        return *entry;
      }
      ++entry;
    }
    return nullptr;
  }
  auto member = std::next(
      std::find(infoset->GetMembers().begin(), infoset->GetMembers().end(), p_entry->m_node));
  while (member != infoset->GetMembers().end()) {
    auto member_entry = GetNodeEntry(*member);
    if (member_entry != nullptr && p_entry->m_level == member_entry->m_level) {
      return member_entry;
    }
    ++member;
  }
  return nullptr;
}

void TreeLayout::ComputeSublevel(const std::shared_ptr<NodeEntry> &p_entry)
{
  /*
  try {
    p_entry->m_sublevel = m_infosetSublevels.at({p_entry->m_level, p_entry->m_node->GetInfoset()});
  }
  catch (std::out_of_range &) {
    p_entry->m_sublevel = ++m_numSublevels[p_entry->m_level];
    m_infosetSublevels[{p_entry->m_level, p_entry->m_node->GetInfoset()}] = p_entry->m_sublevel;
  }
  */
  p_entry->m_nextMember = ComputeNextInInfoset(p_entry);
}

void TreeLayout::ComputeNodeDepths(const Gambit::Layout &p_layout) const
{
  std::vector<int> aggregateSublevels(m_maxLevel + 1);
  std::partial_sum(p_layout.GetNumSublevels().cbegin(), p_layout.GetNumSublevels().cend(),
                   aggregateSublevels.begin());
  m_maxX = 0;
  for (const auto &entry : m_nodeList) {
    entry->m_x = c_leftMargin + entry->m_level * m_doc->GetStyle().GetNodeLevelLength();
    if (entry->m_level != 0) {
      entry->m_x +=
          (aggregateSublevels[entry->m_level - 1] + entry->m_sublevel) * m_infosetSpacing;
    }
    m_maxX = std::max(m_maxX, entry->m_x + entry->m_size);
  }
}

void TreeLayout::ComputeRenderedParents() const
{
  for (const auto &e : m_nodeList) {
    e->m_parent = (e->m_node == m_doc->GetGame()->GetRoot()) ? e : GetRenderedAncestor(e->m_node);
  }
}

void TreeLayout::BuildNodeList(const GameNode &p_node, const BehaviorSupportProfile &p_support,
                               const int p_level)
{
  const auto entry = std::make_shared<NodeEntry>(p_node);
  m_nodeList.push_back(entry);
  m_nodeMap[p_node] = entry;
  entry->m_size = m_doc->GetStyle().GetNodeSize();
  entry->m_branchLength = m_doc->GetStyle().GetBranchLength();
  entry->m_level = p_level;
  if (m_doc->GetStyle().RootReachable()) {
    if (const GameInfoset infoset = p_node->GetInfoset()) {
      if (infoset->GetPlayer()->IsChance()) {
        for (const auto &child : p_node->GetChildren()) {
          BuildNodeList(child, p_support, p_level + 1);
        }
      }
      else {
        for (const auto &action : p_support.GetActions(infoset)) {
          BuildNodeList(p_node->GetChild(action), p_support, p_level + 1);
        }
      }
    }
  }
  else {
    for (const auto &child : p_node->GetChildren()) {
      BuildNodeList(child, p_support, p_level + 1);
    }
  }
  m_maxLevel = std::max(p_level, m_maxLevel);
}

void TreeLayout::BuildNodeList(const BehaviorSupportProfile &p_support)
{
  m_nodeList.clear();
  m_nodeMap.clear();
  m_maxLevel = 0;
  BuildNodeList(m_doc->GetGame()->GetRoot(), p_support, 0);
}

void TreeLayout::Layout(const BehaviorSupportProfile &p_support)
{
  m_infosetSpacing = (m_doc->GetStyle().GetInfosetJoin() == GBT_INFOSET_JOIN_LINES) ? 10 : 40;

  if (m_nodeList.size() != m_doc->GetGame()->NumNodes()) {
    // We only rebuild the node list if the number of nodes changes.  If we only have
    // information set changes this can be handled just by the traversal below
    BuildNodeList(p_support);
  }

  int maxy = c_topMargin;
  ComputeOffsets(m_doc->GetGame()->GetRoot(), p_support, maxy);
  m_maxY = maxy + c_bottomMargin;

  auto layout = Gambit::Layout(m_doc->GetGame());
  layout.LayoutTree(p_support);

  for (auto [node, entry] : layout.GetNodeMap()) {
    m_nodeMap[node]->m_level = entry->m_level;
    m_nodeMap[node]->m_sublevel = entry->m_sublevel;
  }

  m_infosetSublevels.clear();
  m_numSublevels = std::vector<int>(m_maxLevel + 1);
  for (auto entry : m_nodeList) {
    ComputeSublevel(entry);
  }
  ComputeNodeDepths(layout);

  ComputeRenderedParents();
  GenerateLabels();
}

void TreeLayout::GenerateLabels() const
{
  const TreeRenderConfig &settings = m_doc->GetStyle();
  for (const auto &entry : m_nodeList) {
    entry->SetNodeAboveLabel(CreateNodeLabel(entry, settings.GetNodeAboveLabel()));
    entry->SetNodeBelowLabel(CreateNodeLabel(entry, settings.GetNodeBelowLabel()));
    if (entry->GetChildNumber() > 0) {
      entry->SetBranchAboveLabel(CreateBranchLabel(entry, settings.GetBranchAboveLabel()));
      entry->SetBranchBelowLabel(CreateBranchLabel(entry, settings.GetBranchBelowLabel()));

      const GameNode parent = entry->GetNode()->GetParent();
      if (parent->GetPlayer()->IsChance()) {
        entry->SetActionProb(static_cast<double>(parent->GetInfoset()->GetActionProb(
            parent->GetInfoset()->GetAction(entry->GetChildNumber()))));
      }
      else {
        const int profile = m_doc->GetCurrentProfile();
        if (profile > 0) {
          try {
            entry->SetActionProb((double)lexical_cast<Rational>(
                m_doc->GetProfiles().GetActionProb(parent, entry->GetChildNumber())));
          }
          catch (ValueException &) {
            // This occurs when the probability is undefined
            entry->SetActionProb(0.0);
          }
        }
      }
    }
  }
}

//
// RenderSubtree: Render branches and labels
//
// The following speed optimizations have been added:
// The algorithm now traverses the tree as a linear linked list, eliminating
// expensive searches.
//
// There was some clipping code in here, but it didn't correctly
// deal with drawing information sets while scrolling.  So, the code
// has temporarily been removed.  It remains to be seen whether
// performance will require a more sophisticated solution to the
// problem.  (TLT 5/2001)
//
void TreeLayout::RenderSubtree(wxDC &p_dc, bool p_noHints) const
{
  const TreeRenderConfig &settings = m_doc->GetStyle();

  for (const auto &entry : m_nodeList) {
    auto parentEntry = entry->GetParent();

    if (entry->GetChildNumber() == 1) {
      DrawNode(p_dc, parentEntry, m_doc->GetSelectNode(), p_noHints);

      if (parentEntry->GetNextMember()) {
        const int nextX = parentEntry->GetNextMember()->m_x;
        const int nextY = parentEntry->GetNextMember()->m_y;

        if (parentEntry->m_x == nextX) {
#ifdef __WXGTK__
          // A problem with using styled pens and user scaling on wxGTK
          p_dc.SetPen(wxPen(m_doc->GetStyle().GetPlayerColor(parentEntry->m_node->GetPlayer()), 1,
                            wxPENSTYLE_SOLID));
#else
          p_dc.SetPen(wxPen(m_doc->GetStyle().GetPlayerColor(parentEntry->m_node->GetPlayer()), 1,
                            wxPENSTYLE_DOT));
#endif // __WXGTK__
          p_dc.DrawLine(parentEntry->m_x, parentEntry->m_y, parentEntry->m_x, nextY);
          if (settings.GetInfosetJoin() == GBT_INFOSET_JOIN_CIRCLES) {
            p_dc.DrawLine(parentEntry->m_x + parentEntry->GetSize(), parentEntry->m_y,
                          parentEntry->m_x + parentEntry->GetSize(), nextY);
          }

          if (parentEntry->GetNextMember()->m_x != parentEntry->m_x) {
            // Draw a little arrow in the direction of the iset.
            int startX, endX;
            if (settings.GetInfosetJoin() == GBT_INFOSET_JOIN_LINES) {
              startX = parentEntry->m_x;
              endX =
                  (startX + m_infosetSpacing *
                                ((parentEntry->GetNextMember()->m_x > parentEntry->m_x) ? 1 : -1));
            }
            else {
              if (parentEntry->GetNextMember()->m_x < parentEntry->m_x) {
                // information set is continued to the left
                startX = parentEntry->m_x + parentEntry->GetSize();
                endX = parentEntry->m_x - m_infosetSpacing;
              }
              else {
                // information set is continued to the right
                startX = parentEntry->m_x;
                endX = (parentEntry->m_x + parentEntry->GetSize() + m_infosetSpacing);
              }
            }
            p_dc.DrawLine(startX, nextY, endX, nextY);
            if (startX > endX) {
              p_dc.DrawLine(endX, nextY, endX + m_infosetSpacing / 2,
                            nextY + m_infosetSpacing / 2);
              p_dc.DrawLine(endX, nextY, endX + m_infosetSpacing / 2,
                            nextY - m_infosetSpacing / 2);
            }
            else {
              p_dc.DrawLine(endX, nextY, endX - m_infosetSpacing / 2,
                            nextY + m_infosetSpacing / 2);
              p_dc.DrawLine(endX, nextY, endX - m_infosetSpacing / 2,
                            nextY - m_infosetSpacing / 2);
            }
          }
        }
      }
    }

    if (entry->GetNode()->IsTerminal()) {
      DrawNode(p_dc, entry, m_doc->GetSelectNode(), p_noHints);
    }

    // As we draw, we determine the outcome label extents.  Adjust the
    // overall size of the plot accordingly.
    if (entry->GetOutcomeExtent().GetRight() > m_maxX) {
      m_maxX = entry->GetOutcomeExtent().GetRight();
    }
  }
}

void TreeLayout::Render(wxDC &p_dc, bool p_noHints) const { RenderSubtree(p_dc, p_noHints); }
} // namespace Gambit::GUI
