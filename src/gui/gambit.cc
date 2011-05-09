//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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
#endif  // WX_PRECOMP
#include <wx/image.h>
#include <wx/splash.h>

#include "libgambit/libgambit.h"

#include "gambit.h"
#include "gameframe.h"

gbtApplication::gbtApplication(void)
  : m_fileHistory(5)
{ }

bool gbtApplication::OnInit(void)
{
#include "bitmaps/gambitbig.xpm"
  wxConfig config(_T("Gambit"));
  m_fileHistory.Load(config);
  // Immediately saving this back forces the entries to be created at
  // the "top level" of the config file when using the wxFileConfig
  // implementation (which seems to still be buggy).
  m_fileHistory.Save(config);

  config.Read(_T("/General/CurrentDirectory"), &m_currentDir, _T(""));

  wxBitmap bitmap(gambitbig_xpm);
  /*wxSplashScreen *splash =*/
    new wxSplashScreen(bitmap,
		       wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
		       2000, NULL, -1, wxDefaultPosition, wxDefaultSize,
		       wxSIMPLE_BORDER | wxSTAY_ON_TOP);
  wxYield();

  // The number of game files successfully opened
  int nGames = 0;

  // Process command line arguments, if any.
  for (int i = 1; i < argc; i++) {
    gbtAppLoadResult result = LoadFile(argv[i]);
    if (result == GBT_APP_OPEN_FAILED) {
      wxMessageDialog dialog(0,
			     wxT("Gambit could not open file '") + 
			     wxString(argv[i], *wxConvCurrent) + 
			     wxT("' for reading."), 
			     wxT("Unable to open file"),
			     wxOK | wxICON_ERROR);
      dialog.ShowModal();
    }
    else if (result == GBT_APP_PARSE_FAILED) {
      wxMessageDialog dialog(0,
			     wxT("File '") +
			     wxString(argv[i], *wxConvCurrent) +
			     wxT("' is not in a format Gambit recognizes."),
			     wxT("Unable to read file"),
			     wxOK | wxICON_ERROR);
      dialog.ShowModal();
    }
    else {
      nGames++;
    }
  }

  if (nGames == 0) {
    // If we don't have any game files -- whether because none were
    // specified on the command line, or because those specified couldn't
    // be read -- create a default document.
    Gambit::Game efg = new Gambit::GameRep;
    efg->NewPlayer()->SetLabel("Player 1");
    efg->NewPlayer()->SetLabel("Player 2");
    efg->SetTitle("Untitled Extensive Game");

    gbtGameDocument *game = new gbtGameDocument(efg);
    (void) new gbtGameFrame(0, game);
  }

  // Set up the help system.
  wxInitAllImageHandlers();

  return true;
}

gbtApplication::~gbtApplication()
{
  wxConfig config(_T("Gambit"));
  m_fileHistory.Save(config);
}

gbtAppLoadResult gbtApplication::LoadFile(const wxString &p_filename)
{    
  std::ifstream infile((const char *) p_filename.mb_str());
  if (!infile.good()) {
    return GBT_APP_OPEN_FAILED;
  }

  gbtGameDocument *doc = new gbtGameDocument(new Gambit::GameRep);
  if (doc->LoadDocument(p_filename)) {
    doc->SetFilename(p_filename);
    m_fileHistory.AddFileToHistory(p_filename);
    (void) new gbtGameFrame(0, doc);
    return GBT_APP_FILE_OK;
  }
  else {
    delete doc;
  }

  try {
    Gambit::Game nfg = Gambit::ReadGame(infile);

    m_fileHistory.AddFileToHistory(p_filename);
    gbtGameDocument *doc = new gbtGameDocument(nfg);
    doc->SetFilename(wxT(""));
    (void) new gbtGameFrame(0, doc);
    return GBT_APP_FILE_OK;
  }
  catch (Gambit::InvalidFileException) {
    return GBT_APP_PARSE_FAILED;
  }
}

bool gbtApplication::AreDocumentsModified(void) const
{
  for (int i = 1; i <= m_documents.Length(); i++) {
    if (m_documents[i]->IsModified()) {
      return true;
    }
  }
  return false;
}


IMPLEMENT_APP(gbtApplication)

