//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute dominated strategies on normal forms
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

#ifndef NFDOM_H
#define NFDOM_H

#include "base/gstatus.h"
#include "math/rational.h"
#include "nfg.h"
#include "nfstrat.h"
#include "nfgciter.h"
#include "mixed.h"

bool IsMixedDominated(const gbtNfgSupport &, gbtNfgStrategy,
		      bool strong, gPrecision precision,
		      gOutput &tracefile);

bool IsMixedDominated(const MixedProfile<gNumber> &sol, int pl,
		      bool strong, gPrecision precision, gOutput &tracefile);

bool IsMixedDominated(const MixedProfile<gNumber> &sol,
		      bool strong, gPrecision precision, gOutput &tracefile);

#endif    // NFDOM_H


