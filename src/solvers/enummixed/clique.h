//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/src/enummixed/clique.h
// Maximal cliques and connected components
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

//  connected components and their maximal cliques in bipartite graphs 
//  Bernhard von Stengel
// 
//  update 15 August 1998:
//
//    - candidate passed to  candtry12  without  poscand,  similarly
//      candidates and not their stack positions stored in nonconnected-list
//      (removes serious bug)
//
//    - if CAND1 and CLIQUE1 both empty, terminate search;
//      dito CAND2 and CLIQUE2
//    - outgraph left in;  
//    
//    8 March 1998
//
//    For a bipartite graph given as a set of pairs  (i,j), it outputs 
//    - the connected components of that graph, and for each component
//    - the maximal product sets   U x V
//      so that all  (i,j)  in U x V  are edges of the graph
//      (so these are the maximal complete bipartite subgraphs or CLIQUES).
//
//    INPUT:
//    The edges (i, j) are given by pairs of nonnegative integers separated
//    by blanks on standard input.
//
//    OUTPUT:
//    On standard output,
//    a headline for each connected component, then
//    the cliques  U x V  listing  U  and  V  separately as lists
//    of integers, separated by  "x"  and each set enclosed in braces, 
//    one clique per line.
//
//    METHOD:
//    Connected components by a primitive version of union-find,
//    cliques with a variant of the algorithm by
//    [BK] C. Bron and J. Kerbosch, Finding all cliques of an undirected
//         graph, Comm. ACM 16:9 (1973), 575-577.
//
//    APPROXIMATE STORAGE REQUIREMENTS:
//    for integer arrays, 4 bytes per integer, using constants
//    maxinp1, maxinp2  max. node indices in input
//    MAXEDGES          max. no. edges in input
//    MAXM, MAXN        max. dimension of incidence matrix
//   	              per connected component
//    2 x MAXM x MAXN  integers for incidence matrix and stack
//	   [2 MB  if MAXM = MAXN = 700 ]
//    3 x MAXEDGES  integers for edge list
//	   [0.6 MB  if  MAXEDGES = 50000 ]
//    3 x maxinp1  integers for input nodes and list of components
//	   [60 kB   if  maxinp1 = maxinp2 = 5000 ]
//    If these constants are exceeded certain edges will be rejected
//    from the input with an error message.  Program shouldn't crash.
//    No error value is returned by  main().
//
//    DETAILS OF METHODS:
//
//    a) Connected components
//
//    Designed for minimum storage requirement, running time
//    possibly quadratic in number of edges.
//    For each node that is read, a component  co1[i]  resp.  co2[j]
//    ( i  left node,  j  right node) is kept, initially  0  if node
//    is not yet input.  (Isolated nodes are treated as absent.)
//    For an edge  (i, j),  i and j  must be put in the same component.
//    Each component  co  points to the first edge in  edgelist,
//    where the edges are linked.  Merging two components is done
//    by traversing the edgelist with the higher number, updating
//    the component number of the nodes therein, and prepending
//    it to the list of the other component.
//    Components and edges are numbered starting with 1,  so "no
//    component" and the end of an edgelist is represented by 0.
//
//    Sets are represented by C arrays, if starting with 0
//    (as usually in C),  then the elements of a  k-set are the
//    array elements [0..k) i.e. [0..k-1],  if starting with 1
//    they are [1..k].
//
//    A possible improvement is to keep extra lists of the
//    equivalence classes for the nodes for each component so only
//    these have to be updated, which makes it faster.  
//    
//    b) Clique enumeration
//
//    The procedure  extend  recursively extends a current set of pairs
//    clique1, clique2  that eventually will form a maximal clique.
//    In [BK], this is only a single set  COMPSUB (here called CLIQUE),
//    here two sets are used since the graph is bipartite.  
//    Cliques of a bipartite graph are equivalent to the cliques of
//    the ordinary graph obtained by connecting all left nodes by
//    themselves and all right nodes by themselves, except for
//    the cliques consisting exclusively of left or right points.
//
//    The recursive calls use a self-made stack  stk  containing
//    local small arrays of variable size.  Intervals of this stack
//    are indicated by their endpoints which ARE local variables
//    to the recursive call.  The top of the stack  tos  is passed
//    as a parameter.
//
//    The extension is done by adding points from a set CAND of 
//    candidates to CLIQUE.  Throughout, the points in CAND are
//    connected to all points in  CLIQUE,  which holds at initialization
//    when CAND contains all points and CLIQUE is empty.  
//
//    Traversing the backtracking tree:  Extending its depth is
//    done by picking  c  (cand  in the code below) from CAND,
//    adding  c  to CLIQUE, removing all points not connected to  c 
//    from CAND, and handing the new sets CLIQUE and CAND
//    to the recursive call.
//    For extending the backtracking tree in its breadth, this is
//    done in a loop (called backtracking cycle in [BK]) where repeatedly
//    different candidates  c  are added to CLIQUE (after the respective
//    return from the recursive call).  In order to avoid the output
//    of cliques that are not maximal, an additional set NOT is passed
//    down as a parameter to the recursive call of  extend.
//    This set NOT contains candidates  c  that 
//    - are all connected to the elements in CLIQUE but
//    - have already been tried out, that is, all extensions of CLIQUE
//      containing any point in NOT have already been generated [BK, p.577].  
//
//    Hence, the recursive call proceeds downwards by
//    - removing  c  from CAND and adding it to CLIQUE
//    - removing all points disconnected from  c  from the new
//      sets  NOT  and  CAND  used in the recursive call.
//    After extension,  c  is then moved to  NOT  and the next
//    candidate is tried.
//
//    To reduce the breadth of the backtracking tree, the first
//    candidate (or the subsequent ones) are chosen such that
//    as early as possible there is a node in NOT connected to all
//    remaining candidates.  Then NOT will never become empty and
//    hence no clique will be output, so the backtracking tree can
//    be pruned here.  This is done by choosing first a  fixpoint  fixp
//    in the set  NOT  or  CAND, such that after extension, when
//    fixp  is definitely in  NOT,  only points disconnected to  fixp
//    are added.  Their number is the smallest possible.
//
//    This is version2 of the algorithm in [BK]:
//    a - pick  fixp  in NOT or CAND  with the smallest number of 
//        disconnections to the other nodes in CAND,
//    b - if  fixp  is a candidate, try it out as a candidate, i.e.
//        extend  CLIQUE  with  fixp  (procedures  candtry  below),
//	and then move   fixp  to  NOT after extension.
//    c - then try out only points disconnected to  fixp,  as
//	determined in a.  (In contrast to [BK], we compute
//	a local list of these disconnected points while looking
//	for the smallest number of disconnections.)
//
//    Amendments for the bipartite graph are here:  a  is done
//    by inspecting both sides of the graph.
//    For the single extension in  b  (if  fixp  is a candidate)
//    and the extensions in  c , only the sets NOT and CAND
//    on the other side of the candidate used for extension
//    have to be updated.  Hence,  NOT and CAND are kept as
//    separate sets  NOT1,  NOT2  and CAND1, CAND2.

#ifndef GAMBIT_ENUMMIXED_CLIQUE_H
#define GAMBIT_ENUMMIXED_CLIQUE_H

#include <cstdio>
#include "gambit.h"

namespace Gambit {
namespace Nash {

const int MAXM = 700;     // max. no of left nodes for incidence matrix
const int MAXN = MAXM;    // max. no of right nodes for incidence matrix

// #define MAXINP1 5000        // max. no of left nodes in input
// #define MAXINP2 MAXINP1  // max. no of right nodes in input
// #define MAXEDGES   50000       // max. no of edges in input
// #define MAXCO  MIN(MAXINP1, MAXINP2) + 1
  // max. no of connected components;  on the smaller side,
  // each node could be in different component 
const int STKSIZE = (MAXM + 1) * (MAXN + 1);
  // largest stack usage for full graph 


class CliqueEnumerator {
public:
  class Edge {
  public:
    int node1;
    int node2;
    int nextedge;
    Edge() = default;
    ~Edge() = default; 
    bool operator==(const Edge &y) const 
    { return (node1 == y.node1 && node2 == y.node2); }
    bool operator!=(const Edge &y) const
    { return !(*this == y); }
  };

  CliqueEnumerator(Array<Edge> &, int, int);
  ~CliqueEnumerator() = default;

  const List<Array<int> > &GetCliques1() const { return m_cliques1; }
  const List<Array<int> > &GetCliques2() const { return m_cliques2; }

private:
  Array<int> firstedge;
  int maxinp1,maxinp2;
  List<Array<int> > m_cliques1, m_cliques2;

  void candtry1 (int stk[], // stack 
	 bool connected[MAXM][MAXN],
	 int cand,  // the candidate from NODES1  to be added to CLIQUE	 
	 int clique1[], int cliqsize1,  // CLIQUE so far in NODES1 
	 int clique2[], int cliqsize2,  // CLIQUE so far in NODES2 
	 int sn1, int *sc1, int ec1,   // start NOT1, start CAND1, end CAND1 
	 int sn2, int sc2, int ec2,   // start NOT2, start CAND2, end CAND2 
	 int tos,  // top of stack 
	 int orignode1[],
	 int orignode2[]
         );
  void candtry2 (int stk[], // stack 
	 bool connected[MAXM][MAXN],
	 int cand,  // the candidate from NODES2  to be added to CLIQUE 
	 int clique1[], int cliqsize1,  // CLIQUE so far in NODES1 
	 int clique2[], int cliqsize2,  // CLIQUE so far in NODES2 
	 int sn1, int sc1, int ec1,   // start NOT1, start CAND1, end CAND1 
	 int sn2, int *sc2, int ec2,   // start NOT2, start CAND2, end CAND2 
	 int tos,  // top of stack 
	 int orignode1[MAXM],
	 int orignode2[MAXN]);
  void extend (int stk[], // stack 
       bool connected[MAXM][MAXN],
       int clique1[], int cliqsize1,  // CLIQUE so far in NODES1 
       int clique2[], int cliqsize2,  // CLIQUE so far in NODES2 
       int sn1, int sc1, int ec1,   // start NOT1, start CAND1, end CAND1 
       int sn2, int sc2, int ec2,   // start NOT2, start CAND2, end CAND2 
       int tos,   // top of stack,   tos >= ec1, ec2  
       int orignode1[MAXM],   // original node numbers as input 
       int orignode2[MAXN]);
  void findfixpoint(int stk[], // stack 
	    bool connected[MAXM][MAXN],
	    int *savelist,      // position of savelist on the stack 
	    int *tmplist,       // position of tmplist on the stack 
	    int *minnod,        // currently lowest no. of disconnections 
	    int sninspect, int ecinspect,
	    int scother, int ecother,
	    bool binspect1,  // inspected nodes are in class1, o/w class2 
	    bool *bfound,  // a new lower no. of disconnections was found 
	    int *fixp,     // the new fixpoint, if *bfound = true  
	    int *posfix);    // position of fixpoint on the stack, if *bfound 

  void genincidence(int e,
		    Array<Edge> &edgelist,
		    int orignode1[MAXM],
		    int orignode2[MAXN],
		    bool connected[MAXM][MAXN],
		    int *m,
		    int *n);
  int getconnco(Array<int> &firstedge,
		Array<Edge> &edgelist);
  void outCLIQUE(int clique1[], int cliqsize1, 
		 int clique2[], int cliqsize2,
		 int orignode1[MAXM],
		 int orignode2[MAXN]);
  void workonco(int numco,
		Array<int> &firstedge,
		Array<Edge> &edgelist);

};

} // end namespace Gambit::Nash
} // end namespace Gambit

#endif // GAMBIT_ENUMMIXED_CLIQUE_H


