//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for revealing actions to players
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "game/efg.h"
#include "dlefgreveal.h"


//=========================================================================
//                  dialogInfosetReveal: Member functions
//=========================================================================

dialogInfosetReveal::dialogInfosetReveal(wxWindow *p_parent,
					 const efgGame &p_efg)
  : wxDialog(p_parent, -1, "Reveal move"), m_efg(p_efg)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *playerBox =
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Reveal to players"),
			 wxHORIZONTAL);

  wxFlexGridSizer *gridSizer = new wxFlexGridSizer(wxVERTICAL,
						   (m_efg.NumPlayers() <= 2) ?
						   2 : 3);
  m_players = new wxCheckBox *[m_efg.NumPlayers()];
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    EFPlayer *player = m_efg.Players()[pl];
    if (player->GetName() != "") {
      m_players[pl-1] = new wxCheckBox(this, -1, (char *) player->GetName());
    }
    else {
      m_players[pl-1] = new wxCheckBox(this, -1, 
				       wxString::Format("Player %d", pl));
    }
    m_players[pl-1]->SetValue(1);
    gridSizer->Add(m_players[pl-1], 1, wxALL | wxEXPAND, 0);
  }
  playerBox->Add(gridSizer, 1, wxALL, 5);
  topSizer->Add(playerBox, 1, wxALL | wxCENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

dialogInfosetReveal::~dialogInfosetReveal()
{
  delete [] m_players;
}

gArray<EFPlayer *> dialogInfosetReveal::GetPlayers(void) const
{
  gBlock<EFPlayer *> players;

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    if (m_players[pl-1]->GetValue()) {
      players.Append(m_efg.Players()[pl]);
    }
  }

  return players;
}
