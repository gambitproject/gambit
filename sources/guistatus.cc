//
// FILE: guistatus.cc -- Progress/Cancel dialog
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

#include "guistatus.h"

BEGIN_EVENT_TABLE(guiStatus, wxDialog)
  EVT_BUTTON(wxID_CANCEL, OnCancel)
END_EVENT_TABLE()

guiStatus::guiStatus(wxFrame *p_parent, const char *p_title)
  : wxDialog(p_parent, -1, p_title, wxDefaultPosition, wxSize(300, 250),
	     wxDEFAULT_DIALOG_STYLE | wxDIALOG_MODELESS), m_sig(false)
{
  m_gauge = new wxGauge(this, -1, 100, wxPoint(20, 10), wxSize(240, 50));

  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel",
					wxPoint(100, 170), wxSize(100, 30));
  Show(TRUE);
}

guiStatus::~guiStatus()
{
  Enable(FALSE);
  Show(FALSE);
}

void guiStatus::OnCancel(wxCommandEvent &)
{
  SetSignal();
}

void guiStatus::Get(void) const  
{
  wxYield();
  if (m_sig)
    throw gSignalBreak();
}

void guiStatus::SetProgress(double p_value)
{
  m_gauge->SetValue((int) (p_value * 100.0));
  wxYield();
}

