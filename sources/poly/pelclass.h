#include <iostream.h>
#include "poly/gpolylst.h"

/* extern "C" { */
#include "pelican/pelprgen.h" 
/* } */


/*************************************************************/
/*********************** The class PelView *******************/
/*************************************************************/

class PelView {
 private:
  gPolyList<gDouble>        input;

  gList<gVector<gComplex> > complexroots;
  gList<gVector<gDouble> >  realroots;
  int                       mixedvolume;
  bool                      solutionsarecorrect;

  // All private routines contribute to the constructor //
  void        InitializePelicanMemory()                                 const;
  Pring       MakePring(const int num)                                  const;
  void        PrintPring(const Pring &ring)                             const;
  void        Initialize_Idf_T_Gen_node(const Gen_node &node, 
					const char * label)             const;
  Gen_node    CreateRing(const int numvar)                              const;
  polynomial1 GamPolyToPelPoly(const gPoly<gDouble> &p, 
			       const int n, 
			       const Pring ring)                        const;
  Gen_node    CreatePelicanVersionOfSystem(const gPolyList<gDouble> &input,
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

  gList<gVector<gComplex> > GambitRootsFromPelRoots(const Gen_node g)   const;
  void        DisplayComplexRootList(const gList<gVector<gComplex> > &) const;
  gList<gVector<gDouble> > RealRoots(const gList<gVector<gComplex> > &) const;
  bool        CheckSolutions(const Gen_node g)                          const;

 public:
  PelView(const gPolyList<gDouble> &mylist);
  PelView(const PelView &);
  ~PelView();

  PelView &operator =  (const PelView &);
  bool     operator == (const PelView &) const;
  bool     operator != (const PelView &) const;

  int                       Dmnsn()           const;
  gList<gVector<gComplex> > ComplexRoots()    const;
  gList<gVector<gDouble> >  RealRoots()       const;
  int                       MixedVolume()     const;
  int                       NumComplexRoots() const;
  bool                      FoundAllRoots()   const;
  int                       NumRealRoots()    const;
};

int old_main();
