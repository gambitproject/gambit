//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to show QRE correspondence and optionally write PXI file
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP

#include "game/game.h"
#include "nash/behavsol.h"
#include "nash/mixedsol.h"

#include "id.h"
#include "dlqrefile.h"
#include "corplot.h"

BEGIN_EVENT_TABLE(dialogQreFile, wxFrame)
  EVT_MENU(wxID_CLOSE, dialogQreFile::Close)
  EVT_MENU(GBT_MENU_QRE_EXPORT_PXI, dialogQreFile::OnFileExportPxi)
  EVT_MENU(GBT_MENU_QRE_PLOT, dialogQreFile::OnToolsPlot)
END_EVENT_TABLE()

dialogQreFile::dialogQreFile(wxWindow *p_parent, gbtGameDocument *p_doc,
			     const gbtList<MixedSolution> &p_profiles)
  : wxFrame(p_parent, -1, _("Quantal response equilibria"),
	    wxDefaultPosition),
    gbtGameView(p_doc), m_mixedProfiles(p_profiles)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_qreList = new wxListCtrl(this, -1, wxDefaultPosition,
			     wxSize(500, 300), wxLC_REPORT | wxLC_SINGLE_SEL);
  m_qreList->InsertColumn(0, _("Lambda"));

  int maxColumn = 0;
  const gbtNfgSupport &support = p_profiles[1].Support();
  for (int pl = 1; pl <= support->NumPlayers(); pl++) {
    for (int st = 1; st <= support->GetPlayer(pl)->NumStrategies(); st++) {
      m_qreList->InsertColumn(++maxColumn,
			      wxString::Format(wxT("%d:%d"), pl, st));
    }
  }

  for (int i = 1; i <= p_profiles.Length(); i++) {
    m_qreList->InsertItem(i - 1, 
			  wxString::Format(wxT("%s"),
					   (char *) ToText(p_profiles[i].QreLambda())));
    const gbtMixedProfile<gbtNumber> &profile = p_profiles[i].Profile();
    for (int j = 1; j <= profile->MixedProfileLength(); j++) {
      m_qreList->SetItem(i - 1, j, 
			 wxString::Format(wxT("%s"),
					  (char *) ToText(profile[j])));
    }
  }
  topSizer->Add(m_qreList, 1, wxALL, 0);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(GBT_MENU_QRE_EXPORT_PXI, _("&Export to PXI"),
		   _("Save profile list in PXI-compatible format"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));

  wxMenu *toolsMenu = new wxMenu;
  toolsMenu->Append(GBT_MENU_QRE_PLOT, _("&Plot"), 
		    _("Plot the correspondence"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(toolsMenu, _("&Tools"));
  SetMenuBar(menuBar);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  Show(true);
}

dialogQreFile::dialogQreFile(wxWindow *p_parent, gbtGameDocument *p_doc,
			     const gbtList<BehavSolution> &p_profiles)
  : wxFrame(p_parent, -1, _("Quantal response equilibria"),
	    wxDefaultPosition),
    gbtGameView(p_doc), m_behavProfiles(p_profiles)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_qreList = new wxListCtrl(this, -1, wxDefaultPosition,
			     wxSize(500, 300), wxLC_REPORT | wxLC_SINGLE_SEL);
  m_qreList->InsertColumn(0, _("Lambda"));

  int maxColumn = 0;
  const gbtEfgSupport &support = p_profiles[1].Support();
  for (int pl = 1; pl <= support.NumPlayers(); pl++) {
    for (int iset = 1; iset <= support.GetPlayer(pl)->NumInfosets(); iset++) {
      for (int act = 1; act <= support.NumActions(pl, iset); act++) {
	m_qreList->InsertColumn(++maxColumn,
				wxString::Format(wxT("%d:(%d,%d)"),
						 pl, iset, act));
      }
    }
  }

  for (int i = 1; i <= p_profiles.Length(); i++) {
    m_qreList->InsertItem(i - 1, 
			  wxString::Format(wxT("%s"),
					   (char *) ToText(p_profiles[i].QreLambda())));
    const gbtPVector<gbtNumber> &profile = p_profiles[i].Profile()->GetPVector();
    for (int j = 1; j <= profile.Length(); j++) {
      m_qreList->SetItem(i - 1, j, 
			 wxString::Format(wxT("%s"),
					  (char *) ToText(profile[j])));
    }
  }
  topSizer->Add(m_qreList, 0, wxALL, 0);

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(GBT_MENU_QRE_EXPORT_PXI, _("&Export to PXI"),
		   _("Save profile list in PXI-compatible format"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_CLOSE, _("&Close"), _("Close this window"));

  wxMenu *toolsMenu = new wxMenu;
  toolsMenu->Append(GBT_MENU_QRE_PLOT, _("&Plot"), 
		    _("Plot the correspondence"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _("&File"));
  menuBar->Append(toolsMenu, _("&Tools"));
  SetMenuBar(menuBar);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  Show(true);
}

void dialogQreFile::OnUpdate(gbtGameView *)
{

}

//
// OnFileExportPXI writes out a PXI file. 
// This functionality should be broken out into a separate library
//
void dialogQreFile::OnFileExportPxi(wxCommandEvent &)
{
  wxFileDialog dialog(this, _("Save PXI file"), wxT(""), wxT(""),
		      wxT("*.pxi"), wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      gbtFileOutput file(dialog.GetPath().mb_str());

      if (m_mixedProfiles.Length() > 0) {
	file << "Dimensionality:\n";
	file << m_mixedProfiles[1].GetGame()->NumPlayers() << ' ';
	for (int pl = 1; pl <= m_mixedProfiles[1].GetGame()->NumPlayers(); pl++) {
	  file << m_mixedProfiles[1].Support()->GetPlayer(pl)->NumStrategies() << ' ';
	}
	file << "\n";
	
	file << "Settings:\n";
	file << ((double) m_mixedProfiles[1].QreLambda()) << '\n';
	file << ((double) m_mixedProfiles[m_mixedProfiles.Length()].QreLambda()) << '\n';
	file << 0.1 << '\n';
	file << 0 << '\n' << 1 << '\n' << 1 << '\n';

	file << "DataFormat:\n";
	int numcols = m_mixedProfiles[1].Support()->MixedProfileLength() + 2;
	file << numcols << ' ';
	for (int i = 1; i <= numcols; i++) {
	  file << i << ' ';
	}
	file << '\n';

	file << "Data:\n";

	for (int i = 1; i <= m_mixedProfiles.Length(); i++) {
	  const gbtMixedProfile<gbtNumber> &profile = m_mixedProfiles[i].Profile();
	  file << ((double) m_mixedProfiles[i].QreLambda()) << " 0.000000 ";
	  
	  for (int j = 1; j <= profile->MixedProfileLength(); j++) {
	    file << ((double) profile[j]) << ' ';
	  }

	  file << '\n';
	}
      }
      else {
	// Export behavior profiles
	file << "Dimensionality:\n";
	file << m_behavProfiles[1].GetGame()->NumPlayers() << ' ';
	for (int pl = 1; pl <= m_behavProfiles[1].GetGame()->NumPlayers(); pl++) {
	  gbtGamePlayer player = m_behavProfiles[1].GetGame()->GetPlayer(pl);
	  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	    file << m_behavProfiles[1].Support().NumActions(pl, iset) << ' ';
	  }
	}
	file << "\n";
	
	file << "Settings:\n";
	file << ((double) m_behavProfiles[1].QreLambda()) << '\n';
	file << ((double) m_behavProfiles[m_behavProfiles.Length()].QreLambda()) << '\n';
	file << 0.1 << '\n';
	file << 0 << '\n' << 1 << '\n' << 1 << '\n';

	file << "DataFormat:\n";
	int numcols = m_behavProfiles[1].Profile()->Length() + 2;
	file << numcols << ' ';
	for (int i = 1; i <= numcols; i++) {
	  file << i << ' ';
	}
	file << '\n';

	file << "Data:\n";

	for (int i = 1; i <= m_behavProfiles.Length(); i++) {
	  const gbtBehavProfile<gbtNumber> &profile = *m_behavProfiles[i].Profile();
	  file << ((double) m_behavProfiles[i].QreLambda()) << " 0.000000 ";
	  
	  for (int j = 1; j <= profile.Length(); j++) {
	    file << ((double) profile[j]) << ' ';
	  }

	  file << '\n';
	}
      }
    }
    catch (...) { }
  }
}

void dialogQreFile::OnToolsPlot(wxCommandEvent &)
{
  if (m_mixedProfiles.Length() > 0) {
    gbtNfgSupport support(m_mixedProfiles[1].GetGame()->NewNfgSupport());
    support->SetLabel("Displayed support");
    gbtNfgCorPlotFrame *plotFrame = 
      new gbtNfgCorPlotFrame(support, this, wxDefaultPosition,
			     wxSize(500, 300));
    plotFrame->SetCorrespondence(new gbtCorBranchMixed(m_mixedProfiles));
  }
  else {
    gbtEfgSupport support(m_behavProfiles[1].GetGame());
    support.SetLabel("Displayed support");
    gbtEfgCorPlotFrame *plotFrame = 
      new gbtEfgCorPlotFrame(support, this, wxDefaultPosition,
			     wxSize(500, 300));
    plotFrame->SetCorrespondence(new gbtCorBranchBehav(m_behavProfiles));
  }
}
