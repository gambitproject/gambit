//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of support list handlers
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#include "base/base.h"
#include "game/efg.h"
#include "game/efstrat.h"
#include "game/nfg.h"
#include "game/nfstrat.h"

#include "support.h"
#include "gamedoc.h"    // to define gbtGameDocument

//==========================================================================
//                      class gbtEfgSupportList
//==========================================================================

gbtEfgSupportList::gbtEfgSupportList(gbtGameDocument *p_doc)
  : m_doc(p_doc), m_current(0)
{
  BuildDefaultSupport();
}

gbtEfgSupportList::~gbtEfgSupportList()
{ }

void gbtEfgSupportList::BuildDefaultSupport(void)
{
  if (m_doc->HasEfg()) {
    m_supports.Append(gbtEfgSupport(m_doc->GetEfg()));
    m_supports[1].SetLabel("Full Support");
    m_current = 1;
  }
}

void gbtEfgSupportList::Append(const gbtEfgSupport &p_support)
{
  m_supports.Append(p_support);
  m_current = m_supports.Length();
}

void gbtEfgSupportList::Remove(void)
{
  m_supports.Remove(m_current);
  if (m_supports.Length() == 0) {
    BuildDefaultSupport();
  }
  else {
    m_current = 1;
  }
}

void gbtEfgSupportList::Flush(void)
{
  m_supports = gbtList<gbtEfgSupport>();
  BuildDefaultSupport();
}

gbtText gbtEfgSupportList::GenerateUniqueLabel(void) const
{
  int number = m_supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_supports.Length(); i++) {
      if (m_supports[i].GetLabel() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

//==========================================================================
//                      class gbtNfgSupportList
//==========================================================================

gbtNfgSupportList::gbtNfgSupportList(gbtGameDocument *p_doc)
  : m_doc(p_doc), m_current(0)
{
  BuildDefaultSupport();
}

gbtNfgSupportList::~gbtNfgSupportList()
{ }

void gbtNfgSupportList::BuildDefaultSupport(void)
{
  m_supports.Append(gbtNfgSupport(m_doc->GetNfg()));
  m_supports[1].SetLabel("Full Support");
  m_current = 1;
}

void gbtNfgSupportList::Append(const gbtNfgSupport &p_support)
{
  m_supports.Append(p_support);
  m_current = m_supports.Length();
}

void gbtNfgSupportList::Remove(void)
{
  m_supports.Remove(m_current);
  if (m_supports.Length() == 0) {
    BuildDefaultSupport();
  }
  else {
    m_current = 1;
  }
}

void gbtNfgSupportList::Flush(void)
{
  m_supports = gbtList<gbtNfgSupport>();
  BuildDefaultSupport();
}

gbtText gbtNfgSupportList::GenerateUniqueLabel(void) const
{
  int number = m_supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_supports.Length(); i++) {
      if (m_supports[i].GetLabel() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}

#include "base/glist.imp"

template class gbtList<gbtNfgSupport>;
