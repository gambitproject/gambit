//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// gbtTableau class for Lemke-Howson algorithm
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

#include "lhtab.imp"

template class gbtLemkeHowsonTableau<double>;
template class gbtLemkeHowsonTableau<gbtRational>;
template gbtMatrix<double> Make_A1(const gbtGame &,
				 const double &);
template gbtMatrix<gbtRational> Make_A1(const gbtGame &,
				  const gbtRational &);
template gbtVector<double> Make_b1(const gbtGame &,
				 const double &);
template gbtVector<gbtRational> Make_b1(const gbtGame &,
				    const gbtRational &);
template gbtMatrix<double> Make_A2(const gbtGame &,
				 const double &);
template gbtMatrix<gbtRational> Make_A2(const gbtGame &,
				    const gbtRational &);
template gbtVector<double> Make_b2(const gbtGame &,
				 const double &);
template gbtVector<gbtRational> Make_b2(const gbtGame &,
				  const gbtRational &);
