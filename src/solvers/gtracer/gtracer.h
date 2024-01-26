//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gtracer/gtracer.h
// Top-level include file for Gametracer embedding in Gambit
// This file is based on GameTracer v0.2, which is
// Copyright (c) 2002, Ben Blum and Christian Shelton
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

#ifndef GAMBIT_GTRACER_GTRACER_H
#define GAMBIT_GTRACER_GTRACER_H

#include <functional>
#include "cmatrix.h"
#include "nfgame.h"
#include "aggame.h"
#include "gambit.h"

namespace Gambit {
namespace gametracer {

/// @brief Executes the GNM algorithm on a game
/// @param g     perturbation ray
/// @param Eq    an array of equilibria will be stored here
/// @param steps  number of steps to take within a support cell; higher
///               values of this parameter slow GNM down, but may help it
///               avoid getting off the path.
/// @param fuzz   a small floating point cutoff for a variety of things.
///               can probably be left at 1e-12
/// @param LNMFreq  a Local Newton Method subroutine will be run every
///                 LNMFreq steps to decrease accumulated errors.  This
///                 executes fairly quickly, so LNMFreq can be around 3.
/// @param LNMMax   the maximum allowed iterations within the LNM algorithm.
/// @param LambdaMin  should always be negative.  Once the trajectory
///                   gets this far out, the algorithm terminates, under the
///                   assumption there are no more equilibria on the path.
/// @param wobble     this is a boolean value indicating whether to use
///                   "wobbles" of the perturbation vector to remove
///                   accumulated errors.  This removes the theoretical guarantee
///                   of convergence, but in practice may help keep GNM on the path.
/// @param threshold  the equilibrium error threshold for doing a wobble.  If
///                   wobbles are disabled, GNM will terminate if the error
///                   reaches this threshold.
/// @param p_onStep   a callback function executed on each step of the algorithm
void GNM(gnmgame &A, cvector &g, std::list<cvector> &Eq, int steps, double fuzz, int LNMFreq,
         int LNMMax, double LambdaMin, bool wobble, double threshold,
         std::function<void(const std::string &, const cvector &)> p_onStep,
         std::string &returnMessage);

/// @brief Execute the Govindan-Wilson Iterated Polymatrix algorithm for computing a
///        Nash equilibrium
/// @param A       the game
/// @param g       the perturbation ray; a bonus to add to each strategy
/// @param zh      initial approximation for z.  Can be set to vector of all 1's
/// @param alpha   stepsize.  Must be a number between 0 and 1, to be interpreted
///                as the fraction of a complete step to take.
/// @param fuzz    the cutoff accuracy for an equilibrium after which the algorithm
///                stops refining it
/// @param ans     the vector in which the equilibrium will be stored
/// @param maxiter the maximum number of iterations to attempt
/// @param verbose whether to print intermediate information on the progress of the
///                algorithm
int IPA(const gnmgame &A, const cvector &g, cvector &zh, double alpha, double fuzz, cvector &ans,
        unsigned int maxiter = 100, bool p_verbose = false);

/// @brief Build a Gametracer representation based on a Gambit game
/// @param p_game  The game to convert to Gametracer's representation
/// @param p_scaled  Whether to rescale the payoffs to be on [0, 1]
std::shared_ptr<gnmgame> BuildGame(const Game &p_game, bool p_scaled);

/// @brief Convert a Gambit mixed strategy profile to a normalized Gametracer
///        perturbation vector
/// @exception std::domain_error  If the perturbation vector \p g is not generic,
///                               that is, if each player does not have a unique
///                               maximizing strategy
cvector ToPerturbation(const MixedStrategyProfile<double> &p_profile);

/// @brief Convert a Gametracer vector to a mixed strategy profile on a Gambit game
MixedStrategyProfile<double> ToProfile(const Game &p_game, const cvector &p_profile);

} // namespace gametracer
} // end namespace Gambit

#endif // GAMBIT_GTRACER_GTRACER_H
