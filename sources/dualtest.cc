#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include "gambitio.h"
#include "rational.h"
#include "dualtope.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

// BAD type
  struct int_pair {  // For recording the endpoints of an edge
    int head;
    int tail;
  };
// This is inherited from earlier versions of polytope.h.  

#include "glist.imp"
#include "dualtope.imp"
/*
TEMPLATE class gList<gPolytope<int>*>;
TEMPLATE class gNode<gPolytope<int>*>;
TEMPLATE class gList<int_pair*>;
TEMPLATE class gNode<int_pair*>;
*/


int V_CT, P_CT, M_CT;

int main(int argc, char *argv[])
{
  assert (argc == 1);

  gSpace* Space = new gSpace(2);

  gVector<gRational> normal1(2);
  normal1[1] = (gRational)1;
  normal1[2] = (gRational)2;
  gRational constant1 = (gRational)3;
  gHalfSpc<gRational> halfspace1(Space,constant1,normal1);

  gVector<gRational> normal2(2);
  normal2[1] = (gRational)-1;
  normal2[2] = (gRational)-2;
  gRational constant2 = (gRational)-6;
  gHalfSpc<gRational> halfspace2(Space,constant2,normal2);

  gVector<gRational> normal3(2);
  normal3[1] = (gRational)-2;
  normal3[2] = (gRational)-1;
  gRational constant3 = (gRational)-6;
  gHalfSpc<gRational> halfspace3(Space,constant3,normal3);

  gList<gHalfSpc<gRational> > list;
  list += halfspace1; 
  list += halfspace2; 
  list += halfspace3; 

  gDualTpe<gRational> dualtope(Space,list);

  gout << "Our first dualtope is\n" << dualtope << ".\n";

  exit(0);     
}
