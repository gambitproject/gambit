#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include "gstream.h"
#include "rational.h"
#include "dualtope.h"

int main(int argc, char *argv[])
{
  assert (argc == 1);

  gSpace* Space = new gSpace(2);

  gVector<gRational> normal1(2);
  normal1[1] = (gRational)1;
  normal1[2] = (gRational)0;
  gRational constant1 = (gRational)0;
  gHalfSpc<gRational> halfspace1(Space,constant1,normal1);

  gVector<gRational> normal2(2);
  normal2[1] = (gRational)0;
  normal2[2] = (gRational)1;
  gRational constant2 = (gRational)0;
  gHalfSpc<gRational> halfspace2(Space,constant2,normal2);

  gVector<gRational> normal3(2);
  normal3[1] = (gRational)-2;
  normal3[2] = (gRational)-1;
  gRational constant3 = (gRational)-3;
  gHalfSpc<gRational> halfspace3(Space,constant3,normal3);

  gVector<gRational> normal4(2);
  normal4[1] = (gRational)-2;
  normal4[2] = (gRational)-1;
  gRational constant4 = (gRational)-5;
  gHalfSpc<gRational> halfspace4(Space,constant4,normal4);

  gList<gHalfSpc<gRational> > list1;
  list1 += halfspace1; 
  list1 += halfspace2; 
  list1 += halfspace3; 
  list1 += halfspace4; 

  gDualTope<gRational> dualtope1(Space,list1);
  gout << "Our first dualtope is\n" << dualtope1;
  gout << "Its bounding rectangle is " 
       << dualtope1.BoundingRectangle() << "\n";
  gout << "The derived dualtope is \n" << 
    gDualTope<gRational>(Space,dualtope1.BoundingRectangle()) << "\n";

  gVector<gRational> normal5(2);
  normal5[1] = (gRational)-1;
  normal5[2] = (gRational)0;
  gRational constant5 = (gRational)-4;
  gHalfSpc<gRational> halfspace5(Space,constant5,normal5);

  gVector<gRational> normal6(2);
  normal6[1] = (gRational)0;
  normal6[2] = (gRational)1;
  gRational constant6 = (gRational)0;
  gHalfSpc<gRational> halfspace6(Space,constant6,normal6);

  gVector<gRational> normal7(2);
  normal7[1] = (gRational) 1;
  normal7[2] = (gRational)-1;
  gRational constant7 = (gRational)1;
  gHalfSpc<gRational> halfspace7(Space,constant7,normal7);

  gList<gHalfSpc<gRational> > list2;
  list2 += halfspace5; 
  list2 += halfspace6; 
  list2 += halfspace7; 

  gDualTope<gRational> dualtope2(Space,list2);
  gout << "Our second dualtope is\n" << dualtope2;
  gout << "Its bounding rectangle is " 
       << dualtope2.BoundingRectangle() << "\n";
  gout << "The derived dualtope is \n" << 
    gDualTope<gRational>(Space,dualtope2.BoundingRectangle()) << "\n";

  gDualTope<gRational> dualtope3(dualtope1.IntersectedWith(dualtope2));
  gout << "The intersection of the two dualtopes is\n"
    << dualtope3;
  gout << "Its bounding rectangle is " 
       << dualtope3.BoundingRectangle() << "\n";
  gout << "The derived dualtope is \n" << 
    gDualTope<gRational>(Space,dualtope3.BoundingRectangle()) << "\n";

  exit(0);     
}
