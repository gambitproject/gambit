//
// FILE: dialoglp.cc -- Implementation of LpSolve dialog
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

#include "dialoglp.h"

//=======================================================================
//                        dialogLp: Member functions
//=======================================================================

dialogLp::dialogLp(wxWindow *p_parent, bool p_subgames, bool p_vianfg)
  : dialogAlgorithm(p_parent, "LpSolve Parameters", p_vianfg)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  AutoSize();
}

dialogLp::~dialogLp()
{ }

void dialogLp::AlgorithmFields(void)
{
  m_algorithmGroup = new wxStaticBox(this, -1, "Algorithm parameters");
  PrecisionField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_precision, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_precision, wxBottom, -10);

  m_precision->SetConstraints(new wxLayoutConstraints);
  m_precision->GetConstraints()->top.SameAs(m_algorithmGroup, wxTop, 20);
  m_precision->GetConstraints()->left.SameAs(m_algorithmGroup, wxLeft, 10);
  m_precision->GetConstraints()->width.AsIs();
  m_precision->GetConstraints()->height.AsIs();
}

