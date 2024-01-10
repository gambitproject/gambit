//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/pelclass.h
// Declaration of interface to Pelican
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

#include <iostream>
#include "gpolylst.h"

/* extern "C" { */
#include "pelican/pelprgen.h"
/* } */


/*************************************************************/
/*********************** The class PelView *******************/
/*************************************************************/

class PelView {
 private:
  gPolyList<double>        input;

  Gambit::List<Gambit::Vector<gComplex> > complexroots;
  Gambit::List<Gambit::Vector<double> >  realroots;
  int                       mixedvolume;
  bool                      solutionsarecorrect;

  // All private routines contribute to the constructor //
  void        InitializePelicanMemory()                                 const;
  Pring       MakePring(const int num)                                  const;
  void        PrintPring(const Pring &ring)                             const;
  void        Initialize_Idf_T_Gen_node(const Gen_node &node, 
					const char * label)             const;
  Gen_node    CreateRing(const int numvar)                              const;
  polynomial1 GamPolyToPelPoly(const gPoly<double> &p, 
			       const int n, 
			       const Pring ring)                        const;
  Gen_node    CreatePelicanVersionOfSystem(const gPolyList<double> &input,
					   const Pring ring)            const;
  int         GutsOfGetMixedVolume(      node A, 
		   	                 node norms, 
			           const Imatrix T)                     const;
  int         GetMixedVolume(const Gen_node g)                          const;

  // Routines associated with the homotopy continuation
  Gen_node    Make_scl_Gen_node()                                       const;
  Gen_node    ToDmatrixGen_node(const Gen_node g)                       const;
  polynomial1 IdentityElementPoly(const Pring ring)                     const;
  polynomial1 HomotopyVariableMonomialPoly(const Pring ring, 
					   const int comp)              const;
  Gen_node    SolutionsDerivedFromContinuation(const Pring &ring,
					    const Gen_node &Genpoly,
					    const Gen_node &Solve,
					    const Gen_node &pel_system,
					       int tweak) const;
  Gen_node    SolveCheckMaybeTryAgain(const Pring &ring,
				      const Gen_node &Genpoly,
				      const Gen_node &Qtrig,
				      const Gen_node &pel_system)            ;

  Gambit::List<Gambit::Vector<gComplex> > GambitRootsFromPelRoots(const Gen_node g)   const;
  void        DisplayComplexRootList(const Gambit::List<Gambit::Vector<gComplex> > &) const;
  Gambit::List<Gambit::Vector<double> > RealRoots(const Gambit::List<Gambit::Vector<gComplex> > &) const;
  bool        CheckSolutions(const Gen_node g)                          const;

 public:
  PelView(const gPolyList<double> &mylist);
  PelView(const PelView &);
  ~PelView();

  PelView &operator =  (const PelView &);
  bool     operator == (const PelView &) const;
  bool     operator != (const PelView &) const;

  int                       Dmnsn()           const;
  Gambit::List<Gambit::Vector<gComplex> > ComplexRoots()    const;
  Gambit::List<Gambit::Vector<double> >  RealRoots()       const;
  int                       MixedVolume()     const;
  int                       NumComplexRoots() const;
  bool                      FoundAllRoots()   const;
  int                       NumRealRoots()    const;
};

int old_main();
