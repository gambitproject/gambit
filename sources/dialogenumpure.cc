//
// FILE: dialogenumpure.cc -- Implementation of EnumPureSolve dialog
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

#include "dialogenumpure.h"

//=======================================================================
//                   dialogEnumPure: Member functions
//=======================================================================

#include "dialogenumpure.h"

dialogEnumPure::dialogEnumPure(wxWindow *p_parent, bool p_subgames,
			       bool p_vianfg)
  : dialogAlgorithm(p_parent, "EnumPureSolve Parameters", p_vianfg)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  AutoSize();
}

dialogEnumPure::~dialogEnumPure()
{ }

void dialogEnumPure::AlgorithmFields(void)
{
  m_algorithmGroup = new wxStaticBox(this, -1, "Algorithm parameters");
  StopAfterField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_stopAfter, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_stopAfter, wxBottom, -10);

  m_findAll->SetConstraints(new wxLayoutConstraints);
  m_findAll->GetConstraints()->centreY.SameAs(m_stopAfter, wxCentreY);
  m_findAll->GetConstraints()->left.SameAs(m_algorithmGroup, wxLeft, 10);
  m_findAll->GetConstraints()->width.AsIs();
  m_findAll->GetConstraints()->height.AsIs();
  
  m_stopAfter->SetConstraints(new wxLayoutConstraints);
  m_stopAfter->GetConstraints()->top.SameAs(m_algorithmGroup, wxTop, 20);
  m_stopAfter->GetConstraints()->left.SameAs(m_findAll, wxRight, 10);
  m_stopAfter->GetConstraints()->width.AsIs();
  m_stopAfter->GetConstraints()->height.AsIs();
}

int dialogEnumPure::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return atoi(m_stopAfter->GetValue().c_str()); 
}
