//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Representation of outcomes for table games
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

#ifndef TABLE_OUTCOME_H
#define TABLE_OUTCOME_H

class gbtTableOutcomeRep : public gbtGameOutcomeRep {
public:
  int m_id;
  gbtTableGameRep *m_nfg;
  bool m_deleted;
  std::string m_label;
  gbtBlock<gbtRational> m_payoffs;

  /// @name Constructor and destructor
  //@{
  gbtTableOutcomeRep(gbtTableGameRep *, int);
  virtual ~gbtTableOutcomeRep() { }
  //@}

  /// @name General information about the outcome
  //@{
  int GetId(void) const;
  void SetLabel(const std::string &);
  std::string GetLabel(void) const;
  //@}

  /// @name Accessing payoff information about the outcome
  //@{
  gbtArray<gbtRational> GetPayoff(void) const;
  gbtRational GetPayoff(const gbtGamePlayer &) const;
  void SetPayoff(const gbtGamePlayer &, const gbtRational &);
  //@}

  /// @name Editing the outcomes in a game
  //@{
  void DeleteOutcome(void);
  //@}
};

#endif  // TABLE_OUTCOME_H
