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

gbtApplication::gbtApplication(void)
  : m_fileHistory(5)
{ }

bool gbtApplication::OnInit(void)
{
#include "bitmaps/gambit.xpm"
  wxConfig config(wxT("Gambit"));
  m_fileHistory.Load(config);
  config.Read(wxT("/General/CurrentDirectory"), &m_currentDir, wxT(""));

  wxBitmap bitmap(wxBITMAP(gambit));
  (void) new wxSplashScreen(bitmap,
			    wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
			    2000, NULL, -1, wxDefaultPosition, wxDefaultSize,
			    wxSIMPLE_BORDER | wxSTAY_ON_TOP);
  wxYield();

  // Process command line arguments, if any.
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      LoadFile(argv[i]);
    }
  }
  else {
    gbtEfgGame efg = NewEfg();
    efg.NewPlayer().SetLabel("Player 1");
    efg.NewPlayer().SetLabel("Player 2");
    efg.SetLabel("Untitled Extensive Form Game");
    (void) new gbtEfgFrame(new gbtGameDocument(efg), 0);
  }

  // Set up the help system.
  wxInitAllImageHandlers();

  return true;
}

gbtApplication::~gbtApplication()
{
  wxConfig config(wxT("Gambit"));
  m_fileHistory.Save(config);
}

void gbtApplication::OnFileNew(wxWindow *p_parent)
{
  dialogNewGame dialog(p_parent);

  if (dialog.ShowModal() == wxID_OK) {
    if (dialog.CreateEfg()) {
      gbtEfgGame efg = NewEfg();
      efg.SetLabel("Untitled Extensive Form Game");
      for (int pl = 1; pl <= dialog.NumPlayers(); pl++) {
	efg.NewPlayer().SetLabel(gbtText("Player") + ToText(pl));
      }
      (void) new gbtEfgFrame(new gbtGameDocument(efg), 0);
    }
    else {
      gbtNfgGame nfg(dialog.NumStrategies());
      nfg.SetLabel("Untitled Normal Form Game");
      for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
	nfg.GetPlayer(pl).SetLabel(gbtText("Player") + ToText(pl));
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
			     ToText(iter.GetProfile().GetStrategy(pl).GetId()));
	  }
	  iter.SetOutcome(outcome);
	} while (iter.NextContingency());
      }
      (void) new gbtNfgFrame(new gbtGameDocument(nfg), 0);
    }
  }
}

void gbtApplication::OnFileOpen(wxWindow *p_parent)
{
  wxFileDialog dialog(p_parent, _("Choose file"), CurrentDir(), wxT(""), 
		      _("Extensive form games (*.efg)|*.efg|"
			"Normal form games (*.nfg)|*.nfg|"
			"All files|*.*"));

  if (dialog.ShowModal() == wxID_OK) {
    SetCurrentDir(wxPathOnly(dialog.GetPath()));
    wxConfig config(wxT("Gambit"));
    config.Write(wxT("/General/CurrentDirectory"),
		 wxPathOnly(dialog.GetPath()));
    LoadFile(dialog.GetPath());
  }
}

void gbtApplication::OnFileMRUFile(wxCommandEvent &p_event)
{
  LoadFile(m_fileHistory.GetHistoryFile(p_event.GetId() - wxID_FILE1));
}

void gbtApplication::OnHelpContents(void)
{
}

void gbtApplication::OnHelpIndex(void)
{
}

void gbtApplication::OnHelpAbout(wxWindow *p_parent)
{
  dialogAbout dialog(p_parent, _("About Gambit..."),
		     _("Gambit Graphical User Interface"),
		     _("Version " VERSION));
  dialog.ShowModal();
}

void gbtApplication::LoadFile(const wxString &p_filename)
{    
  try {
    gbtFileInput infile(p_filename.mb_str());
    gbtNfgGame nfg = ReadNfgFile(infile);
    m_fileHistory.AddFileToHistory(p_filename);
    (void) new gbtNfgFrame(new gbtGameDocument(nfg, p_filename), 0);
    return;
  }
  catch (gbtFileInput::OpenFailed &) {
    wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				  (const char *) p_filename.mb_str()),
		 _("Error"), wxOK, 0);
    return;
  }
  catch (gbtNfgParserError &) {
    // Not a valid normal form file; try extensive form next
  }

  try {
    gbtFileInput infile(p_filename.mb_str());
    gbtEfgGame efg = ReadEfg(infile);
    m_fileHistory.AddFileToHistory(p_filename);
    (void) new gbtEfgFrame(new gbtGameDocument(efg, p_filename), 0);
  }
  catch (gbtFileInput::OpenFailed &) { 
    wxMessageBox(wxString::Format(_("Could not open '%s' for reading"),
				  (const char *) p_filename.mb_str()),
		 _("Error"), wxOK, 0);
    return;
  }
  catch (...) {
    wxMessageBox(wxString::Format(_("File '%s' not in a recognized format"),
				  (const char *) p_filename.mb_str()),
		 _("Error"), wxOK, 0);
    return;

  }
}


IMPLEMENT_APP(gbtApplication)

//
// A general-purpose dialog box to display the description of the exception
//
void guiExceptionDialog(const gbtText &p_message, wxWindow *p_parent,
            long p_style /*= wxOK | wxCENTRE*/)
{
  gbtText message = "An internal error occurred in Gambit:\n" + p_message;
  wxMessageBox(wxString::Format(wxT("%s"), (char *) message),
	       _("Gambit Error"), p_style, p_parent);
}
