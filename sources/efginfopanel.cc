//
// FILE: efginfopanel.cc -- Implementation of extensive form information panel
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

#include "guiapp.h"
#include "gameview.h"
#include "efginfopanel.h"

guiEfgInfoPanel::guiEfgInfoPanel(guiEfgView *p_parent, wxWindow *p_window,
				 Efg &p_efg)
  : wxPanel(p_window, -1), m_efg(p_efg), m_parent(p_parent)
{
  Show(TRUE);
}

guiEfgInfoPanel::~guiEfgInfoPanel()
{ }


