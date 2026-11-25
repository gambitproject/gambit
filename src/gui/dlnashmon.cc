//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlnashmon.cc
// Dialog for monitoring progress of logit equilibrium computation
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
#include <wx/thread.h>
#include <variant>

#include "gamedoc.h"
#include "workspace/nashspec.h"
#include "dialogstyle.h"

#include "efgprofile.h"
#include "nfgprofile.h"

using namespace Gambit;
using namespace Gambit::GUI;

namespace {

wxDECLARE_EVENT(wxEVT_SOLVER_PROFILE, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_SOLVER_PROFILE, wxThreadEvent);

wxDECLARE_EVENT(wxEVT_SOLVER_FINISHED, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_SOLVER_FINISHED, wxThreadEvent);

#include "bitmaps/stop.h"

template <class T>
void AddProfile(AnalysisOutput &p_output, const MixedStrategyProfile<T> &p_profile)
{
  dynamic_cast<AnalysisProfileList<T> &>(p_output).AddProfile(p_profile);
}

template <class T>
void AddProfile(AnalysisOutput &p_output, const MixedBehaviorProfile<T> &p_profile)
{
  dynamic_cast<AnalysisProfileList<T> &>(p_output).AddProfile(p_profile);
}

//
// Runs a solver on a worker thread. Knows nothing about any particular
// method -- it just calls whatever SolverFunction it was given (see
// NashComputationSpec::MakeSolver()).
class SolverThreadRunner final : public wxThread {
  wxEvtHandler *m_parent;
  Game m_game;
  SolverFunction m_solver;
  CancelToken m_cancel;

  void PostProfile(const ComputedProfile &p_profile) const
  {
    auto *event = new wxThreadEvent(wxEVT_SOLVER_PROFILE);
    event->SetPayload(p_profile);
    wxQueueEvent(m_parent, event);
  }

  ExitCode Entry() override
  {
    int exitCode = 0;
    try {
      m_solver(m_game, [this](const ComputedProfile &p) { PostProfile(p); }, m_cancel);
    }
    // NOLINTNEXTLINE(bugprone-empty-catch)
    catch (const ComputationCanceledException &) {
      // Not an error -- NashMonitorDialog's m_stopRequested flag (already
      // set before RequestCancel() was called) is what determines that the
      // "finished" event below is reported as a stop rather than a failure.
    }
    catch (const std::exception &) {
      exitCode = 1;
    }

    auto *evt = new wxThreadEvent(wxEVT_SOLVER_FINISHED);
    evt->SetInt(exitCode);
    wxQueueEvent(m_parent, evt);
    return static_cast<wxThread::ExitCode>(nullptr);
  }

public:
  SolverThreadRunner(wxEvtHandler *p_parent, Game p_game, SolverFunction p_solver)
    : wxThread(wxTHREAD_JOINABLE), m_parent(p_parent), m_game(std::move(p_game)),
      m_solver(std::move(p_solver))
  {
  }

  void RequestCancel() { m_cancel.RequestCancel(); }
};

class NashMonitorDialog final : public wxDialog {
  std::shared_ptr<GameDocument> m_doc;
  std::unique_ptr<SolverThreadRunner> m_runner;
  wxWindow *m_profileList;
  wxStaticText *m_statusText, *m_countText;
  wxButton *m_stopButton, *m_okButton;
  std::shared_ptr<AnalysisOutput> m_output;
  bool m_stopRequested{false};

  void Start(const std::shared_ptr<AnalysisOutput> &p_command);

  void OnClose(wxCloseEvent &);
  void OnStop(wxCommandEvent &);
  void OnRunnerProfile(wxThreadEvent &);
  void OnRunnerFinished(wxThreadEvent &);

  void SetStatusRunning() const
  {
    m_statusText->SetLabel(_("The computation is currently in progress."));
    m_statusText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    m_stopButton->Enable(true);
    m_okButton->Enable(false);
  }
  void SetStatusStopping() const
  {
    m_statusText->SetLabel(_("Stopping computation..."));
    m_statusText->SetForegroundColour(wxColour(196, 128, 0));
    m_stopButton->Enable(false);
  }
  void SetStatusStopped() const
  {
    m_statusText->SetLabel(_("The computation has been stopped."));
    m_statusText->SetForegroundColour(wxColour(196, 128, 0));
    m_okButton->Enable(true);
    m_stopButton->Enable(false);
  }
  void SetStatusFinishedNormally() const
  {
    m_statusText->SetLabel(_("The computation has completed."));
    m_statusText->SetForegroundColour(wxColour(0, 192, 0));
    m_okButton->Enable(true);
    m_stopButton->Enable(false);
  }
  void SetStatusFinishedAbnormally(const wxString &p_message) const
  {
    m_statusText->SetLabel(p_message);
    m_statusText->SetForegroundColour(ErrorTextColour());
    m_okButton->Enable(true);
    m_stopButton->Enable(false);
  }

public:
  NashMonitorDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                    const std::shared_ptr<AnalysisOutput> &p_command);
};

NashMonitorDialog::NashMonitorDialog(wxWindow *p_parent,
                                     const std::shared_ptr<GameDocument> &p_doc,
                                     const std::shared_ptr<AnalysisOutput> &p_command)
  : wxDialog(p_parent, wxID_ANY, _("Computing equilibria"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc), m_output(p_command)
{
  const int gap = FromDIP(c_spaceRelated);

  auto *sizer = new wxBoxSizer(wxVERTICAL);

  auto *startSizer = new wxBoxSizer(wxHORIZONTAL);

  m_statusText =
      new wxStaticText(this, wxID_STATIC, _("The computation is currently in progress."));
  startSizer->Add(m_statusText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);

  m_countText = new wxStaticText(this, wxID_STATIC, _("Number of equilibria found so far: 0  "));
  startSizer->Add(m_countText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);

  m_stopButton =
      new wxBitmapButton(this, wxID_ANY, wxBitmapBundle::FromSVG(stop_svg, wxSize(24, 24)));
  m_stopButton->Enable(false);
  m_stopButton->SetToolTip(_("Stop the computation"));
  startSizer->Add(m_stopButton, 0, wxALIGN_CENTER_VERTICAL);

  sizer->Add(startSizer, 0, wxALIGN_CENTER | wxBOTTOM, gap);

  if (p_command->IsBehavior()) {
    m_profileList = new MixedBehaviorProfileList(this, m_doc);
  }
  else {
    m_profileList = new MixedStrategyProfileList(this, m_doc);
  }
  m_profileList->SetMinSize(wxSize(FromDIP(500), FromDIP(300)));
  sizer->Add(m_profileList, 1, wxEXPAND);

  SetDialogLayout(this, sizer, wxOK);
  m_okButton = dynamic_cast<wxButton *>(FindWindow(wxID_OK));
  m_okButton->Enable(false);

  Bind(wxEVT_SOLVER_PROFILE, &NashMonitorDialog::OnRunnerProfile, this);
  Bind(wxEVT_SOLVER_FINISHED, &NashMonitorDialog::OnRunnerFinished, this);
  m_stopButton->Bind(wxEVT_BUTTON, &NashMonitorDialog::OnStop, this);
  Bind(wxEVT_CLOSE_WINDOW, &NashMonitorDialog::OnClose, this);

  Start(p_command);
}

void NashMonitorDialog::Start(const std::shared_ptr<AnalysisOutput> &p_command)
{
  if (!p_command->IsBehavior()) {
    // Make sure we have a normal form representation
    m_doc->BuildNfg();
  }

  m_doc->DoAddEquilibriumOutput(p_command);

  const auto &spec = p_command->GetComputationSpec();
  if (!spec) {
    throw std::logic_error("Equilibrium computation has no specification");
  }

  // Force whichever of the game's lazily-built caches this computation
  // will need to be built on this (the main) thread before handing the
  // game to the worker thread -- GameRep's lazy caches are not
  // synchronized against concurrent first touch from two threads. Only
  // warm what's actually needed for the representation in play: forcing
  // GetStrategies()'s reduced-strategy derivation for a behavior-form
  // solve would needlessly do (potentially expensive) extra work on the
  // main thread that the computation itself never uses.
  if (spec->representation == NashRepresentation::Behavior) {
    // EnsureInfosetOrdering() itself is protected; constructing a
    // profile forces it via the same public code path the worker's own
    // starting profile will use.
    const MixedBehaviorProfile<double> warmup(m_doc->GetGame());
    m_doc->GetGame()->EnsureSequences();
  }
  else {
    m_doc->GetGame()->GetStrategies();
  }

  m_runner = std::make_unique<SolverThreadRunner>(this, m_doc->GetGame(), spec->MakeSolver());
  if (m_runner->Run() != wxTHREAD_NO_ERROR) {
    m_runner.reset();
    SetStatusFinishedAbnormally(_("Failed to launch solver thread."));
    return;
  }
  SetStatusRunning();
}

void NashMonitorDialog::OnRunnerProfile(wxThreadEvent &p_event)
{
  const auto &profile = p_event.GetPayload<ComputedProfile>();
  std::visit([this](const auto &p) { AddProfile(*m_output, p); }, profile);
  m_doc->DoAnalysisOutputChanged();
  wxString label;
  label << _("Number of equilibria found so far: ") << m_output->NumProfiles();
  m_countText->SetLabel(label);
}

void NashMonitorDialog::OnRunnerFinished(wxThreadEvent &p_event)
{
  m_stopButton->Enable(false);

  if (m_runner) {
    // Joinable thread: must be waited on before it can be safely destroyed.
    // Entry() has already posted this event and is returning, so this
    // should not block for any appreciable time.
    m_runner->Wait();
    m_runner.reset();
  }

  if (m_stopRequested) {
    SetStatusStopped();
  }
  else if (p_event.GetInt() == 0) {
    SetStatusFinishedNormally();
  }
  else {
    SetStatusFinishedAbnormally(_("The computation ended abnormally."));
  }
}

void NashMonitorDialog::OnStop(wxCommandEvent &)
{
  m_stopRequested = true;
  SetStatusStopping();
  if (m_runner) {
    m_runner->RequestCancel();
  }
}

void NashMonitorDialog::OnClose(wxCloseEvent &p_event)
{
  if (!m_runner || !m_stopButton->IsEnabled()) {
    p_event.Skip();
    return;
  }

  m_stopRequested = true;
  SetStatusStopping();
  m_runner->RequestCancel();

  if (p_event.CanVeto()) {
    p_event.Veto();
  }
  else {
    p_event.Skip();
  }
}

} // anonymous namespace

namespace Gambit::GUI {

void ShowNashMonitorDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                           const std::shared_ptr<AnalysisOutput> &p_command)
{
  NashMonitorDialog dialog(p_parent, p_doc, p_command);
  dialog.ShowModal();
}

} // namespace Gambit::GUI
