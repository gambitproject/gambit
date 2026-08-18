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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/stdpaths.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>

#include <wx/grid.h>
#include "dlefglogit.h"

namespace Gambit::GUI {

//=========================================================================
//                       class LogitBehavTable
//=========================================================================

//!
//! Data model for the logit correspondence display: one row per
//! computed profile, received incrementally from the gambit-logit
//! subprocess. Row growth is deliberately batched (see AddProfile) so
//! the grid isn't resized/repainted on every single profile received.
//!
class LogitBehavTable final : public wxGridTableBase {
  GameDocument *m_doc;
  Array<double> m_lambdas;
  Array<std::shared_ptr<MixedBehaviorProfile<double>>> m_profiles;
  int m_numCols{0};
  int m_numRows{0};

public:
  explicit LogitBehavTable(GameDocument *p_doc) : m_doc(p_doc) {}

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

  void AddProfile(const wxString &p_text, bool p_forceShow);
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

void LogitBehavTable::AddProfile(const wxString &p_text, bool p_forceShow)
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

  wxStringTokenizer tok(p_text, wxT(","));
  const auto next = tok.GetNextToken();
  if (next == "NE") {
    return;
  }
  m_lambdas.push_back(std::stod(next.ToStdString()));
  for (size_t i = 1; i <= profile->BehaviorProfileLength(); i++) {
    try {
      (*profile)[i] = std::stod(tok.GetNextToken().ToStdString());
    }
    catch (std::out_of_range &) {
      (*profile)[i] = 0.0;
    }
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
  LogitBehavGrid(wxWindow *p_parent, GameDocument *p_doc)
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

  void AddProfile(const wxString &p_text, bool p_forceShow)
  {
    m_gridTable->AddProfile(p_text, p_forceShow);
  }
};

constexpr int GBT_ID_TIMER = 1000;
constexpr int GBT_ID_PROCESS = 1001;

BEGIN_EVENT_TABLE(LogitBehavDialog, wxDialog)
EVT_END_PROCESS(GBT_ID_PROCESS, LogitBehavDialog::OnEndProcess)
EVT_IDLE(LogitBehavDialog::OnIdle)
EVT_TIMER(GBT_ID_TIMER, LogitBehavDialog::OnTimer)
EVT_BUTTON(wxID_SAVE, LogitBehavDialog::OnSave)
END_EVENT_TABLE()

#include "bitmaps/stop.xpm"

LogitBehavDialog::LogitBehavDialog(wxWindow *p_parent, GameDocument *p_doc)
  : wxDialog(p_parent, wxID_ANY, wxT("Compute quantal response equilibria"), wxDefaultPosition),
    m_doc(p_doc), m_process(nullptr), m_behavList(new LogitBehavGrid(this, m_doc)),
    m_timer(this, GBT_ID_TIMER)
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
  Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(LogitBehavDialog::OnStop));

  sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, 5);

  m_behavList->SetSizeHints(wxSize(600, 400));
  sizer->Add(m_behavList, 0, wxALL | wxALIGN_CENTER, 5);

  auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_saveButton = new wxButton(this, wxID_SAVE, wxT("Save correspondence to .csv file"));
  m_saveButton->Enable(false);
  buttonSizer->Add(m_saveButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_okButton = new wxButton(this, wxID_OK, wxT("OK"));
  buttonSizer->Add(m_okButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_okButton->Enable(false);

  sizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  wxTopLevelWindowBase::Layout();
  CenterOnParent();
  Start();
}

void LogitBehavDialog::Start()
{
  m_process = new wxProcess(this, GBT_ID_PROCESS);
  m_process->Redirect();

#ifdef __WXMAC__
  m_pid = wxExecute(wxStandardPaths::Get().GetExecutablePath() + wxT("-logit"), wxEXEC_ASYNC,
                    m_process);
#else
  m_pid = wxExecute(wxT("gambit-logit"), wxEXEC_ASYNC, m_process);
#endif // __WXMAC__

  std::ostringstream s;
  m_doc->GetGame()->Write(s, "efg");
  wxString str(wxString(s.str().c_str(), *wxConvCurrent));

  // It is possible that the whole string won't write on one go, so
  // we should take this possibility into account.  If writing doesn't
  // complete the whole way, we take a 100-millisecond siesta and try
  // again.  (This seems to primarily be an issue with -- you guessed it --
  // Windows!)
  while (str.length() > 0) {
    wxTextOutputStream os(*m_process->GetOutputStream());

    // It appears that (at least with mingw) the string itself contains
    // only '\n' for newlines.  If we don't SetMode here, these get
    // converted to '\r\n' sequences, and so the number of characters
    // LastWrite() returns does not match the number of characters in
    // our string.  Setting this explicitly solves this problem.
    os.SetMode(wxEOL_UNIX);
    os.WriteString(str);
    str.Remove(0, m_process->GetOutputStream()->LastWrite());
    wxMilliSleep(100);
  }
  m_process->CloseOutput();

  m_timer.Start(1000, false);
}

void LogitBehavDialog::OnIdle(wxIdleEvent &p_event)
{
  if (!m_process) {
    return;
  }

  if (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();
    m_behavList->AddProfile(msg, false);
    m_output += msg;
    m_output += wxT("\n");

    p_event.RequestMore();
  }
  else {
    m_timer.Start(1000, false);
  }
}

void LogitBehavDialog::OnTimer(wxTimerEvent &) { wxWakeUpIdle(); }

void LogitBehavDialog::OnEndProcess(wxProcessEvent &p_event)
{
  m_stopButton->Enable(false);
  m_timer.Stop();

  while (m_process->IsInputAvailable()) {
    wxTextInputStream tis(*m_process->GetInputStream());

    wxString msg;
    msg << tis.ReadLine();

    if (msg != wxT("")) {
      m_behavList->AddProfile(msg, true);
      m_output += msg;
      m_output += wxT("\n");
    }
  }

  if (p_event.GetExitCode() == 0) {
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
  // Per the wxWidgets wiki, under Windows, programs that run
  // without a console window don't respond to the more polite
  // SIGTERM, so instead we must be rude and SIGKILL it.
  m_stopButton->Enable(false);

#ifdef __WXMSW__
  wxProcess::Kill(m_pid, wxSIGKILL);
#else
  wxProcess::Kill(m_pid, wxSIGTERM);
#endif // __WXMSW__
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

} // end namespace Gambit::GUI
