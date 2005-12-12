//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of analysis storage classes
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

#ifndef ANALYSIS_H
#define ANALYSIS_H

class gbtGameDocument;

//
// This file contains classes which manage the output of analysis tools.
// Currently, one class is supported, which is simply a list of profiles
// output by, e.g., an algorithm to compute Nash equilibria.
// Future classes will be adapted to other types of output (e.g., 
// output of stable sets or convex components of equilibria, etc.) 
//

class TiXmlNode;

//!
//! This class manages a list of profiles, computed, e.g., by an
//! algorithm for finding Nash equilibria.  It maintains two lists
//! of profiles, one with the behavior representation and the other
//! with the mixed representation.
//!
class gbtAnalysisProfileList {
private:
  gbtGameDocument *m_doc;
  wxString m_label, m_description, m_command;
  int m_current;
  gbtList<gbtBehavProfile<gbtNumber> > m_behavProfiles;
  gbtList<gbtMixedProfile<gbtNumber> > m_mixedProfiles;

public:
  //!
  //! @name Lifecycle
  //!
  //@{
  /// Construct a new profile list for the specified document
  gbtAnalysisProfileList(gbtGameDocument *p_doc)
    : m_doc(p_doc), m_current(0) { }
  //@}

  //!
  //! @name General data access
  //!
  //@{
  /// The number of profiles in the list
  int NumProfiles(void) const;

  /// Return the p_index'th profile in the list, in behavior representation
  const gbtBehavProfile<gbtNumber> &GetBehav(int p_index) const
  { return m_behavProfiles[p_index]; }

  /// Return the p_index'th profile in the list, in mixed representation
  const gbtMixedProfile<gbtNumber> &GetMixed(int p_index) const
  { return m_mixedProfiles[p_index]; }

  /// Get the index of the currently selected profile
  int GetCurrent(void) const { return m_current; }

  /// Set the index of the currently selected profile
  void SetCurrent(int p_index) { m_current = p_index; }

  /// Get the label (short description) of the list
  const wxString &GetLabel(void) const { return m_label; }

  /// Set the label (short description) of the list
  void SetLabel(const wxString &p_label) { m_label = p_label; }

  /// Get the description of the list
  const wxString &GetDescription(void) const { return m_description; }
    
  /// Set the description of the list
  void SetDescription(const wxString &p_desc) { m_description = p_desc; }

  /// Get the command used to generate the list
  const wxString &GetCommand(void) const { return m_command; }

  /// Set the command used to generate the list
  void SetCommand(const wxString &p_command) { m_command = p_command; }
  //@}

  //!
  //! @name Adding profiles to the list
  //!
  //@{
  /// Add a behavior profile to the list
  void Append(const gbtBehavProfile<gbtNumber> &p_profile);
  /// Add a mixed profile to the list
  void Append(const gbtMixedProfile<gbtNumber> &p_profile);
  /// Map all behavior profiles to corresponding mixed profiles
  void BuildNfg(void);

  /// Remove all profiles from the list
  void Clear(void);
  //@}

  //!
  //! @name Saving and loading profile lists
  //!
  //@{
  /// Build a profile list from XML savefile 
  void Load(TiXmlNode *analysis);
  //@}
};

#endif  // ANALYSIS_H
