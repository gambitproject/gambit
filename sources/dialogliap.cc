//
// FILE: dialogliap.cc -- Implementation of LiapSolve dialog
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

#include "dialogliap.h"

//=======================================================================
//                       dialogLiap: Member functions
//=======================================================================

dialogLiap::dialogLiap(wxWindow *p_parent, bool p_subgames, bool p_vianfg)
  : dialogAlgorithm(p_parent, "LiapSolve Parameters", p_vianfg)
{
  MakeCommonFields(true, p_subgames, p_vianfg);
  AutoSize();
}

dialogLiap::~dialogLiap()
{
  /*
  if (Completed() == wxOK) {
    wxWriteResource("Algorithm Params", "Liap-nTries", NumTries(), 
		    gambitApp.ResourceFile());
    wxWriteResource("Algorithm Params", "Liap-accuracy", Accuracy(),
		    gambitApp.ResourceFile());
  }
  */
}

void dialogLiap::AlgorithmFields(void)
{
  m_algorithmGroup = new wxStaticBox(this, -1, "Algorithm parameters");
  StopAfterField();

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

  int nTries = 0;
  //  wxGetResource("Algorithm Params", "Liap-nTries", &nTries, gambitApp.ResourceFile());
  m_nTries = new wxTextCtrl(this, -1, "nTries");
  m_nTries->SetValue((char *) ToText(nTries));
  m_nTries->SetConstraints(new wxLayoutConstraints);
  m_nTries->GetConstraints()->top.SameAs(m_stopAfter, wxBottom, 10);
  m_nTries->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_nTries->GetConstraints()->width.AsIs();
  m_nTries->GetConstraints()->height.AsIs();

  int accuracy = 4;
  //  wxGetResource("Algorithm Params", "Liap-accuracy", 
  //		&accuracy, gambitApp.ResourceFile());
  m_accuracy = new wxTextCtrl(this, -1, "Accuracy: 1.0 e -");
  m_accuracy->SetValue((char *) ToText(accuracy));
  m_accuracy->SetConstraints(new wxLayoutConstraints);
  m_accuracy->GetConstraints()->top.SameAs(m_nTries, wxBottom, 10);
  m_accuracy->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_accuracy->GetConstraints()->width.AsIs();
  m_accuracy->GetConstraints()->height.AsIs();

  int startOption = 0;
  //  wxGetResource("Algorithm Params", "Start-Option", &startOption,
  //		gambitApp.ResourceFile());
  wxString startOptions[] = { "Default", "Saved", "Prompt" };
  m_startOption = new wxRadioBox(this, -1, "Start", 
				 wxDefaultPosition, wxDefaultSize,
				 3, startOptions);
  if (startOption >= 0 && startOption <= 2)
    m_startOption->SetSelection(startOption);
  m_startOption->SetConstraints(new wxLayoutConstraints);
  m_startOption->GetConstraints()->top.SameAs(m_accuracy, wxBottom, 10);
  m_startOption->GetConstraints()->left.SameAs(m_findAll, wxLeft);
  m_startOption->GetConstraints()->width.AsIs();
  m_startOption->GetConstraints()->height.AsIs();

  m_algorithmGroup->SetConstraints(new wxLayoutConstraints);
  if (m_subgames) {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_subgamesGroup, wxBottom, 15);
  }
  else {
    m_algorithmGroup->GetConstraints()->top.SameAs(m_dominanceGroup, wxBottom, 15);
  }
  m_algorithmGroup->GetConstraints()->left.SameAs(m_dominanceGroup, wxLeft);
  m_algorithmGroup->GetConstraints()->right.SameAs(m_stopAfter, wxRight, -10);
  m_algorithmGroup->GetConstraints()->bottom.SameAs(m_startOption, wxBottom, -10);
}

int dialogLiap::StopAfter(void) const
{
  if (m_findAll->GetValue())
    return 0;
  else
    return atoi(m_stopAfter->GetValue()); 
}

