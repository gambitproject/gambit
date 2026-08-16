//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

#include "efglayout.h"

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

int ComputeLevelProportionalX(const std::list<std::shared_ptr<NodeEntry>> &p_nodeList,
                              const Gambit::Layout &p_layout, int p_leftMargin, int p_levelLength,
                              int p_infosetSpacing, int p_nodeSize)
{
  std::vector<int> aggregateSublevels;
  std::partial_sum(p_layout.GetNumSublevels().cbegin(), p_layout.GetNumSublevels().cend(),
                   std::back_inserter(aggregateSublevels));
  int maxX = 0;
  for (const auto &entry : p_nodeList) {
    int x = p_leftMargin + entry->GetLevel() * p_levelLength;
    if (entry->GetLevel() != 0) {
      x += (aggregateSublevels[entry->GetLevel() - 1] + entry->GetSublevel()) * p_infosetSpacing;
    }
    entry->SetX(x);
    maxX = std::max(maxX, x + p_nodeSize);
  }
  return maxX;
}

// Label generators can throw (e.g. querying a profile value that isn't
// computed yet); a missing label should just render as blank, not crash.
wxString SafeGenerate(const LabelGenerator &p_generator, const GameNode &p_node)
{
  try {
    return p_generator(p_node);
  }
  catch (...) {
    return wxT("");
  }
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
  const int nodeSize = m_doc->GetStyle().GetNodeSize();
  const NodeTokenStyle tokenStyle = GetTokenForNode(m_doc->GetStyle(), p_entry->m_node);
  const bool isLine = (tokenStyle == GBT_NODE_TOKEN_LINE);
  // For every style but "line", this is exactly the shape drawn below --
  // computed once by ComputeTokenGeometry, not recomputed here, so drawing
  // and hit-testing (NodeEntry::NodeHitTest) can never silently disagree.
  // "Line" has no real width of its own, so its selection ring keeps its own
  // (differently-sized) padding rather than reusing the hit-test region.
  const wxRect &token = p_entry->m_geometry.token;

  if (p_entry->m_node->GetParent()) {
    DrawIncomingBranch(p_dc, p_entry);
  }

  const auto color = m_doc->GetStyle().GetPlayerColor(p_entry->m_node->GetPlayer());
  const bool selected = (p_selection == p_entry->m_node);
  constexpr int selectionPadding = 6;

  if (selected) {
    p_dc.SetPen(*wxTRANSPARENT_PEN);
    p_dc.SetBrush(wxBrush(wxColour(235, 235, 235), wxBRUSHSTYLE_SOLID));

    if (isLine) {
      p_dc.DrawRoundedRectangle(
          p_entry->GetX() - selectionPadding, p_entry->GetY() - selectionPadding,
          nodeSize + 2 * selectionPadding, 2 * selectionPadding, selectionPadding);
    }
    else {
      p_dc.DrawEllipse(token.Inflate(selectionPadding, selectionPadding));
    }
  }

  p_dc.SetPen(*wxThePenList->FindOrCreatePen(color, 3, wxPENSTYLE_SOLID));
  p_dc.SetTextForeground(color);

  if (isLine) {
    p_dc.DrawLine(p_entry->GetX(), p_entry->GetY(), p_entry->GetX() + nodeSize, p_entry->GetY());
    if (m_doc->GetStyle().GetBranchStyle() == GBT_BRANCH_STYLE_FORKTINE) {
      // "classic" Gambit style: draw a small 'token' to separate
      // the fork from the node
      p_dc.DrawEllipse(p_entry->GetX() - 1, p_entry->GetY() - 1, 3, 3);
    }
  }
  else if (tokenStyle == GBT_NODE_TOKEN_BOX) {
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawRectangle(token);
  }
  else if (tokenStyle == GBT_NODE_TOKEN_DIAMOND) {
    wxPoint points[4] = {
        wxPoint(token.GetX() + token.GetWidth() / 2, token.GetY()),
        wxPoint(token.GetX(), token.GetY() + token.GetHeight() / 2),
        wxPoint(token.GetX() + token.GetWidth() / 2, token.GetY() + token.GetHeight()),
        wxPoint(token.GetX() + token.GetWidth(), token.GetY() + token.GetHeight() / 2)};
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawPolygon(4, points);
  }
  else if (tokenStyle == GBT_NODE_TOKEN_DOT) {
    p_dc.SetBrush(wxBrush(m_doc->GetStyle().GetPlayerColor(p_entry->m_node->GetPlayer()),
                          wxBRUSHSTYLE_SOLID));
    p_dc.DrawEllipse(token);
  }
  else {
    // Default: draw circles
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawEllipse(token);
  }

  if (selected) {
    p_dc.SetBrush(*wxTRANSPARENT_BRUSH);
    p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 1, wxPENSTYLE_SOLID));

    if (isLine) {
      p_dc.DrawRoundedRectangle(
          p_entry->GetX() - selectionPadding, p_entry->GetY() - selectionPadding,
          nodeSize + 2 * selectionPadding, 2 * selectionPadding, selectionPadding);
    }
    else {
      p_dc.DrawEllipse(token.Inflate(selectionPadding, selectionPadding));
    }
  }

  int textWidth, textHeight;
  p_dc.SetFont(m_doc->GetStyle().GetFont());
  p_dc.GetTextExtent(p_entry->m_nodeAboveLabel, &textWidth, &textHeight);
  p_dc.DrawText(p_entry->m_nodeAboveLabel, p_entry->GetX() + (nodeSize - textWidth) / 2,
                p_entry->GetY() - textHeight - 9);
  p_dc.SetFont(m_doc->GetStyle().GetFont());
  p_dc.GetTextExtent(p_entry->m_nodeBelowLabel, &textWidth, &textHeight);
  p_dc.DrawText(p_entry->m_nodeBelowLabel, p_entry->GetX() + (nodeSize - textWidth) / 2,
                p_entry->GetY() + 9);

  p_dc.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  DrawOutcome(p_dc, p_entry, p_noHints);
}

void TreeLayout::DrawIncomingBranch(wxDC &p_dc, const std::shared_ptr<NodeEntry> &p_entry) const
{
  const int xStart = p_entry->m_parent->GetX() + m_doc->GetStyle().GetNodeSize();
  const int xEnd = p_entry->GetX();
  const int yStart = p_entry->m_parent->GetY();
  const int yEnd = p_entry->GetY();

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

    p_dc.SetFont(m_doc->GetStyle().GetFont());

    if (m_doc->GetStyle().GetBranchLabels() == GBT_BRANCH_LABEL_ORIENT_HORIZONTAL) {
      // Position/size were already computed by ComputeBranchGeometry.
      p_dc.DrawText(p_entry->m_branchAboveLabel, p_entry->m_geometry.branchAbove.GetX(),
                    p_entry->m_geometry.branchAbove.GetY());
      p_dc.DrawText(p_entry->m_branchBelowLabel, p_entry->m_geometry.branchBelow.GetX(),
                    p_entry->m_geometry.branchBelow.GetY());
    }
    else {
      // Rotated labels are not independently hit-testable (ComputeBranchGeometry
      // leaves their rects empty), so their position is only ever needed here,
      // at paint time.
      int textWidth, textHeight;
      p_dc.GetTextExtent(p_entry->m_branchAboveLabel, &textWidth, &textHeight);

      // The angle of the branch
      const double theta =
          -std::atan(static_cast<double>(yEnd - yStart) / static_cast<double>(xEnd - xStart));
      // The "centerpoint" of the branch
      const int xbar = (xStart + xEnd) / 2;
      const int ybar = (yStart + yEnd) / 2;

      p_dc.DrawRotatedText(
          p_entry->m_branchAboveLabel,
          (xbar - textHeight * std::sin(theta) - textWidth * std::cos(theta) / 2.0),
          (ybar - textHeight * std::cos(theta) + textWidth * std::sin(theta) / 2.0),
          theta * 180.0 / 3.14159);

      p_dc.GetTextExtent(p_entry->m_branchBelowLabel, &textWidth, &textHeight);
      p_dc.DrawRotatedText(p_entry->m_branchBelowLabel, (xbar - textWidth * std::cos(theta) / 2.0),
                           (ybar + textWidth * std::sin(theta) / 2.0), theta * 180.0 / 3.14159);
    }
  }
  else {
    // Old style fork-tine
    const int branchLength = m_doc->GetStyle().GetBranchLength();
    p_dc.DrawLine(xStart, yStart, xStart + branchLength, yEnd);
    p_dc.DrawLine(xStart + branchLength, yEnd, xEnd, yEnd);

    // Draw in the highlight indicating action probabilities
    if (p_entry->m_actionProb >= 0.0) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxPENSTYLE_SOLID));
      p_dc.DrawLine(xStart, yStart, xStart + branchLength * p_entry->m_actionProb,
                    yStart + (yEnd - yStart) * p_entry->m_actionProb);
    }

    p_dc.SetFont(m_doc->GetStyle().GetFont());
    p_dc.DrawText(p_entry->m_branchAboveLabel, p_entry->m_geometry.branchAbove.GetX(),
                  p_entry->m_geometry.branchAbove.GetY());
    p_dc.DrawText(p_entry->m_branchBelowLabel, p_entry->m_geometry.branchBelow.GetX(),
                  p_entry->m_geometry.branchBelow.GetY());
  }
}

//
// Computes (without painting) the regions occupied by the incoming branch's
// above/below labels -- the same geometry DrawIncomingBranch used to compute
// as a side effect of painting. Rotated-orientation labels are intentionally
// left as empty rects: they aren't independently hit-testable, and their
// paint-time position is derived by a formula that doesn't reduce to a rect
// (see DrawIncomingBranch).
//
void TreeLayout::ComputeBranchGeometry(wxDC &p_dc, const std::shared_ptr<NodeEntry> &p_entry) const
{
  if (!p_entry->m_node->GetParent()) {
    p_entry->m_geometry.branchAbove = wxRect();
    p_entry->m_geometry.branchBelow = wxRect();
    return;
  }

  const int xStart = p_entry->m_parent->GetX() + m_doc->GetStyle().GetNodeSize();
  const int xEnd = p_entry->GetX();
  const int yStart = p_entry->m_parent->GetY();
  const int yEnd = p_entry->GetY();

  if (m_doc->GetStyle().GetBranchStyle() != GBT_BRANCH_STYLE_LINE) {
    // Old style fork-tine
    const int branchLength = m_doc->GetStyle().GetBranchLength();
    int textWidth, textHeight;
    p_dc.SetFont(m_doc->GetStyle().GetFont());
    p_dc.GetTextExtent(p_entry->m_branchAboveLabel, &textWidth, &textHeight);
    p_entry->m_geometry.branchAbove =
        wxRect(xStart + branchLength + 3, yEnd - textHeight - 3, textWidth, textHeight);

    p_dc.GetTextExtent(p_entry->m_branchBelowLabel, &textWidth, &textHeight);
    p_entry->m_geometry.branchBelow =
        wxRect(xStart + branchLength + 3, yEnd + 3, textWidth, textHeight);
    return;
  }

  if (m_doc->GetStyle().GetBranchLabels() != GBT_BRANCH_LABEL_ORIENT_HORIZONTAL) {
    p_entry->m_geometry.branchAbove = wxRect();
    p_entry->m_geometry.branchBelow = wxRect();
    return;
  }

  int textWidth, textHeight;
  p_dc.SetFont(m_doc->GetStyle().GetFont());
  p_dc.GetTextExtent(p_entry->m_branchAboveLabel, &textWidth, &textHeight);

  const int xbar = (xStart + xEnd) / 2;
  const int ybar = (yStart + yEnd) / 2;

  if (yStart >= yEnd) {
    p_entry->m_geometry.branchAbove =
        wxRect(xbar - textWidth / 2,
               ybar - textHeight + textWidth / 2 * (yEnd - yStart) / (xEnd - xStart), textWidth,
               textHeight);
  }
  else {
    p_entry->m_geometry.branchAbove =
        wxRect(xbar - textWidth / 2,
               ybar - textHeight - textWidth / 2 * (yEnd - yStart) / (xEnd - xStart), textWidth,
               textHeight);
  }

  p_dc.GetTextExtent(p_entry->m_branchBelowLabel, &textWidth, &textHeight);

  if (yStart >= yEnd) {
    p_entry->m_geometry.branchBelow =
        wxRect(xbar - textWidth / 2, ybar - textWidth / 2 * (yEnd - yStart) / (xEnd - xStart),
               textWidth, textHeight);
  }
  else {
    p_entry->m_geometry.branchBelow =
        wxRect(xbar - textWidth / 2, ybar + textWidth / 2 * (yEnd - yStart) / (xEnd - xStart),
               textWidth, textHeight);
  }
}

// Computes a fraction's layout at p_point, painting it unless p_paint is false.
// Always returns the point advanced past the fraction's width, so that both
// painting (DrawOutcome) and geometry computation (ComputeOutcomeGeometry) can
// share this single measurement, rather than keeping two copies in sync.
static wxPoint DrawFraction(wxDC &p_dc, wxPoint p_point, const Rational &p_value,
                            bool p_paint = true)
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

  if (p_paint) {
    p_dc.DrawLine(p_point.x, p_point.y, p_point.x + width + 4, p_point.y);
    p_dc.DrawText(num, p_point.x + 2 + (width - numWidth) / 2, p_point.y - 2 - numHeight);
    p_dc.DrawText(den, p_point.x + 2 + (width - denWidth) / 2, p_point.y + 2);
  }

  p_point.x += width + 14;
  return p_point;
}

void TreeLayout::DrawOutcome(wxDC &p_dc, const std::shared_ptr<NodeEntry> &p_entry,
                             bool p_noHints) const
{
  const GameOutcome outcome = p_entry->m_node->GetOutcome();
  if (!outcome) {
    if (p_noHints) {
      return;
    }
    // Position/size were already computed by ComputeOutcomeGeometry.
    p_dc.SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD));
    p_dc.SetTextForeground(*wxLIGHT_GREY);
    p_dc.DrawText(wxT("(u)"), p_entry->m_geometry.outcome.GetX(),
                  p_entry->m_geometry.outcome.GetY());
    return;
  }

  int playerIndex = 1;
  for (const auto &player : p_entry->m_node->GetGame()->GetPlayers()) {
    p_dc.SetTextForeground(m_doc->GetStyle().GetPlayerColor(player));

    const auto &payoff = outcome->GetPayoff<std::string>(player);
    const wxRect &cell = p_entry->m_geometry.payoffs[playerIndex];

    if (payoff.find('/') != std::string::npos) {
      p_dc.SetPen(wxPen(m_doc->GetStyle().GetPlayerColor(player), 1, wxPENSTYLE_SOLID));
      DrawFraction(p_dc, wxPoint(cell.GetX() + 5, p_entry->GetY()),
                   outcome->GetPayoff<Rational>(player));
    }
    else {
      const wxString label = wxString(payoff.c_str(), *wxConvCurrent);
      p_dc.SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
      p_dc.DrawText(label, cell.GetX() + 5, cell.GetY());
    }
    ++playerIndex;
  }
}

//
// Computes (without painting) the outcome/payoff regions for a node -- the
// same geometry DrawOutcome used to compute as a side effect of painting --
// and grows m_maxX to account for the outcome label's width.  Unlike
// DrawOutcome, this always measures as if hints were shown (i.e. as if
// p_noHints were false): the "no outcome" hint's width is a safe upper bound
// on what any render mode could need, so using it uniformly for sizing/
// hit-testing purposes avoids under-sizing bitmap/SVG export (which renders
// with hints suppressed) relative to on-screen display (which shows them).
//
void TreeLayout::ComputeOutcomeGeometry(wxDC &p_dc,
                                        const std::shared_ptr<NodeEntry> &p_entry) const
{
  const int nodeSize = m_doc->GetStyle().GetNodeSize();
  wxPoint point(p_entry->GetX() + nodeSize + 20, p_entry->GetY());

  const GameOutcome outcome = p_entry->m_node->GetOutcome();
  if (!outcome) {
    int width, height;
    p_dc.SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD));
    p_dc.GetTextExtent(wxT("(u)"), &width, &height);
    p_entry->m_geometry.outcome = wxRect(point.x, point.y - height / 2, width, height);
    p_entry->m_geometry.payoffs = Array<wxRect>();
    m_maxX = std::max(m_maxX, p_entry->m_geometry.outcome.GetRight());
    return;
  }

  int width, height = 25;
  p_entry->m_geometry.payoffs = Array<wxRect>();
  for (const auto &player : p_entry->m_node->GetGame()->GetPlayers()) {
    const auto &payoff = outcome->GetPayoff<std::string>(player);

    if (payoff.find('/') != std::string::npos) {
      const int oldX = point.x;
      point = DrawFraction(p_dc, point, outcome->GetPayoff<Rational>(player), false);
      p_entry->m_geometry.payoffs.push_back(
          wxRect(oldX - 5, point.y - height / 2, point.x - oldX + 10, height));
    }
    else {
      const wxString label = wxString(payoff.c_str(), *wxConvCurrent);
      p_dc.SetFont(wxFont(9, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
      p_dc.GetTextExtent(label, &width, &height);
      p_entry->m_geometry.payoffs.push_back(
          wxRect(point.x - 5, point.y - height / 2, width + 10, height));
      point.x += width + 10;
    }
  }

  if (height == 0) {
    // Happens if all payoffs are fractional
    height = 25;
  }

  p_entry->m_geometry.outcome =
      wxRect(p_entry->GetX() + nodeSize + 20, p_entry->GetY() - height / 2,
             point.x - (p_entry->GetX() + nodeSize + 20), height);
  m_maxX = std::max(m_maxX, p_entry->m_geometry.outcome.GetRight());
}

//
// Computes the region occupied by the node's own token -- a bounding box
// sized to the node for most token styles, or a thin horizontal band (a
// fudge factor around the drawn line) for the "line" style, which has no
// real width of its own to hit-test against.
//
void TreeLayout::ComputeTokenGeometry(const std::shared_ptr<NodeEntry> &p_entry) const
{
  const int nodeSize = m_doc->GetStyle().GetNodeSize();
  if (GetTokenForNode(m_doc->GetStyle(), p_entry->m_node) == GBT_NODE_TOKEN_LINE) {
    constexpr int DELTA = 8; // a fudge factor for "almost" hitting the node
    p_entry->m_geometry.token =
        wxRect(p_entry->GetX(), p_entry->GetY() - DELTA, nodeSize, 2 * DELTA);
    return;
  }
  p_entry->m_geometry.token =
      wxRect(p_entry->GetX(), p_entry->GetY() - nodeSize / 2, nodeSize, nodeSize);
}

//-----------------------------------------------------------------------
//                class TreeLayout: Member functions
//-----------------------------------------------------------------------

GameNode TreeLayout::NodeHitTest(int p_x, int p_y) const
{
  const auto hit = std::find_if(m_nodeList.begin(), m_nodeList.end(),
                                [p_x, p_y](const std::shared_ptr<NodeEntry> &p_entry) -> bool {
                                  return p_entry->NodeHitTest(p_x, p_y);
                                });
  return (hit != m_nodeList.end()) ? (*hit)->GetNode() : nullptr;
}

//
// Answers "what, if anything, is at (p_x, p_y)" as a single discriminated
// result, replacing what used to be four independent hit-test entry points
// (NodeHitTest/OutcomeHitTest/BranchAboveHitTest/BranchBelowHitTest) that
// callers had to try in sequence -- and a fifth, manual per-player Contains()
// loop at the one call site that needed to know which payoff was hit.
//
// Region priority (Node, then Outcome/Payoff, then BranchAbove, then
// BranchBelow) matches the order those separate calls used to be tried in at
// the one call site that used all of them (EfgDisplay::OnLeftDoubleClick), so
// behavior is unchanged even though this is now one query: each region is
// still checked tree-wide before falling through to the next, rather than
// checking all regions for one node before moving to the next node (the
// latter would let one node's outcome-label rect -- which can extend
// arbitrarily far right -- pre-empt a Node hit on a different, further-right
// node, which the old code never did).
//
HitResult TreeLayout::HitTest(int p_x, int p_y) const
{
  using enum HitRegion;
  for (const auto &entry : m_nodeList) {
    if (entry->NodeHitTest(p_x, p_y)) {
      return {Node, entry->GetNode()};
    }
  }
  for (const auto &entry : m_nodeList) {
    if (entry->OutcomeHitTest(p_x, p_y)) {
      const auto &payoffs = entry->m_geometry.payoffs;
      for (int pl = payoffs.front_index(); pl <= payoffs.back_index(); ++pl) {
        if (payoffs[pl].Contains(p_x, p_y)) {
          return {Payoff, entry->GetNode(), pl};
        }
      }
      return {Outcome, entry->GetNode()};
    }
  }
  for (const auto &entry : m_nodeList) {
    if (entry->BranchAboveHitTest(p_x, p_y)) {
      return {BranchAbove, entry->GetNode()};
    }
  }
  for (const auto &entry : m_nodeList) {
    if (entry->BranchBelowHitTest(p_x, p_y)) {
      return {BranchBelow, entry->GetNode()};
    }
  }
  return {};
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

std::shared_ptr<NodeEntry>
TreeLayout::ComputeNextInInfoset(const std::shared_ptr<NodeEntry> &p_entry) const
{
  const auto &members = p_entry->m_node->GetInfoset()->GetMembers();
  const auto member = std::next(std::find(members.begin(), members.end(), p_entry->m_node));
  return (member != members.end()) ? GetNodeEntry(*member) : nullptr;
}

void TreeLayout::ComputeNodeDepths(const Gambit::Layout &p_layout) const
{
  m_maxX = ComputeLevelProportionalX(m_nodeList, p_layout, c_leftMargin,
                                     m_doc->GetStyle().GetNodeLevelLength(), m_infosetSpacing,
                                     m_doc->GetStyle().GetNodeSize());
}

void TreeLayout::ComputeGeometry() const
{
  // A throwaway off-screen DC, used only to measure text -- wxMemoryDC needs no
  // on-screen window, unlike wxClientDC, so this needs no window/DC to be
  // threaded in from the caller. Matches this codebase's existing idiom for
  // off-screen DC work (e.g. EfgDisplay::MakeOutcomeBitmap, EfgPanel::GetBitmap).
  wxMemoryDC measureDC;
  wxBitmap measureBitmap(1, 1);
  measureDC.SelectObject(measureBitmap);

  for (const auto &entry : m_nodeList) {
    ComputeTokenGeometry(entry);
    ComputeBranchGeometry(measureDC, entry);
    ComputeOutcomeGeometry(measureDC, entry);
  }
}

void TreeLayout::BuildNodeList(const GameNode &p_node,
                               const std::shared_ptr<NodeEntry> &p_parentEntry)
{
  const auto entry = std::make_shared<NodeEntry>(p_node);
  m_nodeList.push_back(entry);
  m_nodeMap[p_node] = entry;
  entry->m_parent = p_parentEntry;
  for (const auto &child : p_node->GetChildren()) {
    BuildNodeList(child, entry);
  }
}

void TreeLayout::BuildNodeList(const Game &p_game)
{
  m_nodeList.clear();
  m_nodeMap.clear();
  BuildNodeList(p_game->GetRoot(), nullptr);
}

void TreeLayout::Layout(const Game &p_game)
{
  m_infosetSpacing = (m_doc->GetStyle().GetInfosetJoin() == GBT_INFOSET_JOIN_LINES) ? 10 : 40;

  if (m_nodeList.size() != m_doc->GetGame()->NumNodes()) {
    // We only rebuild the node list if the number of nodes changes.  If we only have
    // information set changes this can be handled just by the traversal below
    BuildNodeList(p_game);
  }

  auto layout = Gambit::Layout(m_doc->GetGame());
  layout.LayoutTree(p_game);

  const auto spacing = m_doc->GetStyle().GetTerminalSpacing();
  for (auto [node, entry] : layout.GetNodeMap()) {
    m_nodeMap[node]->m_level = entry->m_level;
    m_nodeMap[node]->m_sublevel = entry->m_sublevel;
    m_nodeMap[node]->SetY(entry->m_offset * spacing + c_topMargin);
  }
  m_maxY =
      c_topMargin + c_bottomMargin + spacing * (layout.GetMaxOffset() - layout.GetMinOffset());
  ComputeNodeDepths(layout);

  GenerateLabels();
  ComputeGeometry();
}

void TreeLayout::GenerateLabels() const
{
  const TreeRenderConfig &settings = m_doc->GetStyle();
  const auto &workspace = m_doc->GetWorkspace();
  // The style resolves each label choice into a function once here, rather
  // than TreeLayout re-interpreting the style enum for every node.
  const LabelGenerator nodeAbove =
      settings.GetNodeLabelGenerator(settings.GetNodeAboveLabel(), workspace);
  const LabelGenerator nodeBelow =
      settings.GetNodeLabelGenerator(settings.GetNodeBelowLabel(), workspace);
  const LabelGenerator branchAbove =
      settings.GetBranchLabelGenerator(settings.GetBranchAboveLabel(), workspace);
  const LabelGenerator branchBelow =
      settings.GetBranchLabelGenerator(settings.GetBranchBelowLabel(), workspace);

  for (const auto &entry : m_nodeList) {
    entry->SetNodeAboveLabel(SafeGenerate(nodeAbove, entry->GetNode()));
    entry->SetNodeBelowLabel(SafeGenerate(nodeBelow, entry->GetNode()));
    if (entry->GetChildNumber() > 0) {
      entry->SetBranchAboveLabel(SafeGenerate(branchAbove, entry->GetNode()));
      entry->SetBranchBelowLabel(SafeGenerate(branchBelow, entry->GetNode()));

      const GameNode parent = entry->GetNode()->GetParent();
      if (parent->GetPlayer()->IsChance()) {
        entry->SetActionProb(static_cast<double>(parent->GetInfoset()->GetActionProb(
            parent->GetInfoset()->GetAction(entry->GetChildNumber()))));
      }
      else {
        const int profile = m_doc->GetWorkspace().GetCurrentProfile();
        if (profile > 0) {
          try {
            entry->SetActionProb(
                (double)lexical_cast<Rational>(m_doc->GetWorkspace().GetProfiles().GetActionProb(
                    parent, entry->GetChildNumber())));
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

void TreeLayout::RenderSubtree(wxDC &p_dc, bool p_noHints) const
{
  const TreeRenderConfig &settings = m_doc->GetStyle();

  for (const auto &entry : m_nodeList) {
    auto parentEntry = entry->GetParent();

    if (entry->GetChildNumber() == 1) {
      DrawNode(p_dc, parentEntry, m_doc->GetSelectNode(), p_noHints);

      if (auto nextMember = ComputeNextInInfoset(parentEntry)) {
        const int nextY = nextMember->GetY();
        p_dc.SetPen(wxPen(m_doc->GetStyle().GetPlayerColor(parentEntry->m_node->GetPlayer()), 1,
                          wxPENSTYLE_DOT));
        p_dc.DrawLine(parentEntry->GetX(), parentEntry->GetY(), parentEntry->GetX(), nextY);
        if (settings.GetInfosetJoin() == GBT_INFOSET_JOIN_CIRCLES) {
          p_dc.DrawLine(parentEntry->GetX() + settings.GetNodeSize(), parentEntry->GetY(),
                        parentEntry->GetX() + settings.GetNodeSize(), nextY);
        }

        if (nextMember->GetX() != parentEntry->GetX()) {
          // Draw a little arrow in the direction of the iset.
          int startX, endX;
          if (settings.GetInfosetJoin() == GBT_INFOSET_JOIN_LINES) {
            startX = parentEntry->GetX();
            endX = (startX +
                    m_infosetSpacing * ((nextMember->GetX() > parentEntry->GetX()) ? 1 : -1));
          }
          else {
            if (nextMember->GetX() < parentEntry->GetX()) {
              // information set is continued to the left
              startX = parentEntry->GetX() + settings.GetNodeSize();
              endX = parentEntry->GetX() - m_infosetSpacing;
            }
            else {
              // information set is continued to the right
              startX = parentEntry->GetX();
              endX = (parentEntry->GetX() + settings.GetNodeSize() + m_infosetSpacing);
            }
          }
          p_dc.DrawLine(startX, nextY, endX, nextY);
          if (startX > endX) {
            p_dc.DrawLine(endX, nextY, endX + m_infosetSpacing / 2, nextY + m_infosetSpacing / 2);
            p_dc.DrawLine(endX, nextY, endX + m_infosetSpacing / 2, nextY - m_infosetSpacing / 2);
          }
          else {
            p_dc.DrawLine(endX, nextY, endX - m_infosetSpacing / 2, nextY + m_infosetSpacing / 2);
            p_dc.DrawLine(endX, nextY, endX - m_infosetSpacing / 2, nextY - m_infosetSpacing / 2);
          }
        }
      }
    }

    if (entry->GetNode()->IsTerminal()) {
      DrawNode(p_dc, entry, m_doc->GetSelectNode(), p_noHints);
    }
  }
}

void TreeLayout::Render(wxDC &p_dc, bool p_noHints) const { RenderSubtree(p_dc, p_noHints); }
} // namespace Gambit::GUI
