//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/ideal.cc
// Instantiations of gBasis types
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

#include "ideal.imp"

//template class gIdeal<int>;
//template class gBasis<int>;
//template gOutput &operator<<(gOutput &f, const gBasis<int> &y);

//template class gIdeal<Gambit::Rational>;
//template class gBasis<gbtRational>;
//template gOutput &operator<<(gOutput &f, const gBasis<gbtRational> &y);

//template class gIdeal<double>;
//template class gBasis<double>;
//template gOutput &operator<<(gOutput &f, const gBasis<double> &y);

template class gIdeal<double>;


