//
// FILE: dlabout.cc -- Implementation of "about" dialog
//
// $Id$
//

#include "dlabout.h"
#include "bitmaps/gambit.xpm"

dialogAbout::dialogAbout(wxWindow *p_parent,
			 const wxString &p_title,
			 const wxString &p_programName,
			 const wxString &p_versionString)
  : wxDialog(p_parent, -1, p_title, wxDefaultPosition, wxDefaultSize)
{
  SetAutoLayout(true);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  topSizer->Add(new wxStaticBitmap(this, -1, wxBITMAP(gambit)), 0, wxALL, 5);
  topSizer->Add(new wxStaticText(this, -1, p_programName),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, p_versionString),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, "Part of the Gambit Project"),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, "http://www.hss.caltech.edu/gambit"),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, "gambit@hss.caltech.edu"),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, -1, "Built with " wxVERSION_STRING),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, "http://www.wxwindows.org"),
		0, wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, -1, "Copyright (C) 1999-2001"),
		0, wxTOP | wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, "California Institute of Technology"),
		0, wxALIGN_CENTER, 5);
  topSizer->Add(new wxStaticText(this, -1, "Funding provided by the National Science Foundation"),
		0, wxALL | wxALIGN_CENTER, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  topSizer->Add(okButton, 0, wxALL | wxALIGN_CENTER, 10);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}
