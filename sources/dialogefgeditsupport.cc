//
// FILE: dialogfgeditsupport.cc -- Dialog for editing supports
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

#include "efg.h"
#include "efstrat.h"

#include "dialogauto.h"
#include "dialogefgeditsupport.h"

const int idPLAYER = 1001;
const int idINFOSET = 1002;
const int idACTION = 1003;

BEGIN_EVENT_TABLE(dialogEfgEditSupport, wxDialog)
  EVT_LISTBOX(idPLAYER, OnPlayer)
  EVT_LISTBOX(idINFOSET, OnInfoset)
  EVT_LISTBOX(idACTION, OnAction)
END_EVENT_TABLE()

//=========================================================================
//                   dialogEfgEditSupport: Member functions
//=========================================================================

dialogEfgEditSupport::dialogEfgEditSupport(wxWindow *p_parent,
					   const EFSupport &p_support)
  : guiAutoDialog(p_parent, "Edit support"),
    m_efg(p_support.Game()), m_support(p_support)
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

  m_infosetItem = new wxListBox(this, idINFOSET);
  m_infosetItem->SetConstraints(new wxLayoutConstraints);
  m_infosetItem->GetConstraints()->top.SameAs(m_playerItem, wxTop);
  m_infosetItem->GetConstraints()->left.SameAs(m_playerItem, wxRight, 10);
  m_infosetItem->GetConstraints()->width.AsIs();
  m_infosetItem->GetConstraints()->height.AsIs();

  m_actionItem = new wxListBox(this, idACTION,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0, wxLB_MULTIPLE);
  m_actionItem->SetConstraints(new wxLayoutConstraints);
  m_actionItem->GetConstraints()->top.SameAs(m_infosetItem, wxTop);
  m_actionItem->GetConstraints()->left.SameAs(m_infosetItem, wxRight, 10);
  m_actionItem->GetConstraints()->width.AsIs();
  m_actionItem->GetConstraints()->height.AsIs();

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append((char *) (ToText(pl) + ": " + 
				   m_efg.Players()[pl]->GetName()));
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

  m_playerItem->SetSelection(0);
  OnPlayer(wxCommandEvent());
  AutoSize();
}

void dialogEfgEditSupport::OnPlayer(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  m_infosetItem->Clear();
  m_actionItem->Clear();
  for (int iset = 1; iset <= player->NumInfosets(); iset++) {
    m_infosetItem->Append((char *) (ToText(iset) + ": " +
				    player->Infosets()[iset]->GetName()));
  }

  if (player->NumInfosets() > 0) {
    m_infosetItem->SetSelection(0);
    Infoset *infoset = player->Infosets()[1];
    for (int act = 1; act <= infoset->NumActions(); act++) {
      m_actionItem->Append((char *) (ToText(act) + ": " +
				     infoset->Actions()[act]->GetName()));
      if (m_support.Find(infoset->Actions()[act])) {
	m_actionItem->SetSelection(act - 1, TRUE);
      }
    }
  }
}

void dialogEfgEditSupport::OnInfoset(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];
  m_actionItem->Clear();
  for (int act = 1; act <= infoset->NumActions(); act++) {
    m_actionItem->Append((char *) (ToText(act) + ": " +
				   infoset->Actions()[act]->GetName()));
    if (m_support.Find(infoset->Actions()[act])) {
      m_actionItem->SetSelection(act - 1, TRUE);
    }
  }
}

void dialogEfgEditSupport::OnAction(wxCommandEvent &)
{
  EFPlayer *player = m_efg.Players()[m_playerItem->GetSelection() + 1];
  Infoset *infoset = player->Infosets()[m_infosetItem->GetSelection() + 1];
  for (int act = 0; act < m_actionItem->Number(); act++) {
    Action *action = infoset->Actions()[act+1];
    if (m_actionItem->Selected(act)) {
      m_support.AddAction(action);
    }
    else {
      if (m_support.NumActions(infoset) > 1) {
	m_support.RemoveAction(action);
      }
      else if (m_support.Find(action)) {
	m_actionItem->SetSelection(act, true);
      }
    }
  }
}

gText dialogEfgEditSupport::Name(void) const
{
  return m_nameItem->GetValue().c_str();
}
