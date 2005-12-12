//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations of polynomial list types
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

#include "gpolylst.imp"
#include "double.h"
#include "libgambit/gnumber.h"
#include "libgambit/gmatrix.imp"

//template class gPolyList<int>;
//template gOutput &operator<<(gOutput &f, const gPolyList<int> &y);

template class gPolyList<gbtRational>;

//template class gPolyList<double>;
//template gOutput &operator<<(gOutput &f, const gPolyList<double> &y);

template class gPolyList<gDouble>;

template class gbtRectArray<gPoly<gDouble>*>;

template class gbtRectArray<gPoly<gbtRational>*>;

