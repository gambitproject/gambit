//
// FILE: dialogenummixed.cc -- Implementation of EnumMixedSolve dialog
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

#include "dialogenummixed.h"

//=======================================================================
//                   dialogEnumMixed: Member functions
//=======================================================================

dialogEnumMixed::dialogEnumMixed(wxWindow *p_parent, bool p_subgames)
  : dialogAlgorithm(p_parent, "EnumMixedSolve Parameters", true)
{
  MakeCommonFields(true, p_subgames, true);
  AutoSize();
}

dialogEnumMixed::~dialogEnumMixed()
{ }

void dialogEnumMixed::AlgorithmFields(void)
{
  m_algorithmGroup = new wxStaticBox(this, -1, "Algorithm parameters");
  StopAfterField();
  PrecisionField();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_stopAfter, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_precision, wxBottom, -10);

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

  m_precision->SetConstraints(new wxLayoutConstraints);
  m_precision->GetConstraints()->top.SameAs(m_stopAfter, wxBottom, 10);
  m_precision->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_precision->GetConstraints()->width.AsIs();
  m_precision->GetConstraints()->height.AsIs();
}

int dialogEnumMixed::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return atoi(m_stopAfter->GetValue().c_str());
}

