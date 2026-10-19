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

#include <concepts>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/stdpaths.h>

#include "games.h"
#include "games/gametree.h"

#include "dlnash.h"

namespace Gambit::GUI {
static wxString s_recommended(wxT("with Gambit's recommended method"));

namespace {

NashGoal GoalFromSelection(int p_selection)
{
  if (p_selection == 0) {
    return NashGoal::One;
  }
  if (p_selection == 1) {
    return NashGoal::Several;
  }
  return NashGoal::All;
}

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

wxString MethodLabel(NashMethodId p_method)
{
  switch (p_method) {
  case NashMethodId::EnumMixed:
    return wxT("by enumerating extreme points");
  case NashMethodId::EnumPoly:
    return wxT("by solving systems of polynomial equations");
  case NashMethodId::EnumPure:
    return wxT("by looking for pure strategy equilibria");
  case NashMethodId::GNM:
    return wxT("by global Newton tracing");
  case NashMethodId::HP:
    return wxT("by the Herings-Peeters homotopy");
  case NashMethodId::IPA:
    return wxT("by iterated polymatrix approximation");
  case NashMethodId::LCP:
    return wxT("by solving a linear complementarity program");
  case NashMethodId::Liap:
    return wxT("by minimizing the Lyapunov function");
  case NashMethodId::Logit:
    return wxT("by tracing logit equilibria");
  case NashMethodId::LP:
    return wxT("by solving a linear program");
  case NashMethodId::Simpdiv:
    return wxT("by simplicial subdivision");
  }
  throw std::logic_error("Unknown Nash equilibrium method");
}

// The methods the dialog lists under each goal.
std::vector<NashMethodId> ListedMethods(NashGoal p_goal)
{
  using enum NashMethodId;
  switch (p_goal) {
  case NashGoal::One:
    return {LP, Simpdiv, Logit, EnumPoly};
  case NashGoal::Several:
    return {LCP, EnumPure, Liap, GNM, IPA, HP, EnumPoly};
  default:
    return {EnumMixed};
  }
}

// Agent-form liap and enumpure are not yet offered.
bool IsOffered(const NashVariant &p_variant)
{
  return p_variant.representation == VariantRepresentation::Strategic ||
         (p_variant.method != NashMethodId::Liap && p_variant.method != NashMethodId::EnumPure);
}

std::vector<const NashVariant *> RunnableVariants(NashMethodId p_method, const Game &p_game)
{
  std::vector<const NashVariant *> result;
  for (const auto *variant : NashVariantsOf(p_method)) {
    if (IsOffered(*variant) && !UnavailableReason(*variant, p_game)) {
      result.push_back(variant);
    }
  }
  return result;
}

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
          return prefix + wxT("enumpure") + strategic;
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

NashChoiceDialog::NashChoiceDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
  : wxDialog(p_parent, wxID_ANY, wxT("Compute Nash equilibria"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc)
{
  const int S = FromDIP(5);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  if (m_doc->GetGame()->NumPlayers() == 2) {
    wxString countChoices[] = {wxT("Compute one Nash equilibrium"),
                               wxT("Compute some Nash equilibria"),
                               wxT("Compute all Nash equilibria")};
    m_countChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, countChoices);
  }
  else {
    wxString countChoices[] = {wxT("Compute one Nash equilibrium"),
                               wxT("Compute some Nash equilibria")};
    m_countChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, countChoices);
  }
  m_countChoice->SetSelection(0);

  Connect(m_countChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
          wxCommandEventHandler(NashChoiceDialog::OnCount));
  topSizer->Add(m_countChoice, 0, wxALL | wxEXPAND, S);

  m_methodChoice = new wxChoice(this, wxID_ANY);
  topSizer->Add(m_methodChoice, 0, wxALL | wxEXPAND, S);

  if (As<GameTreeRep>(m_doc->GetGame())) {
    wxString repChoices[] = {wxT("using the extensive game"), wxT("using the strategic game")};
    m_repChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, repChoices);
    m_repChoice->SetSelection(0);
    topSizer->Add(m_repChoice, 0, wxALL | wxEXPAND, S);

    // We only need to respond to changes in method when we have an
    // extensive game
    Connect(m_methodChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
            wxCommandEventHandler(NashChoiceDialog::OnMethod));
  }
  else {
    m_repChoice = nullptr;
  }

  UpdateMethodChoice();

  if (auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL)) {
    topSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, S);
  }

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  wxTopLevelWindowBase::Layout();
  CenterOnParent();
}

NashGoal NashChoiceDialog::GetGoal() const
{
  return GoalFromSelection(m_countChoice->GetSelection());
}

void NashChoiceDialog::OnCount(wxCommandEvent &) { UpdateMethodChoice(); }

void NashChoiceDialog::OnMethod(wxCommandEvent &) { UpdateRepresentationChoice(); }

void NashChoiceDialog::UpdateMethodChoice()
{
  const auto game = m_doc->GetGame();
  m_methodChoice->Clear();
  m_methods.clear();

  m_methodChoice->Append(s_recommended);
  m_methods.emplace_back(std::nullopt);
  for (const auto method : ListedMethods(GetGoal())) {
    if (!RunnableVariants(method, game).empty()) {
      m_methodChoice->Append(MethodLabel(method));
      m_methods.emplace_back(method);
    }
  }
  m_methodChoice->SetSelection(0);
  UpdateRepresentationChoice();
}

std::vector<const NashVariant *> NashChoiceDialog::Candidates() const
{
  const auto game = m_doc->GetGame();
  const auto goal = GetGoal();
  const auto &method = m_methods.at(m_methodChoice->GetSelection());
  if (!method) {
    std::vector<const NashVariant *> result;
    for (const auto *variant : RunnableVariants(RecommendedMethod(goal, game), game)) {
      if (MeetsGoal(*variant, goal)) {
        result.push_back(variant);
      }
    }
    return result;
  }
  return RunnableVariants(*method, game);
}

void NashChoiceDialog::UpdateRepresentationChoice()
{
  if (!m_repChoice) {
    return;
  }
  const auto candidates = Candidates();
  if (candidates.size() == 1) {
    m_repChoice->SetSelection(
        candidates.front()->representation == VariantRepresentation::Strategic ? 1 : 0);
    m_repChoice->Enable(false);
  }
  else {
    m_repChoice->Enable(true);
  }
}

bool NashChoiceDialog::UseStrategic() const
{
  return GetComputation().representation == NashRepresentation::Strategic;
}

NashComputationSpec NashChoiceDialog::GetComputation() const
{
  const auto goal = GetGoal();
  const auto &chosen = m_methods.at(m_methodChoice->GetSelection());
  const bool recommended = !chosen.has_value();
  const auto candidates = Candidates();
  const auto methodId = recommended ? RecommendedMethod(goal, m_doc->GetGame()) : *chosen;

  VariantRepresentation representation;
  if (candidates.size() == 1) {
    representation = candidates.front()->representation;
  }
  else {
    representation = m_repChoice->GetSelection() == 0 ? VariantRepresentation::Behavior
                                                      : VariantRepresentation::Strategic;
  }
  return {representation == VariantRepresentation::Strategic ? NashRepresentation::Strategic
                                                             : NashRepresentation::Behavior,
          GetTarget(goal), ResolveMethod(methodId, goal), recommended};
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

  wxString count;
  switch (computation.target) {
  case NashEquilibriumTarget::One:
    count = wxT("One equilibrium ");
    break;
  case NashEquilibriumTarget::Some:
    count = wxT("Some equilibria ");
    break;
  case NashEquilibriumTarget::All:
    count = wxT("All equilibria ");
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
