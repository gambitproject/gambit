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

template class gbtPolyPartialTree<gbtRational>;
template std::ostream &operator<<(std::ostream &f, const gbtPolyPartialTree<gbtRational> &y);
template class gbtList<gbtPolyPartialTree<gbtRational> >;
template class gbtList<gbtList<gbtPolyPartialTree<gbtRational> > >;
template class gbtList<gbtList<gbtList<gbtPolyPartialTree<gbtRational> > > >;
template class gbtPolyPartialTreeList<gbtRational>;


template class gbtPolyPartialTree<double>;
template std::ostream &operator<<(std::ostream &f, const gbtPolyPartialTree<double> &y);
//template class gbtList<gbtPolyPartialTree<double> >;
//template std::ostream &operator<<(std::ostream &f, 
//			     const gbtList<gbtPolyPartialTree<double> > &y);
//template class gbtList<gbtList<gbtPolyPartialTree<double> > >;
//template std::ostream &operator<<(std::ostream &f, 
//			     const gbtList<gbtList<gbtPolyPartialTree<double> > > &y);
//template class gbtList<gbtList<gbtList<gbtPolyPartialTree<double> > > >;
template class gbtPolyPartialTreeList<double>;
template std::ostream &operator<<(std::ostream &f, 
			     const gbtPolyPartialTreeList<double> &y);

#include "base/gtree.imp"

template class gbtTreeNode<gbtPolyMulti<gbtRational> >;
template class gbtList<gbtTreeNode<gbtPolyMulti<gbtRational> >*>;
template class gbtTree<gbtPolyMulti<gbtRational> >;
template class gbtList<gbtTree<gbtPolyMulti<gbtRational> > >;
#ifndef __BCC55__
template std::ostream& operator << (std::ostream& output, 
			       const gbtTree<gbtPolyMulti<gbtRational> >&);
#endif  // __BCC55__

