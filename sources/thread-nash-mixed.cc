//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of worker threads for computing mixed strategy equilibria
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

#include <libgambit/libgambit.h>

#include "thread-nash-mixed.h"
#include "panel-nash.h"

//==========================================================================
//                      class gbtNashMixedThread
//==========================================================================

gbtNashMixedThread::gbtNashMixedThread(gbtNashPanel *p_parent,
				       gbtList<gbtMixedProfile<double> > &p_eqa)
  : m_parent(p_parent), m_eqa(p_eqa)
{ }

void gbtNashMixedThread::OnExit(void)
{ }

void *gbtNashMixedThread::Entry(void)
{
  try {
    Compute();
  }
  catch (gbtException &) {
    // Just eat it for now... need to issue a message
  }

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, GBT_THREAD_DONE);
  wxPostEvent(m_parent, event);

  return NULL;
}

//==========================================================================
//                     class gbtNashOneMixedThread
//==========================================================================

void gbtNashOneMixedThread::Compute(void)
{
  m_eqa = gbtNashBfsLcpNfg(m_parent->GetDocument()->GetGame(),
			   (double) 0.0);
}

//==========================================================================
//                     class gbtNashAllMixedThread
//==========================================================================

void gbtNashAllMixedThread::Compute(void)
{
  m_eqa = gbtNashPNSNfg(m_parent->GetDocument()->GetGame(), 0);
}

//==========================================================================
//                     class gbtNashEnumMixedThread
//==========================================================================

void gbtNashEnumMixedThread::Compute(void)
{
  m_eqa = gbtNashEnumMixedNfg(m_parent->GetDocument()->GetGame(),
			      (double) 0.0, m_stopAfter);
}

//=========================================================================
//                     class gbtNashLcpMixedThread
//==========================================================================

void gbtNashLcpMixedThread::Compute(void)
{
  m_eqa = gbtNashLcpNfg(m_parent->GetDocument()->GetGame(),
			(double) 0.0, m_stopAfter);
}

//==========================================================================
//                   class gbtNashBfsLcpMixedThread
//==========================================================================

void gbtNashBfsLcpMixedThread::Compute(void)
{
  m_eqa = gbtNashBfsLcpNfg(m_parent->GetDocument()->GetGame(),
			   (double) 0.0, m_stopAfter);
}

//==========================================================================
//                    class gbtNashLpMixedThread
//==========================================================================

void gbtNashLpMixedThread::Compute(void)
{
  m_eqa = gbtNashLpNfg(m_parent->GetDocument()->GetGame(), (double) 0.0);
}

//==========================================================================
//                   class gbtNashLiapMixedThread
//==========================================================================

void gbtNashLiapMixedThread::Compute(void)
{
  m_eqa = gbtNashLiapNfg(m_parent->GetDocument()->GetGame()->NewMixedProfile(0.0));
}

//==========================================================================
//                  class gbtNashEnumPolyMixedThread
//==========================================================================

void gbtNashEnumPolyMixedThread::Compute(void)
{
  m_eqa = gbtNashEnumPolyNfg(m_parent->GetDocument()->GetGame());
}

//==========================================================================
//                   class gbtNashLogitMixedThread
//==========================================================================

void gbtNashLogitMixedThread::Compute(void)
{
  m_eqa = gbtNashLogitNfg(m_parent->GetDocument()->GetGame());
}

//==========================================================================
//                    class gbtNashPNSMixedThread
//==========================================================================

void gbtNashPNSMixedThread::Compute(void)
{
  m_eqa = gbtNashPNSNfg(m_parent->GetDocument()->GetGame());
}

//==========================================================================
//                 class gbtNashYamamotoMixedThread
//==========================================================================

void gbtNashYamamotoMixedThread::Compute(void)
{
  m_eqa = gbtNashYamamotoNfg(m_parent->GetDocument()->GetGame());
}

