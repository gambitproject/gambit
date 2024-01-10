//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/pelclass.cc
// Implementation of interface to Pelican
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

#include <cstdlib>
#include "pelclass.h"

/*
#include "glist.h"
#include "gvector.h"
#include "complex.h"
*/

/*************************************************************/
/************** Implementation of class Pelview **************/
/*************************************************************/

node SaveList=nullptr; 

void PelView::InitializePelicanMemory() const
{
  // mimicking main in Shell.c

  LOCS(1);
  init_symbol_table();
  node_init_store();
  PUSH_LOC(SaveList);
  SaveList=node_new();
}

Pring PelView::MakePring(const int num) const
{
  const char* q[] = {"","n1","n2" , "n3", "n4", "n5", "n6", "n7", "n8", 
		     "n9", "n10", "n11", "n12", "n13", "n14", "n15", "n16", 
		     "n17", "n18","n19", "n20", "n21", "n22", "n23", "n24", 
		     "n25", "n26", "n27", "n28","n29", "n30"};
  
  Pring R = makePR(num);    // makePR is a memory malloc routine

  for(int j=1; j<=R->n; j++) 
    R->vars[j-1] = const_cast<char *>(q[j]);
  R->def = const_cast<char *>("t");
  R->n = num;
  return R;
}

void PelView::PrintPring(const Pring &ring) const
{
  //  gout << "The Pring has " << ring->n << " variables: ";
  for (int i = 1; i <= ring->n; i++) {
    //    gout << ring->vars[i-1];
    //if (i < ring->n) 
    //  gout << ", ";
    //else
    //  gout << ".\n";
  }
  // gout << "  The homotopy variable is " << ring->def << ".\n";
}

void PelView::Initialize_Idf_T_Gen_node(const Gen_node &node, 
					const char * label) const
{
  node->type=Idf_T;
  node->Genval.gval=Copy_String_NQ((char *)label);
  node->Genval.idval=Copy_String_NQ((char *)label);
}

Gen_node PelView::CreateRing(const int numvar) const
{
  const char* q[] = {" ", "","n1","n2" , "n3", "n4", "n5", "n6", "n7", "n8", 
		     "n9", "n10", "n11", "n12", "n13", "n14", "n15", "n16", 
		     "n17", "n18","n19", "n20", "n21", "n22", "n23", "n24", 
		     "n25", "n26", "n27", "n28","n29", "n30"};

  Gen_node a1 = gen_node();
  Initialize_Idf_T_Gen_node(a1,"n1");

  Gen_node atemp;
  atemp = gen_node();
  if (numvar == 1) a1->next= atemp;
  else {
    a1->next=nullptr;
    atemp = a1;
    for(int j=2; j<=numvar; j++)
      {
	Gen_node a = gen_node();
	Initialize_Idf_T_Gen_node(a,q[j]);
	a->next = nullptr;
	
	atemp->next = a;
	atemp = a;	
      }
  }
  atemp->next=nullptr;
  Initialize_Idf_T_Gen_node(atemp,"t");
  
  return a1;
}

polynomial1 PelView::GamPolyToPelPoly(const gPoly<double> &p, 
				      const int n, 
				      const Pring ring) const
{
  if ((p.MonomialList().Length() == 0))  {
      
    polynomial1 P;
    P = makeP(ring);
    P->coef.r=0;
    P->coef.i=0;
    
    for (int i=0;i<P->R->n;i++)
      P->exps[i]=0;
    P->next= nullptr;
    return P;
  }
  
  else {
    polynomial1  P,Ptemp;
    P = makeP(ring);
    
    Ptemp= P;
    
    if (p.MonomialList()[1].IsConstant()) {
      Ptemp->coef.r= p.MonomialList()[1].Coef();
      Ptemp->coef.i= 0;
      
      for (int i=0;i<Ptemp->R->n;i++)
	Ptemp->exps[i]=0;
      Ptemp->next=nullptr;
    }
    
    else {
      Ptemp->coef.r= p.MonomialList()[1].Coef();
      Ptemp->coef.i= 0;
      
      for (int i=0;i<Ptemp->R->n;i++)
	Ptemp->exps[i]= p.MonomialList()[1].ExpV() [i+1];
      Ptemp->next=nullptr;
    }
    
    for (int j = 2; j <=p.MonomialList().Length(); j++) {
      polynomial1 a;
      a = makeP(ring);
      if (p.MonomialList()[j].IsConstant()) {
	a->coef.r= p.MonomialList()[j].Coef();
	a->coef.i= 0;
	
	for (int i=0;i<a->R->n;i++)
	  a->exps[i]=0;
	a->next = nullptr;
	Ptemp->next = a;
	Ptemp = a;
	
      }
      
      else {
	a->coef.r= p.MonomialList()[j].Coef();
	a->coef.i= 0;
	
	
	for (int i=0;i<a->R->n;i++)
	  a->exps[i]= p.MonomialList()[j].ExpV() [i+1];
		
	//     printP(a);
	//	      cout;
	a->next = nullptr;
	Ptemp->next = a;
	Ptemp = a;
	
      }  
    }
    
    return P;
  }
}  

Gen_node 
PelView::CreatePelicanVersionOfSystem(const gPolyList<double> &input, 
				      const Pring ring) const
{
  Gen_node a;
  a = gen_node();
  a->type= Mtx_T;
  a->next= nullptr;

  Gmatrix V;
  V = Gmatrix_new(1, input.Length());
  V->store = input.Length();
  V->topc = input.Length();
  V->ncols = input.Length();
  V->topr=1;
  
  for(int j=1; j<=input.Length(); j++)
    {
      Gen_node b;
      b = gen_node();
      b->type = Ply_T;
      b->next = nullptr;
      b->Genval.pval=(GamPolyToPelPoly(input[j], input.Length(), ring));
      b->Genval.gval=(char*) (GamPolyToPelPoly(input[j], input.Length(), ring));
      b->Genval.idval=(char*) (GamPolyToPelPoly(input[j], input.Length(), ring)); 
      
      (V->coords[j-1])= b;
    }
  
  a->Genval.gval=(char *)V;
  return G_System(a);
}

int PelView::GutsOfGetMixedVolume(      node A, 
				        node norms, 
				  const Imatrix T) const
{
  node ptr = nullptr, ptc = nullptr, res = nullptr;
  Imatrix M = nullptr, Tp = nullptr;
  int v, mv = 0, t = 0;
  LOCS(5);
  PUSH_LOC(A);
  PUSH_LOC(res);
  PUSH_LOC(ptc);
  PUSH_LOC(ptr);
  PUSH_LOC(norms);
  
  ptr = norms;
  while (ptr != nullptr) {
    
    ptc = aset_face(A, (Imatrix) Car((node) Car(ptr)));
    
    Tp = aset_type(ptc, Tp);
    
    if (T != nullptr && Imatrix_equal(Tp, T) == TRUE) {
      t = 1;
      list_insert(Car(ptr),&res, &(list_Imatrix_comp),FALSE);
    } else
      t = 0;
    M = aset_M(ptc, M);
    if (
	(Imatrix_rref(M, &v) == aset_dim(A) - 1) &&
	(IMrows(M) == aset_dim(A) - 1)
	) {
      
      if (t == 1)
	mv += abs(v);
    }
    
    ptr = Cdr(ptr);
  }
  
  Imatrix_free(Tp);
  Imatrix_free(M);
  POP_LOCS();
  return mv;
}

int PelView::GetMixedVolume(const Gen_node g) const
{
  aset A=nullptr;
  Ivector T=nullptr;
  int r = 0;
  int CP;
  int nargs;
  LOCS(2);
  PUSH_LOC(A);
  
  nargs=Gen_length(g);
  //Something happens right here and is necessary for the code, 
  //but don't ask me what...       
  if ((nargs ==0) ||
      (nargs==1  && Can_Be_Aset(Gen_elt(g,1))!=TRUE )||
      (nargs==2  && (r=Can_Be_Vector(Gen_elt(g,2),Int_T))<0)){
  }

  A=Gen_aset(Gen_elt(g,1));
 
  if (nargs==2 )
    if (r==aset_r(A)) 
      T=Gen_to_Imatrix(Gen_elt(g,2));
 
  CP= GutsOfGetMixedVolume(A,aset_lower_facets(A),T);    

  return CP;
}

Gen_node PelView::Make_scl_Gen_node() const
{
  Gen_node g= gen_node();

  g->type= Idf_T;
  g->next= nullptr;
  g->Genval.gval= const_cast<char *>("scl");
  g->Genval.idval= const_cast<char *>("scl");

  return g;
}

Gen_node PelView::ToDmatrixGen_node(const Gen_node g) const
{
  Gen_node a;
  psys PS;
  Dmatrix S;
  
  PS=Gen_to_psys(g);
  S=psys_scale(PS);
  a= Dmatrix_to_Gen(S);
  
  return a;
}

polynomial1 PelView::IdentityElementPoly(const Pring ring) const
{
  polynomial1 P;

  P = makeP(ring);
  P->coef.r=1;
  P->coef.i=0;
  for (int i=0;i<P->R->n;i++)
    P->exps[i]=0;
  P->def =0;
  P->next= nullptr;

  return P;
}

polynomial1 PelView::HomotopyVariableMonomialPoly(const Pring ring, 
						  const int comp) const
{
  polynomial1 P;

  P = makeP(ring);
  P->coef.r=1;
  P->coef.i=0;
  P->remaining=0;
  P->homog=0;
  for (int i=0;i<P->R->n;i++)
    P->exps[i]=0;
  P->def = comp;
  P->next= nullptr;
  
  return P;
}

Gen_node PelView::SolutionsDerivedFromContinuation(const Pring &ring,
					   const Gen_node &Genpoly,
					   const Gen_node &Solve,
					   const Gen_node &pel_system,
						   int tweak) const
{
  pel_system->next = nullptr;

  Gen_node temp= Link(pel_system ,Make_scl_Gen_node()); 
  Gen_node scl= ToDmatrixGen_node(temp);
  Gen_node fs = G_Scale(temp);

  //  gout<< " Step 12 \n";
// polynomials used for the homothopy
  int unity = 1;
  Gen_node tee, one, tee1;
  tee = Ply_To_Gen(HomotopyVariableMonomialPoly(ring, unity));
  one=Ply_To_Gen(IdentityElementPoly(ring));
  tee1=Ply_To_Gen(HomotopyVariableMonomialPoly(ring, unity));
  
  tee->next=nullptr;
  one->next=nullptr;
  
// Define the homothopy

  //  gout<< " Step 13 \n";
  Gen_node h = PROC_MUL(Link(tee, fs));
  Gen_node h1 = PROC_SUB(Link(one, tee1));
//A little trick
  Genpoly->next=nullptr;
  Gen_node h2 = G_UnLift(Genpoly);

  //  gout<< " Step 14 \n";
  Gen_node h4 = PROC_MUL(Link(h1, h2));
  Gen_node h5 = PROC_ADD(Link(h,h4));

  Gen_node cs = G_Cont(Link(h5,G_UnLift(Solve)), tweak);

  //  gout<< " Step 15 \n";
  Gen_node sols = G_Affine(G_UnScale(Link(cs, scl)));  

  free_Gen_list(scl);

  return sols;
}

template <class T> bool HasARedundancy(const Gambit::List<T> &p_list)
{
  int i = 1; int j = 2;		
  while (i < p_list.Length()) {
    if (p_list[i] == p_list[j])
      return true;
    else 
      j++;
    if (j > p_list.Length()) { i++; j = i+1; }
  }
  return false;
}


//#define PELVIEW_DEBUG
Gen_node PelView::SolveCheckMaybeTryAgain(const Pring &ring,
					  const Gen_node &Genpoly,
					  const Gen_node &Qtrig,
					  const Gen_node &pel_system)
{
  Gen_node sols, Solve;
  bool done(false);
  for (int tweak = 0; tweak <= 3 && !done; tweak ++)
    {
      Solve = G_Solve(Link(Genpoly, Qtrig), tweak);
#ifdef PELVIEW_DEBUG
      print_Gen_list(Solve);
      //      gout<< " \n Step 11 \n";
#endif

      sols = SolutionsDerivedFromContinuation(ring,
					      Genpoly,
					      Solve,
					      pel_system,
					      tweak);
#ifdef PELVIEW_DEBUG
      //      gout<<" \n The solution list produced by Pelican is:\n";
      print_Gen_list(sols);
#endif
      
      //The same little trick
      pel_system->next= nullptr;
      
#ifdef PELVIEW_DEBUG
      //      gout << "\n Step 12 \n";
#endif
      solutionsarecorrect = CheckSolutions(G_Verify(Link(pel_system,sols)));
      if (solutionsarecorrect && 
	  !HasARedundancy(GambitRootsFromPelRoots(sols))) 
	done = true;
      if (!done) free_Gen_list(sols);
#ifdef PELVIEW_DEBUG
      //      gout << "The solutions are ";
      //    if (!solutionsarecorrect)
	//	gout << "not ";
      //      gout << "correct.\n";
#endif
    }
  
  return sols;
  
   // gout<< "\n Solve \n";  print_Gen_list(Solve);
//   free_Gen_list(Solve);
// Solve doesn't seem to exist at this point, yet this is very ambiguous.  
}

Gambit::List<Gambit::Vector<gComplex> > 
PelView::GambitRootsFromPelRoots(const Gen_node g) const
{
  Gambit::List<Gambit::Vector<gComplex> > alist;

  node ptr;
  ptr = Gen_to_Dvector_list(Gen_lval(copy_Gen_node(g)));

  Dmatrix P;
  int i;

  while(ptr!=nullptr) {
    P=(Dmatrix)(Car(Car(ptr)));

    int numbervar;
    numbervar= (int)(DVlength(P)-3)/2;  
    
    Gambit::Vector<gComplex> vector(1, numbervar);

    int j = 1;
    for(i=3; i<DVlength(P);i++) {
      if (i%2==0 && i<DVlength(P)) {
	double re,im;
	re = DVref(P,i-1);
	im = DVref(P,i);
	gComplex complexsol(re, im);

	vector[j] = complexsol; 
	j+=1;	  
      }
    }
    alist.push_back(vector);
    ptr=Cdr(ptr); 
  }
  
  return alist;
}

void PelView::DisplayComplexRootList(const Gambit::List<Gambit::Vector<gComplex> > 
				                       &complexroots) const
{
#ifdef UNUSED
  for (int k = 1; k <= complexroots.Length(); k++)
    for (int m = 1; m <= complexroots[k].Length(); m++) {
      if (m ==1) 
	gout << "{";
      for (int n = 1; n <= m; n++)
	gout << "  ";
      gout << complexroots[k][m];
      if (m < complexroots[k].Length())
	gout << ",\n";
      else
	gout << " }\n";
    }
#endif  // UNUSED
}

int PelView::Dmnsn() const
{
  return input.Dmnsn();
}

Gambit::List<Gambit::Vector<double> > 
PelView::RealRoots(const Gambit::List<Gambit::Vector<gComplex> > &clist) const
{
  Gambit::List<Gambit::Vector<double> > answer;

  for (int i = 1; i <= clist.Length(); i++) {

    bool is_real = true;
    for (int j = 1; j <= Dmnsn(); j++)
      if (Gambit::abs(clist[i][j].ImaginaryPart()) > 0.0001) 
	is_real = false;

    if (is_real) {
      Gambit::Vector<double> next(Dmnsn());
      for (int j = 1; j <= Dmnsn(); j++) 
	next[j] = (double)clist[i][j].RealPart();
      answer.push_back(next);
    }

  }

  return answer;
}

bool PelView::CheckSolutions(const Gen_node g) const
{
  Gen_node goo;
  goo = g->Genval.lval;
  while (goo!=nullptr) { 
    if (Gambit::abs(goo->Genval.dval) > 0.01) 
      return false;

    goo = goo->next;
  }

  return true;
}

PelView::PelView(const gPolyList<double> &mylist):input(mylist)
{
  InitializePelicanMemory();
  
#ifdef PELVIEW_DEBUG
  //  gout << "We begin with the polynomial list\n" << mylist << "\n";
#endif

  Pring ring = MakePring(input.Length());  

#ifdef PELVIEW_DEBUG
  //  gout<< " Step 1 \n";
  PrintPring(ring);
#endif

  Gen_node vic = Set_Ring((CreateRing(input.Length()+1)));

#ifdef PELVIEW_DEBUG
  //  gout<< "\n Step 2 \n";
  print_Gen_node(vic);
#endif

  //  Gen_node pel_system = G_System(translate(input,ring)); 
  Gen_node pel_system = CreatePelicanVersionOfSystem(input,ring); 

#ifdef PELVIEW_DEBUG
  //  gout<< "\n\n Step 3 \n";
  //  gout << "The translated system is:\n";
  print_Gen_node(pel_system); 
#endif

  Gen_node Atype= G_AType(pel_system); // Atype is the vector of numbers of

#ifdef PELVIEW_DEBUG
  //  gout<< " Step 4 \n";
  print_Gen_list(Atype);               // polys with each support type
#endif

  Gen_node Aset = G_Aset(pel_system);

#ifdef PELVIEW_DEBUG
  //  gout<< "\n Step 5 \n";
  //  gout << "The list of unlifted vertex tuples is:\n";
  print_Gen_node(Aset);
#endif

  Gen_node Randlift = G_RandLift(Aset);

#ifdef PELVIEW_DEBUG
  //  gout<< "\n\n Step 6 \n";
  //  gout<< "After the random lift, the vertex tuples are:\n";
  print_Gen_list(Randlift);

  //  gout << "\n\n Step 7 \n";
  //  gout << "To see the cells in the subdivision, define ACTUALLY_PRINT in pelclqhl.cc:\n";
#endif

  Gen_node Qtrig = G_Qtrig(Link(Randlift, Atype));

  /* - There is a commented out error in node_push_local that was triggered 
by this in various conditions.  In particular, commented out the final print 
avoids the error, somehow!!
//  gout<< "\n Step 8 \n";
//  gout<< "Before Randlift is ";
//  gout << Randlift << "\n";
  silent_print_Gen_list(Randlift);
//  gout<< "After Randlift is " << Randlift << "\n";
  */

  mixedvolume = GetMixedVolume(Randlift);

#ifdef PELVIEW_DEBUG
  //  gout<< "\n Step 8 \n";
  //  gout << "The mixed volume is " << mixedvolume << ".\n";
#endif

  Gen_node Genpoly = G_Gen_Poly(Randlift);

#ifdef PELVIEW_DEBUG
  //  gout<< "\n Step 9 \n";
  //  gout << "The homotopy system is:\n";
  print_Gen_list(Genpoly);

  //  gout<< "\n Step 10 \n";
  //  gout << "The Gen_node Solve (solutions of start system) is: \n";
#endif

  Gen_node sols = SolveCheckMaybeTryAgain(ring, Genpoly, Qtrig, pel_system);
  complexroots = GambitRootsFromPelRoots(sols);

#ifdef PELVIEW_DEBUG
  //  gout << "\n Step 13 \n";
  //  gout<<" After conversion to Gambit, the complex roots are...\n";
  DisplayComplexRootList(complexroots);
#endif

  realroots = RealRoots(complexroots);

#ifdef PELVIEW_DEBUG
  //  gout << "\n Step 14 \n";
  //  gout<<" The real solutions are...\n";
  // for (int k = 1; k <= realroots.Length(); k++)
  //    gout << realroots[k] << "\n";
  //  gout<< "\n Step 15 \n";
  //  gout<< "\n Memory testing \n";
  print_Gen_list(Genpoly); 
#endif
  
  free_Gen_list(Genpoly);

#ifdef PELVIEW_DEBUG
  //  gout<< "\n Qtrig \n";  print_Gen_list(Qtrig);   
#endif

  free_Gen_list(Qtrig);
  free_Gen_list(vic);
  free_Gen_list(sols);

#ifdef PELVIEW_DEBUG
  //  gout << "And the memory should be clean...\n";
#endif
}

PelView::PelView(const PelView & given)
  
    
= default;

PelView::~PelView()
= default;

PelView& PelView::operator =(const PelView &rhs)
{
  //  gout << "For (eventual) const'ness, operator = not allowed for PelView\n";
  exit (0);
  return *this;  
}

bool PelView::operator ==(const PelView &rhs) const
{
  return (input               == rhs.input        &&
	  complexroots        == rhs.complexroots &&
	  realroots           == rhs.realroots    &&
	  mixedvolume         == rhs.mixedvolume  &&
	  solutionsarecorrect == rhs.solutionsarecorrect);
}

bool PelView::operator !=(const PelView &rhs) const
{
  return !(*this == rhs);
}

Gambit::List<Gambit::Vector<gComplex> > PelView::ComplexRoots() const
{
  return complexroots;
}

Gambit::List<Gambit::Vector<double> > PelView::RealRoots() const
{
  return realroots;
}

int PelView::MixedVolume() const
{
  return mixedvolume;
}

int PelView::NumComplexRoots() const
{
  return complexroots.Length();
}

bool PelView::FoundAllRoots() const
{
  return (NumComplexRoots() == MixedVolume()); 
}

int PelView::NumRealRoots() const
{
  return realroots.Length();
}

/**********************************************************************/

// was the original main in the pelican driver
int old_main()
{
 //stuff for Gambit-text to Gambit-data
  for (int loop = 1; loop <= 3; loop++) {

  gSpace Space(4); 
  ORD_PTR ptr = &lex;
  term_order Lex(&Space, ptr);
  ptr =  &reversedeglex;
  term_order  ReverseDegLex(&Space, ptr);
  ptr = &reverselex;
  term_order ReverseLex(&Space, ptr);
  
  //Default system
  std::string gx = " 2 + n2 ";
  std::string gy = " 1 + 78 * n1 + 2 * n2  + n4 * n1^2";
  std::string gz = " 3 + n3 + n4";
  std::string gu = " 4 * n1 - n2 * n3 + 6 * n1 * n4^3";
  gPoly<double> px(&Space,gx,&Lex);
  gPoly<double> py(&Space,gy,&Lex); 
  gPoly<double> pz(&Space,gz,&Lex);
  gPoly<double> pu(&Space,gu,&Lex);
  gPolyList<double> mylist(&Space, &ReverseDegLex);
  mylist += px;
  mylist += py;
  mylist += pz;
  mylist += pu;
  
  PelView atlast(mylist);


  gSpace NewSpace(3); 
  ptr = &lex;
  term_order NewLex(&NewSpace, ptr);
  ptr =  &reversedeglex;
  term_order  NewReverseDegLex(&NewSpace, ptr);
  ptr = &reverselex;
  term_order NewReverseLex(&NewSpace, ptr);
  
  //Default system
  std::string newgx = " 2 + n2 ";
  std::string newgy = " 1 + 78 * n1 + 2 * n2  + n3 * n1^2";
  std::string newgz = " 3 + n3";
  gPoly<double> newpx(&NewSpace,newgx,&NewLex);
  gPoly<double> newpy(&NewSpace,newgy,&NewLex); 
  gPoly<double> newpz(&NewSpace,newgz,&NewLex);
  gPolyList<double> mynewlist(&NewSpace, &NewReverseDegLex);
  mynewlist += newpx;
  mynewlist += newpy;
  mynewlist += newpz;

  
  PelView newatlast(mynewlist);

  }

  return 0;
}
