//
// FILE: dialogefgoutcome.h -- Outcome properties dialog
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

#include "efg.h"
#include "dialogefgoutcome.h"

//=========================================================================
//                    dialogEfgOutcome: Member functions
//=========================================================================

dialogEfgOutcome::dialogEfgOutcome(wxWindow *p_parent,
				   const FullEfg &p_efg, EFOutcome *p_outcome)
  : guiPagedDialog(p_parent, "Change Payoffs", p_efg.NumPlayers()),
    m_outcome(p_outcome), m_efg(p_efg)
{
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++)
    SetValue(pl, ToText(m_efg.Payoff(p_outcome,
				     m_efg.Players()[pl])));

  m_outcomeName = new wxTextCtrl(this, -1);
  if (p_outcome)
    m_outcomeName->SetValue((char *) p_outcome->GetName());
  else
    m_outcomeName->SetValue((char *) 
			    ("Outcome" + ToText(p_efg.NumOutcomes() + 1)));

  m_outcomeName->SetConstraints(new wxLayoutConstraints);
  m_outcomeName->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_outcomeName->GetConstraints()->left.SameAs(m_dataFields[0], wxLeft);
  m_outcomeName->GetConstraints()->right.SameAs(m_dataFields[0], wxRight);
  m_outcomeName->GetConstraints()->height.AsIs();

  m_dataFields[0]->GetConstraints()->top.SameAs(m_outcomeName, wxBottom, 10);

  AutoSize();
}

gArray<gNumber> dialogEfgOutcome::Payoffs(void) const
{
  gArray<gNumber> ret(m_efg.NumPlayers());
  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = ToNumber(GetValue(pl));
  }
  return ret;
}

gText dialogEfgOutcome::Name(void) const
{
  return m_outcomeName->GetValue().c_str();
}

