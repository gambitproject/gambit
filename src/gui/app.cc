//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/app.cc
// Implementation of main wxApp class
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
#include <wx/display.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/scopeguard.h>

#include "games.h"

#include "app.h"
#include "dialogstyle.h"
#include "gameframe.h"
#include "welcome.h"

#include "bitmaps/gambitbig.xpm"

namespace Gambit::GUI {

static wxBitmap MakeScaledSplashBitmap(const wxBitmap &srcBmp, double fracOfShortSide)
{
  const wxPoint mouse = wxGetMousePosition();
  const int dispIdx = wxDisplay::GetFromPoint(mouse);
  const wxDisplay disp(dispIdx == wxNOT_FOUND ? 0 : dispIdx);

  const wxRect geom = disp.GetGeometry(); // pixels in that display
  const int shortSide = std::min(geom.width, geom.height);
  const int targetMax = std::max(200, int(shortSide * fracOfShortSide));

  const int w = srcBmp.GetWidth();
  const int h = srcBmp.GetHeight();
  const double s = double(targetMax) / double(std::max(w, h));

  const int newW = std::max(1, int(std::lround(w * s)));
  const int newH = std::max(1, int(std::lround(h * s)));

  wxImage img = srcBmp.ConvertToImage();
  img.Rescale(newW, newH, wxIMAGE_QUALITY_HIGH);

  return {img};
}

wxBEGIN_EVENT_TABLE(Application, wxApp) EVT_TIMER(wxID_ANY, Application::OnSplashDismissTimer)
    wxEND_EVENT_TABLE()

        bool Application::OnInit()
{
  // wxStandardPaths::GetDataDir(), where Linux installs the catalog, derives from this.
  SetAppName("gambit");

  wxConfigBase::Set(new wxConfig(_T("Gambit"), _T("Gambit")));
  m_fileHistory.Load(*wxConfigBase::Get());
  // Immediately saving this back forces the entries to be created at
  // the "top level" of the config file when using the wxFileConfig
  // implementation (which seems to still be buggy).
  // m_fileHistory.Save(config);
  wxConfigBase::Get()->Read(_T("/General/CurrentDirectory"), &m_currentDir, _T(""));

  // Process command line arguments, if any.
  for (int i = 1; i < argc; i++) {
    LoadFile(argv[i], nullptr);
  }

  if (m_documents.empty()) {
    auto *frame = new WelcomeFrame(nullptr);
    frame->Show(true);
    SetTopWindow(frame);
  }
  else {
    const wxBitmap bitmap(gambitbig_xpm);
    m_splashTimer.Start();
    m_splash = new wxSplashScreen(MakeScaledSplashBitmap(bitmap, 0.45),
                                  wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 0, nullptr,
                                  wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_splash->Show();
    m_splash->Update();
    this->CallAfter(&Application::DismissSplash);
  }
  // Set up the help system.
  wxInitAllImageHandlers();

  return true;
}

void Application::DismissSplash()
{
  if (!m_splash) {
    return;
  }

  const long minDisplay = 1000;
  const long elapsed = m_splashTimer.Time();

  if (elapsed < minDisplay) {
    m_splashDismissTimer.SetOwner(this);
    m_splashDismissTimer.StartOnce(minDisplay - elapsed);
    return;
  }

  m_splash->Destroy();
  m_splash = nullptr;
}

std::pair<AppLoadResult, std::shared_ptr<GameDocument>>
Application::ReadDocument(const wxString &p_filename, wxWindow *p_parent)
{
  const auto [result, doc] = GameDocument::Load(p_filename);

  switch (result) {
  case GameDocument::LoadResult::OpenFailed:
    if (!wxFileName::FileExists(p_filename) && ForgetRecentFile(p_filename)) {
      ShowOperationError(p_parent, _("File not found"), _("This file no longer exists."),
                         p_filename + "\n\n" +
                             _("It has been removed from the list of recent games."));
    }
    else {
      ShowOperationError(p_parent, _("Unable to open file"),
                         _("Gambit could not open this file for reading."), p_filename);
    }
    return {AppLoadResult::OpenFailed, nullptr};

  case GameDocument::LoadResult::ParseFailed:
    ShowOperationError(p_parent, _("Unable to read file"),
                       _("File is not in a format Gambit recognizes."), p_filename);
    return {AppLoadResult::ParseFailed, nullptr};

  case GameDocument::LoadResult::UnsupportedRepresentation:
    ShowOperationError(p_parent, _("Unsupported game representation"),
                       _("Action graph games are not currently supported by the "
                         "graphical interface."),
                       p_filename);
    return {AppLoadResult::UnsupportedRepresentation, nullptr};

  case GameDocument::LoadResult::Success:
    return {AppLoadResult::Success, doc};
  }
  // Every LoadResult is handled above.
  return {AppLoadResult::OpenFailed, nullptr};
}

AppLoadResult Application::LoadFile(const wxString &p_filename, wxWindow *p_parent)
{
  const auto [result, doc] = ReadDocument(p_filename, p_parent);
  if (doc) {
    m_fileHistory.AddFileToHistory(p_filename);
    m_fileHistory.Save(*wxConfigBase::Get());
    (void)new GameFrame(nullptr, doc);
  }
  return result;
}

AppLoadResult Application::LoadCatalogGame(const wxString &p_filename, const wxString &p_slug,
                                           const wxString &p_version, wxWindow *p_parent)
{
  const auto [result, doc] = ReadDocument(p_filename, p_parent);
  if (doc) {
    doc->SetCatalogOrigin(p_slug, p_version);
    (void)new GameFrame(nullptr, doc);
  }
  return result;
}

bool Application::ChooseAndLoadFile(wxWindow *p_parent)
{
  wxFileDialog dialog(p_parent, _("Choose file to open"), GetCurrentDir(), _T(""),
                      _("Gambit workspaces (*.gbt)|*.gbt|Gambit extensive games (*.efg)|*.efg|"
                        "Gambit strategic games (*.nfg)|*.nfg|All files (*.*)|*.*"));

  if (dialog.ShowModal() != wxID_OK) {
    return false;
  }
  const wxString filename = dialog.GetPath();
  SetCurrentDir(wxPathOnly(filename));
  return LoadFile(filename, p_parent) == AppLoadResult::Success;
}

std::vector<wxString> Application::GetRecentFiles() const
{
  std::vector<wxString> files;
  files.reserve(m_fileHistory.GetCount());
  for (size_t i = 0; i < m_fileHistory.GetCount(); i++) {
    files.push_back(m_fileHistory.GetHistoryFile(i));
  }
  return files;
}

namespace {
// Must be called from within a catch block.
std::string DescribeCurrentException()
{
  try {
    throw;
  }
  catch (const std::exception &ex) {
    return ex.what();
  }
  catch (...) {
    return "An unknown exception was thrown.";
  }
}

// How long after a report is dismissed that the same message is treated as a repeat of it.
constexpr long c_repeatSuppressionMs = 2000;
} // namespace

// Reached when an exception escapes an event handler. The wx default is a bare Abort/Retry/Ignore
// box that says nothing about what was thrown; report the message instead and carry on, as
// "Ignore" would.
//
// The dialog's modal loop dispatches paint and focus events, so an exception that recurs from one
// of those handlers would otherwise open a further dialog from inside this one, without end.
// Likewise, a handler that throws on every repaint would reopen the dialog each time it is
// dismissed. Exceptions raised while a report is showing, and the same message arising again
// just after one is dismissed, are therefore dropped.
bool Application::OnExceptionInMainLoop()
{
  const std::string message = DescribeCurrentException();

  if (m_reportingException ||
      (message == m_lastExceptionMessage &&
       (wxGetLocalTimeMillis() - m_lastExceptionReportEnd).ToLong() < c_repeatSuppressionMs)) {
    return true;
  }

  m_reportingException = true;
  {
    wxON_BLOCK_EXIT_SET(m_reportingException, false);
    ShowInternalError(GetTopWindow(), message);
  }
  m_lastExceptionMessage = message;
  m_lastExceptionReportEnd = wxGetLocalTimeMillis();
  return true;
}

bool Application::ForgetRecentFile(const wxString &p_filename)
{
  for (size_t i = 0; i < m_fileHistory.GetCount(); i++) {
    if (m_fileHistory.GetHistoryFile(i) == p_filename) {
      m_fileHistory.RemoveFileFromHistory(i);
      m_fileHistory.Save(*wxConfigBase::Get());
      return true;
    }
  }
  return false;
}

void Application::SetCurrentDir(const wxString &p_dir)
{
  m_currentDir = p_dir;
  wxConfigBase::Get()->Write(_T("/General/CurrentDirectory"), p_dir);
}

} // namespace Gambit::GUI

IMPLEMENT_APP(Gambit::GUI::Application)
