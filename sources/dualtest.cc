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
TEMPLATE class gList<gPolytope<int>*>;
TEMPLATE class gNode<gPolytope<int>*>;
TEMPLATE class gList<int_pair*>;
TEMPLATE class gNode<int_pair*>;

int V_CT, P_CT, M_CT;


gList<gPolytope<int>*> read_input(char* filename, int** nos_vrtcs, int& D, int& N)
{ 
  gFileInput data(filename);
  data >> D >> N;
  *nos_vrtcs = new int[N];
  for(int i=0 ; i<N; i++) 
    data >> (*nos_vrtcs)[i];
  gList<gPolytope<int>*> p_list;
  gSpace* list = new gSpace(N);

  for (int i = 0; i < N; i++)  {
    gPolytope<int>* temp = new gPolytope<int>(list, D, (*nos_vrtcs)[i], data);
    p_list+=temp;
  }
  return p_list;
}

int main(int argc, char *argv[])
{
  assert (argc == 2);

  gout << "reading input ...\n\n";

  int D, N;
  gList<gPolytope<int>*> input_polys;
  int *NumVerts;
  input_polys = read_input(argv[argc - 1], &NumVerts, D, N);

  gout << "finding extreme vertices and edges ...\n\n";

  int* nos_edges = new int[N];
  int_pair** edges = new (int_pair *)[N];
  for (int i = 1; i <= N; i++)
    {
      gList<ind_pair*> edge_pairs;

//DEBUG
//      gout << "We have  i = " << i << "  and  N = " << N << ".\n";

      edge_pairs = input_polys[i]->ind_pairs_of_edges();
      nos_edges[i] = edge_pairs.Length();

      edges[i] = new int_pair[nos_edges[i]];

      for (int j = 1; j <= nos_edges[i]; j++)
	{
	  edges[i][j-1].head = edge_pairs[j]->head - 1;
	  edges[i][j-1].tail = edge_pairs[j]->tail - 1;
	}
    }

  delete [] NumVerts;

  for (int i = 1; i <= N; i++)
    gout << "Polytope " << i+1 << " is\n" << *(input_polys[i]);

  for (int i = 0; i < N; i++)
    delete [] edges[i];
  delete [] edges;
  delete [] nos_edges;
  
  exit(0);     
}
