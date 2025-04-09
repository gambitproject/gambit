//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#include <wx/image.h>
#include <wx/splash.h>

#include "gambit.h"

#include "app.h"
#include "gameframe.h"

gbtApplication::gbtApplication() : m_fileHistory(10) {}

bool gbtApplication::OnInit()
{
#include "bitmaps/gambitbig.xpm"
  wxConfigBase::Set(new wxConfig(_T("Gambit"), _T("Gambit")));
  m_fileHistory.Load(*wxConfigBase::Get());
  // Immediately saving this back forces the entries to be created at
  // the "top level" of the config file when using the wxFileConfig
  // implementation (which seems to still be buggy).
  // m_fileHistory.Save(config);
  wxConfigBase::Get()->Read(_T("/General/CurrentDirectory"), &m_currentDir, _T(""));

  const wxBitmap bitmap(gambitbig_xpm);
  /*wxSplashScreen *splash =*/
  new wxSplashScreen(bitmap, wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT, 2000, nullptr, -1,
                     wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER | wxSTAY_ON_TOP);
  wxYield();

  // Process command line arguments, if any.
  for (int i = 1; i < wxApp::argc; i++) {
    const gbtAppLoadResult result = LoadFile(wxApp::argv[i]);
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
    const Gambit::Game efg = Gambit::NewTree();
    efg->NewPlayer()->SetLabel("Player 1");
    efg->NewPlayer()->SetLabel("Player 2");
    efg->SetTitle("Untitled Extensive Game");

    auto *game = new gbtGameDocument(efg);
    (void)new gbtGameFrame(nullptr, game);
  }

  // Set up the help system.
  wxInitAllImageHandlers();

  return true;
}

gbtAppLoadResult gbtApplication::LoadFile(const wxString &p_filename)
{
  std::ifstream infile((const char *)p_filename.mb_str());
  if (!infile.good()) {
    return GBT_APP_OPEN_FAILED;
  }

  auto *doc = new gbtGameDocument(Gambit::NewTree());
  if (doc->LoadDocument(p_filename)) {
    doc->SetFilename(p_filename);
    m_fileHistory.AddFileToHistory(p_filename);
    m_fileHistory.Save(*wxConfigBase::Get());
    (void)new gbtGameFrame(nullptr, doc);
    return GBT_APP_FILE_OK;
  }
  else {
    delete doc;
  }

  try {
    const Gambit::Game nfg = Gambit::ReadGame(infile);

    m_fileHistory.AddFileToHistory(p_filename);
    m_fileHistory.Save(*wxConfigBase::Get());
    doc = new gbtGameDocument(nfg);
    doc->SetFilename(wxT(""));
    (void)new gbtGameFrame(nullptr, doc);
    return GBT_APP_FILE_OK;
  }
  catch (Gambit::InvalidFileException &) {
    return GBT_APP_PARSE_FAILED;
  }
}

void gbtApplication::SetCurrentDir(const wxString &p_dir)
{
  m_currentDir = p_dir;
  wxConfigBase::Get()->Write(_T("/General/CurrentDirectory"), p_dir);
}

bool gbtApplication::AreDocumentsModified() const
{
  return std::any_of(m_documents.begin(), m_documents.end(),
                     std::mem_fn(&gbtGameDocument::IsModified));
}

IMPLEMENT_APP(gbtApplication)
