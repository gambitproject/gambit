//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations of tree of partials classes
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

#include "gpartltr.imp"

template class TreeOfPartials<gDouble>;
template class ListOfPartialTrees<gDouble>;

template class TreeOfPartials<Gambit::Rational>;
template class ListOfPartialTrees<Gambit::Rational>;


//template class TreeOfPartials<double>;
//template gOutput &operator<<(gOutput &f, const TreeOfPartials<double> &y);
//template class gbtList<TreeOfPartials<double> >;
//template gOutput &operator<<(gOutput &f, 
//			     const gbtList<TreeOfPartials<double> > &y);
//template class gbtList<gbtList<TreeOfPartials<double> > >;
//template gOutput &operator<<(gOutput &f, 
//			     const gbtList<gbtList<TreeOfPartials<double> > > &y);
//template class gbtList<gbtList<gbtList<TreeOfPartials<double> > > >;
//template class ListOfPartialTrees<double>;
//template gOutput &operator<<(gOutput &f, 
//			     const ListOfPartialTrees<double> &y);

#include "gtree.imp"

template class gTreeNode<gPoly<Gambit::Rational> >;
template class gTree<gPoly<Gambit::Rational> >;

template class gTreeNode<gPoly<gDouble> >;
template class gTree<gPoly<gDouble> >;
