//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/ludecomp.cc
// Instantiation of LU decomposition
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

#include "ludecomp.imp"

namespace Gambit {

namespace linalg {

template class EtaMatrix<double>;
template class LUdecomp<double>;

template class EtaMatrix<Rational>;
template class LUdecomp<Rational>;

}  // end namespace Gambit::linalg

}  // end namespace Gambit
