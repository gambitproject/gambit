//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of efg<->nfg strategy correspondence
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

#ifndef LEXICON_H
#define LEXICON_H

#include "base/base.h"

typedef gArray<int> lexCorrespondence;
class Nfg;

class Lexicon   {
  public:
    Nfg *N;
    gArray<gList<lexCorrespondence *> > strategies;

    Lexicon(const efgGame &);
    ~Lexicon();

    void MakeStrategy(gbtEfgPlayer p);
    void MakeReducedStrats(const class EFSupport &, gbtEfgPlayer, 
			   gbtEfgNode, gbtEfgNode);
    void MakeLink(efgGame *, Nfg *);
};


#endif   // LEXICON_H
