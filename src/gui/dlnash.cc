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

#include <sstream>
#include <stdexcept>
#include <type_traits>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/stdpaths.h>

#include "dlnash.h"

namespace Gambit::GUI {
static wxString s_recommended(wxT("with Gambit's recommended method"));
static wxString s_enumpure(wxT("by looking for pure strategy equilibria"));
static wxString s_enummixed(wxT("by enumerating extreme points"));
static wxString s_enumpoly(wxT("by solving systems of polynomial equations"));
static wxString s_gnm(wxT("by global Newton tracing"));
static wxString s_ipa(wxT("by iterated polymatrix approximation"));
static wxString s_lp(wxT("by solving a linear program"));
static wxString s_lcp(wxT("by solving a linear complementarity program"));
static wxString s_liap(wxT("by minimizing the Lyapunov function"));
static wxString s_logit(wxT("by tracing logit equilibria"));
static wxString s_simpdiv(wxT("by simplicial subdivision"));

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
  if (p_method == s_recommended) {
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
  if (p_method == s_enumpure) {
    return EnumPureNashSpec{};
  }
  if (p_method == s_enummixed) {
    return EnumMixedNashSpec{};
  }
  if (p_method == s_enumpoly) {
    auto spec = EnumPolyNashSpec{};
    if (p_target == NashEquilibriumTarget::One) {
      spec.stopAfter = 1;
    }
    return spec;
  }
  if (p_method == s_gnm) {
    return GNMNashSpec{};
  }
  if (p_method == s_ipa) {
    return IPANashSpec{};
  }
  if (p_method == s_lp) {
    return LPNashSpec{};
  }
  if (p_method == s_lcp) {
    return LCPNashSpec{};
  }
  if (p_method == s_liap) {
    return LiapNashSpec{};
  }
  if (p_method == s_logit) {
    return LogitNashSpec{};
  }
  if (p_method == s_simpdiv) {
    auto spec = SimpdivNashSpec{};
    if (p_target == NashEquilibriumTarget::One) {
      spec.startingPoints = 1;
    }
    return spec;
  }
  throw std::logic_error("Unknown Nash equilibrium method");
}

bool RequiresStrategicRepresentation(const NashMethodSpec &p_method)
{
  return std::visit(
      [](const auto &method) {
        using Method = std::decay_t<decltype(method)>;
        return std::is_same_v<Method, EnumPureNashSpec> ||
               std::is_same_v<Method, EnumMixedNashSpec> || std::is_same_v<Method, GNMNashSpec> ||
               std::is_same_v<Method, IPANashSpec> || std::is_same_v<Method, LiapNashSpec> ||
               std::is_same_v<Method, SimpdivNashSpec>;
      },
      p_method);
}

bool UsesRationalOutput(const NashMethodSpec &p_method)
{
  return std::visit(
      [](const auto &method) {
        using Method = std::decay_t<decltype(method)>;
        return std::is_same_v<Method, EnumPureNashSpec> ||
               std::is_same_v<Method, EnumMixedNashSpec> || std::is_same_v<Method, LPNashSpec> ||
               std::is_same_v<Method, LCPNashSpec>;
      },
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
      [&](const auto &method) {
        using Method = std::decay_t<decltype(method)>;
        if constexpr (std::is_same_v<Method, EnumPureNashSpec>) {
          return prefix + wxT("enumpure") + strategic;
        }
        else if constexpr (std::is_same_v<Method, EnumMixedNashSpec>) {
          return prefix + wxT("enummixed") + strategic;
        }
        else if constexpr (std::is_same_v<Method, EnumPolyNashSpec>) {
          return prefix +
                 wxString::Format("enumpoly -d 10 -e %d -m %.17g", method.stopAfter,
                                  method.maxRegret) +
                 strategic;
        }
        else if constexpr (std::is_same_v<Method, GNMNashSpec>) {
          return prefix +
                 wxString::Format("gnm -d 10 -n %d -m %.17g -c %d -f %d -i %d",
                                  method.perturbations, method.lambdaEnd, method.steps,
                                  method.localNewtonInterval, method.localNewtonMaxIterations) +
                 strategic;
        }
        else if constexpr (std::is_same_v<Method, IPANashSpec>) {
          return prefix + wxString::Format("ipa -d 10 -n %d", method.perturbations) + strategic;
        }
        else if constexpr (std::is_same_v<Method, LPNashSpec>) {
          return prefix + wxT("lp") + strategic;
        }
        else if constexpr (std::is_same_v<Method, LCPNashSpec>) {
          return prefix + wxString::Format("lcp -e %d -r %d", method.stopAfter, method.maxDepth) +
                 strategic;
        }
        else if constexpr (std::is_same_v<Method, LiapNashSpec>) {
          return prefix +
                 wxString::Format("liap -d 10 -n %d -i %d -m %.17g", method.startingPoints,
                                  method.maxIterations, method.maxRegret) +
                 strategic;
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
                 wxString(regret.str()) + strategic;
        }
      },
      p_spec.method);
}

wxString MethodDescription(const NashMethodSpec &p_method)
{
  return std::visit(
      [](const auto &method) {
        using Method = std::decay_t<decltype(method)>;
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
      [](const auto &method) {
        using Method = std::decay_t<decltype(method)>;
        if constexpr (std::is_same_v<Method, EnumPolyNashSpec>) {
          if (method.stopAfter == 1) {
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

NashChoiceDialog::NashChoiceDialog(wxWindow *p_parent, GameDocument *p_doc)
  : wxDialog(p_parent, wxID_ANY, wxT("Compute Nash equilibria"), wxDefaultPosition), m_doc(p_doc)
{
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
  topSizer->Add(m_countChoice, 0, wxALL | wxEXPAND, 5);

  if (p_doc->GetGame()->NumPlayers() == 2 && m_doc->GetGame()->IsConstSum()) {
    wxString methodChoices[] = {s_recommended, s_lp, s_simpdiv, s_logit, s_enumpoly};
    m_methodChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, methodChoices);
  }
  else {
    wxString methodChoices[] = {s_recommended, s_simpdiv, s_logit, s_enumpoly};
    m_methodChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, methodChoices);
  }
  m_methodChoice->SetSelection(0);
  topSizer->Add(m_methodChoice, 0, wxALL | wxEXPAND, 5);

  if (m_doc->GetGame()->IsTree()) {
    wxString repChoices[] = {wxT("using the extensive game"), wxT("using the strategic game")};
    m_repChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, repChoices);
    m_repChoice->SetSelection(0);
    topSizer->Add(m_repChoice, 0, wxALL | wxEXPAND, 5);

    // We only need to respond to changes in method when we have an
    // extensive game
    Connect(m_methodChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
            wxCommandEventHandler(NashChoiceDialog::OnMethod));
  }
  else {
    m_repChoice = nullptr;
  }

  UpdateRepresentationChoice();

  auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  auto *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  wxTopLevelWindowBase::Layout();
  CenterOnParent();
}

void NashChoiceDialog::OnCount(wxCommandEvent &p_event)
{
  m_methodChoice->Clear();
  m_methodChoice->Append(s_recommended);

  if (p_event.GetSelection() == 0) {
    if (m_doc->GetGame()->NumPlayers() == 2 && m_doc->GetGame()->IsConstSum()) {
      m_methodChoice->Append(s_lp);
    }
    m_methodChoice->Append(s_simpdiv);
    m_methodChoice->Append(s_logit);
    m_methodChoice->Append(s_enumpoly);
  }
  else if (p_event.GetSelection() == 1) {
    if (m_doc->GetGame()->NumPlayers() == 2) {
      m_methodChoice->Append(s_lcp);
    }
    m_methodChoice->Append(s_enumpure);
    m_methodChoice->Append(s_liap);
    m_methodChoice->Append(s_gnm);
    m_methodChoice->Append(s_ipa);
    m_methodChoice->Append(s_enumpoly);
  }
  else {
    if (m_doc->GetGame()->NumPlayers() == 2) {
      m_methodChoice->Append(s_enummixed);
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
          m_methodChoice->GetStringSelection() == s_recommended};
}

std::shared_ptr<AnalysisOutput> NashChoiceDialog::GetCommand() const
{
  const auto computation = GetComputation();
  const bool useBehavior = computation.representation == NashRepresentation::Behavior;
  std::shared_ptr<AnalysisOutput> output;
  if (UsesRationalOutput(computation.method)) {
    output = std::make_shared<AnalysisProfileList<Rational>>(m_doc, useBehavior);
  }
  else {
    output = std::make_shared<AnalysisProfileList<double>>(m_doc, useBehavior);
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
