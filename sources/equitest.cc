// test file for the class polylist

#include <time.h>
#include "equisolv.h"
#include "rational.h"
#include "gwatch.h"

// This program is a test program for class EquiSolv

int main()
{ 
  gSpace Space(4);

  ORD_PTR ptr = &lex;
  term_order Lex(&Space, ptr);
  ptr = &reversedeglex;
  term_order ReverseDegLex(&Space, ptr);
  ptr = &reverselex;
  term_order ReverseLex(&Space, ptr);

  gPoly<gRational> pw(&Space,&ReverseDegLex);
  gPoly<gRational> px(&Space,&ReverseDegLex);
  gPoly<gRational> py(&Space,&ReverseDegLex);
  gPoly<gRational> pz(&Space,&ReverseDegLex);

  gPoly<gRational> pa(&Space,&ReverseDegLex);
  gPoly<gRational> pb(&Space,&ReverseDegLex);

  gText gw; gText gx; gText gy; gText gz; 
  gText ga; gText gb;

/* simple test of root solving 
  gw = "1 - 2n1"; pw = gw;
  gx = "1 - 3n2"; px = gx;
  gy = "1 - 4n3"; py = gy;
  gz = "1 - 5n4"; pz = gz;
*/

/* simple test of root solving */
  gw = "2 - 9n1 + 9n1^2"; pw = gw;
  gx = "2 - 9n2 + 9n2^2"; px = gx;
  gy = "2 - 9n3 + 9n3^2"; py = gy;
  gz = "2 - 9n4 + 9n4^2"; pz = gz;

  ga = "2n3 - 1";         pa = ga;
  gb = "2n4 - 1";         pb = gb;
/**/

/* 3x2x2 game form 
  gw = "3 + 5n3 - 4n4 + 25n3 * n4";                         pw = gw;
  gx = "4 + 3n3 - 7n4 + 10n3 * n4";                         px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 3n1 * n4 - 5n2 * n4";        py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 6n1 * n3 -2n2 * n3";        pz = gz;
*/

/* 2x2x2x2 game form - seven simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3";      pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - six simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3";      pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - five simplifications
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3";      pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - four simplifications
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4";      pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - three simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4";      pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - two simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4";      pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4 - 5n2 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - one simplification 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4";                pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4 - 10n3 * n4";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4 - 5n2 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - no cubic terms 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4 + 25n3 * n4";    pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4 - 10n3 * n4";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4 - 5n2 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3";   pz = gz;
*/

/* 2x2x2x2 game form 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4 + 5n3 * n4 + n2 * n3 * n4";  
  pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4 - 10n3 * n4 + n1 * n3 * n4";
  px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4 - 5n2 * n4 + n1 * n2 * n4";
  py = gy;
  gz = "7 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3 + n1 * n2 * n3";
  pz = gz;
*/

  // Set up the rectangle of search
  gVector<gDouble> bottoms(4); 
  gVector<gDouble> tops(4); 
  for (int i = 1; i <= 4; i++) {
    bottoms[i] = (gDouble)-4.0; tops[i] = (gDouble)4.0;
  }
  //  bottoms[1] = (gRational)2; tops[1] = (gRational)3;
  //  bottoms[2] = (gRational)1; tops[2] = (gRational)2;
  //  bottoms[3] = (gRational)-4; tops[3] = (gRational)-3;
  //  bottoms[4] = (gRational)-1; tops[4] = (gRational)0;

  gRectangle<gDouble> Cube(bottoms, tops); 

/*
  // First System
  gPolyList<gRational> test_bss1(&Space,&ReverseDegLex);
  test_bss1 += pw;
  test_bss1 += px;
  test_bss1 += py;
  test_bss1 += pz;
  test_bss1 += pa;
  test_bss1 += pb;
*/


  gList<gPolyList<gRational> > input_list;
  gPolyList<gRational> list1(&Space,&ReverseDegLex);
  gPolyList<gRational> list2(&Space,&ReverseDegLex);
  list1 += pw;
  list1 += px;
  list1 += py;
  list2 += pz;
  list2 += pa;
  list2 += pb;
  input_list += list1;
  input_list += list2;

  gWatch timer;

  timer.Start();

  EquiSolv<gRational> quickie(input_list);

/*
  gTree<gPoly<gRational> > partials = quickie.TreeOfPartials(test_bss1[4]);
  gout << "The tree of partials is\n" << partials << "\n";

  if (quickie.PolyHasNoRootsIn(Cube,partials)) 
    gout << "There are no roots in the cube " << Cube << ".\n";
  else
    gout << "There might be roots in the cube " << Cube << ".\n";
  exit(0);
  */

/*
  if (quickie.MightHaveSingularRoots())
    gout << "The system is potentially singular.\n";
  else
    gout << "The system is guaranteed to have only regular roots.\n";

  timer.Stop();
  gout << "The EquiSolv computation of potential singularity took " 
    << (int)timer.Elapsed() << " seconds.\n\n";
*/
  gout << "Starting computation ... \n";
  if (quickie.FindRoots(Cube,100000))
    gout << "\nThe first test system is\n" 
	 << quickie.UnderlyingEquations() 
	 << "It has the following roots in [-4,4]^4:\n"
	 << quickie.RootList();
  else
    gout << "The system\n" << quickie.UnderlyingEquations()
	 << " could not be resolved by FindRootsRec.\n";

  timer.Stop();
  gout << "The EquiSolv computation of roots using FindRootsRec took " 
    << (int)timer.Elapsed() << " seconds.\n\n";


  timer.Start();

  if (quickie.FindRoots(Cube,100000))
    gout << "\nThe first test system is\n" 
	 << quickie.UnderlyingEquations() 
	 << "It has the following roots in [-4,4]^4:\n"
	 << quickie.RootList();
  else
    gout << "The system\n" << quickie.UnderlyingEquations()
	 << " could not be resolved by FindRootsRec.\n";

  timer.Stop();
  gout << "The EquiSolv computation of roots Using FindRoots took " 
    << (int)timer.Elapsed() << " seconds.\n\n";

exit(0);

}
