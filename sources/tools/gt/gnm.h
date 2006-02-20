/* Copyright 2002 Ben Blum, Christian Shelton
 *
 * This file is part of GameTracer.
 *
 * GameTracer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GameTracer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GameTracer; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GNM_H
#define __GNM_H

#include "cmatrix.h"
#include "gnmgame.h"

int GNM(gnmgame &A, cvector &g, cvector **&Eq, int steps, double fuzz, int LNMFreq, int LNMMax, double LambdaMin, bool wobble, double threshold);

#endif
