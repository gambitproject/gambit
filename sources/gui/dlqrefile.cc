//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog to show QRE correspondence and optionally write PXI file
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "game/nfg.h"
#include "nash/mixedsol.h"

#include "dlqrefile.h"

const int idBUTTON_PXIFILE = 2000;

BEGIN_EVENT_TABLE(dialogQreFile, wxDialog)
  EVT_BUTTON(idBUTTON_PXIFILE, dialogQreFile::OnPxiFile)
END_EVENT_TABLE()

dialogQreFile::dialogQreFile(wxWindow *p_parent,
			     const gList<MixedSolution> &p_profiles)
  : wxDialog(p_parent, -1, "Quantal response equilibria"),
    m_profiles(p_profiles)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_qreList = new wxListCtrl(this, -1, wxDefaultPosition, wxSize(500, 300),
			     wxLC_REPORT | wxLC_SINGLE_SEL);
  m_qreList->InsertColumn(0, "Lambda");

  int maxColumn = 0;
  const NFSupport &support = p_profiles[1].Support();
  for (int pl = 1; pl <= support.Game().NumPlayers(); pl++) {
    for (int st = 1; st <= support.NumStrats(pl); st++) {
      m_qreList->InsertColumn(++maxColumn,
			      wxString::Format("%d:%d", pl, st));
    }
  }

  for (int i = 1; i <= p_profiles.Length(); i++) {
    m_qreList->InsertItem(i - 1, (char *) ToText(p_profiles[i].QreLambda()));
    const gPVector<gNumber> &profile = *p_profiles[i].Profile();
    for (int j = 1; j <= profile.Length(); j++) {
      m_qreList->SetItem(i - 1, j, (char *) ToText(profile[j]));
    }
  }
  topSizer->Add(m_qreList, 1, wxALL | wxEXPAND, 5);

  topSizer->Add(new wxButton(this, idBUTTON_PXIFILE, "Export to PXI file..."),
		0, wxALL | wxCENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

//
// OnPxiFile writes out a PXI file. 
// This functionality should be broken out into a separate library
//
void dialogQreFile::OnPxiFile(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Save PXI file", "", "", "*.pxi", wxSAVE);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      gFileOutput file(dialog.GetPath().c_str());

      file << "Dimensionality:\n";
      file << m_profiles[1].Game().NumPlayers() << ' ';
      for (int pl = 1; pl <= m_profiles[1].Game().NumPlayers(); pl++) {
	file << m_profiles[1].Support().NumStrats(pl) << ' ';
      }
      file << "\n";


      file << "Settings:\n";
      file << ((double) m_profiles[1].QreLambda()) << '\n';
      file << ((double) m_profiles[m_profiles.Length()].QreLambda()) << '\n';
      file << 0.1 << '\n';
      file << 0 << '\n' << 1 << '\n' << 1 << '\n';

      file << "DataFormat:\n";
      int numcols = m_profiles[1].Support().TotalNumStrats() + 2;
      file << numcols << ' ';
      for (int i = 1; i <= numcols; i++) {
	file << i << ' ';
      }
      file << '\n';

      file << "Data:\n";

      for (int i = 1; i <= m_profiles.Length(); i++) {
	const MixedProfile<gNumber> &profile = *m_profiles[i].Profile();
	file << ((double) m_profiles[i].QreLambda()) << " 0.000000 ";

	for (int j = 1; j <= profile.Length(); j++) {
	  file << ((double) profile[j]) << ' ';
	}

	file << '\n';
      }
    }
    catch (...) { }
  }
}



