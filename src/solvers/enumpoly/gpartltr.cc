//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpartltr.cc
// Instantiations of tree of partials classes
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

#include "gpartltr.imp"

template class TreeOfPartials<double>;
template class ListOfPartialTrees<double>;

//template class TreeOfPartials<Gambit::Rational>;
//template class ListOfPartialTrees<Gambit::Rational>;


//template class TreeOfPartials<double>;
//template gOutput &operator<<(gOutput &f, const TreeOfPartials<double> &y);
//template class Gambit::List<TreeOfPartials<double> >;
//template gOutput &operator<<(gOutput &f, 
//			     const Gambit::List<TreeOfPartials<double> > &y);
//template class Gambit::List<Gambit::List<TreeOfPartials<double> > >;
//template gOutput &operator<<(gOutput &f, 
//			     const Gambit::List<Gambit::List<TreeOfPartials<double> > > &y);
//template class Gambit::List<Gambit::List<Gambit::List<TreeOfPartials<double> > > >;
//template class ListOfPartialTrees<double>;
//template gOutput &operator<<(gOutput &f, 
//			     const ListOfPartialTrees<double> &y);

#include "gtree.imp"

//template class gTreeNode<gPoly<Gambit::Rational> >;
//template class gTree<gPoly<Gambit::Rational> >;

template class gTreeNode<gPoly<double> >;
template class gTree<gPoly<double> >;
