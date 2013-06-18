//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lcp/lhtab.cc
// Tableau class for Lemke-Howson algorithm
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

#include "lhtab.imp"

template class LHTableau<double>;
template class LHTableau<Rational>;

template Matrix<double> Make_A1<double>(const StrategySupport &);
template Matrix<Rational> Make_A1<Rational>(const StrategySupport &);

template Vector<double> Make_b1<double>(const StrategySupport &);
template Vector<Rational> Make_b1<Rational>(const StrategySupport &);

template Matrix<double> Make_A2<double>(const StrategySupport &);
template Matrix<Rational> Make_A2<Rational>(const StrategySupport &);

template Vector<double> Make_b2<double>(const StrategySupport &);
template Vector<Rational> Make_b2<Rational>(const StrategySupport &);



