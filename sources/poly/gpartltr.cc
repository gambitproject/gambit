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

template class gbtPolyPartialTree<gbtDouble>;
template gbtOutput &operator<<(gbtOutput &f, const gbtPolyPartialTree<gbtDouble> &y);
template class gbtList<gbtPolyPartialTree<gbtDouble> >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<gbtPolyPartialTree<gbtDouble> > &y);
template class gbtList<gbtList<gbtPolyPartialTree<gbtDouble> > >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<gbtList<gbtPolyPartialTree<gbtDouble> > > &y);
template class gbtList<gbtList<gbtList<gbtPolyPartialTree<gbtDouble> > > >;
template class gbtPolyPartialTreeList<gbtDouble>;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtPolyPartialTreeList<gbtDouble> &y);

template class gbtPolyPartialTree<gbtRational>;
template gbtOutput &operator<<(gbtOutput &f, const gbtPolyPartialTree<gbtRational> &y);
template class gbtList<gbtPolyPartialTree<gbtRational> >;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtList<gbtPolyPartialTree<gbtRational> > &y);
template class gbtList<gbtList<gbtPolyPartialTree<gbtRational> > >;
template gbtOutput &operator<<(gbtOutput &f, 
			   const gbtList<gbtList<gbtPolyPartialTree<gbtRational> > > &y);
template class gbtList<gbtList<gbtList<gbtPolyPartialTree<gbtRational> > > >;
template class gbtPolyPartialTreeList<gbtRational>;
template gbtOutput &operator<<(gbtOutput &f, 
			     const gbtPolyPartialTreeList<gbtRational> &y);


//template class gbtPolyPartialTree<double>;
//template gbtOutput &operator<<(gbtOutput &f, const gbtPolyPartialTree<double> &y);
//template class gbtList<gbtPolyPartialTree<double> >;
//template gbtOutput &operator<<(gbtOutput &f, 
//			     const gbtList<gbtPolyPartialTree<double> > &y);
//template class gbtList<gbtList<gbtPolyPartialTree<double> > >;
//template gbtOutput &operator<<(gbtOutput &f, 
//			     const gbtList<gbtList<gbtPolyPartialTree<double> > > &y);
//template class gbtList<gbtList<gbtList<gbtPolyPartialTree<double> > > >;
//template class gbtPolyPartialTreeList<double>;
//template gbtOutput &operator<<(gbtOutput &f, 
//			     const gbtPolyPartialTreeList<double> &y);

#include "base/gtree.imp"

template class gbtTreeNode<gbtPolyMulti<gbtRational> >;
template class gbtList<gbtTreeNode<gbtPolyMulti<gbtRational> >*>;
template class gbtTree<gbtPolyMulti<gbtRational> >;
template class gbtList<gbtTree<gbtPolyMulti<gbtRational> > >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtTree<gbtPolyMulti<gbtRational> >&);
#endif  // __BCC55__

template class gbtTreeNode<gbtPolyMulti<gbtDouble> >;
template class gbtList<gbtTreeNode<gbtPolyMulti<gbtDouble> >*>;
template class gbtTree<gbtPolyMulti<gbtDouble> >;
template class gbtList<gbtTree<gbtPolyMulti<gbtDouble> > >;
#ifndef __BCC55__
template gbtOutput& operator << (gbtOutput& output, 
			       const gbtTree<gbtPolyMulti<gbtDouble> >&);
#endif  // __BCC55__
