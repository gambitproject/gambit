//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dllogit.h
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

#ifndef GAMBIT_GUI_DLLOGIT_H
#define GAMBIT_GUI_DLLOGIT_H

#include <fstream>
#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/thread.h>
#include <wx/grid.h>

#include "gamedoc.h"
#include "solvers/logit/logit.h"

namespace Gambit::GUI {

namespace {
wxDECLARE_EVENT(wxEVT_LOGIT_POINT, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_LOGIT_POINT, wxThreadEvent);

wxDECLARE_EVENT(wxEVT_LOGIT_FINISHED, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_LOGIT_FINISHED, wxThreadEvent);

#include "bitmaps/stop.h"
} // namespace

//!
//! Traits distinguishing the extensive-form (behavior) and strategic-form
//! (mixed) instantiations of the logit correspondence dialog: which profile
//! and QRE types to trace, how a column maps to a player/label/shading
//! group, how to prepare the game for tracing, and which solver to run.
//!
struct BehavLogitTraits {
  using ProfileType = MixedBehaviorProfile<double>;
  using QREType = LogitQRE<ProfileType>;

  static wxString ColumnLabel(const std::shared_ptr<GameDocument> &p_doc, int p_col)
  {
    const GameAction action = p_doc->GetAction(p_col);
    wxString label;
    label << action->GetInfoset()->GetNumber() << ": " << action->GetLabel();
    return label;
  }
  static GamePlayer ColumnPlayer(const std::shared_ptr<GameDocument> &p_doc, int p_col)
  {
    return p_doc->GetAction(p_col)->GetInfoset()->GetPlayer();
  }
  static int ColumnGroup(const std::shared_ptr<GameDocument> &p_doc, int p_col)
  {
    return p_doc->GetAction(p_col)->GetInfoset()->GetNumber();
  }
  // GameRep's lazy caches are not synchronized against concurrent first touch from two
  // threads, so force the sequence-form derivation the worker will need on this (the main)
  // thread before handing the game over. Constructing a profile forces
  // EnsureInfosetOrdering() via the same public code path the worker's own starting profile
  // will use.
  static void PrepareGame(const std::shared_ptr<GameDocument> &p_doc)
  {
    const MixedBehaviorProfile<double> warmup(p_doc->GetGame());
    p_doc->GetGame()->EnsureSequences();
  }
  static void Solve(const Game &p_game, const LogitEventCallbackType<QREType> &p_onEvent,
                    const CancelToken &p_cancel)
  {
    const QREType start(p_game);
    LogitBehaviorSolve(start, 1.0e-8, PathTracer::TraceDirection::Positive, 0.03, 1.1,
                       Nash::NullBehaviorCallback<double>, p_onEvent, p_cancel);
  }
};

struct MixedLogitTraits {
  using ProfileType = MixedStrategyProfile<double>;
  using QREType = LogitQRE<ProfileType>;

  static wxString ColumnLabel(const std::shared_ptr<GameDocument> &p_doc, int p_col)
  {
    const GameStrategy strategy = p_doc->GetGame()->GetStrategy(p_col);
    wxString label;
    label << strategy->GetPlayer()->GetNumber() << ": " << strategy->GetLabel();
    return label;
  }
  static GamePlayer ColumnPlayer(const std::shared_ptr<GameDocument> &p_doc, int p_col)
  {
    return p_doc->GetGame()->GetStrategy(p_col)->GetPlayer();
  }
  static int ColumnGroup(const std::shared_ptr<GameDocument> &p_doc, int p_col)
  {
    return p_doc->GetGame()->GetStrategy(p_col)->GetPlayer()->GetNumber();
  }
  // GameRep's lazy caches are not synchronized against concurrent first touch from two
  // threads, so force the strategy derivation the worker will need on this (the main) thread
  // before handing the game over.
  static void PrepareGame(const std::shared_ptr<GameDocument> &p_doc)
  {
    p_doc->BuildNfg();
    p_doc->GetGame()->GetStrategies();
  }
  static void Solve(const Game &p_game, const LogitEventCallbackType<QREType> &p_onEvent,
                    const CancelToken &p_cancel)
  {
    const QREType start(p_game);
    LogitStrategySolve(start, 1.0e-8, PathTracer::TraceDirection::Positive, 0.03, 1.1,
                       Nash::NullStrategyCallback<double>, p_onEvent, p_cancel);
  }
};

//=========================================================================
//                       class LogitTable
//=========================================================================

//!
//! Data model for the logit correspondence display: one row per
//! computed profile, received incrementally from the solver thread.
//! Row growth is deliberately batched (see AddProfile) so the grid
//! isn't resized/repainted on every single profile received.
//!
template <class Traits> class LogitTable final : public wxGridTableBase {
  std::shared_ptr<GameDocument> m_doc;
  Array<double> m_lambdas;
  Array<std::shared_ptr<typename Traits::ProfileType>> m_profiles;
  int m_numCols{0};
  int m_numRows{0};

public:
  explicit LogitTable(const std::shared_ptr<GameDocument> &p_doc) : m_doc(p_doc) {}

  int GetNumberRows() override { return m_numRows; }
  int GetNumberCols() override { return m_numCols; }
  bool IsEmptyCell(int, int) override { return false; }

  wxString GetValue(int row, int col) override
  {
    if (col == 0) {
      return {
          lexical_cast<std::string>(m_lambdas[row + 1], m_doc->GetStyle().NumDecimals()).c_str(),
          *wxConvCurrent};
    }
    const auto profile = m_profiles[row + 1];
    return {lexical_cast<std::string>((*profile)[col], m_doc->GetStyle().NumDecimals()).c_str(),
            *wxConvCurrent};
  }

  void SetValue(int, int, const wxString &) override {}

  wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind) override
  {
    auto *attr = new wxGridCellAttr();
    attr->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    attr->SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTRE);
    attr->SetReadOnly(true);

    if (col > 0) {
      attr->SetTextColour(m_doc->GetStyle().GetPlayerColor(Traits::ColumnPlayer(m_doc, col)));
      attr->SetBackgroundColour(Traits::ColumnGroup(m_doc, col) % 2 == 0
                                    ? wxColour(250, 250, 250)
                                    : wxColour(225, 225, 225));
    }
    else {
      attr->SetTextColour(*wxBLACK);
      attr->SetBackgroundColour(wxColour(250, 250, 250));
    }

    return attr;
  }

  //!
  //! wxGrid::SetColLabelValue/SetCornerLabelValue on the grid just forward
  //! to the table's own (default no-op) methods of the same name, so
  //! headers must be supplied by overriding the getters here instead.
  //!
  wxString GetColLabelValue(int col) override
  {
    if (col == 0) {
      return wxT("Lambda");
    }
    return Traits::ColumnLabel(m_doc, col);
  }
  wxString GetCornerLabelValue() const override { return wxT("#"); }

  void AddProfile(const typename Traits::QREType &p_qre, bool p_forceShow)
  {
    wxGrid *view = GetView();

    if (m_numCols == 0) {
      m_numCols = static_cast<int>(p_qre.size()) + 1;
      if (view) {
        wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, m_numCols);
        view->ProcessTableMessage(msg);
      }
    }

    m_lambdas.push_back(p_qre.GetLambda());
    m_profiles.push_back(std::make_shared<typename Traits::ProfileType>(p_qre.GetProfile()));

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
};

//=========================================================================
//                       class LogitGrid
//=========================================================================

template <class Traits> class LogitGrid final : public wxGrid {
  LogitTable<Traits> *m_gridTable;

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
  LogitGrid(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
    : wxGrid(p_parent, wxID_ANY), m_gridTable(new LogitTable<Traits>(p_doc))
  {
    SetTable(m_gridTable, true);
    EnableEditing(false);
    SetRowLabelSize(FromDIP(40));
    SetColLabelSize(FromDIP(25));

    SetCellHighlightPenWidth(0);
    SetCellHighlightROPenWidth(0);

    Bind(wxEVT_GRID_RANGE_SELECTING, &LogitGrid::OnRangeSelecting, this);
    Bind(wxEVT_GRID_SELECT_CELL, &LogitGrid::OnSelectCell, this);
  }

  void AddProfile(const typename Traits::QREType &p_qre, bool p_forceShow)
  {
    m_gridTable->AddProfile(p_qre, p_forceShow);
  }
};

//
// Traces the logit equilibrium correspondence directly, in-process, on a
// worker thread, rather than shelling out to gambit-logit and parsing its
// output. Posts each point on the branch back to the dialog as it is found.
//
template <class Traits> class LogitThreadRunner final : public wxThread {
  wxEvtHandler *m_parent;
  Game m_game;
  CancelToken m_cancel;

  void PostPoint(const typename Traits::QREType &p_qre) const
  {
    auto *event = new wxThreadEvent(wxEVT_LOGIT_POINT);
    event->SetPayload(p_qre);
    wxQueueEvent(m_parent, event);
  }

  ExitCode Entry() override
  {
    int exitCode = 0;
    try {
      Traits::Solve(
          m_game,
          [this](const LogitEvent<typename Traits::QREType> &p_event) {
            PostPoint(std::get<LogitPathEvent<typename Traits::QREType>>(p_event).qre);
          },
          m_cancel);
    }
    catch (const ComputationCanceledException &) {
      // Not an error -- LogitDialog's m_stopRequested flag (already set before
      // RequestCancel() was called) is what determines that the "finished" event below is
      // reported as a stop rather than a failure.
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
  LogitThreadRunner(wxEvtHandler *p_parent, Game p_game)
    : wxThread(wxTHREAD_JOINABLE), m_parent(p_parent), m_game(std::move(p_game))
  {
  }

  void RequestCancel() { m_cancel.RequestCancel(); }
};

//=========================================================================
//                       class LogitDialog
//=========================================================================

template <class Traits> class LogitDialog final : public wxDialog {
  std::shared_ptr<GameDocument> m_doc;
  std::unique_ptr<LogitThreadRunner<Traits>> m_runner;
  LogitGrid<Traits> *m_list;
  wxStaticText *m_statusText;
  wxButton *m_stopButton, *m_okButton, *m_saveButton;
  wxString m_output;
  bool m_stopRequested{false};

  void Start()
  {
    Traits::PrepareGame(m_doc);

    m_runner = std::make_unique<LogitThreadRunner<Traits>>(this, m_doc->GetGame());
    if (m_runner->Run() != wxTHREAD_NO_ERROR) {
      m_runner.reset();
      m_statusText->SetLabel(wxT("Failed to launch computation."));
      m_statusText->SetForegroundColour(*wxRED);
      m_okButton->Enable(true);
    }
  }

  void OnRunnerPoint(wxThreadEvent &p_event)
  {
    const auto &qre = p_event.GetPayload<typename Traits::QREType>();
    m_list->AddProfile(qre, false);

    const int decimals = m_doc->GetStyle().NumDecimals();
    wxString line(lexical_cast<std::string>(qre.GetLambda(), decimals).c_str(), *wxConvCurrent);
    for (size_t i = 1; i <= qre.size(); i++) {
      line +=
          wxT(",") + wxString(lexical_cast<std::string>(qre[i], decimals).c_str(), *wxConvCurrent);
    }
    m_output += line + wxT("\n");
  }

  void OnRunnerFinished(wxThreadEvent &p_event)
  {
    m_stopButton->Enable(false);

    if (m_runner) {
      // Joinable thread: must be waited on before it can be safely destroyed. Entry() has
      // already posted this event and is returning, so this should not block for any
      // appreciable time.
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

  void OnStop(wxCommandEvent &)
  {
    m_stopRequested = true;
    m_stopButton->Enable(false);
    if (m_runner) {
      m_runner->RequestCancel();
    }
  }

  void OnClose(wxCloseEvent &p_event)
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

  void OnSave(wxCommandEvent &)
  {
    wxFileDialog dialog(this, _("Choose file"), wxT(""), wxT(""),
                        wxT("CSV files (*.csv)|*.csv|") wxT("All files (*.*)|*.*"),
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK) {
      std::ofstream file(dialog.GetPath().mb_str());
      file << static_cast<const char *>(m_output.mb_str());
    }
  }

public:
  LogitDialog(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc)
    : wxDialog(p_parent, wxID_ANY, wxT("Compute quantal response equilibria"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_doc(p_doc), m_list(new LogitGrid<Traits>(this, m_doc))
  {
    const int S = FromDIP(5);

    auto *sizer = new wxBoxSizer(wxVERTICAL);

    auto *startSizer = new wxBoxSizer(wxHORIZONTAL);

    m_statusText =
        new wxStaticText(this, wxID_STATIC, wxT("The computation is currently in progress."));
    m_statusText->SetForegroundColour(*wxBLUE);
    startSizer->Add(m_statusText, 0, wxALL | wxALIGN_CENTER, S);

    m_stopButton =
        new wxBitmapButton(this, wxID_CANCEL, wxBitmapBundle::FromSVG(stop_svg, wxSize(24, 24)));
    m_stopButton->SetToolTip(_("Stop the computation"));
    startSizer->Add(m_stopButton, 0, wxALL | wxALIGN_CENTER, S);
    m_stopButton->Bind(wxEVT_BUTTON, &LogitDialog::OnStop, this);

    sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, S);

    m_list->SetSizeHints(wxSize(FromDIP(600), FromDIP(400)));
    sizer->Add(m_list, 1, wxALL | wxEXPAND, S);

    auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_saveButton = new wxButton(this, wxID_SAVE, wxT("Save correspondence to .csv file"));
    m_saveButton->Enable(false);
    buttonSizer->Add(m_saveButton, 0, wxALL | wxALIGN_CENTER, S);
    m_saveButton->Bind(wxEVT_BUTTON, &LogitDialog::OnSave, this);

    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(CreateStdDialogButtonSizer(wxOK), 0, wxALL | wxALIGN_CENTER, S);
    m_okButton = dynamic_cast<wxButton *>(FindWindow(wxID_OK));
    m_okButton->Enable(false);

    sizer->Add(buttonSizer, 0, wxEXPAND | wxALL, S);

    SetSizer(sizer);
    sizer->SetSizeHints(this);
    SetSize(GetBestSize());
    SetMinSize(GetSize());
    CenterOnParent();

    Bind(wxEVT_LOGIT_POINT, &LogitDialog::OnRunnerPoint, this);
    Bind(wxEVT_LOGIT_FINISHED, &LogitDialog::OnRunnerFinished, this);
    Bind(wxEVT_CLOSE_WINDOW, &LogitDialog::OnClose, this);

    Start();
  }

  ~LogitDialog() override = default;
};

using LogitBehavDialog = LogitDialog<BehavLogitTraits>;
using LogitMixedDialog = LogitDialog<MixedLogitTraits>;

void LogitStrategic(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc);

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLLOGIT_H
