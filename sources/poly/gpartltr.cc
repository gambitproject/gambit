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

#include "base/glist.imp"
#include "gpartltr.imp"

template class TreeOfPartials<gbtDouble>;
template gbtOutput &operator<<(gbtOutput &f, const TreeOfPartials<gbtDouble> &y);
template class gbtList<TreeOfPartials<gbtDouble> >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<TreeOfPartials<gbtDouble> > &y);
template class gbtList<gbtList<TreeOfPartials<gbtDouble> > >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<gbtList<TreeOfPartials<gbtDouble> > > &y);
template class gbtList<gbtList<gbtList<TreeOfPartials<gbtDouble> > > >;
template class ListOfPartialTrees<gbtDouble>;
template gbtOutput &operator<<(gbtOutput &f, 
			     const ListOfPartialTrees<gbtDouble> &y);

template class TreeOfPartials<gbtRational>;
template gbtOutput &operator<<(gbtOutput &f, const TreeOfPartials<gbtRational> &y);
template class gbtList<TreeOfPartials<gbtRational> >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<TreeOfPartials<gbtRational> > &y);
template class gbtList<gbtList<TreeOfPartials<gbtRational> > >;
template gbtOutput &operator<<(gbtOutput &f, 
			   const gbtList<gbtList<TreeOfPartials<gbtRational> > > &y);
template class gbtList<gbtList<gbtList<TreeOfPartials<gbtRational> > > >;
template class ListOfPartialTrees<gbtRational>;
template gbtOutput &operator<<(gbtOutput &f, 
			     const ListOfPartialTrees<gbtRational> &y);


//template class TreeOfPartials<double>;
//template gbtOutput &operator<<(gbtOutput &f, const TreeOfPartials<double> &y);
//template class gbtList<TreeOfPartials<double> >;
//template gbtOutput &operator<<(gbtOutput &f, 
//			     const gbtList<TreeOfPartials<double> > &y);
//template class gbtList<gbtList<TreeOfPartials<double> > >;
//template gbtOutput &operator<<(gbtOutput &f, 
//			     const gbtList<gbtList<TreeOfPartials<double> > > &y);
//template class gbtList<gbtList<gbtList<TreeOfPartials<double> > > >;
//template class ListOfPartialTrees<double>;
//template gbtOutput &operator<<(gbtOutput &f, 
//			     const ListOfPartialTrees<double> &y);

#include "base/gtree.imp"

template class gbtTreeNode<gPoly<gbtRational> >;
template class gbtList<gbtTreeNode<gPoly<gbtRational> >*>;
template class gbtTree<gPoly<gbtRational> >;
template class gbtList<gbtTree<gPoly<gbtRational> > >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtTree<gPoly<gbtRational> >&);
#endif  // __BCC55__

template class gbtTreeNode<gPoly<gbtDouble> >;
template class gbtList<gbtTreeNode<gPoly<gbtDouble> >*>;
template class gbtTree<gPoly<gbtDouble> >;
template class gbtList<gbtTree<gPoly<gbtDouble> > >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtTree<gPoly<gbtDouble> >&);
#endif  // __BCC55__
