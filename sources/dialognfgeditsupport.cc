//
// FILE: dialognfgeditsupport.cc -- Dialog for editing supports
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "gstream.h"
#include "gtext.h"
#include "gnumber.h"

#include "nfg.h"
#include "nfstrat.h"

#include "dialogauto.h"
#include "dialognfgeditsupport.h"

const int idPLAYER = 1001;
const int idSTRATEGY = 1002;

BEGIN_EVENT_TABLE(dialogNfgEditSupport, wxDialog)
  EVT_LISTBOX(idPLAYER, OnPlayer)
  EVT_LISTBOX(idSTRATEGY, OnStrategy)
END_EVENT_TABLE()

//=========================================================================
//                   dialogNfgEditSupport: Member functions
//=========================================================================

dialogNfgEditSupport::dialogNfgEditSupport(wxWindow *p_parent,
					   const NFSupport &p_support)
  : guiAutoDialog(p_parent, "Edit support"),
    m_nfg(p_support.Game()), m_support(p_support)
{
  m_nameItem = new wxTextCtrl(this, -1);
  m_nameItem->SetValue((char *) p_support.GetName());
  m_nameItem->SetConstraints(new wxLayoutConstraints);
  m_nameItem->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_nameItem->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_nameItem->GetConstraints()->width.AsIs();
  m_nameItem->GetConstraints()->height.AsIs();

  m_playerItem = new wxListBox(this, idPLAYER);
  m_playerItem->SetConstraints(new wxLayoutConstraints);
  m_playerItem->GetConstraints()->top.SameAs(m_nameItem, wxBottom, 10);
  m_playerItem->GetConstraints()->left.SameAs(m_nameItem, wxLeft);
  m_playerItem->GetConstraints()->width.AsIs();
  m_playerItem->GetConstraints()->height.AsIs();

  m_strategyItem = new wxListBox(this, idSTRATEGY,
				 wxDefaultPosition, wxDefaultSize,
				 0, 0, wxLB_MULTIPLE);
  m_strategyItem->SetConstraints(new wxLayoutConstraints);
  m_strategyItem->GetConstraints()->top.SameAs(m_playerItem, wxTop);
  m_strategyItem->GetConstraints()->left.SameAs(m_playerItem, wxRight, 10);
  m_strategyItem->GetConstraints()->width.AsIs();
  m_strategyItem->GetConstraints()->height.AsIs();

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " + 
				   m_nfg.Players()[pl]->GetName()));
    m_playerItem->SetSelection(pl - 1, true);
  }

  m_okButton->GetConstraints()->top.SameAs(m_playerItem, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->height.AsIs();
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  m_cancelButton->GetConstraints()->height.AsIs();
  m_cancelButton->GetConstraints()->width.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->height.AsIs();
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);

  OnPlayer(wxCommandEvent());
  AutoSize();
}

void dialogNfgEditSupport::OnPlayer(wxCommandEvent &)
{
  NFPlayer *player = m_nfg.Players()[m_playerItem->GetSelection() + 1];
  m_strategyItem->Clear();
  for (int st = 1; st <= player->NumStrats(); st++) {
    m_strategyItem->Append((char *) (ToText(st) + ": " +
				     player->Strategies()[st]->Name()));
    if (m_support.Find(player->Strategies()[st])) {
      m_strategyItem->SetSelection(st - 1, TRUE);
    }
  }
}

void dialogNfgEditSupport::OnStrategy(wxCommandEvent &)
{
  int player = m_playerItem->GetSelection() + 1;
  for (int st = 0; st < m_strategyItem->Number(); st++) {
    Strategy *strategy = m_nfg.Players()[player]->Strategies()[st+1];
    if (m_strategyItem->Selected(st)) {
      m_support.AddStrategy(strategy);
    }
    else {
      if (m_support.NumStrats(player) > 1) {
	m_support.RemoveStrategy(strategy);
      }
      else if (m_support.Find(strategy)) {
	m_strategyItem->SetSelection(st, true);
      }
    }
  }
}

gText dialogNfgEditSupport::Name(void) const
{
  return m_nameItem->GetValue().c_str();
}
