//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria by minimizing Liapunov function
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef NFGLIAP_H
#define NFGLIAP_H

#include "game/game.h"

/// Attempt to compute an approximate Nash equilibrium via Lyapunov minimization.

//!
//! Attempts to compute an approximate Nash equilibrium using Lyapunov function
//! minimization (McKelvey, working paper) over strategy profiles.  The function
//! is minimized using Polak-Ribiere conjugate gradient descent, with the 
//! implementation based on the implementation in the GNU Scientific Library.
//!
//! @param p_maxitsN The maximum number of steps to take before giving up.
//! @param p_tolN The tolerance criterion for accepting a candidate profile.
//! @param p_maxits1 The maximum number of steps to take in one-dimensional 
//! optimization.
//! @param p_tol1 The tolerance criterion for one-dimensional optimization.
//!
gbtList<gbtMixedProfile<double> >
gbtNashLiapNfg(const gbtMixedProfile<double> &,
	       int p_maxitsN = 100, double p_tolN = 1.0e-8,
	       int p_maxits1 = 100, double p_tol1 = .0001);

#endif // NFGLIAP_H
