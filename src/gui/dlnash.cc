//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlnash.cc
// Dialog for selecting algorithms to compute Nash equilibria
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

#include <algorithm>
#include <concepts>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>
#include <vector>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/collpane.h>
#include <wx/display.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/stdpaths.h>

#include "games.h"
#include "games/gametree.h"

#include "dlnash.h"

namespace Gambit::GUI {

namespace {

NashEquilibriumTarget GetTarget(NashGoal p_goal)
{
  switch (p_goal) {
  case NashGoal::One:
    return NashEquilibriumTarget::One;
  case NashGoal::Several:
    return NashEquilibriumTarget::Some;
  default:
    return NashEquilibriumTarget::All;
  }
}

// Agent-form liap is not yet offered.
bool IsOffered(const NashVariant &p_variant)
{
  return p_variant.representation == VariantRepresentation::Strategic ||
         p_variant.method != NashMethodId::Liap;
}

std::vector<const NashVariant *> OfferedVariants(const Game &p_game)
{
  std::vector<const NashVariant *> result;
  for (const auto &variant : NashVariants()) {
    if (IsOffered(variant) &&
        (variant.representation == VariantRepresentation::Strategic || As<GameTreeRep>(p_game))) {
      result.push_back(&variant);
    }
  }
  return result;
}

wxString VariantLabel(const NashVariant &p_variant)
{
  const bool strategic = p_variant.representation == VariantRepresentation::Strategic;
  const wxString form = strategic ? wxT(" on the strategic form") : wxT(" on the sequence form");
  switch (p_variant.method) {
  case NashMethodId::EnumMixed:
    return wxT("Enumerate extreme points") + form;
  case NashMethodId::EnumPoly:
    return wxT("Solve systems of polynomial equations") + form;
  case NashMethodId::EnumPure:
    return wxT("Look for pure strategy equilibria") +
           (strategic ? form : wxString(wxT(" on the agent form")));
  case NashMethodId::GNM:
    return wxT("Global Newton tracing") + form;
  case NashMethodId::HP:
    return wxT("Herings-Peeters homotopy") + form;
  case NashMethodId::IPA:
    return wxT("Iterated polymatrix approximation") + form;
  case NashMethodId::LCP:
    return strategic ? wxString(wxT("Lemke-Howson linear complementarity") + form)
                     : wxString(wxT("Lemke linear complementarity") + form);
  case NashMethodId::Liap:
    return wxT("Minimize the Lyapunov function") +
           (strategic ? form : wxString(wxT(" on the agent form")));
  case NashMethodId::Logit:
    return strategic ? wxString(wxT("Trace logit quantal response equilibria"))
                     : wxString(wxT("Trace agent logit quantal response equilibria"));
  case NashMethodId::LP:
    return wxT("Solve a linear program") + form;
  case NashMethodId::Simpdiv:
    return wxT("Simplicial subdivision") + form;
  }
  throw std::logic_error("Unknown Nash equilibrium method");
}

wxString FromStd(std::string_view p_text)
{
  return wxString::FromUTF8(p_text.data(), p_text.size());
}

wxString GoalName(NashGoal p_goal)
{
  switch (p_goal) {
  case NashGoal::One:
    return wxT("One");
  case NashGoal::Several:
    return wxT("Several");
  default:
    return wxT("All");
  }
}

wxString CoverageLabel(VariantCoverage p_coverage)
{
  switch (p_coverage) {
  case VariantCoverage::One:
    return wxT("One");
  case VariantCoverage::Some:
    return wxT("Some");
  case VariantCoverage::AllPure:
    return wxT("All pure");
  default:
    return wxT("All");
  }
}

wxString ReliabilityLabel(VariantReliability p_reliability)
{
  switch (p_reliability) {
  case VariantReliability::Always:
    return wxT("Always");
  case VariantReliability::NumericalFailurePossible:
    return wxT("Numerical failure possible");
  default:
    return wxT("No guarantee");
  }
}

wxString ConceptLabel(VariantConcept p_concept)
{
  return p_concept == VariantConcept::Nash ? wxT("Nash") : wxT("Agent Nash");
}

wxString GoalMissWarning(const NashVariant &p_variant, NashGoal p_goal)
{
  wxString what;
  switch (p_goal) {
  case NashGoal::One:
    what = wxT("may find no equilibrium");
    break;
  case NashGoal::Several:
    what = wxT("returns only one equilibrium");
    break;
  default:
    what = wxT("does not guarantee to return all equilibria");
    break;
  }
  return wxString::Format(wxT("Warning: this variant %s; your Goal is %s."), what,
                          GoalName(p_goal));
}

const wxString s_allUnavailable(wxT("Gambit has no method that guarantees all equilibria for "
                                    "games with more than two players."));

NashMethodSpec ResolveMethod(NashMethodId p_method, NashGoal p_goal)
{
  switch (p_method) {
  case NashMethodId::EnumMixed:
    return EnumMixedNashSpec{};
  case NashMethodId::EnumPoly:
    if (p_goal == NashGoal::One) {
      return EnumPolyNashSpec{.stopAfter = 1};
    }
    return EnumPolyNashSpec{};
  case NashMethodId::EnumPure:
    return EnumPureNashSpec{};
  case NashMethodId::GNM:
    return GNMNashSpec{};
  case NashMethodId::HP:
    return HPNashSpec{};
  case NashMethodId::IPA:
    return IPANashSpec{};
  case NashMethodId::LCP:
    return LCPNashSpec{};
  case NashMethodId::Liap:
    return LiapNashSpec{};
  case NashMethodId::Logit:
    return LogitNashSpec{};
  case NashMethodId::LP:
    return LPNashSpec{};
  case NashMethodId::Simpdiv:
    if (p_goal == NashGoal::One) {
      return SimpdivNashSpec{.startingPoints = 1};
    }
    return SimpdivNashSpec{};
  }
  throw std::logic_error("Unknown Nash equilibrium method");
}

template <class M>
concept RationalOutputMethod =
    std::same_as<M, EnumPureNashSpec> || std::same_as<M, EnumMixedNashSpec> ||
    std::same_as<M, LPNashSpec> || std::same_as<M, LCPNashSpec>;

bool UsesRationalOutput(const NashMethodSpec &p_method)
{
  return std::visit([]<typename Method>(const Method &) { return RationalOutputMethod<Method>; },
                    p_method);
}

wxString ExternalCommand(const NashComputationSpec &p_spec)
{
#ifdef __WXMAC__
  wxString prefix = wxStandardPaths::Get().GetExecutablePath() + wxT("-");
#else
  wxString prefix = wxT("gambit-");
#endif
  const wxString strategic =
      p_spec.representation == NashRepresentation::Strategic ? wxT(" -S") : wxString{};

  return std::visit(
      [&]<typename Method>(const Method &method) {
        if constexpr (std::is_same_v<Method, EnumPureNashSpec>) {
          return prefix + wxT("enumpure") + (p_spec.agentNash ? wxT(" -A") : strategic);
        }
        else if constexpr (std::is_same_v<Method, EnumMixedNashSpec>) {
          return prefix + wxT("enummixed");
        }
        else if constexpr (std::is_same_v<Method, EnumPolyNashSpec>) {
          wxString command = wxString::Format("enumpoly -d 10 -m %.17g", method.maxRegret);
          if (method.stopAfter.has_value()) {
            command += wxString::Format(" -e %llu",
                                        static_cast<unsigned long long>(method.stopAfter.value()));
          }
          return prefix + command + strategic;
        }
        else if constexpr (std::is_same_v<Method, GNMNashSpec>) {
          return prefix + wxString::Format("gnm -d 10 -n %d -m %.17g -c %d -f %d -i %d",
                                           method.perturbations, method.lambdaEnd, method.steps,
                                           method.localNewtonInterval,
                                           method.localNewtonMaxIterations);
        }
        else if constexpr (std::is_same_v<Method, HPNashSpec>) {
          return prefix +
                 wxString::Format("hp -d 10 -n %d -m %.17g", method.priors, method.maxRegret);
        }
        else if constexpr (std::is_same_v<Method, IPANashSpec>) {
          return prefix + wxString::Format("ipa -d 10 -n %d", method.perturbations);
        }
        else if constexpr (std::is_same_v<Method, LPNashSpec>) {
          return prefix + wxT("lp") + strategic;
        }
        else if constexpr (std::is_same_v<Method, LCPNashSpec>) {
          wxString command = wxString::Format("lcp -r %d", method.maxDepth);
          if (method.stopAfter.has_value()) {
            command += wxString::Format(" -e %llu",
                                        static_cast<unsigned long long>(method.stopAfter.value()));
          }
          return prefix + command + strategic;
        }
        else if constexpr (std::is_same_v<Method, LiapNashSpec>) {
          return prefix + wxString::Format("liap -d 10 -n %d -i %d -m %.17g",
                                           method.startingPoints, method.maxIterations,
                                           method.maxRegret);
        }
        else if constexpr (std::is_same_v<Method, LogitNashSpec>) {
          return prefix +
                 wxString::Format("logit -e -d 10 -m %.17g -s %.17g -a %.17g", method.maxRegret,
                                  method.firstStep, method.maxAcceleration) +
                 strategic;
        }
        else {
          std::ostringstream regret;
          regret << method.maxRegret;
          return prefix +
                 wxString::Format("simpdiv -d 10 -n %d -r %d -g %d -m ", method.startingPoints,
                                  method.randomDenominator, method.gridResize) +
                 wxString(regret.str());
        }
      },
      p_spec.method);
}

wxString MethodDescription(const NashMethodSpec &p_method)
{
  return std::visit(
      []<typename Method>(const Method &method) {
        if constexpr (std::is_same_v<Method, EnumPureNashSpec>) {
          return wxT("in pure strategies");
        }
        else if constexpr (std::is_same_v<Method, EnumMixedNashSpec>) {
          return wxT("by enumeration of mixed-strategy extreme points");
        }
        else if constexpr (std::is_same_v<Method, EnumPolyNashSpec>) {
          return wxT("by solving polynomial systems");
        }
        else if constexpr (std::is_same_v<Method, GNMNashSpec>) {
          return wxT("by global Newton tracing");
        }
        else if constexpr (std::is_same_v<Method, HPNashSpec>) {
          return wxT("by the Herings-Peeters homotopy");
        }
        else if constexpr (std::is_same_v<Method, IPANashSpec>) {
          return wxT("by iterated polymatrix approximation");
        }
        else if constexpr (std::is_same_v<Method, LPNashSpec>) {
          return wxT("by solving a linear program");
        }
        else if constexpr (std::is_same_v<Method, LCPNashSpec>) {
          return wxT("by solving a linear complementarity program");
        }
        else if constexpr (std::is_same_v<Method, LiapNashSpec>) {
          return wxT("by function minimization");
        }
        else if constexpr (std::is_same_v<Method, LogitNashSpec>) {
          return wxT("by logit tracing");
        }
        else {
          return wxT("by simplicial subdivision");
        }
      },
      p_method);
}

wxString ParameterDescription(const NashMethodSpec &p_method)
{
  return std::visit(
      []<typename Method>(const Method &method) {
        if constexpr (std::is_same_v<Method, EnumPolyNashSpec>) {
          if (method.stopAfter.has_value() && method.stopAfter.value() == 1) {
            return wxString::Format(" (stop after one equilibrium; maximum regret %.4g)",
                                    method.maxRegret);
          }
          return wxString::Format(" (all supports; maximum regret %.4g)", method.maxRegret);
        }
        else if constexpr (std::is_same_v<Method, GNMNashSpec>) {
          return wxString::Format(
              " (%d perturbation; ending lambda %.4g; %d steps per support cell; local Newton "
              "every %d steps, at most %d iterations)",
              method.perturbations, method.lambdaEnd, method.steps, method.localNewtonInterval,
              method.localNewtonMaxIterations);
        }
        else if constexpr (std::is_same_v<Method, HPNashSpec>) {
          return wxString::Format(" (%d random priors; maximum regret %.4g)", method.priors,
                                  method.maxRegret);
        }
        else if constexpr (std::is_same_v<Method, IPANashSpec>) {
          return wxString::Format(" (%d perturbation)", method.perturbations);
        }
        else if constexpr (std::is_same_v<Method, LCPNashSpec>) {
          return wxString(wxT(" (all accessible equilibria; unlimited recursion depth)"));
        }
        else if constexpr (std::is_same_v<Method, LiapNashSpec>) {
          return wxString::Format(" (%d random starting points; at most %d iterations; maximum "
                                  "regret %.4g)",
                                  method.startingPoints, method.maxIterations, method.maxRegret);
        }
        else if constexpr (std::is_same_v<Method, LogitNashSpec>) {
          return wxString::Format(
              " (maximum regret %.4g; initial step %.4g; maximum acceleration %.4g)",
              method.maxRegret, method.firstStep, method.maxAcceleration);
        }
        else if constexpr (std::is_same_v<Method, SimpdivNashSpec>) {
          std::ostringstream regret;
          regret << method.maxRegret;
          return wxString::Format(" (%d random starting points with denominator %d; grid resize "
                                  "factor %d; maximum regret ",
                                  method.startingPoints, method.randomDenominator,
                                  method.gridResize) +
                 wxString(regret.str()) + wxT(")");
        }
        else {
          return wxString{};
        }
      },
      p_method);
}

} // namespace

namespace {
enum ListColumn { COL_STAR, COL_METHOD, COL_COVERAGE, COL_RELIABILITY, COL_SOLVES };
} // namespace

NashChoiceDialog::NashChoiceDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
  : wxDialog(p_parent, wxID_ANY, wxT("Compute equilibria"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc), m_variants(OfferedVariants(p_doc->GetGame()))
{
  const int S = FromDIP(5);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *goalSizer = new wxBoxSizer(wxHORIZONTAL);
  goalSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Goal:")), 0,
                 wxALIGN_CENTER_VERTICAL | wxRIGHT, S * 2);
  const wxString goalLabels[] = {wxT("One equilibrium"), wxT("Several equilibria"),
                                 wxT("All equilibria")};
  for (int i = 0; i < 3; i++) {
    m_goalButtons[i] = new wxRadioButton(this, wxID_ANY, goalLabels[i], wxDefaultPosition,
                                         wxDefaultSize, i == 0 ? wxRB_GROUP : 0);
    goalSizer->Add(m_goalButtons[i], 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, S * 2);
    m_goalButtons[i]->Bind(wxEVT_RADIOBUTTON, &NashChoiceDialog::OnGoal, this);
  }
  m_goalButtons[0]->SetValue(true);
  topSizer->Add(goalSizer, 0, wxALL | wxEXPAND, S);

  const auto game = m_doc->GetGame();
  const bool allAvailable = std::ranges::any_of(m_variants, [&](const NashVariant *v) {
    return !UnavailableReason(*v, game) && MeetsGoal(*v, NashGoal::All);
  });
  m_goalButtons[2]->Enable(allAvailable);
  m_goalNote = new wxStaticText(this, wxID_ANY, allAvailable ? wxString{} : s_allUnavailable);
  m_goalNote->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
  topSizer->Add(m_goalNote, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, S);

  const NashChoice &remembered = m_doc->GetNashChoice();
  if (remembered.goal != NashGoal::All || allAvailable) {
    SetGoal(remembered.goal);
    if (std::ranges::find(m_variants, remembered.variant) != m_variants.end() &&
        !UnavailableReason(*remembered.variant, game)) {
      m_userChoice = remembered.variant;
    }
  }

  m_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                          wxLC_REPORT | wxLC_SINGLE_SEL);
  m_list->AppendColumn(wxT(""), wxLIST_FORMAT_CENTER, FromDIP(24));
  m_list->AppendColumn(wxT("Method"));
  m_list->AppendColumn(wxT("Coverage"));
  m_list->AppendColumn(wxT("Reliability"));
  m_list->AppendColumn(wxT("Solves for"));
  m_list->Bind(wxEVT_LIST_ITEM_SELECTED, &NashChoiceDialog::OnItemSelected, this);
  m_list->Bind(wxEVT_LIST_COL_CLICK, &NashChoiceDialog::OnColumnClick, this);
  for (wxWindow *child : m_list->GetChildren()) {
    child->Bind(wxEVT_MOTION,
                [this, child](wxMouseEvent &p_event) { OnListMotion(child, p_event); });
  }

  int displayIndex = wxDisplay::GetFromWindow(p_parent ? p_parent : this);
  const wxRect area = wxDisplay(displayIndex == wxNOT_FOUND ? 0 : displayIndex).GetClientArea();
  m_list->SetMinSize(wxSize(std::min(FromDIP(660), area.width * 8 / 10),
                            std::min(FromDIP(300), area.height * 4 / 10)));
  topSizer->Add(m_list, 1, wxLEFT | wxRIGHT | wxEXPAND, S);

  auto *legend = new wxStaticText(this, wxID_ANY,
                                  wxString::FromUTF8("\xE2\x98\x85") +
                                      wxT(" marks the variant Gambit recommends for the Goal."));
  topSizer->Add(legend, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, S);

  m_detail =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, FromDIP(130)),
                     wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP);
  topSizer->Add(m_detail, 0, wxALL | wxEXPAND, S);

  m_parameters = new wxCollapsiblePane(this, wxID_ANY, wxT("Parameters"));
  m_parameters->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &NashChoiceDialog::OnParametersToggled, this);
  m_parametersText = new wxStaticText(m_parameters->GetPane(), wxID_ANY, wxEmptyString);
  auto *paneSizer = new wxBoxSizer(wxVERTICAL);
  paneSizer->Add(m_parametersText, 0, wxALL | wxEXPAND, S);
  m_parameters->GetPane()->SetSizer(paneSizer);
  m_parameters->GetPane()->Bind(wxEVT_SIZE, [this](wxSizeEvent &p_event) {
    p_event.Skip();
    WrapParameters();
  });
  topSizer->Add(m_parameters, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, S);

  if (auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL)) {
    topSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, S);
  }

  RefreshList();
  for (const int column : {COL_METHOD, COL_COVERAGE, COL_RELIABILITY, COL_SOLVES}) {
    m_list->SetColumnWidth(column, wxLIST_AUTOSIZE_USEHEADER);
  }

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  wxTopLevelWindowBase::Layout();
  m_bestHeight = ClientToWindowSize(topSizer->GetMinSize()).y;
  CenterOnParent();
}

NashGoal NashChoiceDialog::GetGoal() const
{
  if (m_goalButtons[2]->GetValue()) {
    return NashGoal::All;
  }
  return m_goalButtons[1]->GetValue() ? NashGoal::Several : NashGoal::One;
}

void NashChoiceDialog::SetGoal(NashGoal p_goal)
{
  m_goalButtons[static_cast<int>(p_goal)]->SetValue(true);
}

const NashVariant &NashChoiceDialog::GetSelectedVariant() const
{
  return m_userChoice ? *m_userChoice : RecommendedVariant(GetGoal(), m_doc->GetGame());
}

void NashChoiceDialog::OnGoal(wxCommandEvent &) { UpdateRecommendation(); }

void NashChoiceDialog::OnItemSelected(wxListEvent &p_event)
{
  if (m_refreshing) {
    return;
  }
  const auto *variant = m_rows.at(p_event.GetIndex());
  if (UnavailableReason(*variant, m_doc->GetGame())) {
    SelectRow(GetSelectedVariant());
    return;
  }
  m_userChoice = variant;
  UpdateDetail();
}

void NashChoiceDialog::OnListMotion(wxWindow *p_window, wxMouseEvent &p_event)
{
  p_event.Skip();
  int flags = 0;
  const wxPoint position = m_list->ScreenToClient(p_window->ClientToScreen(p_event.GetPosition()));
  const long row = m_list->HitTest(position, flags);
  wxString tip;
  if (row != wxNOT_FOUND && row < static_cast<long>(m_rows.size())) {
    if (const auto reason = UnavailableReason(*m_rows[row], m_doc->GetGame())) {
      tip = FromStd(*reason);
    }
  }
  if (tip != m_tip) {
    m_tip = tip;
    p_window->UnsetToolTip();
    if (!tip.empty()) {
      p_window->SetToolTip(tip);
    }
  }
}

void NashChoiceDialog::OnColumnClick(wxListEvent &p_event)
{
  const int column = p_event.GetColumn();
  if (column == COL_STAR) {
    return;
  }
  m_sortAscending = (column != m_sortColumn) || !m_sortAscending;
  m_sortColumn = column;
  RefreshList();
}

void NashChoiceDialog::SelectRow(const NashVariant &p_variant)
{
  const bool wasRefreshing = m_refreshing;
  m_refreshing = true;
  for (int row = 0; row < static_cast<int>(m_rows.size()); row++) {
    if (m_rows[row] == &p_variant) {
      m_list->SetItemState(row, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                           wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
      m_list->EnsureVisible(row);
    }
    else {
      m_list->SetItemState(row, 0, wxLIST_STATE_SELECTED);
    }
  }
  m_refreshing = wasRefreshing;
}

void NashChoiceDialog::RefreshList()
{
  const auto game = m_doc->GetGame();
  const auto goal = GetGoal();
  const auto *recommended = &RecommendedVariant(goal, game);

  m_rows = m_variants;
  if (m_sortColumn != -1) {
    const auto key = [&](const NashVariant *v) -> std::pair<int, std::string> {
      switch (m_sortColumn) {
      case COL_METHOD:
        return {0, std::string(v->methodName)};
      case COL_COVERAGE:
        return {static_cast<int>(v->coverage), {}};
      case COL_RELIABILITY:
        return {static_cast<int>(v->reliability), {}};
      default:
        return {static_cast<int>(v->solves), {}};
      }
    };
    std::ranges::stable_sort(m_rows, [&](const NashVariant *a, const NashVariant *b) {
      return m_sortAscending ? key(a) < key(b) : key(b) < key(a);
    });
  }

  m_refreshing = true;
  m_list->DeleteAllItems();
  const wxColour grey = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
  for (int row = 0; row < static_cast<int>(m_rows.size()); row++) {
    const auto &variant = *m_rows[row];
    const auto reason = UnavailableReason(variant, game);
    const wxString label = VariantLabel(variant);
    m_list->InsertItem(row,
                       &variant == recommended ? wxString::FromUTF8("\xE2\x98\x85") : wxString{});
    m_list->SetItem(row, COL_METHOD, label);
    m_list->SetItem(row, COL_COVERAGE, CoverageLabel(variant.coverage));
    m_list->SetItem(row, COL_RELIABILITY, ReliabilityLabel(variant.reliability));
    m_list->SetItem(row, COL_SOLVES, ConceptLabel(variant.solves));
    if (reason) {
      m_list->SetItemTextColour(row, grey);
    }
  }
  m_refreshing = false;
  SelectRow(m_userChoice ? *m_userChoice : *recommended);
  UpdateDetail();
}

void NashChoiceDialog::UpdateRecommendation()
{
  const auto *recommended = &RecommendedVariant(GetGoal(), m_doc->GetGame());
  for (int row = 0; row < static_cast<int>(m_rows.size()); row++) {
    m_list->SetItem(row, COL_STAR,
                    m_rows[row] == recommended ? wxString::FromUTF8("\xE2\x98\x85") : wxString{});
  }
  SelectRow(m_userChoice ? *m_userChoice : *recommended);
  UpdateDetail();
}

wxString NashChoiceDialog::DescribeVariant(const NashVariant &p_variant)
{
  const auto game = m_doc->GetGame();
  wxString text = VariantLabel(p_variant) + wxT("\n");
  text +=
      wxT("Method: ") + FromStd(p_variant.methodName) + wxT(".  Arithmetic: ") +
      (p_variant.arithmetic == VariantArithmetic::Exact ? wxT("exact.") : wxT("floating point.")) +
      wxT("  Solves for: ") + ConceptLabel(p_variant.solves) + wxT(" equilibria.\n");
  if (const auto goal = GetGoal(); !MeetsGoal(p_variant, goal)) {
    text += GoalMissWarning(p_variant, goal) + wxT("\n");
  }
  text += FromStd(p_variant.caveat) + wxT("\n");
  if (p_variant.representation == VariantRepresentation::Strategic && As<GameTreeRep>(game)) {
    if (!m_contingencies) {
      m_contingencies = game->GetStrategies().extent_product();
    }
    text += wxString::Format(wxT("The reduced strategic form has %llu contingencies.\n"),
                             static_cast<unsigned long long>(*m_contingencies));
  }
  return text;
}

void NashChoiceDialog::UpdateDetail()
{
  m_detail->ChangeValue(DescribeVariant(GetSelectedVariant()));
  wxString parameters = ParameterDescription(GetComputation().method);
  parameters.Trim(false);
  m_parametersLabel =
      parameters.empty() ? wxString(wxT("This variant has no parameters.")) : parameters;
  WrapParameters();
}

void NashChoiceDialog::WrapParameters()
{
  const int width = m_parameters->GetPane()->GetClientSize().GetWidth() - 2 * FromDIP(5);
  m_parametersText->SetLabel(m_parametersLabel);
  if (width > 0) {
    m_parametersText->Wrap(width);
  }
  m_parameters->GetPane()->Layout();
}

void NashChoiceDialog::OnParametersToggled(wxCollapsiblePaneEvent &)
{
  Layout();
  const wxSize best = ClientToWindowSize(GetSizer()->GetMinSize());
  const wxSize size = GetSize();
  SetMinSize(best);
  SetSize(size.x, std::max(best.y, size.y + best.y - m_bestHeight));
  m_bestHeight = best.y;
}

bool NashChoiceDialog::UseStrategic() const
{
  return GetComputation().representation == NashRepresentation::Strategic;
}

NashComputationSpec NashChoiceDialog::GetComputation() const
{
  const auto goal = GetGoal();
  const auto &variant = GetSelectedVariant();
  return {variant.representation == VariantRepresentation::Strategic
              ? NashRepresentation::Strategic
              : NashRepresentation::Behavior,
          GetTarget(goal), ResolveMethod(variant.method, goal), m_userChoice == nullptr,
          variant.solves == VariantConcept::AgentNash};
}

std::shared_ptr<AnalysisOutput> NashChoiceDialog::GetCommand() const
{
  const auto computation = GetComputation();
  const bool useBehavior = computation.representation == NashRepresentation::Behavior;
  std::shared_ptr<AnalysisOutput> output;
  if (UsesRationalOutput(computation.method)) {
    output = std::make_shared<AnalysisProfileList<Rational>>(m_doc.get(), useBehavior);
  }
  else {
    output = std::make_shared<AnalysisProfileList<double>>(m_doc.get(), useBehavior);
  }

  const wxString kind = computation.agentNash ? wxT("agent Nash ") : wxString{};
  wxString count;
  switch (computation.target) {
  case NashEquilibriumTarget::One:
    count = wxT("One ") + kind + wxT("equilibrium ");
    break;
  case NashEquilibriumTarget::Some:
    count = wxT("Some ") + kind + wxT("equilibria ");
    break;
  case NashEquilibriumTarget::All:
    count = wxT("All ") + kind + wxT("equilibria ");
    break;
  }
  const wxString representation =
      useBehavior ? wxT(" in extensive game") : wxT(" in strategic game");
  output->SetComputationSpec(computation);
  output->SetCommand(ExternalCommand(computation));
  output->SetDescription(count + MethodDescription(computation.method) +
                         ParameterDescription(computation.method) + representation);
  return output;
}
} // namespace Gambit::GUI
