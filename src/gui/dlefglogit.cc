//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlefglogit.cc
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

#include <fstream>
#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/thread.h>

#include <wx/grid.h>
#include "dlefglogit.h"
#include "solvers/logit/logit.h"

namespace Gambit::GUI {

//=========================================================================
//                       class LogitBehavTable
//=========================================================================

//!
//! Data model for the logit correspondence display: one row per
//! computed profile, received incrementally from the solver thread.
//! Row growth is deliberately batched (see AddProfile) so the grid
//! isn't resized/repainted on every single profile received.
//!
class LogitBehavTable final : public wxGridTableBase {
  std::shared_ptr<GameDocument> m_doc;
  Array<double> m_lambdas;
  Array<std::shared_ptr<MixedBehaviorProfile<double>>> m_profiles;
  int m_numCols{0};
  int m_numRows{0};

public:
  explicit LogitBehavTable(const std::shared_ptr<GameDocument> &p_doc) : m_doc(p_doc) {}

  int GetNumberRows() override { return m_numRows; }
  int GetNumberCols() override { return m_numCols; }
  bool IsEmptyCell(int, int) override { return false; }
  wxString GetValue(int row, int col) override;
  void SetValue(int, int, const wxString &) override {}
  wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) override;

  //!
  //! wxGrid::SetColLabelValue/SetCornerLabelValue on the grid just forward
  //! to the table's own (default no-op) methods of the same name, so
  //! headers must be supplied by overriding the getters here instead.
  //!
  wxString GetColLabelValue(int col) override;
  wxString GetCornerLabelValue() const override { return wxT("#"); }

  void AddProfile(const LogitQREMixedBehaviorProfile &p_qre, bool p_forceShow);
};

wxString LogitBehavTable::GetColLabelValue(int col)
{
  if (col == 0) {
    return wxT("Lambda");
  }

  const GameAction action = m_doc->GetAction(col);
  wxString label;
  label << action->GetInfoset()->GetNumber() << ": " << action->GetLabel();
  return label;
}

wxString LogitBehavTable::GetValue(int row, int col)
{
  if (col == 0) {
    return {lexical_cast<std::string>(m_lambdas[row + 1], m_doc->GetStyle().NumDecimals()).c_str(),
            *wxConvCurrent};
  }

  const auto profile = m_profiles[row + 1];
  return {lexical_cast<std::string>((*profile)[col], m_doc->GetStyle().NumDecimals()).c_str(),
          *wxConvCurrent};
}

wxGridCellAttr *LogitBehavTable::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind)
{
  auto *attr = new wxGridCellAttr();
  attr->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  attr->SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTRE);
  attr->SetReadOnly(true);

  if (col > 0) {
    const GameAction action = m_doc->GetAction(col);
    attr->SetTextColour(m_doc->GetStyle().GetPlayerColor(action->GetInfoset()->GetPlayer()));
    attr->SetBackgroundColour(action->GetInfoset()->GetNumber() % 2 == 0
                                  ? wxColour(250, 250, 250)
                                  : wxColour(225, 225, 225));
  }
  else {
    attr->SetTextColour(*wxBLACK);
    attr->SetBackgroundColour(wxColour(250, 250, 250));
  }

  return attr;
}

void LogitBehavTable::AddProfile(const LogitQREMixedBehaviorProfile &p_qre, bool p_forceShow)
{
  wxGrid *view = GetView();

  if (m_numCols == 0) {
    m_numCols = static_cast<int>(m_doc->GetGame()->BehavProfileLength()) + 1;
    if (view) {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, m_numCols);
      view->ProcessTableMessage(msg);
    }
  }

  const auto profile = std::make_shared<MixedBehaviorProfile<double>>(m_doc->GetGame());
  m_lambdas.push_back(p_qre.GetLambda());
  for (size_t i = 1; i <= profile->BehaviorProfileLength(); i++) {
    (*profile)[i] = p_qre[i];
  }
  m_profiles.push_back(profile);

  if (p_forceShow || static_cast<int>(m_profiles.size()) - m_numRows > 20) {
    const int newRows = static_cast<int>(m_profiles.size()) - m_numRows;
    m_numRows = static_cast<int>(m_profiles.size());
    if (view) {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, newRows);
      view->ProcessTableMessage(msg);
      view->MakeCellVisible(m_numRows - 1, 0);
    }
  }

  // Lambda tends to get large, so this column usually needs resized
  if (view) {
    view->AutoSizeColumn(0);
  }
}

//=========================================================================
//                       class LogitBehavGrid
//=========================================================================

class LogitBehavGrid final : public wxGrid {
  LogitBehavTable *m_gridTable;

  //!
  //! @name Overriding wxGrid behavior to disable selection & the cursor
  //! highlight rectangle, matching the read-only, non-interactive table
  //! this replaces.
  //!
  //@{
  void OnRangeSelecting(wxGridRangeSelectEvent &p_event) { p_event.Veto(); }
  void OnSelectCell(wxGridEvent &p_event) { p_event.Skip(); }
  //@}

public:
  LogitBehavGrid(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
    : wxGrid(p_parent, wxID_ANY), m_gridTable(new LogitBehavTable(p_doc))
  {
    SetTable(m_gridTable, true);
    EnableEditing(false);
    SetRowLabelSize(40);
    SetColLabelSize(25);

    SetCellHighlightPenWidth(0);
    SetCellHighlightROPenWidth(0);

    Bind(wxEVT_GRID_RANGE_SELECTING, &LogitBehavGrid::OnRangeSelecting, this);
    Bind(wxEVT_GRID_SELECT_CELL, &LogitBehavGrid::OnSelectCell, this);
  }

  void AddProfile(const LogitQREMixedBehaviorProfile &p_qre, bool p_forceShow)
  {
    m_gridTable->AddProfile(p_qre, p_forceShow);
  }
};

namespace {

wxDECLARE_EVENT(wxEVT_LOGIT_POINT, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_LOGIT_POINT, wxThreadEvent);

wxDECLARE_EVENT(wxEVT_LOGIT_FINISHED, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_LOGIT_FINISHED, wxThreadEvent);

#include "bitmaps/stop.xpm"

} // namespace

//
// Traces the logit equilibrium correspondence directly, in-process, on a
// worker thread, rather than shelling out to gambit-logit and parsing its
// output. Posts each point on the branch back to the dialog as it is found.
//
class LogitBehavThreadRunner final : public wxThread {
  wxEvtHandler *m_parent;
  Game m_game;
  CancelToken m_cancel;

  void PostPoint(const LogitQREMixedBehaviorProfile &p_qre) const
  {
    auto *event = new wxThreadEvent(wxEVT_LOGIT_POINT);
    event->SetPayload(p_qre);
    wxQueueEvent(m_parent, event);
  }

  ExitCode Entry() override
  {
    int exitCode = 0;
    try {
      const LogitQREMixedBehaviorProfile start(m_game);
      LogitBehaviorSolve(
          start, 1.0e-8, 1.0, 0.03, 1.1, Nash::NullBehaviorCallback<double>,
          [this](const LogitEvent<LogitQREMixedBehaviorProfile> &p_event) {
            PostPoint(std::get<LogitPathEvent<LogitQREMixedBehaviorProfile>>(p_event).qre);
          },
          m_cancel);
    }
    catch (const ComputationCanceledException &) {
      // Not an error -- LogitBehavDialog's m_stopRequested flag (already set
      // before RequestCancel() was called) is what determines that the
      // "finished" event below is reported as a stop rather than a failure.
    }
    catch (const std::exception &) {
      exitCode = 1;
    }

    auto *evt = new wxThreadEvent(wxEVT_LOGIT_FINISHED);
    evt->SetInt(exitCode);
    wxQueueEvent(m_parent, evt);
    return static_cast<wxThread::ExitCode>(0);
  }

public:
  LogitBehavThreadRunner(wxEvtHandler *p_parent, Game p_game)
    : wxThread(wxTHREAD_JOINABLE), m_parent(p_parent), m_game(std::move(p_game))
  {
  }

  void RequestCancel() { m_cancel.RequestCancel(); }
};

LogitBehavDialog::LogitBehavDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
  : wxDialog(p_parent, wxID_ANY, wxT("Compute quantal response equilibria"), wxDefaultPosition),
    m_doc(p_doc), m_behavList(new LogitBehavGrid(this, m_doc))
{
  auto *sizer = new wxBoxSizer(wxVERTICAL);

  auto *startSizer = new wxBoxSizer(wxHORIZONTAL);

  m_statusText =
      new wxStaticText(this, wxID_STATIC, wxT("The computation is currently in progress."));
  m_statusText->SetForegroundColour(*wxBLUE);
  startSizer->Add(m_statusText, 0, wxALL | wxALIGN_CENTER, 5);

  m_stopButton = new wxBitmapButton(this, wxID_CANCEL, wxBitmap(stop_xpm));
  m_stopButton->SetToolTip(_("Stop the computation"));
  startSizer->Add(m_stopButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_stopButton->Bind(wxEVT_BUTTON, &LogitBehavDialog::OnStop, this);

  sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, 5);

  m_behavList->SetSizeHints(wxSize(600, 400));
  sizer->Add(m_behavList, 0, wxALL | wxALIGN_CENTER, 5);

  auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_saveButton = new wxButton(this, wxID_SAVE, wxT("Save correspondence to .csv file"));
  m_saveButton->Enable(false);
  buttonSizer->Add(m_saveButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_saveButton->Bind(wxEVT_BUTTON, &LogitBehavDialog::OnSave, this);

  m_okButton = new wxButton(this, wxID_OK, wxT("OK"));
  buttonSizer->Add(m_okButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_okButton->Enable(false);

  sizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  wxTopLevelWindowBase::Layout();
  CenterOnParent();

  Bind(wxEVT_LOGIT_POINT, &LogitBehavDialog::OnRunnerPoint, this);
  Bind(wxEVT_LOGIT_FINISHED, &LogitBehavDialog::OnRunnerFinished, this);
  Bind(wxEVT_CLOSE_WINDOW, &LogitBehavDialog::OnClose, this);

  Start();
}

LogitBehavDialog::~LogitBehavDialog() = default;

void LogitBehavDialog::Start()
{
  // GameRep's lazy caches are not synchronized against concurrent first
  // touch from two threads, so force the sequence-form derivation the
  // worker will need on this (the main) thread before handing the game
  // over. Constructing a profile forces EnsureInfosetOrdering() via the
  // same public code path the worker's own starting profile will use.
  const MixedBehaviorProfile<double> warmup(m_doc->GetGame());
  m_doc->GetGame()->EnsureSequences();

  m_runner = std::make_unique<LogitBehavThreadRunner>(this, m_doc->GetGame());
  if (m_runner->Run() != wxTHREAD_NO_ERROR) {
    m_runner.reset();
    m_statusText->SetLabel(wxT("Failed to launch computation."));
    m_statusText->SetForegroundColour(*wxRED);
    m_okButton->Enable(true);
  }
}

void LogitBehavDialog::OnRunnerPoint(wxThreadEvent &p_event)
{
  const auto &qre = p_event.GetPayload<LogitQREMixedBehaviorProfile>();
  m_behavList->AddProfile(qre, false);

  const int decimals = m_doc->GetStyle().NumDecimals();
  wxString line(lexical_cast<std::string>(qre.GetLambda(), decimals).c_str(), *wxConvCurrent);
  for (size_t i = 1; i <= qre.size(); i++) {
    line +=
        wxT(",") + wxString(lexical_cast<std::string>(qre[i], decimals).c_str(), *wxConvCurrent);
  }
  m_output += line + wxT("\n");
}

void LogitBehavDialog::OnRunnerFinished(wxThreadEvent &p_event)
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
    m_statusText->SetLabel(wxT("The computation has been stopped."));
    m_statusText->SetForegroundColour(wxColour(196, 128, 0));
  }
  else if (p_event.GetInt() == 0) {
    m_statusText->SetLabel(wxT("The computation has completed."));
    m_statusText->SetForegroundColour(wxColour(0, 192, 0));
  }
  else {
    m_statusText->SetLabel(wxT("The computation ended abnormally."));
    m_statusText->SetForegroundColour(*wxRED);
  }

  m_okButton->Enable(true);
  m_saveButton->Enable(true);
}

void LogitBehavDialog::OnStop(wxCommandEvent &)
{
  m_stopRequested = true;
  m_stopButton->Enable(false);
  if (m_runner) {
    m_runner->RequestCancel();
  }
}

void LogitBehavDialog::OnClose(wxCloseEvent &p_event)
{
  if (!m_runner || !m_stopButton->IsEnabled()) {
    p_event.Skip();
    return;
  }

  m_stopRequested = true;
  m_stopButton->Enable(false);
  m_runner->RequestCancel();

  if (p_event.CanVeto()) {
    p_event.Veto();
  }
  else {
    p_event.Skip();
  }
}

void LogitBehavDialog::OnSave(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), wxT(""), wxT(""),
                      wxT("CSV files (*.csv)|*.csv|") wxT("All files (*.*)|*.*"),
                      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    std::ofstream file(dialog.GetPath().mb_str());
    file << static_cast<const char *>(m_output.mb_str());
  }
}

} // namespace Gambit::GUI
