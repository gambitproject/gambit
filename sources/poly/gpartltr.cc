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

template class TreeOfPartials<gDouble>;
template gbtOutput &operator<<(gbtOutput &f, const TreeOfPartials<gDouble> &y);
template class gbtList<TreeOfPartials<gDouble> >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<TreeOfPartials<gDouble> > &y);
template class gbtList<gbtList<TreeOfPartials<gDouble> > >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<gbtList<TreeOfPartials<gDouble> > > &y);
template class gbtList<gbtList<gbtList<TreeOfPartials<gDouble> > > >;
template class ListOfPartialTrees<gDouble>;
template gbtOutput &operator<<(gbtOutput &f, 
			     const ListOfPartialTrees<gDouble> &y);

template class TreeOfPartials<gRational>;
template gbtOutput &operator<<(gbtOutput &f, const TreeOfPartials<gRational> &y);
template class gbtList<TreeOfPartials<gRational> >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<TreeOfPartials<gRational> > &y);
template class gbtList<gbtList<TreeOfPartials<gRational> > >;
template gbtOutput &operator<<(gbtOutput &f, 
			   const gbtList<gbtList<TreeOfPartials<gRational> > > &y);
template class gbtList<gbtList<gbtList<TreeOfPartials<gRational> > > >;
template class ListOfPartialTrees<gRational>;
template gbtOutput &operator<<(gbtOutput &f, 
			     const ListOfPartialTrees<gRational> &y);


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

template class gbtTreeNode<gPoly<gRational> >;
template class gbtList<gbtTreeNode<gPoly<gRational> >*>;
template class gbtTree<gPoly<gRational> >;
template class gbtList<gbtTree<gPoly<gRational> > >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtTree<gPoly<gRational> >&);
#endif  // __BCC55__

template class gbtTreeNode<gPoly<gDouble> >;
template class gbtList<gbtTreeNode<gPoly<gDouble> >*>;
template class gbtTree<gPoly<gDouble> >;
template class gbtList<gbtTree<gPoly<gDouble> > >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtTree<gPoly<gDouble> >&);
#endif  // __BCC55__
