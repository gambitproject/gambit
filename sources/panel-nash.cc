//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of panel for controlling equilibrium computation
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include "sheet.h"    // for wxSheet

#include "panel-nash.h"
#include "text-window.h"
#include "panel-mixed-list.h"
#include "panel-mixed-detail.h"
#include "thread-nash-mixed.h"

const int GBT_BUTTON_COUNT = 899;
const int GBT_BUTTON_METHOD = 898;

const int GBT_MENU_FIRST = 900;
const int GBT_MENU_COUNT_ONE = 900;
const int GBT_MENU_COUNT_ALL = 902;
const int GBT_MENU_METHOD_BEST = 903;
const int GBT_MENU_METHOD_ENUMMIXED = 904;
const int GBT_MENU_METHOD_LCP = 905;
const int GBT_MENU_METHOD_BFSLCP = 906;
const int GBT_MENU_METHOD_LP = 907;
const int GBT_MENU_METHOD_LIAP = 908;
const int GBT_MENU_METHOD_LOGIT = 909;
const int GBT_MENU_METHOD_ENUMPOLY = 910;
const int GBT_MENU_METHOD_PNS = 911;
const int GBT_MENU_METHOD_YAMAMOTO = 912;
const int GBT_MENU_LAST = 950;

const int GBT_BUTTON_START = 997;
const int GBT_BUTTON_CANCEL = 998;

BEGIN_EVENT_TABLE(gbtNashPanel, wxPanel)
  EVT_BUTTON(GBT_BUTTON_START, gbtNashPanel::OnStartButton)
  EVT_BUTTON(GBT_BUTTON_CANCEL, gbtNashPanel::OnCancelButton)
  EVT_MENU(GBT_THREAD_DONE, gbtNashPanel::OnThreadDone)
  EVT_MENU_RANGE(GBT_MENU_FIRST, GBT_MENU_LAST, gbtNashPanel::OnMenu)
  EVT_BUTTON(GBT_BUTTON_COUNT, gbtNashPanel::OnCountButton)
  EVT_BUTTON(GBT_BUTTON_METHOD, gbtNashPanel::OnMethodButton)
  EVT_MIXED_PROFILE_SELECTED(gbtNashPanel::OnProfileSelected)
END_EVENT_TABLE()

gbtNashPanel::gbtNashPanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc),
    m_countValue(GBT_MENU_COUNT_ONE), m_methodValue(GBT_MENU_METHOD_BEST),
    m_thread(0)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer *paramSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Computing Nash equilibria"),
			 wxVERTICAL);

  wxBoxSizer *countSizer = new wxBoxSizer(wxHORIZONTAL);
  countSizer->Add(new gbtTextWindow(this, wxID_STATIC,
				    "Compute"),
		  0, wxALL | wxALIGN_CENTER, 5);
  m_count = new gbtTextWindow(this, GBT_BUTTON_COUNT, "one Nash equilibrium");
  m_count->SetUnderline(true);
  countSizer->Add(m_count, 0, wxALL | wxALIGN_CENTER, 5);
  paramSizer->Add(countSizer, 0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *methodSizer = new wxBoxSizer(wxHORIZONTAL);
  methodSizer->Add(new gbtTextWindow(this, wxID_STATIC, "using"),
		   0, wxALL | wxALIGN_CENTER, 5);
  m_method = new gbtTextWindow(this, GBT_BUTTON_METHOD,
			       "Gambit's recommended method",
			       "linear complementarity programming");
  m_method->SetUnderline(true);
  methodSizer->Add(m_method, 0, wxALL | wxALIGN_CENTER, 5);
  paramSizer->Add(methodSizer, 0, wxALL | wxALIGN_CENTER, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  
  m_startButton = new wxButton(this, GBT_BUTTON_START, "Start");
  buttonSizer->Add(m_startButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_cancelButton = new wxButton(this, GBT_BUTTON_CANCEL, "Cancel");
  buttonSizer->Add(m_cancelButton, 0, wxALL | wxALIGN_CENTER, 5);
  m_cancelButton->Enable(false);

  paramSizer->Add(buttonSizer, 0, wxALIGN_CENTER, 0);

  topSizer->Add(paramSizer, 1, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *listSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "List of computed equilibria"),
			 wxVERTICAL);
  m_profileCtrl = new gbtMixedProfileCtrl(this, p_doc, m_eqa);
  listSizer->Add(m_profileCtrl, 1, wxALL | wxEXPAND, 5);

  topSizer->Add(listSizer, 1, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *detailSizer = 
    new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC,
					 "Equilibrium details"),
			 wxVERTICAL);
  m_profileDetail = new gbtMixedProfileDetail(this, p_doc, m_eqa);
  detailSizer->Add(m_profileDetail, 1, wxALL | wxEXPAND, 5);
    
  topSizer->Add(detailSizer, 1, wxALL | wxEXPAND, 5);

  SetSizer(topSizer);
  Layout();
}

void gbtNashPanel::OnStartButton(wxCommandEvent &)
{
  m_startButton->Enable(false);
  m_cancelButton->Enable(true);
  m_eqa.Flush();

  switch (m_methodValue) {
  case GBT_MENU_METHOD_BEST:
    if (m_countValue == GBT_MENU_COUNT_ONE) {
      m_thread = new gbtNashOneMixedThread(this, m_eqa);
    }
    else {
      m_thread = new gbtNashAllMixedThread(this, m_eqa);
    }
    break;

  case GBT_MENU_METHOD_ENUMMIXED:
    m_thread = new gbtNashEnumMixedThread(this, m_eqa,
					  (m_countValue == GBT_MENU_COUNT_ALL) ? 0 : 1);
    break;
  case GBT_MENU_METHOD_LCP:
    m_thread = new gbtNashLcpMixedThread(this, m_eqa,
					 (m_countValue == GBT_MENU_COUNT_ALL) ? 0 : 1);
    break;
  case GBT_MENU_METHOD_BFSLCP:
    m_thread = new gbtNashBfsLcpMixedThread(this, m_eqa,
					    (m_countValue == GBT_MENU_COUNT_ALL) ? 0 : 1);
    break;
  case GBT_MENU_METHOD_LP:
    m_thread = new gbtNashLpMixedThread(this, m_eqa);
    break;
  case GBT_MENU_METHOD_LIAP:
    m_thread = new gbtNashLiapMixedThread(this, m_eqa,
					  (m_countValue == GBT_MENU_COUNT_ALL) ? 0 : 1);
    break;
  case GBT_MENU_METHOD_ENUMPOLY:
    m_thread = new gbtNashEnumPolyMixedThread(this, m_eqa,
					      (m_countValue == GBT_MENU_COUNT_ALL) ? 0 : 1);
    break;
  case GBT_MENU_METHOD_LOGIT:
    m_thread = new gbtNashLogitMixedThread(this, m_eqa);
    break;
  case GBT_MENU_METHOD_PNS:
    m_thread = new gbtNashPNSMixedThread(this, m_eqa,
					 (m_countValue == GBT_MENU_COUNT_ALL) ? 0 : 1);
    break;
  case GBT_MENU_METHOD_YAMAMOTO:
    m_thread = new gbtNashYamamotoMixedThread(this, m_eqa);
    break;
  default:
    return;
  }

    
  m_thread->Create();
  if (m_thread->Run() != wxTHREAD_NO_ERROR) {
    printf("Whoops!\n");
  }
}

void gbtNashPanel::OnCancelButton(wxCommandEvent &)
{
  // Doesn't actually do anything yet!
}

void gbtNashPanel::OnThreadDone(wxCommandEvent &)
{
  printf("Number of equilibria found = %d\n", m_eqa.Length());
  m_thread = 0;
  m_cancelButton->Enable(false);
  m_startButton->Enable(true);
  m_profileCtrl->OnUpdate();
}

void gbtNashPanel::OnUpdate(void)
{ }

void gbtNashPanel::OnCountButton(wxCommandEvent &)
{
  wxMenu *menu = new wxMenu;
  menu->Append(GBT_MENU_COUNT_ONE, "one Nash equilibrium");
  menu->Append(GBT_MENU_COUNT_ALL, "all Nash equilibria");
  PopupMenu(menu,
	    m_count->GetPosition().x,
	    m_count->GetPosition().y + m_count->GetSize().GetHeight());
}

void gbtNashPanel::OnMethodButton(wxCommandEvent &)
{
  wxMenu *menu = new wxMenu;
  menu->Append(GBT_MENU_METHOD_BEST, "Gambit's recommended method");
  
  if (m_doc->GetGame()->NumPlayers() == 2) {
    menu->Append(GBT_MENU_METHOD_ENUMMIXED,
		 "enumeration of extreme points");
    menu->Append(GBT_MENU_METHOD_LCP, "linear complementarity programming");
    menu->Append(GBT_MENU_METHOD_BFSLCP,
		 "linear complementarity (breadth-first)");
  }
  
  if (m_countValue == GBT_MENU_COUNT_ONE && 
      m_doc->GetGame()->NumPlayers() == 2 &&
      m_doc->GetGame()->IsConstSum()) {
    menu->Append(GBT_MENU_METHOD_LP, "linear programming");
  }
  menu->Append(GBT_MENU_METHOD_LIAP, "minimization of the Lyapunov function");
  menu->Append(GBT_MENU_METHOD_ENUMPOLY,
	       "systems of polynomial equations");
  if (m_countValue == GBT_MENU_COUNT_ONE) {
    menu->Append(GBT_MENU_METHOD_LOGIT,
		 "tracing logit equilibria");
  }
  menu->Append(GBT_MENU_METHOD_PNS,
	       "enumeration of possible supports");
  if (m_countValue == GBT_MENU_COUNT_ONE) {
    menu->Append(GBT_MENU_METHOD_YAMAMOTO, 
		 "Yamamoto's path-following procedure");
  }
  
  PopupMenu(menu,
	    m_method->GetPosition().x,
	    m_method->GetPosition().y + m_method->GetSize().GetHeight());
}

void gbtNashPanel::OnMenu(wxCommandEvent &p_event)
{
  switch (p_event.GetId()) {
  case GBT_MENU_COUNT_ONE:
    m_count->SetLabel("one Nash equilibrium");
    m_countValue = GBT_MENU_COUNT_ONE;
    break;
  case GBT_MENU_COUNT_ALL:
    m_count->SetLabel("all Nash equilibria");
    m_countValue = GBT_MENU_COUNT_ALL;
    break;
  case GBT_MENU_METHOD_ENUMMIXED:
    m_method->SetLabel("enumeration of extreme points");
    m_methodValue = GBT_MENU_METHOD_ENUMMIXED;
    break;
  case GBT_MENU_METHOD_LCP:
    m_method->SetLabel("linear complementarity programming");
    m_methodValue = GBT_MENU_METHOD_LCP;
    break;
  case GBT_MENU_METHOD_BFSLCP:
    m_method->SetLabel("linear complementarity (breadth-first)");
    m_methodValue = GBT_MENU_METHOD_BFSLCP;
    break;
  case GBT_MENU_METHOD_LP:
    m_method->SetLabel("linear programming");
    m_methodValue = GBT_MENU_METHOD_LP;
    break;
  case GBT_MENU_METHOD_LIAP:
    m_method->SetLabel("minimization of the Lyapunov function");
    m_methodValue = GBT_MENU_METHOD_LIAP;
    break;
  case GBT_MENU_METHOD_LOGIT:
    m_method->SetLabel("tracing logit equilibria");
    m_methodValue = GBT_MENU_METHOD_LOGIT;
    break;
  case GBT_MENU_METHOD_ENUMPOLY:
    m_method->SetLabel("systems of polynomial equations");
    m_methodValue = GBT_MENU_METHOD_ENUMPOLY;
    break;
  case GBT_MENU_METHOD_PNS:
    m_method->SetLabel("enumeration of possible supports");
    m_methodValue = GBT_MENU_METHOD_PNS;
    break;
  case GBT_MENU_METHOD_YAMAMOTO:
    m_method->SetLabel("Yamamoto's path-following procedure");
    m_methodValue = GBT_MENU_METHOD_YAMAMOTO;
    break;
  default:
    break;
  }
}

void gbtNashPanel::OnProfileSelected(wxCommandEvent &p_event)
{
  m_profileDetail->SetIndex(p_event.GetInt());
}
