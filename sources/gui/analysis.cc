//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of analysis storage classesy
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#include "tinyxml.h"    // for XML parser for Load()


#include "libgambit/libgambit.h"
#include "analysis.h"
#include "gamedoc.h"

//=========================================================================
//                     class gbtAnalysisProfileList
//=========================================================================

void 
gbtAnalysisProfileList::Append(const gbtBehavProfile<gbtNumber> &p_profile)
{
  m_behavProfiles.Append(p_profile);
  if (m_doc->GetNfg()) {
    m_mixedProfiles.Append(gbtMixedProfile<gbtNumber>(p_profile));
  }
}

void
gbtAnalysisProfileList::Append(const gbtMixedProfile<gbtNumber> &p_profile)
{
  m_mixedProfiles.Append(p_profile);
  if (m_doc->GetEfg()) {
    m_behavProfiles.Append(gbtBehavProfile<gbtNumber>(p_profile));
  }
}

void gbtAnalysisProfileList::BuildNfg(void)
{
  for (int i = 1; i <= m_behavProfiles.Length(); i++) {
    m_mixedProfiles.Append(gbtMixedProfile<gbtNumber>(m_behavProfiles[i]));
  }
}

int gbtAnalysisProfileList::NumProfiles(void) const
{
  if (m_doc->GetEfg()) {
    return m_behavProfiles.Length();
  }
  else {
    return m_mixedProfiles.Length();
  }
}

void gbtAnalysisProfileList::Clear(void)
{
  m_behavProfiles = gbtList<gbtBehavProfile<gbtNumber> >();
  m_mixedProfiles = gbtList<gbtMixedProfile<gbtNumber> >();
  m_current = 0;
}

//-------------------------------------------------------------------------
//         gbtAnalysisProfileList: Saving and loading profile lists 
//-------------------------------------------------------------------------

#include <wx/tokenzr.h>

static gbtMixedProfile<gbtNumber> 
TextToMixedProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  gbtMixedProfile<gbtNumber> profile(p_doc->GetNfg());

  wxStringTokenizer tok(p_text, wxT(","));

  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = ToNumber(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  return profile;
}

static gbtBehavProfile<gbtNumber> 
TextToBehavProfile(gbtGameDocument *p_doc, const wxString &p_text)
{
  gbtBehavProfile<gbtNumber> profile(*p_doc->GetEfg());

  wxStringTokenizer tok(p_text, wxT(","));
  for (int i = 1; i <= profile.Length(); i++) {
    profile[i] = ToNumber(std::string((const char *) tok.GetNextToken().mb_str()));
  }

  return profile;
}

//
// Load a profile list from XML.  Pass a node pointing to an
// <analysis> entry in the workbook file
//
void gbtAnalysisProfileList::Load(TiXmlNode *p_analysis)
{
  Clear();

  TiXmlNode *description = p_analysis->FirstChild("description");
  if (description) {
    m_description = wxString(description->FirstChild()->Value(),
			     *wxConvCurrent);
  }

  if (m_doc->GetEfg()) {
    for (TiXmlNode *node = p_analysis->FirstChild("profile"); node;
	 node = node->NextSiblingElement()) {
      Append(TextToBehavProfile(m_doc,
				wxString(node->FirstChild()->Value(),
					 *wxConvCurrent)));
    }
  }
  else {
    for (TiXmlNode *node = p_analysis->FirstChild("profile"); node;
	 node = node->NextSiblingElement()) {
      Append(TextToMixedProfile(m_doc,
				wxString(node->FirstChild()->Value(),
					 *wxConvCurrent)));
    }
  }
}
