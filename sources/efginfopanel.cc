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

#include "efg.h"

#include "guiapp.h"
#include "efgview.h"
#include "efginfopanel.h"

guiEfgInfoPanel::guiEfgInfoPanel(guiEfgView *p_parent, Efg &p_efg)
  : wxPanel(p_parent, -1), m_efg(p_efg), m_parent(p_parent)
{
  Show(TRUE);
}

guiEfgInfoPanel::~guiEfgInfoPanel()
{ }


