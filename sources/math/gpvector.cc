//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of partitioned vector types
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

#include "base/base.h"
#include "gpvector.imp"
#include "rational.h"
#include "mpfloat.h"

template class gbtPVector<int>;
template class gbtPVector<double>;
template class gbtPVector<gbtRational>;

template std::ostream &operator<<(std::ostream &, const gbtPVector<int> &);
template std::ostream &operator<<(std::ostream &, const gbtPVector<double> &);
template std::ostream &operator<<(std::ostream &, const gbtPVector<gbtRational> &);

#if GBT_WITH_MP_FLOAT
template class gbtPVector<gbtMPFloat>;
template std::ostream &operator<<(std::ostream &, const gbtPVector<gbtMPFloat> &);
#endif // GBT_WITH_MP_FLOAT
