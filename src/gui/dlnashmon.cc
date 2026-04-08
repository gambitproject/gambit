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
#include <wx/txtstrm.h>
#include <wx/process.h>
#include "wx/sheet/sheet.h"

#include "dlnashmon.h"
#include "gamedoc.h"

#include "efgprofile.h"
#include "nfgprofile.h"

namespace Gambit::GUI {

wxDECLARE_EVENT(wxEVT_EXTERNAL_RUNNER_LINE, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_EXTERNAL_RUNNER_LINE, wxThreadEvent);

wxDECLARE_EVENT(wxEVT_EXTERNAL_RUNNER_FINISHED, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_EXTERNAL_RUNNER_FINISHED, wxThreadEvent);

#include "bitmaps/stop.xpm"

class ExternalProcessRunner final : public wxEvtHandler {
  wxEvtHandler *m_parent;
  wxProcess *m_process{nullptr};
  long m_pid{0};
  wxTimer m_timer;
  wxString m_pending;

  void OnTimer(wxTimerEvent &)
  {
    PollOutput();
    if (m_process) {
      m_timer.StartOnce(1000);
    }
  }

  void OnEndProcess(wxProcessEvent &p_event)
  {
    m_timer.Stop();
    PollOutput();
    FlushPendingLine();

    auto *evt = new wxThreadEvent(wxEVT_EXTERNAL_RUNNER_FINISHED);
    evt->SetInt(p_event.GetExitCode());
    wxQueueEvent(m_parent, evt);

    m_process = nullptr;
    m_pid = 0;
  }

public:
  enum class RunnerStartResult { Ok, LaunchFailed, NoOutputPipe, StdinWriteFailed };

  explicit ExternalProcessRunner(wxEvtHandler *p_parent) : m_parent(p_parent), m_timer(this)
  {
    Bind(wxEVT_TIMER, &ExternalProcessRunner::OnTimer, this);
    Bind(wxEVT_END_PROCESS, &ExternalProcessRunner::OnEndProcess, this);
  }

  ~ExternalProcessRunner() override
  {
    m_timer.Stop();
    if (m_process) {
      delete m_process;
    }
  }

  RunnerStartResult Start(const wxString &p_command, const wxString &p_stdin)
  {
    m_process = new wxProcess(this);
    m_process->Redirect();
    m_pid = wxExecute(p_command, wxEXEC_ASYNC, m_process);
    if (m_pid == 0) {
      delete m_process;
      m_process = nullptr;
      return RunnerStartResult::LaunchFailed;
    }

    auto out = m_process->GetOutputStream();
    if (!out || !out->IsOk()) {
      Stop();
      return RunnerStartResult::NoOutputPipe;
    }

    const wxScopedCharBuffer bytes = p_stdin.utf8_str();
    const char *data = bytes.data();
    const size_t len = std::strlen(data);

    if (!out->WriteAll(data, len)) {
      Stop();
      return RunnerStartResult::StdinWriteFailed;
    }
    m_process->CloseOutput();
    m_timer.StartOnce(1000);
    return RunnerStartResult::Ok;
  }

  bool Stop()
  {
    m_timer.Stop();
    if (!m_process || m_pid == 0) {
      return false;
    }
#ifdef __WXMSW__
    constexpr wxSignal signal = wxSIGKILL;
#else
    constexpr wxSignal signal = wxSIGTERM;
#endif

    switch (const auto rc = wxProcess::Kill(m_pid, signal)) {
    case wxKILL_OK:
      return true;
    case wxKILL_NO_PROCESS:
      m_pid = 0;
      return false;
    default:
      return false;
    }
  }

  bool PollOutput()
  {
    bool result = false;

    if (!m_process || !m_process->IsInputAvailable()) {
      return result;
    }

    wxInputStream *stream = m_process->GetInputStream();

    while (m_process->IsInputAvailable()) {
      char ch;
      stream->Read(&ch, 1);
      if (stream->LastRead() != 1) {
        break;
      }

      if (ch == '\n') {
        PostLine(m_pending);
        m_pending.clear();
        result = true;
      }
      else if (ch != '\r') {
        m_pending += ch;
      }
    }
    return result;
  }

  void FlushPendingLine()
  {
    if (!m_pending.empty()) {
      PostLine(m_pending);
      m_pending.clear();
    }
  }

  void PostLine(const wxString &line) const
  {
    auto *evt = new wxThreadEvent(wxEVT_EXTERNAL_RUNNER_LINE);
    evt->SetString(line);
    wxQueueEvent(m_parent, evt);
  }
};

NashMonitorDialog::NashMonitorDialog(wxWindow *p_parent, GameDocument *p_doc,
                                     const std::shared_ptr<AnalysisOutput> &p_command)
  : wxDialog(p_parent, wxID_ANY, wxT("Computing Nash equilibria"), wxDefaultPosition),
    m_doc(p_doc), m_output(p_command)
{
  auto *sizer = new wxBoxSizer(wxVERTICAL);

  auto *startSizer = new wxBoxSizer(wxHORIZONTAL);

  m_statusText =
      new wxStaticText(this, wxID_STATIC, wxT("The computation is currently in progress."));
  m_statusText->SetForegroundColour(*wxBLUE);
  startSizer->Add(m_statusText, 0, wxALL | wxALIGN_CENTER, 5);

  m_countText = new wxStaticText(this, wxID_STATIC, wxT("Number of equilibria found so far: 0  "));
  startSizer->Add(m_countText, 0, wxALL | wxALIGN_CENTER, 5);

  m_stopButton = new wxBitmapButton(this, wxID_CANCEL, wxBitmap(stop_xpm));
  m_stopButton->Enable(false);
  m_stopButton->SetToolTip(_("Stop the computation"));
  startSizer->Add(m_stopButton, 0, wxALL | wxALIGN_CENTER, 5);

  Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(NashMonitorDialog::OnStop));

  sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, 5);

  if (p_command->IsBehavior()) {
    m_profileList = new BehaviorProfileList(this, m_doc);
  }
  else {
    m_profileList = new MixedProfileList(this, m_doc);
  }
  m_profileList->SetSizeHints(wxSize(500, 300));
  sizer->Add(m_profileList, 1, wxALL | wxEXPAND, 5);

  m_okButton = new wxButton(this, wxID_OK, wxT("OK"));
  sizer->Add(m_okButton, 0, wxALL | wxALIGN_RIGHT, 5);
  m_okButton->Enable(false);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  wxTopLevelWindowBase::Layout();
  CenterOnParent();

  Bind(wxEVT_EXTERNAL_RUNNER_LINE, &NashMonitorDialog::OnRunnerLine, this);
  Bind(wxEVT_EXTERNAL_RUNNER_FINISHED, &NashMonitorDialog::OnRunnerFinished, this);

  Start(p_command);
}

void NashMonitorDialog::Start(std::shared_ptr<AnalysisOutput> p_command)
{
  if (!p_command->IsBehavior()) {
    // Make sure we have a normal form representation
    m_doc->BuildNfg();
  }

  m_doc->AddProfileList(p_command);

  std::ostringstream s;
  if (p_command->IsBehavior()) {
    m_doc->GetGame()->Write(s, "efg");
  }
  else {
    m_doc->GetGame()->Write(s, "nfg");
  }

  m_runner = std::make_shared<ExternalProcessRunner>(this);
  if (auto result =
          m_runner->Start(p_command->GetCommand(), wxString(s.str().c_str(), *wxConvCurrent));
      result != ExternalProcessRunner::RunnerStartResult::Ok) {
    m_statusText->SetLabel("Failed to start solver.");
    m_statusText->SetForegroundColour(*wxRED);
    m_okButton->Enable(true);
    return;
  }

  m_stopButton->Enable(true);
}

void NashMonitorDialog::OnRunnerLine(wxThreadEvent &p_event)
{
  const wxString msg = p_event.GetString();
  if (!msg.empty()) {
    m_doc->DoAddOutput(*m_output, msg);
    wxString label;
    label << wxT("Number of equilibria found so far: ") << m_output->NumProfiles();
    m_countText->SetLabel(label);
  }
}

void NashMonitorDialog::OnRunnerFinished(wxThreadEvent &p_event)
{
  m_stopButton->Enable(false);

  if (p_event.GetInt() == 0) {
    m_statusText->SetLabel(wxT("The computation has completed."));
    m_statusText->SetForegroundColour(wxColour(0, 192, 0));
  }
  else {
    m_statusText->SetLabel(wxT("The computation ended abnormally."));
    m_statusText->SetForegroundColour(*wxRED);
  }

  m_okButton->Enable(true);
}

void NashMonitorDialog::OnStop(wxCommandEvent &)
{
  m_runner->Stop();
  m_stopButton->Enable(false);
}

} // namespace Gambit::GUI
