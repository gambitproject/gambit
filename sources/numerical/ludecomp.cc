//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of LU decomposition
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

#include "ludecomp.imp"
#include "base/glist.imp"
#include "math/rational.h"

template class EtaMatrix< double >;
template class gList< EtaMatrix< double > >;
template class LUdecomp< double >;

#ifndef __BCC55__
template gOutput& operator<<( gOutput&, const EtaMatrix< double > &); 
#endif  // __BCC55__

template class EtaMatrix< gRational >;
template class gList< EtaMatrix< gRational > >;
template class LUdecomp< gRational >;

#ifndef __BCC55__
template gOutput& operator<<( gOutput&, const EtaMatrix< gRational > &); 
#endif  // __BCC55__
