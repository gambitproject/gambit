//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for computing quantal response equilibria
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "nash/nfgqre.h"
#include "dlnfgqre.h"


BEGIN_EVENT_TABLE(dialogNfgQre, wxDialog)
END_EVENT_TABLE()

dialogNfgQre::dialogNfgQre(wxWindow *p_parent, const NFSupport &p_support)
  : wxDialog(p_parent, -1, "Compute quantal response equilibria")
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

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
