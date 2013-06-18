//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/analysis.h
// Declaration of analysis storage classes
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


class gbtAnalysisOutput {
protected:
  gbtGameDocument *m_doc;
  wxString m_label, m_description, m_command;

public:
  /// @name Lifecycle
  //@{
  /// Construct a new output group
  gbtAnalysisOutput(gbtGameDocument *p_doc) : m_doc(p_doc) { }
  /// Clean up an output group
  virtual ~gbtAnalysisOutput() { }
  //@}

  /// @name General data access
  //@{
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

  /// The number of profiles in the list
  virtual int NumProfiles(void) const = 0;

  /// Get the index of the currently selected profile
  virtual int GetCurrent(void) const = 0;

  /// Set the index of the currently selected profile
  virtual void SetCurrent(int p_index) = 0;

  /// Are these behavior or strategy profiles natively?
  virtual bool IsBehavior(void) const = 0;

  //@}

  virtual std::string GetPayoff(int pl, int p_index = -1) const = 0;

  virtual std::string GetRealizProb(const Gambit::GameNode &, 
				    int p_index = -1) const = 0;
  virtual std::string GetBeliefProb(const Gambit::GameNode &, 
				    int p_index = -1) const = 0;
  virtual std::string GetNodeValue(const Gambit::GameNode &, int pl, 
				   int p_index = -1) const = 0;
  virtual std::string GetInfosetProb(const Gambit::GameNode &, 
				     int p_index = -1) const = 0;
  virtual std::string GetInfosetValue(const Gambit::GameNode &, 
				      int p_index = -1) const = 0;
  virtual std::string GetActionValue(const Gambit::GameNode &, int act,
				     int p_index = -1) const = 0;
  virtual std::string GetActionProb(const Gambit::GameNode &, int act,
				    int p_index = -1) const = 0;
  virtual std::string GetActionProb(int p_action, int p_index = -1) const = 0;
  virtual std::string GetStrategyProb(int p_strategy,
				      int p_index = -1) const = 0;
  virtual std::string GetStrategyValue(int p_strategy,
				       int p_index = -1) const = 0;


  virtual void AddOutput(const wxString &) = 0;

  /// Map all behavior profiles to corresponding mixed profiles
  virtual void BuildNfg(void) = 0;

  /// Write a profile list to XML savefile
  virtual void Save(std::ostream &) const = 0;
};

//!
//! This class manages a list of profiles, computed, e.g., by an
//! algorithm for finding Nash equilibria.  It maintains two lists
//! of profiles, one with the behavior representation and the other
//! with the mixed representation.
//!
template <class T> class gbtAnalysisProfileList : public gbtAnalysisOutput {
private:
  bool m_isBehav;
  int m_current;
  Gambit::List<Gambit::MixedBehavProfile<T> > m_behavProfiles;
  Gambit::List<Gambit::MixedStrategyProfile<T> > m_mixedProfiles;

public:
  //!
  //! @name Lifecycle
  //!
  //@{
  /// Construct a new profile list for the specified document
  gbtAnalysisProfileList(gbtGameDocument *p_doc, bool p_isBehav)
    : gbtAnalysisOutput(p_doc), m_isBehav(p_isBehav), m_current(0) { }
  /// Cleanup the profile list
  virtual ~gbtAnalysisProfileList() { }
  //@}

  //!
  //! @name General data access
  //!
  //@{
  /// Are these behavior or strategy profiles natively?
  bool IsBehavior(void) const { return m_isBehav; }

  /// The number of profiles in the list
  int NumProfiles(void) const;

  std::string GetPayoff(int pl, int p_index = -1) const;
  /// Return the realization probability of the node for the given 
  /// profile.  (index == -1 for currently selected profile)
  std::string GetRealizProb(const Gambit::GameNode &, int p_index = -1) const;
  std::string GetBeliefProb(const Gambit::GameNode &, int p_index = -1) const;
  std::string GetNodeValue(const Gambit::GameNode &, int pl, 
			   int p_index = -1) const;
  std::string GetInfosetProb(const Gambit::GameNode &, int p_index = -1) const;
  std::string GetInfosetValue(const Gambit::GameNode &, 
			      int p_index = -1) const;
  std::string GetActionValue(const Gambit::GameNode &, int act,
			     int p_index = -1) const;
  std::string GetActionProb(const Gambit::GameNode &, int act,
			    int p_index = -1) const;
  std::string GetActionProb(int p_action, int p_index = -1) const;
  std::string GetStrategyProb(int p_strategy,
			      int p_index = -1) const;
  std::string GetStrategyValue(int p_strategy,
			       int p_index = -1) const;

  /// Get the index of the currently selected profile
  int GetCurrent(void) const { return m_current; }

  /// Set the index of the currently selected profile
  void SetCurrent(int p_index) { m_current = p_index; }

  //@}

  //!
  //! @name Adding profiles to the list
  //!
  //@{
  void AddOutput(const wxString &);
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
  /// Write a profile list to XML savefile
  void Save(std::ostream &) const;
  //@}
};

#endif  // ANALYSIS_H
