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

#include "id.h"
#include "dlqrefile.h"
#include "corplot.h"

BEGIN_EVENT_TABLE(dialogQreFile, wxFrame)
  EVT_MENU(wxID_CLOSE, dialogQreFile::Close)
  EVT_MENU(GBT_MENU_QRE_EXPORT_PXI, dialogQreFile::OnFileExportPxi)
  EVT_MENU(GBT_MENU_QRE_PLOT, dialogQreFile::OnToolsPlot)
END_EVENT_TABLE()

dialogQreFile::dialogQreFile(wxWindow *p_parent, gbtGameDocument *p_doc,
			     const gbtMixedNashSet &p_profiles)
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

  for (int pl = 1; pl <= p_profiles[1]->NumPlayers(); pl++) {
    for (int st = 1; st <= p_profiles[1]->GetPlayer(pl)->NumStrategies(); st++) {
      m_qreList->InsertColumn(++maxColumn,
			      wxString::Format(wxT("%d:%d"), pl, st));
    }
  }

  for (int i = 1; i <= p_profiles.Length(); i++) {
    //    m_qreList->InsertItem(i - 1, 
    //			  wxString::Format(wxT("%s"),
    //				   (char *) ToText(p_profiles[i]->QreLambda())));
    // FIXME: QRE needs to return its own correspondence now
    m_qreList->InsertItem(i - 1, wxT("???"));
    const gbtMixedProfile<gbtNumber> &profile = p_profiles[i];
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
			     const gbtBehavNashSet &p_profiles)
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
  for (int pl = 1; pl <= p_profiles[1]->NumPlayers(); pl++) {
    for (int iset = 1; iset <= p_profiles[1]->GetPlayer(pl)->NumInfosets(); iset++) {
      for (int act = 1; act <= p_profiles[1]->GetPlayer(pl)->GetInfoset(iset)->NumActions(); act++) {
	m_qreList->InsertColumn(++maxColumn,
				wxString::Format(wxT("%d:(%d,%d)"),
						 pl, iset, act));
      }
    }
  }

  for (int i = 1; i <= p_profiles.Length(); i++) {
    m_qreList->InsertItem(i - 1, 
			  wxString::Format(wxT("%s"), "???"));
			  //					   (char *) ToText(p_profiles[i].QreLambda())));
    const gbtBehavProfile<gbtNumber> &profile = p_profiles[i];
    for (int j = 1; j <= profile->BehavProfileLength(); j++) {
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
	file << m_mixedProfiles[1]->NumPlayers() << ' ';
	for (int pl = 1; pl <= m_mixedProfiles[1]->NumPlayers(); pl++) {
	  file << m_mixedProfiles[1]->GetPlayer(pl)->NumStrategies() << ' ';
	}
	file << "\n";
	
	file << "Settings:\n";
	// FIXME: Should be gotten from the QRE correspondence
	//	file << ((double) m_mixedProfiles[1].QreLambda()) << '\n';
	file << ((double) 0);
	//file << ((double) m_mixedProfiles[m_mixedProfiles.Length()].QreLambda()) << '\n';
	file << ((double) 0);
	file << 0.1 << '\n';
	file << 0 << '\n' << 1 << '\n' << 1 << '\n';

	file << "DataFormat:\n";
	int numcols = m_mixedProfiles[1]->MixedProfileLength() + 2;
	file << numcols << ' ';
	for (int i = 1; i <= numcols; i++) {
	  file << i << ' ';
	}
	file << '\n';

	file << "Data:\n";

	for (int i = 1; i <= m_mixedProfiles.Length(); i++) {
	  const gbtMixedProfile<gbtNumber> &profile = m_mixedProfiles[i];
	  //file << ((double) m_mixedProfiles[i].QreLambda()) << " 0.000000 ";
	  file << ((double) 0);
	  
	  for (int j = 1; j <= profile->MixedProfileLength(); j++) {
	    file << ((double) profile[j]) << ' ';
	  }

	  file << '\n';
	}
      }
      else {
	// Export behavior profiles
	file << "Dimensionality:\n";
	file << m_behavProfiles[1]->NumPlayers() << ' ';
	for (int pl = 1; pl <= m_behavProfiles[1]->NumPlayers(); pl++) {
	  gbtGamePlayer player = m_behavProfiles[1]->GetPlayer(pl);
	  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	    file << player->GetInfoset(iset)->NumActions() << ' ';
	  }
	}
	file << "\n";
	
	file << "Settings:\n";
	//	file << ((double) m_behavProfiles[pf1].QreLambda()) << '\n';
	file << 0.0 << '\n';
	//	file << ((double) m_behavProfiles[m_behavProfiles.Length()].QreLambda()) << '\n';
	file << 1.0 << '\n';
	file << 0.1 << '\n';
	file << 0 << '\n' << 1 << '\n' << 1 << '\n';

	file << "DataFormat:\n";
	int numcols = m_behavProfiles[1]->BehavProfileLength() + 2;
	file << numcols << ' ';
	for (int i = 1; i <= numcols; i++) {
	  file << i << ' ';
	}
	file << '\n';

	file << "Data:\n";

	for (int i = 1; i <= m_behavProfiles.Length(); i++) {
	  const gbtBehavProfile<gbtNumber> &profile = m_behavProfiles[i];
	  //	  file << ((double) m_behavProfiles[i].QreLambda()) << " 0.000000 ";
	  file << 0.0 << " 0.000000 ";
	  
	  for (int j = 1; j <= profile->BehavProfileLength(); j++) {
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
    gbtNfgSupport support(m_mixedProfiles[1]->NewNfgSupport());
    support->SetLabel("Displayed support");
    gbtNfgCorPlotFrame *plotFrame = 
      new gbtNfgCorPlotFrame(support, this, wxDefaultPosition,
			     wxSize(500, 300));
    plotFrame->SetCorrespondence(new gbtCorBranchMixed(m_mixedProfiles));
  }
  else {
    gbtEfgSupport support = m_behavProfiles[1]->NewEfgSupport();
    support->SetLabel("Displayed support");
    gbtEfgCorPlotFrame *plotFrame = 
      new gbtEfgCorPlotFrame(support, this, wxDefaultPosition,
			     wxSize(500, 300));
    plotFrame->SetCorrespondence(new gbtCorBranchBehav(m_behavProfiles));
  }
}
