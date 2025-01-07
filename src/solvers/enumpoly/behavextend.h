//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/behavextend.h
// Algorithms for extending behavior profiles to Nash equilibria
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

#ifndef BEHAVEXTEND_H
#define BEHAVEXTEND_H

#include "gambit.h"

namespace Gambit {
namespace Nash {

// This asks whether there is a Nash extension of the MixedBehaviorProfile<double> to
// all information sets at which the behavioral probabilities are not
// specified.  The assumption is that the support has active actions
// at infosets at which the behavioral probabilities are defined, and
// no others.  Also, the BehavSol is assumed to be already a Nash
// equilibrium for the truncated game obtained by eliminating stuff
// outside little_supp.
bool ExtendsToNash(const MixedBehaviorProfile<double> &p_solution,
                   const BehaviorSupportProfile &p_littleSupport,
                   const BehaviorSupportProfile &p_bigSupport);

// This asks whether there is an ANF Nash extension of the MixedBehaviorProfile<double>
// to all information sets at which the behavioral probabilities are not specified.  The
// assumption is that the support has active actions at infosets at which the behavioral
// probabilities are defined, and no others.
bool ExtendsToAgentNash(const MixedBehaviorProfile<double> &p_solution,
                        const BehaviorSupportProfile &p_littleSupport,
                        const BehaviorSupportProfile &p_bigSupport);

} // namespace Nash
} // namespace Gambit

#endif // BEHAVEXTEND_H
