// test file for the class polylist

#include <time.h>
#include "gsolver.h"
#include "rational.h"
#include "gwatch.h"

// This program is a test program for the classes gpolylst and gsolver

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

  gString gw; gString gx; gString gy; gString gz;

/* 3x2x2 game form */
  gw = "3 + 5n3 - 4n4 + 25n3 * n4";                         pw = gw;
  gx = "4 + 3n3 - 7n4 + 10n3 * n4";                         px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 3n1 * n4 - 5n2 * n4";        py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 6n1 * n3 -2n2 * n3";        pz = gz;
/**/


  gPolyList<gRational> test_bss1(&Space);
  test_bss1 = gPolyList<gRational>(&Space);
  test_bss1 += pw;
  test_bss1 += px;
  test_bss1 += py;
  test_bss1 += pz;

  gWatch timer;
  timer.Start();

//  time_t* time_ptr;

//  time_t t1a = time(time_ptr);

  gout << "\nThe first system of polynomials, for totally mixed equilibrium\n"
    << "of a 3x2x2 game, is:\n" << test_bss1;

//  gIdeal<gRational> ideal(&ReverseDegLex,test_bss2);
//  gout << "\nThe canonical basis is\n" << ideal.CanonicalBasis();
//  gout << "\nThe basis of the monomial ideal is\n" 
//    << ideal.MonomialIdeal().CanonicalBasis();
//  gout << "\nThe monomial basis of the ring of polynomial "
//    << "functions on V(I) is\n"
//      << ideal.MonomialBasis();
//  gIdeal<gRational> ideal(&ReverseDegLex,test_bss2);
//  gout << "\nThe canonical basis is\n" << ideal.CanonicalBasis();
//  gout << "\nThe basis of the monomial ideal is\n" 
//    << ideal.MonomialIdeal().CanonicalBasis();
//  gout << "\nThe monomial basis of the ring of polynomial "
//    << "functions on V(I) is\n"
//      << ideal.MonomialBasis();

  gSolver<gRational> solution1(&ReverseDegLex,test_bss1);
  gList<gVector<gDouble> > rootlist1 = solution1.Roots();
  gout << "The list of roots is:\n" << rootlist1;

  timer.Stop();
  gout << "The computation of the roots took " 
    << (int)timer.Elapsed() << " seconds.\n";

/* 2x2x2x2 game form - no cubic terms */
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4 + 25n3 * n4";    pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4 - 10n3 * n4";     px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4 - 5n2 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3";   pz = gz;
/**/

  gPolyList<gRational> test_bss2(&Space);
  test_bss2 = gPolyList<gRational>(&Space);
  test_bss2 += pw;
  test_bss2 += px;
  test_bss2 += py;
  test_bss2 += pz;

  timer.Start();

  gout << "\nThe second system of polynomials, for totally mixed equilibria\n"
    << "of a 2x2x2x2 game with no cubic terms, is:\n" 
      << test_bss2;

  gSolver<gRational> solution2(&ReverseDegLex,test_bss2);
  gList<gVector<gDouble> > rootlist2 = solution2.Roots();
  gout << "The list of roots is:\n" << rootlist2;

  timer.Stop();
  gout << "The computation of the roots took " 
    << (int)timer.Elapsed() << " seconds.\n";

/* 2x2x2x2 game form */
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4 + 5n3 * n4 + n2 * n3 * n4"; 
    pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4 - 10n3 * n4 + n1 * n3 * n4"; 
    px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4 - 5n2 * n4 + n1 * n2 * n4"; 
    py = gy;
  gz = "7 + 4n1 - n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3 + n1 * n2 * n3";   
    pz = gz;
/**/

  gPolyList<gRational> test_bss3(&Space);
  test_bss3 = gPolyList<gRational>(&Space);
  test_bss3 += pw;
  test_bss3 += px;
  test_bss3 += py;
  test_bss3 += pz;

  timer.Start();

  gout << "\nThe third system of polynomials, for totally mixed equilibria of "
    << "a 2x2x2x2 game, is:\n" << test_bss3;

  gSolver<gRational> solution3(&ReverseDegLex,test_bss3);
  gList<gVector<gDouble> > rootlist3 = solution3.Roots();
  gout << "The list of roots is:\n" << rootlist3;

  timer.Stop();
  gout << "The computation of the roots took " 
    << (int)timer.Elapsed() << " seconds.\n";


//   Repository of Polynomial Systems   //

/* // Original 
  gx = "n3 - n2 * n1";               px = gx;
  gy = "n2^2 - 1 - n3 + n1";         py = gy;
  gz = "n3^3 + n3 * n2 * n1";        pz = gz;
*/

/* 2x2x2 game form 
  gx = "3 + 5n2 - 4n3 + 25n2 * n3";        px = gx;
  gy = "7 - 2n1 + 37n3 + 3n1 * n3";        py = gy;
  gz = "25 + 4n1 - 3n2 + 6n1 * n2";        pz = gz;
*/

/* 3x2x2 game form 
  gw = "3 + 5n3 - 4n4 + 25n3 * n4";                         pw = gw;
  gx = "4 + 3n3 - 7n4 + 10n3 * n4";                         px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 3n1 * n4 - 5n2 * n4";        py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 6n1 * n3 -2n2 * n3";        pz = gz;
*/

/* 2x2x2x2 game form - seven simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3";                pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3";                 px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2";               py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - six simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3";                pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3";                 px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - five simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3";                pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";      px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - four simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4";     pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";      px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";    py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - three simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4";                pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";                 px = gx;
  gy = "7 - 2n1 + 4n2 + 37n4 + 7n1 * n2 - 3n1 * n4";               py = gy;
  gz = "25 + 4n1 - -2n2 - 3n3 + 5n1 * n2 - 6n1 * n3 - 2n2 * n3";   pz = gz;
*/

/* 2x2x2x2 game form - two simplifications 
  gw = "3 - 2n2 + 5n3 - 4n4 + 7n2 * n3 - 4n2 * n4";                pw = gw;
  gx = "4 + n1 - 3n3 - 7n4 + 5n1 * n3 - 6n1 * n4";                 px = gx;
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
}

