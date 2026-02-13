//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/gambit.cc
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

#include <fstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/display.h>
#include <wx/image.h>

#include "gambit.h"

#include "app.h"
#include "gameframe.h"

#include "bitmaps/gambitbig.xpm"

namespace Gambit::GUI {

static wxBitmap MakeScaledSplashBitmap(const wxBitmap &srcBmp, double fracOfShortSide)
{
  const wxPoint mouse = wxGetMousePosition();
  const int dispIdx = wxDisplay::GetFromPoint(mouse);
  wxDisplay disp(dispIdx == wxNOT_FOUND ? 0 : dispIdx);

  wxRect geom = disp.GetGeometry(); // pixels in that display
  const int shortSide = std::min(geom.width, geom.height);
  const int targetMax = std::max(200, int(shortSide * fracOfShortSide));

  const int w = srcBmp.GetWidth();
  const int h = srcBmp.GetHeight();
  const double s = double(targetMax) / double(std::max(w, h));

  const int newW = std::max(1, int(std::lround(w * s)));
  const int newH = std::max(1, int(std::lround(h * s)));

  wxImage img = srcBmp.ConvertToImage();
  img.Rescale(newW, newH, wxIMAGE_QUALITY_HIGH);

  return wxBitmap(img);
}

wxBEGIN_EVENT_TABLE(Application, wxApp) EVT_TIMER(wxID_ANY, Application::OnSplashDismissTimer)
    wxEND_EVENT_TABLE()

        bool Application::OnInit()
{
  wxApp::OnInit();

  const wxBitmap bitmap(gambitbig_xpm);
  m_splashTimer.Start();
  m_splash = new wxSplashScreen(MakeScaledSplashBitmap(bitmap, 0.45),
                                wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 0, nullptr,
                                wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
  m_splash->Show();
  m_splash->Update();
  wxConfigBase::Set(new wxConfig(_T("Gambit"), _T("Gambit")));
  m_fileHistory.Load(*wxConfigBase::Get());
  // Immediately saving this back forces the entries to be created at
  // the "top level" of the config file when using the wxFileConfig
  // implementation (which seems to still be buggy).
  // m_fileHistory.Save(config);
  wxConfigBase::Get()->Read(_T("/General/CurrentDirectory"), &m_currentDir, _T(""));

  // Process command line arguments, if any.
  for (int i = 1; i < wxApp::argc; i++) {
    const AppLoadResult result = LoadFile(wxApp::argv[i]);
    if (result == GBT_APP_OPEN_FAILED) {
      wxMessageDialog dialog(
          nullptr, wxT("Gambit could not open file '") + wxApp::argv[i] + wxT("' for reading."),
          wxT("Unable to open file"), wxOK | wxICON_ERROR);
      dialog.ShowModal();
    }
    else if (result == GBT_APP_PARSE_FAILED) {
      wxMessageDialog dialog(
          nullptr, wxT("File '") + wxApp::argv[i] + wxT("' is not in a format Gambit recognizes."),
          wxT("Unable to read file"), wxOK | wxICON_ERROR);
      dialog.ShowModal();
    }
  }

  if (m_documents.size() == 0) {
    // If we don't have any game files -- whether because none were
    // specified on the command line, or because those specified couldn't
    // be read -- create a default document.
    const Game efg = NewTree();
    efg->NewPlayer()->SetLabel("Player 1");
    efg->NewPlayer()->SetLabel("Player 2");
    efg->SetTitle("Untitled Extensive Game");

    auto *game = new GameDocument(efg);
    (void)new GameFrame(nullptr, game);
  }

  // Set up the help system.
  wxInitAllImageHandlers();

  this->CallAfter(&Application::DismissSplash);

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

AppLoadResult Application::LoadFile(const wxString &p_filename)
{
  std::ifstream infile((const char *)p_filename.mb_str());
  if (!infile.good()) {
    return GBT_APP_OPEN_FAILED;
  }

  auto *doc = new GameDocument(NewTree());
  if (doc->LoadDocument(p_filename)) {
    doc->SetFilename(p_filename);
    m_fileHistory.AddFileToHistory(p_filename);
    m_fileHistory.Save(*wxConfigBase::Get());
    (void)new GameFrame(nullptr, doc);
    return GBT_APP_FILE_OK;
  }
  else {
    delete doc;
  }

  try {
    const Game nfg = ReadGame(infile);

    m_fileHistory.AddFileToHistory(p_filename);
    m_fileHistory.Save(*wxConfigBase::Get());
    doc = new GameDocument(nfg);
    doc->SetFilename(wxT(""));
    (void)new GameFrame(nullptr, doc);
    return GBT_APP_FILE_OK;
  }
  catch (InvalidFileException &) {
    return GBT_APP_PARSE_FAILED;
  }
}

void Application::SetCurrentDir(const wxString &p_dir)
{
  m_currentDir = p_dir;
  wxConfigBase::Get()->Write(_T("/General/CurrentDirectory"), p_dir);
}

bool Application::AreDocumentsModified() const
{
  return std::any_of(m_documents.begin(), m_documents.end(),
                     std::mem_fn(&GameDocument::IsModified));
}

} // namespace Gambit::GUI

IMPLEMENT_APP(Gambit::GUI::Application)
