//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Algorithms for extending behavior profiles to Nash equilibria
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

#ifndef BEHAVEXTEND_H
#define BEHAVEXTEND_H

#include "game/game.h"

#ifdef UNUSED
//
// Commented out while support interface is in flux.
//
template <class T> class algExtendsToNash {
public:
  bool ExtendsToNash(const gbtBehavProfile<T> &p_solution,
		     const gbtEfgSupport &p_littleSupport,
		     const gbtEfgSupport &p_bigSupport,
		     gbtStatus &p_status);
};

template <class T> class algExtendsToAgentNash {
public:
  bool ExtendsToAgentNash(const gbtBehavProfile<T> &p_solution,
			  const gbtEfgSupport &p_littleSupport,
			  const gbtEfgSupport &p_bigSupport,
			  gbtStatus &p_status);
};
#endif  // UNUSED


#endif   // BEHAVEXTEND_H
