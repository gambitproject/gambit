// test file for the class polylist

#include "equisolv.h"
#include "gwatch.h"

// This program is a test program for class EquiSolv

int main()
{ 
  gSpace Space(4);

  ORD_PTR ptr = &reversedeglex;
  term_order ReverseDegLex(&Space, ptr);

  gPoly<gRational> pw(&Space,&ReverseDegLex);
  gPoly<gRational> px(&Space,&ReverseDegLex);
  gPoly<gRational> py(&Space,&ReverseDegLex);
  gPoly<gRational> pz(&Space,&ReverseDegLex);

  gPoly<gRational> pa(&Space,&ReverseDegLex);
  gPoly<gRational> pb(&Space,&ReverseDegLex);

  gText gw; gText gx; gText gy; gText gz; 
  gText ga; gText gb;

/* simple test system */
  gw = "2 - 9n1 + 9n1^2"; pw = gw;
  gx = "2 - 9n2 + 9n2^2"; px = gx;
  gy = "2 - 9n3 + 9n3^2"; py = gy;
  gz = "2 - 9n4 + 9n4^2"; pz = gz;

  ga = "2n3 - 1";         pa = ga;
  gb = "2n4 - 1";         pb = gb;

/**/

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


  gList<gPolyList<gRational> > input_list;
  gPolyList<gRational> list1(&Space,&ReverseDegLex);
  gPolyList<gRational> list2(&Space,&ReverseDegLex);

  list1 += pw;
  list1 += px;
  list1 += py;
  list1 += pz;

  list2 += pa;
  list2 += pb;

  input_list += list1;
  input_list += list2;

  gWatch timer;

  timer.Start();

  EquiSolv<gRational> quickie(input_list);

  if (quickie.SystemHasNoEquilibriaIn(Cube))
    gout << "There are no equilibria in Cube.\n\n";
  else
    gout << "There might be roots in Cube.\n\n";

  timer.Stop();
  gout << "The EquiSolv constructor took " 
    << (int)timer.Elapsed() << " seconds.\n\n";


exit(0);

}
