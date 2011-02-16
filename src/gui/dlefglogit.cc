//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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
#endif  // WX_PRECOMP
#include <wx/stdpaths.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>

#include "wx/sheet/sheet.h"
#include "dlefglogit.h"

class gbtLogitBehavList : public wxSheet {
private:
  gbtGameDocument *m_doc;
  Gambit::List<double> m_lambdas;
  Gambit::List<Gambit::MixedBehavProfile<double> > m_profiles;

  // Overriding wxSheet members for data access
  wxString GetCellValue(const wxSheetCoords &);
  wxSheetCellAttr GetAttr(const wxSheetCoords &p_coords, wxSheetAttr_Type) const;
  
  // Overriding wxSheet members to disable selection behavior
  bool SelectRow(int, bool = false, bool = false)
    { return false; }
  bool SelectRows(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCol(int, bool = false, bool = false)
    { return false; }
  bool SelectCols(int, int, bool = false, bool = false)
    { return false; }
  bool SelectCell(const wxSheetCoords&, bool = false, bool = false)
    { return false; }
  bool SelectBlock(const wxSheetBlock&, bool = false, bool = false)
    { return false; }
  bool SelectAll(bool = false) { return false; }

  // Overriding wxSheet member to suppress drawing of cursor
  void DrawCursorCellHighlight(wxDC&, const wxSheetCellAttr &) { }

public:
  gbtLogitBehavList(wxWindow *p_parent, gbtGameDocument *p_doc);
  virtual ~gbtLogitBehavList();

  void AddProfile(const wxString &p_text, bool p_forceShow);
};

gbtLogitBehavList::gbtLogitBehavList(wxWindow *p_parent, 
				     gbtGameDocument *p_doc) 
  : wxSheet(p_parent, -1), m_doc(p_doc)
{
  CreateGrid(0, 0);
  SetRowLabelWidth(40);
  SetColLabelHeight(25);
}

gbtLogitBehavList::~gbtLogitBehavList()
{ }

wxString gbtLogitBehavList::GetCellValue(const wxSheetCoords &p_coords)
{
  if (IsRowLabelCell(p_coords)) {
    return wxString::Format(wxT("%d"), p_coords.GetRow() + 1);
  }
  else if (IsColLabelCell(p_coords)) {
    if (p_coords.GetCol() == 0) {
      return wxT("Lambda");
    }
    else {
      Gambit::GameAction action = m_doc->GetGame()->GetAction(p_coords.GetCol());
      return (wxString::Format(wxT("%d: "), 
			       action->GetInfoset()->GetNumber())+
	      wxString(action->GetLabel().c_str(), *wxConvCurrent));
    }
  }
  else if (IsCornerLabelCell(p_coords)) {
    return wxT("#");
  }


  if (p_coords.GetCol() == 0) {
    return wxString(Gambit::lexical_cast<std::string>(m_lambdas[p_coords.GetRow()+1],
				   m_doc->GetStyle().NumDecimals()).c_str(),
		    *wxConvCurrent);
  }
  else {
    const Gambit::MixedBehavProfile<double> &profile = m_profiles[p_coords.GetRow()+1];
    return wxString(Gambit::lexical_cast<std::string>(profile[p_coords.GetCol()],
				   m_doc->GetStyle().NumDecimals()).c_str(), 
		    *wxConvCurrent);
  }
}

static wxColour GetPlayerColor(gbtGameDocument *p_doc, int p_index)
{
  if (p_index == 0)  return *wxBLACK;

  Gambit::GameAction action = p_doc->GetGame()->GetAction(p_index);
  return p_doc->GetStyle().GetPlayerColor(action->GetInfoset()->GetPlayer()->GetNumber());
}

wxSheetCellAttr gbtLogitBehavList::GetAttr(const wxSheetCoords &p_coords, 
					   wxSheetAttr_Type) const
{
  if (IsRowLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultRowLabelAttr);
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    return attr;
  }
  else if (IsColLabelCell(p_coords)) {
    wxSheetCellAttr attr(GetSheetRefData()->m_defaultColLabelAttr);
    attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    attr.SetAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
    attr.SetOrientation(wxHORIZONTAL);
    attr.SetReadOnly(true);
    attr.SetForegroundColour(GetPlayerColor(m_doc, p_coords.GetCol()));
    return attr;
  }
  else if (IsCornerLabelCell(p_coords)) {
    return GetSheetRefData()->m_defaultCornerLabelAttr;
  }

  wxSheetCellAttr attr(GetSheetRefData()->m_defaultGridCellAttr);
  attr.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL)); 
  attr.SetAlignment(wxALIGN_RIGHT, wxALIGN_CENTER);
  attr.SetOrientation(wxHORIZONTAL);
  if (p_coords.GetCol() > 0) {
    Gambit::GameAction action = m_doc->GetGame()->GetAction(p_coords.GetCol());
    attr.SetForegroundColour(m_doc->GetStyle().GetPlayerColor(action->GetInfoset()->GetPlayer()->GetNumber()));
    if (action->GetInfoset()->GetNumber() % 2 == 0) {
      attr.SetBackgroundColour(wxColour(250, 250, 250));
    }
    else {
      attr.SetBackgroundColour(wxColour(225, 225, 225));
    }
  }
  else {
    attr.SetForegroundColour(*wxBLACK);
    attr.SetBackgroundColour(wxColour(250, 250, 250));
  }
			   
  attr.SetReadOnly(true);
  return attr;
}

void gbtLogitBehavList::AddProfile(const wxString &p_text,
				   bool p_forceShow)
{
  if (GetNumberCols() == 0) {
    AppendCols(m_doc->GetGame()->BehavProfileLength() + 1);
  }

  Gambit::MixedBehavProfile<double> profile(m_doc->GetGame());

  wxStringTokenizer tok(p_text, wxT(","));

  m_lambdas.Append((double) Gambit::lexical_cast<Gambit::Rational>(std::string((const char *) tok.GetNextToken().mb_str())));

  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = Gambit::lexical_cast<Gambit::Rational>(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  m_profiles.Append(profile);
  if (p_forceShow || m_profiles.Length() - GetNumberRows() > 20) {
    AppendRows(m_profiles.Length() - GetNumberRows());
    MakeCellVisible(wxSheetCoords(GetNumberRows() - 1, 0));
  }

  // Lambda tends to get large, so this column usually needs resized
  AutoSizeCol(0);
}


const int GBT_ID_TIMER = 1000;
const int GBT_ID_PROCESS = 1001;

BEGIN_EVENT_TABLE(gbtLogitBehavDialog, wxDialog)
  EVT_END_PROCESS(GBT_ID_PROCESS, gbtLogitBehavDialog::OnEndProcess)
  EVT_IDLE(gbtLogitBehavDialog::OnIdle)
  EVT_TIMER(GBT_ID_TIMER, gbtLogitBehavDialog::OnTimer)
  EVT_BUTTON(wxID_SAVE, gbtLogitBehavDialog::OnSave)
END_EVENT_TABLE()

#include "bitmaps/stop.xpm"

gbtLogitBehavDialog::gbtLogitBehavDialog(wxWindow *p_parent, 
					 gbtGameDocument *p_doc)
  : wxDialog(p_parent, -1, wxT("Compute quantal response equilibria"),
	     wxDefaultPosition),
    m_doc(p_doc), m_process(0), m_timer(this, GBT_ID_TIMER)
{
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *startSizer = new wxBoxSizer(wxHORIZONTAL);

  m_statusText = new wxStaticText(this, wxID_STATIC,
				  wxT("The computation is currently in progress."));
  m_statusText->SetForegroundColour(*wxBLUE);
  startSizer->Add(m_statusText, 0, wxALL | wxALIGN_CENTER, 5);

  m_stopButton = new wxBitmapButton(this, wxID_CANCEL, wxBitmap(stop_xpm));
  m_stopButton->SetToolTip(_("Stop the computation"));
  startSizer->Add(m_stopButton, 0, wxALL | wxALIGN_CENTER, 5);
  Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtLogitBehavDialog::OnStop));

  sizer->Add(startSizer, 0, wxALL | wxALIGN_CENTER, 5);

  m_behavList = new gbtLogitBehavList(this, m_doc);
  m_behavList->SetSizeHints(wxSize(600, 400));
  sizer->Add(m_behavList, 0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  m_saveButton = new wxButton(this, wxID_SAVE, 
			      wxT("Save correspondence to .csv file"));
  m_saveButton->Enable(false);
  buttonSizer->Add(m_saveButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_okButton = new wxButton(this, wxID_OK, wxT("OK"));
  buttonSizer->Add(m_okButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_okButton->Enable(false);

  sizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
  Start();
}

void gbtLogitBehavDialog::Start(void)
{
  m_process = new wxProcess(this, GBT_ID_PROCESS);
  m_process->Redirect();

#ifdef __WXMAC__
  m_pid = wxExecute(wxStandardPaths::Get().GetExecutablePath() + wxT("-logit"),
		    wxEXEC_ASYNC, m_process);
#else	
  m_pid = wxExecute(wxT("gambit-logit"), wxEXEC_ASYNC, m_process);
#endif // __WXMAC__
  
  std::ostringstream s;
  m_doc->GetGame()->WriteEfgFile(s);
  wxString str(wxString(s.str().c_str(), *wxConvCurrent));
  
  // It is possible that the whole string won't write on one go, so
  // we should take this possibility into account.  If the write doesn't
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

void gbtLogitBehavDialog::OnIdle(wxIdleEvent &p_event)
{
  if (!m_process)  return;

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

void gbtLogitBehavDialog::OnTimer(wxTimerEvent &p_event)
{
  wxWakeUpIdle();
}

void gbtLogitBehavDialog::OnEndProcess(wxProcessEvent &p_event)
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

void gbtLogitBehavDialog::OnStop(wxCommandEvent &)
{
  // Per the wxWidgets wiki, under Windows, programs that run
  // without a console window don't respond to the more polite
  // SIGTERM, so instead we must be rude and SIGKILL it.
  m_stopButton->Enable(false);

#ifdef __WXMSW__
  wxProcess::Kill(m_pid, wxSIGKILL);
#else
  wxProcess::Kill(m_pid, wxSIGTERM);
#endif  // __WXMSW__
}

void gbtLogitBehavDialog::OnSave(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Choose file"), wxT(""), wxT(""),
		      wxT("CSV files (*.csv)|*.csv|")
			  wxT("All files (*.*)|*.*"),
		      wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    std::ofstream file((const char *) dialog.GetPath().mb_str());
    file << ((const char *) m_output.mb_str());
  }
}
