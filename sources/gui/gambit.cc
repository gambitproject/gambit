//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of main wxApp class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <math.h>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/image.h"
#include "wx/splash.h"

#include "game/nfg.h"
#include "game/nfstrat.h"
#include "game/nfgciter.h"

#include "gambit.h"
#include "dlabout.h"
#include "dlnewgame.h"
#include "efgshow.h"
#include "nfgshow.h"

GambitApp::GambitApp(void)
  : m_fileHistory(5)
{ }

bool GambitApp::OnInit(void)
{
#include "bitmaps/gambit.xpm"
  wxConfig config("Gambit");
  m_fileHistory.Load(config);
  config.Read("/General/CurrentDirectory", &m_currentDir, "");

  wxBitmap bitmap(wxBITMAP(gambit));
  wxSplashScreen *splash =
    new wxSplashScreen(bitmap,
		       wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
		       2000, NULL, -1, wxDefaultPosition, wxDefaultSize,
		       wxSIMPLE_BORDER | wxSTAY_ON_TOP);
#ifdef __WXMSW__
  wxYield();
#else  // !__WXMSW__
  while (splash->IsShown()) {
    wxYield();
  }
#endif  // !__WXMSW__

  // Process command line arguments, if any.
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      LoadFile(argv[i]);
    }
  }
  else {
    gbtEfgGame efg;
    efg.NewPlayer().SetLabel("Player 1");
    efg.NewPlayer().SetLabel("Player 2");
    efg.SetTitle("Untitled Extensive Form Game");
    (void) new EfgShow(new gbtGameDocument(efg), 0);
  }

  // Set up the help system.
  wxInitAllImageHandlers();

  return true;
}

GambitApp::~GambitApp()
{
  wxConfig config("Gambit");
  m_fileHistory.Save(config);
}

void GambitApp::OnFileNew(wxWindow *p_parent)
{
  dialogNewGame dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.CreateEfg()) {
      gbtEfgGame efg;
      efg.SetTitle("Untitled Extensive Form Game");
      for (int pl = 1; pl <= dialog.NumPlayers(); pl++) {
	efg.NewPlayer().SetLabel(gText("Player") + ToText(pl));
      }
      (void) new EfgShow(new gbtGameDocument(efg), 0);
    }
    else {
      gbtNfgGame nfg(dialog.NumStrategies());
      nfg.SetTitle("Untitled Normal Form Game");
      for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
	nfg.GetPlayer(pl).SetLabel(gText("Player") + ToText(pl));
      }
      if (dialog.CreateOutcomes()) {
	gbtNfgSupport support(nfg);
	gbtNfgContIterator iter(support);
	iter.First();
	do {
	  gbtNfgOutcome outcome = nfg.NewOutcome();
	  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
	    outcome.SetPayoff(nfg.GetPlayer(pl), 0);
	    outcome.SetLabel(outcome.GetLabel() +
			     ToText(iter.GetProfile()[pl].GetId()));
	  }
	  iter.SetOutcome(outcome);
	} while (iter.NextContingency());
      }
      (void) new NfgShow(new gbtGameDocument(nfg), 0);
    }
  }
}

void GambitApp::OnFileOpen(wxWindow *p_parent)
{
  wxFileDialog dialog(p_parent, "Choose file", CurrentDir(), "", 
		      "Games (*.?fg)|*.?fg|"
		      "Extensive form games (*.efg)|*.efg|"
		      "Normal form games (*.nfg)|*.nfg|"
		      "All files|*.*");

  if (dialog.ShowModal() == wxID_OK) {
    SetCurrentDir(wxPathOnly(dialog.GetPath()));
    wxConfig config("Gambit");
    config.Write("/General/CurrentDirectory", wxPathOnly(dialog.GetPath()));
    LoadFile(dialog.GetPath());
  }
}

void GambitApp::OnFileMRUFile(wxCommandEvent &p_event)
{
  LoadFile(m_fileHistory.GetHistoryFile(p_event.GetId() - wxID_FILE1));
}

void GambitApp::OnFileImportComLab(wxWindow *p_parent)
{
  wxFileDialog dialog(p_parent, "Choose file", CurrentDir(), "",
		      "ComLabGames strategic form games (*.sfg)|*.sfg|"
		      "All files|*.*");

  if (dialog.ShowModal() == wxID_OK) {
    try {
      gFileInput infile(dialog.GetPath().c_str());
      gbtNfgGame nfg = ReadComLabSfg(infile);
      (void) new NfgShow(new gbtGameDocument(nfg), 0);
    }
    catch (gFileInput::OpenFailed &) {
      wxMessageBox(wxString::Format("Could not open '%s' for reading",
				    dialog.GetPath().c_str()),
		   "Error", wxOK, 0);
      return;
    }
  }
}

void GambitApp::OnHelpContents(void)
{
}

void GambitApp::OnHelpIndex(void)
{
}

void GambitApp::OnHelpAbout(wxWindow *p_parent)
{
  dialogAbout dialog(p_parent, "About Gambit...",
		     "Gambit Graphical User Interface",
		     "Version " VERSION);
  dialog.ShowModal();
}

void GambitApp::LoadFile(const wxString &p_filename)
{    
  try {
    gFileInput infile(p_filename);
    gbtNfgGame nfg = ReadNfgFile(infile);
    m_fileHistory.AddFileToHistory(p_filename);
    (void) new NfgShow(new gbtGameDocument(nfg, p_filename), 0);
    return;
  }
  catch (gFileInput::OpenFailed &) {
    wxMessageBox(wxString::Format("Could not open '%s' for reading",
				  p_filename.c_str()),
		 "Error", wxOK, 0);
    return;
  }
  catch (gbtNfgParserError &) {
    // Not a valid normal form file; try extensive form next
  }

  try {
    gFileInput infile(p_filename);
    gbtEfgGame efg = ReadEfgFile(infile);
    m_fileHistory.AddFileToHistory(p_filename);
    (void) new EfgShow(new gbtGameDocument(efg, p_filename), 0);
  }
  catch (gFileInput::OpenFailed &) { 
    wxMessageBox(wxString::Format("Could not open '%s' for reading",
				  p_filename.c_str()),
		 "Error", wxOK, 0);
    return;
  }
  catch (...) {
    wxMessageBox(wxString::Format("File '%s' not in a recognized format",
				  p_filename.c_str()),
		 "Error", wxOK, 0);
    return;

  }
}


IMPLEMENT_APP(GambitApp)

//
// A general-purpose dialog box to display the description of the exception
//
void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
            long p_style /*= wxOK | wxCENTRE*/)
{
  gText message = "An internal error occurred in Gambit:\n" + p_message;
  wxMessageBox((char *) message, "Gambit Error", p_style, p_parent);
}

