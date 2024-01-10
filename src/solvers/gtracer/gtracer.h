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

#include "cmatrix.h"
#include "nfgame.h"
#include "aggame.h"

namespace Gambit {
namespace gametracer {

int
GNM(gnmgame &A, cvector &g, std::list<cvector> &Eq, int steps, double fuzz, int LNMFreq, int LNMMax,
    double LambdaMin, bool wobble, double threshold, bool verbose,
    std::string &returnMessage);

int
IPA(gnmgame &A, cvector &g, cvector &zh, double alpha, double fuzz, cvector &ans, int maxiter = -1);

}  // end namespace Gambit::gametracer
}  // end namespace Gambit

#endif  // GAMBIT_GTRACER_GTRACER_H

