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

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/stdpaths.h>

#include "dlnash.h"

namespace Gambit::GUI {
static wxString RecommendedMethodName() { return _("with Gambit's recommended method"); }
static wxString EnumPureMethodName() { return _("by looking for pure strategy equilibria"); }
static wxString EnumMixedMethodName() { return _("by enumerating extreme points"); }
static wxString EnumPolyMethodName() { return _("by solving systems of polynomial equations"); }
static wxString GnmMethodName() { return _("by global Newton tracing"); }
static wxString IpaMethodName() { return _("by iterated polymatrix approximation"); }
static wxString LpMethodName() { return _("by solving a linear program"); }
static wxString LcpMethodName() { return _("by solving a linear complementarity program"); }
static wxString LiapMethodName() { return _("by minimizing the Lyapunov function"); }
static wxString LogitMethodName() { return _("by tracing logit equilibria"); }
static wxString SimpdivMethodName() { return _("by simplicial subdivision"); }

namespace {

NashEquilibriumTarget GetTarget(int p_selection)
{
  if (p_selection == 0) {
    return NashEquilibriumTarget::One;
  }
  if (p_selection == 1) {
    return NashEquilibriumTarget::Some;
  }
  return NashEquilibriumTarget::All;
}

NashMethodSpec ResolveMethod(const wxString &p_method, NashEquilibriumTarget p_target,
                             const Game &p_game)
{
  if (p_method == RecommendedMethodName()) {
    if (p_target == NashEquilibriumTarget::One) {
      if (p_game->NumPlayers() == 2 && p_game->IsConstSum()) {
        return LPNashSpec{};
      }
      return LogitNashSpec{};
    }
    if (p_target == NashEquilibriumTarget::Some) {
      if (p_game->NumPlayers() == 2) {
        return LCPNashSpec{};
      }
      return SimpdivNashSpec{};
    }
    return EnumMixedNashSpec{};
  }
  if (p_method == EnumPureMethodName()) {
    return EnumPureNashSpec{};
  }
  if (p_method == EnumMixedMethodName()) {
    return EnumMixedNashSpec{};
  }
  if (p_method == EnumPolyMethodName()) {
    if (p_target == NashEquilibriumTarget::One) {
      return EnumPolyNashSpec{.stopAfter = 1};
    }
    return EnumPolyNashSpec{};
  }
  if (p_method == GnmMethodName()) {
    return GNMNashSpec{};
  }
  if (p_method == IpaMethodName()) {
    return IPANashSpec{};
  }
  if (p_method == LpMethodName()) {
    return LPNashSpec{};
  }
  if (p_method == LcpMethodName()) {
    return LCPNashSpec{};
  }
  if (p_method == LiapMethodName()) {
    return LiapNashSpec{};
  }
  if (p_method == LogitMethodName()) {
    return LogitNashSpec{};
  }
  if (p_method == SimpdivMethodName()) {
    if (p_target == NashEquilibriumTarget::One) {
      return SimpdivNashSpec{.startingPoints = 1};
    }
    return SimpdivNashSpec{};
  }
  throw std::logic_error("Unknown Nash equilibrium method");
}

template <class M>
concept StrategicMethod =
    std::same_as<M, EnumPureNashSpec> || std::same_as<M, EnumMixedNashSpec> ||
    std::same_as<M, GNMNashSpec> || std::same_as<M, IPANashSpec> ||
    std::same_as<M, LiapNashSpec> || std::same_as<M, SimpdivNashSpec>;

template <class M>
concept RationalOutputMethod =
    std::same_as<M, EnumPureNashSpec> || std::same_as<M, EnumMixedNashSpec> ||
    std::same_as<M, LPNashSpec> || std::same_as<M, LCPNashSpec>;

bool RequiresStrategicRepresentation(const NashMethodSpec &p_method)
{
  return std::visit([]<typename Method>(const Method &) { return StrategicMethod<Method>; },
                    p_method);
}

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
          return _("in pure strategies");
        }
        else if constexpr (std::is_same_v<Method, EnumMixedNashSpec>) {
          return _("by enumeration of mixed-strategy extreme points");
        }
        else if constexpr (std::is_same_v<Method, EnumPolyNashSpec>) {
          return _("by solving polynomial systems");
        }
        else if constexpr (std::is_same_v<Method, GNMNashSpec>) {
          return _("by global Newton tracing");
        }
        else if constexpr (std::is_same_v<Method, IPANashSpec>) {
          return _("by iterated polymatrix approximation");
        }
        else if constexpr (std::is_same_v<Method, LPNashSpec>) {
          return _("by solving a linear program");
        }
        else if constexpr (std::is_same_v<Method, LCPNashSpec>) {
          return _("by solving a linear complementarity program");
        }
        else if constexpr (std::is_same_v<Method, LiapNashSpec>) {
          return _("by function minimization");
        }
        else if constexpr (std::is_same_v<Method, LogitNashSpec>) {
          return _("by logit tracing");
        }
        else {
          return _("by simplicial subdivision");
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
            return wxString::Format(_(" (stop after one equilibrium; maximum regret %.4g)"),
                                    method.maxRegret);
          }
          return wxString::Format(_(" (all supports; maximum regret %.4g)"), method.maxRegret);
        }
        else if constexpr (std::is_same_v<Method, GNMNashSpec>) {
          return wxString::Format(
              _(" (%d perturbation; ending lambda %.4g; %d steps per support cell; local Newton "
                "every %d steps, at most %d iterations)"),
              method.perturbations, method.lambdaEnd, method.steps, method.localNewtonInterval,
              method.localNewtonMaxIterations);
        }
        else if constexpr (std::is_same_v<Method, IPANashSpec>) {
          return wxString::Format(_(" (%d perturbation)"), method.perturbations);
        }
        else if constexpr (std::is_same_v<Method, LCPNashSpec>) {
          return _(" (all accessible equilibria; unlimited recursion depth)");
        }
        else if constexpr (std::is_same_v<Method, LiapNashSpec>) {
          return wxString::Format(_(" (%d random starting points; at most %d iterations; maximum "
                                    "regret %.4g)"),
                                  method.startingPoints, method.maxIterations, method.maxRegret);
        }
        else if constexpr (std::is_same_v<Method, LogitNashSpec>) {
          return wxString::Format(
              _(" (maximum regret %.4g; initial step %.4g; maximum acceleration %.4g)"),
              method.maxRegret, method.firstStep, method.maxAcceleration);
        }
        else if constexpr (std::is_same_v<Method, SimpdivNashSpec>) {
          std::ostringstream regret;
          regret << method.maxRegret;
          return wxString::Format(_(" (%d random starting points with denominator %d; grid resize "
                                    "factor %d; maximum regret "),
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
  : wxDialog(p_parent, wxID_ANY, _("Compute Nash equilibria"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc)
{
  const int S = FromDIP(5);

  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  if (m_doc->GetGame()->NumPlayers() == 2) {
    wxString countChoices[] = {_("Compute one Nash equilibrium"),
                               _("Compute some Nash equilibria"),
                               _("Compute all Nash equilibria")};
    m_countChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, countChoices);
  }
  else {
    wxString countChoices[] = {_("Compute one Nash equilibrium"),
                               _("Compute some Nash equilibria")};
    m_countChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, countChoices);
  }
  m_countChoice->SetSelection(0);

  Connect(m_countChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
          wxCommandEventHandler(NashChoiceDialog::OnCount));
  topSizer->Add(m_countChoice, 0, wxALL | wxEXPAND, S);

  if (p_doc->GetGame()->NumPlayers() == 2 && m_doc->GetGame()->IsConstSum()) {
    wxString methodChoices[] = {RecommendedMethodName(), LpMethodName(), SimpdivMethodName(), LogitMethodName(), EnumPolyMethodName()};
    m_methodChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, methodChoices);
  }
  else {
    wxString methodChoices[] = {RecommendedMethodName(), SimpdivMethodName(), LogitMethodName(), EnumPolyMethodName()};
    m_methodChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, methodChoices);
  }
  m_methodChoice->SetSelection(0);
  topSizer->Add(m_methodChoice, 0, wxALL | wxEXPAND, S);

  if (m_doc->GetGame()->IsTree()) {
    wxString repChoices[] = {_("using the extensive game"), _("using the strategic game")};
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

  UpdateRepresentationChoice();

  if (auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL)) {
    topSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, S);
  }

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  wxTopLevelWindowBase::Layout();
  CenterOnParent();
}

void NashChoiceDialog::OnCount(wxCommandEvent &p_event)
{
  m_methodChoice->Clear();
  m_methodChoice->Append(RecommendedMethodName());

  if (p_event.GetSelection() == 0) {
    if (m_doc->GetGame()->NumPlayers() == 2 && m_doc->GetGame()->IsConstSum()) {
      m_methodChoice->Append(LpMethodName());
    }
    m_methodChoice->Append(SimpdivMethodName());
    m_methodChoice->Append(LogitMethodName());
    m_methodChoice->Append(EnumPolyMethodName());
  }
  else if (p_event.GetSelection() == 1) {
    if (m_doc->GetGame()->NumPlayers() == 2) {
      m_methodChoice->Append(LcpMethodName());
    }
    m_methodChoice->Append(EnumPureMethodName());
    m_methodChoice->Append(LiapMethodName());
    m_methodChoice->Append(GnmMethodName());
    m_methodChoice->Append(IpaMethodName());
    m_methodChoice->Append(EnumPolyMethodName());
  }
  else {
    if (m_doc->GetGame()->NumPlayers() == 2) {
      m_methodChoice->Append(EnumMixedMethodName());
    }
  }
  m_methodChoice->SetSelection(0);
  UpdateRepresentationChoice();
}

void NashChoiceDialog::OnMethod(wxCommandEvent &) { UpdateRepresentationChoice(); }

void NashChoiceDialog::UpdateRepresentationChoice()
{
  if (!m_repChoice) {
    return;
  }
  const auto method = ResolveMethod(m_methodChoice->GetStringSelection(),
                                    GetTarget(m_countChoice->GetSelection()), m_doc->GetGame());
  if (RequiresStrategicRepresentation(method)) {
    m_repChoice->SetSelection(1);
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
  const auto target = GetTarget(m_countChoice->GetSelection());
  auto method = ResolveMethod(m_methodChoice->GetStringSelection(), target, m_doc->GetGame());
  const auto representation =
      !m_repChoice || m_repChoice->GetSelection() == 1 || RequiresStrategicRepresentation(method)
          ? NashRepresentation::Strategic
          : NashRepresentation::Behavior;
  return {representation, target, std::move(method),
          m_methodChoice->GetStringSelection() == RecommendedMethodName()};
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
    count = _("One equilibrium ");
    break;
  case NashEquilibriumTarget::Some:
    count = _("Some equilibria ");
    break;
  case NashEquilibriumTarget::All:
    count = _("All equilibria ");
    break;
  }
  const wxString representation =
      useBehavior ? _(" in extensive game") : _(" in strategic game");
  output->SetComputationSpec(computation);
  output->SetCommand(ExternalCommand(computation));
  output->SetDescription(count + MethodDescription(computation.method) +
                         ParameterDescription(computation.method) + representation);
  return output;
}
} // namespace Gambit::GUI
