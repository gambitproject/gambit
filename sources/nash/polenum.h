//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via solving polynomial equations
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

#ifndef POLENUM_H
#define POLENUM_H

#include "base/base.h"
#include "game/nfg.h"
#include "game/mixed.h"
#include "mixedsol.h"

class PolEnumParams {
public:
  int stopAfter;

  PolEnumParams(void);
};


int PolEnum(const gbtNfgSupport &, const PolEnumParams &, 
	    gList<MixedSolution> &, gStatus &,
	    long &nevals, double &time, bool &is_singular);

MixedSolution PolishEquilibrium(const gbtNfgSupport &, const MixedSolution &, 
				bool &is_singular);

#endif    // POLENUM_H




