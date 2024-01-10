//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enummixed/clique.cc
// Maximal cliques and solution components via von Stengel's algorithm
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

#include "clique.h"
#include "gambit.h"

namespace Gambit {
namespace Nash {

CliqueEnumerator::CliqueEnumerator(Array<Edge> &edgelist, int maxinp1, int maxinp2) 
  : firstedge(std::min(maxinp1,maxinp2)+1), maxinp1(maxinp1), maxinp2(maxinp2)
{
  int numco = getconnco(firstedge, edgelist);
  workonco(numco, firstedge, edgelist);
}

void CliqueEnumerator::
candtry1 (int stk[], // stack 
	  bool connected[MAXM][MAXN],
	  int cand,  // the candidate from NODES1  to be added to CLIQUE 
	  int clique1[], int cliqsize1,  // CLIQUE so far in NODES1 
	  int clique2[], int cliqsize2,  // CLIQUE so far in NODES2 
	  int sn1, int *sc1, int ec1,   // start NOT1, start CAND1, end CAND1 
	  int sn2, int sc2, int ec2,   // start NOT2, start CAND2, end CAND2 
	  int tos,  // top of stack 
	  int orignode1[MAXM],
	  int orignode2[MAXN]
	  )
  /* recurses down by moving  cand  from  CAND1  to  clique1  and
     then to NOT1  after extension.
     clique1  is extended by  cand  where all points in  NOT2 and CAND2 
     relate to  cand.
     pre:  cand  is in CAND1
     post: cand  is moved from  CAND1  to  NOT1
     CAND1 may be shuffled,  o/w stack unchanged 
  */
{
  int i, j, snnew, scnew, ecnew;
  
  clique1[cliqsize1++] = cand ;
  // remove  cand  from CAND1 by replacing it with the last element of CAND1 
  for (i=*sc1; i<ec1; i++)
      if (cand == stk[i]) {
         stk[i] = stk[--ec1] ;
         break ;
         }
  // stk[ec1] is free now but will after extension be needed again 
  // fill new sets NOT2, CAND2 
  snnew = tos ;
  for (j=sn2; j<sc2; j++) 
    if (connected[cand][stk[j]])
      stk[tos++] = stk[j] ;
  scnew = tos ;
  for (j=sc2; j<ec2; j++) 
    if (connected[cand][stk[j]])
      stk[tos++] = stk[j] ;
  ecnew = tos ;
  
  extend(stk, connected, clique1, cliqsize1, clique2, cliqsize2,
	 sn1, *sc1, ec1, snnew, scnew, ecnew, tos, orignode1, orignode2);
  
  /* remove  cand  from  clique1,
     put  cand  into  NOT1  by increasing  *sc1  and moving
     the node at position  *sc1  to the end of CAND1 */
  // The following lines modified since ec1 and cliqsize1 are
  // never used after the increments
  //cliqsize1-- ;
  //stk[ec1++] = stk[*sc1] ;
  stk[ec1] = stk[*sc1];
  stk[*sc1] = cand ;
  (*sc1)++ ;
}

// -------------------------------------------------- 
void CliqueEnumerator::
candtry2 (int stk[], // stack 
	  bool connected[MAXM][MAXN],
	  int cand,  // the candidate from NODES2  to be added to CLIQUE 
	  int clique1[], int cliqsize1,  // CLIQUE so far in NODES1 
	  int clique2[], int cliqsize2,  // CLIQUE so far in NODES2 
	  int sn1, int sc1, int ec1,   // start NOT1, start CAND1, end CAND1 
	  int sn2, int *sc2, int ec2,   // start NOT2, start CAND2, end CAND2 
	  int tos,  // top of stack 
	  int orignode1[MAXM],
	  int orignode2[MAXN]
	  )
  // recurses down by moving  cand  from  CAND2  to  clique2  and
  // then to NOT2  after extension;
  // clique2  is extended by  cand  where all points in  NOT1 and CAND1 
  // relate to  cand.
  // pre:  cand  is in CAND2
  // post: cand  is moved from  CAND2  to  NOT2
  // CAND2 may be shuffled,  o/w stack unchanged
  
{
  int i, j,  snnew, scnew, ecnew;
  
  clique2[cliqsize2++] = cand ;
  // remove  cand  from CAND2 by replacing it with the last element of CAND2 
  for (j=*sc2; j<ec2; j++)
      if (cand == stk[j]) {
	 stk[j] = stk[--ec2] ;
	 break ;
	 }
  // stk[ec2] is free now but will after extension be needed again 
  // fill new sets NOT1, CAND1 
  snnew = tos ;
  for (i=sn1; i<sc1; i++) 
    if (connected[stk[i]][cand])
      stk[tos++] = stk[i] ;
  scnew = tos ;
  for (i=sc1; i<ec1; i++) 
    if (connected[stk[i]][cand])
      stk[tos++] = stk[i] ;
  ecnew = tos ;
  
  extend(stk, connected, clique1, cliqsize1, clique2, cliqsize2,
	 snnew, scnew, ecnew, sn2, *sc2, ec2, tos, orignode1, orignode2);
  
  // remove  cand  from  clique2,
  // put  cand  into  NOT2  by increasing  *sc2  and moving
  // the node at position  sc2  to the end of CAND1 

  // The following lines modified since ec2 and cliqsize2 are not used
  // after the increments
  // cliqsize2-- ;
  // stk[ec2++] = stk[*sc2] ;
  stk[ec2] = stk[*sc2];
  stk[*sc2] = cand ;
  (*sc2)++ ;
}

// -------------------------------------------------- 
void CliqueEnumerator::
extend (int stk[], // stack 
	bool connected[MAXM][MAXN],
	int clique1[], int cliqsize1,  // CLIQUE so far in NODES1 
	int clique2[], int cliqsize2,  // CLIQUE so far in NODES2 
	int sn1, int sc1, int ec1,   // start NOT1, start CAND1, end CAND1 
	int sn2, int sc2, int ec2,   // start NOT2, start CAND2, end CAND2 
	int tos,   // top of stack,   tos >= ec1, ec2  
	int orignode1[MAXM],   // original node numbers as input 
	int orignode2[MAXN]
	)
  /* extends the current set CLIQUE or outputs it if
     NOT and CAND are empty.
     
     pre:  CLIQUE = clique1[0, cliqsize1], clique2[0, cliqsize2]
     NOT1 = stk[sn1, sc1],  CAND1= stk[sc1, ec1]   
     NOT2 = stk[sn2, sc2],  CAND2= stk[sc2, ec2]   
     sn1 <= sc1 <= ec1, sn2 <= sc2 <= ec2 
     all cliques extending  CLIQUE
     containing a node in  NOT1  or  NOT2  have already been generated
     post: output of all maximal cliques extending  CLIQUE  with
     candidates from  CAND1  or  CAND2  but not from NOT1, NOT2.
  */
{
  /* if no further extension is possible then  
     output the current CLIQUE if applicable, and return */
  
  if (sc1 == ec1 && sc2 == ec2) {
    //  CAND is empty  
    if (sn1 == sc1 && sn2 == sc2)
      //  NOT is empty, otherwise do nothing  
      outCLIQUE(clique1, cliqsize1, clique2, cliqsize2,
                orignode1, orignode2) ;
  }
  
  else {  //  CAND not empty 
    int cmax;  // maximal number of candidates on either side  
    int firstlist, savelist, tmplist, posfix ; // stack positions 
    int fixp;  // the fixpoint 
    int minnod ;
    bool bfound, bfixin1, bcandfix;
    
    cmax = std::max(ec1-sc1, ec2-sc2);  // the larger of |CAND1|, |CAND2|  
    
    // reserve two arrays of size cmax on the stack 
    firstlist = tmplist = tos;  tos += cmax;
    savelist = tos;  

    // tos is not used again.. 
    // tos += cmax;
    
    /* find fixpoint  fixp (a node of the graph) in  NOT  or  CAND
       which has the smallest possible number of disconnections  minnod 
       to CAND  */
    minnod = cmax + 1 ;
    
    // look for  fixp  in NODES1  
    findfixpoint(stk, connected, &savelist, &tmplist, &minnod,
		 sn1, ec1, sc2, ec2, true, &bfixin1, &fixp, &posfix) ;
    bcandfix = (posfix >= sc1);
    
    // look for  fixp  in nodes2  
    findfixpoint(stk, connected, &savelist, &tmplist, &minnod,
		 sn2, ec2, sc1, ec1, false, &bfound, &fixp, &posfix) ;
    
    if (bfound) {
      bfixin1 = false ;
      bcandfix = (posfix >= sc2);
    }
    
    /* now:  fixp     = the node that is the fixpoint,
       posfix   = its position on the stack,
       bfixin1  = fixp  is in NODES1
       bcandfix = fixp  is a candidate
       stk[savelist, +minnod] = nodes disconnected to fixp
       which are all either in CAND1 or in CAND2;
    */
    /*    top of stack can be reset to  savelist+minnod  where
	  if savelist  is the second of the two lists, recopy it
	  to avoid that stk[firstlist, +cmax] is wasted
    */
    if (savelist != firstlist) {int i;
    for (i=0; i < minnod; i++) 
      stk[firstlist + i] = stk[savelist + i];
    savelist = firstlist ;
    }
    tos = savelist + minnod;
    
    if (bfixin1) {int j;  // fixpoint in NODES1  
    if (bcandfix)      // fixpoint is a candidate 
      candtry1(stk, connected, fixp, 
	       clique1, cliqsize1, clique2, cliqsize2,
	       sn1, &sc1, ec1, sn2, sc2, ec2, tos, orignode1, orignode2);
    // fixpoint is now in NOT1, try all the nodes disconnected to it 
    for (j=0; j<minnod;  j++) 
      candtry2(stk, connected, stk[savelist+j],
	       clique1, cliqsize1, clique2, cliqsize2,
	       sn1, sc1, ec1, sn2, &sc2, ec2, tos, orignode1, orignode2);
    }
    else {int j;          // fixpoint in NODES2  
    if (bcandfix)      // fixpoint is a candidate 
      candtry2(stk, connected, fixp,
	       clique1, cliqsize1, clique2, cliqsize2,
	       sn1, sc1, ec1, sn2, &sc2, ec2, tos, orignode1, orignode2);
    // fixpoint is now in NOT2, try all the nodes disconnected to it 
    for (j=0; j<minnod;  j++) 
      candtry1(stk, connected, stk[savelist+j],
	       clique1, cliqsize1, clique2, cliqsize2,
	       sn1, &sc1, ec1, sn2, sc2, ec2, tos, orignode1, orignode2);
    }
  }  // end candidates not empty 
} 

// -------------------------------------------------- 
void CliqueEnumerator::
findfixpoint(int stk[], // stack 
	     bool connected[MAXM][MAXN],
	     int *savelist,      // position of savelist on the stack 
	     int *tmplist,       // position of tmplist on the stack 
	     // might be swapped afterwards 
	     int *minnod,        // currently lowest no. of disconnections 
	     int sninspect, int ecinspect, /* range of stack positions
					      containing the nodes inspected 
					      as possible fixpoints */
	     int scother, int ecother,
	     // range of corresponding candidates on the other side  
	     bool binspect1,  // inspected nodes are in class1, o/w class2 
	     bool *bfound,  // a new lower no. of disconnections was found 
	     int *fixp,     // the new fixpoint, if *bfound = true  
	     int *posfix    // position of fixpoint on the stack, if *bfound 
	     )
  /* pre:  enough space on stack for the two lists  savelist,  tmplist
     post: *minnod contains the new minimum no. of disconnections
     stk[*savelist, +*minnod] contains the candidates disconnected to
     the fixpoint
  */
{
  int i,j, p ;
  int count ;
  int tmp ;
  *bfound = false ;
  
  for (i=sninspect; i<ecinspect; i++) {
    p = stk[i] ;
    count = 0;
    /* count number of disconnections to  p,  
       building up stk[tmplist+count] containing the
       disconnected points */
    for (j=scother; (j<ecother) && (count < *minnod); j++) {
      int k = stk[j] ;
      if (!( binspect1 ? connected[p][k] : connected[k][p] )) {
         stk[(*tmplist) + count] = k ;
         count ++ ;
      }
    }  // end loop j, comparing to other side 
    // check if new minimum found, in that case update fixpoint 
    if (count < *minnod) {
      *fixp = p ;
      *posfix = i;
      *minnod = count ;
      // save tmplist by making it the new savelist 
      tmp = *savelist ;
      *savelist = *tmplist;
      *tmplist  = tmp ;
      *bfound = true ;
    }
  }  // end loop  i,  inspecting nodes for fixpoint  
}

// -------------------------------------------------- 
void CliqueEnumerator::genincidence(
			       int e,
			       Array<Edge> &edgelist,
			       int orignode1[MAXM],
			       int orignode2[MAXN],
			       bool connected[MAXM][MAXN],
			       int *m,
			       int *n
			       )
  /* generates the incidence matrix  connected from the edgelist
     starting with edgelist[e]
     pre:  all nodes in edgelist < maxinp1,2
     post: orignode1[0..*m) contains the original node1 numbers
     orignode2[0..*n) contains the original node2 numbers
     connected[][] == true if edge, o/w false
     *m == number of rows 
     *n == number of columns
     */
{
  Array<int> newnode1(0,maxinp1-1) ;
  Array<int> newnode2(0,maxinp2-1) ;
  int i,j, newi, newj ;
  
  // init newnode 
  for (i=0; i<maxinp1; i++)  newnode1[i] = -1;
  for (j=0; j<maxinp2; j++)  newnode2[j] = -1;
  
  /* init connected for test ; note different dimension 
     for (i=0; i<MAXM; i++)
     for (j=0; j<MAXN; j++)  connected[i][j] = 2;    
  */
  
  *m = *n = 0;
  
  while (e) { // process the edge list with edge index e 
    i= edgelist[e].node1;
    j= edgelist[e].node2;
    newi = newnode1[i] ;
    newj = newnode2[j] ;
    if (newi == -1) {
      if (*m >= MAXM) { // out of bounds for connected, reject 
	printf("Left bound %d for incidence matrix ", MAXM ) ;
	printf("reached, edge (%d, %d) rejected\n", i, j);
	goto KEEPGOING ;
      }
      else {
	int k;
	newi = (*m) ++ ;
	// init connected on the fly 
	for (k=0; k<MAXN; k++)  connected[newi][k] = false;
	newnode1[i] = newi ;
	orignode1[newi] = i ;
      }
    }
    if (newj == -1) {
      if (*n >= MAXN) { // out of bounds for connected, reject 
	printf("Right bound %d for incidence matrix ", MAXN ) ;
	printf("reached, edge (%d, %d) rejected\n", i, j);
	goto KEEPGOING ;
      }
      else {
	newj = (*n) ++ ;
	newnode2[j] = newj ;
	orignode2[newj] = j ;
      }
    }
    connected[newi][newj] = true ;
    
  KEEPGOING:
    e = edgelist[e].nextedge ;
  }
}

// -------------------------------------------------- 
int CliqueEnumerator::getconnco(Array<int> &firstedge,
			   Array<Edge> &edgelist
			   )
  
  /* reads edges of bipartite graph from input, puts them in disjoint
     lists of edges representing its connected components 
     pre:  nodes are nonzero integers < maxinp1,2
     other edges are rejected, and so are edges starting
     from the MAXEDGEth edge on and larger, each with a warning msg.
     post: return value == numco  (largest index of a connected component)
     where  numco < MAXCO,  and for   1 <= co <= numco:
     edgelist[co].firstedge == 0    if  co  is not a component
     == edgeindex  e  otherwise where  e > 0  and
     edgelist[e].node1, .node2[e] are endpoints of edge,
     edgelist[e].nextedge == next edgeindex of component,
     zero if  e  is index to the last edge
  */
{
  int numco, newedge ;
  Array<int> co1(0,maxinp1-1), co2(0,maxinp2-1);   // components of node1,2  
  int i, j;  // indices to left and right nodes 
  
  // initialize  component indices of left and right nodes 
  for (i=0; i<maxinp1; i++)
    co1[i] = 0;
  for (j=0; j<maxinp2; j++)
    co2[j] = 0;
  
  numco = 0;
  for(newedge = 1;newedge<=edgelist.Length();newedge++) {
    i = edgelist[newedge].node1;
    j = edgelist[newedge].node2;
    
    if (i < 0 || i>= maxinp1 || j<0 || j>=maxinp2)
      printf("Edge (%d, %d) not in admitted range (0..%d, 0..%d), rejected\n",
	     i,j, maxinp1-1, maxinp2-1) ;
    else { 
      // add edge (i,j) to current componentlist 
      int  ico, jco ;  // current components of i,j  
      ico = co1[i] ;
      jco = co2[j] ;
      
      if (ico == 0) {
	//  i  has not yet been in a component before  
	if (jco == 0) {
	  //  j  has not yet been in a component before  
	  //  add a new component  
	  numco ++;
	  co1[i] = co2[j] = numco ;
	  firstedge[numco] = newedge ;
	  edgelist[newedge].nextedge  = 0;
	}
	else { /* j  is already in a component: add  i  to j's
		  component, adding list elements in front */
	  co1[i] = jco ;
	  edgelist[newedge].nextedge  = firstedge[jco];
	  firstedge[jco] = newedge;
	}
      }
      else { // i  is already in a component 
	if (jco == 0) {
	  //  j  has not yet been in a component before  
	  //  add  j  to  i's component  
	  co2[j] = ico ;
	  edgelist[newedge].nextedge  = firstedge[ico];
	  firstedge[ico] = newedge;
	}
	else { // i  and  j  are already in components  
	  if (ico == jco) {
	    // i, j  in same component: just add the current edge 
	    edgelist[newedge].nextedge  = firstedge[ico];
	    firstedge[ico] = newedge;
	  }
	  else { /*  i  and  j  in different components:  
		     merge these by traversing the edgelists
		     and updating components of all incident nodes
		     (this is wasteful since only nodes need be
		     updated, not edges)  */
	    int e, newco, oldco ;
	    if (ico < jco) { newco = ico; oldco = jco; }
	    else           { newco = jco; oldco = ico; }
	    // insert the current edge 
	    edgelist[newedge].nextedge= firstedge[oldco] ;
	    e = newedge ; 
	    while (true) {
	      co1[edgelist[e].node1] = co2[edgelist[e].node2] = newco ;
	      if (edgelist[e].nextedge == 0) break;
	      e = edgelist[e].nextedge;
	    }
	    //  e  is now the last edge in the updated list  
	    edgelist[e].nextedge = firstedge[newco] ;
	    firstedge[newco] = newedge ;
	    /* oldco is unused now: reuse it if it was the 
	       last component, otherwise just leave empty */
	    if (oldco == numco) numco-- ;
	    firstedge[oldco] = 0;
	  }
	}
      }
    }
  }  // end scanning input 
  return numco;
}


// -------------------------------------------------- 
void CliqueEnumerator::outCLIQUE(int clique1[], int cliqsize1, 
			    int clique2[], int cliqsize2,
			    int orignode1[MAXM],
			    int orignode2[MAXN]
			    ) 
  // outputs  CLIQUE  using the original node numbers in  orignode
  
{
  if (cliqsize1>0 && cliqsize2>0) {
    int i;
    //printf("{") ;
    Array<int> cliq1;
    for (i=0; i<cliqsize1; i++) {
      cliq1.push_back(orignode1[clique1[i]]);
      //printf("%d", orignode1[clique1[i]]);
      //if (i<cliqsize1-1) printf(", "); 
    }
    m_cliques1.push_back(cliq1);

    //printf("}  x  {") ;
    //  the  "x"  in the output symbolizes the product set 

    Array<int> cliq2;
    for (i=0; i<cliqsize2; i++) {
      cliq2.push_back(orignode2[clique2[i]]);
      //printf("%d", orignode2[clique2[i]]);
      //if (i<cliqsize2-1) printf(", "); 
    }
    m_cliques2.push_back(cliq2);
    //    printf("}\n");
  }
}

// -------------------------------------------------- 
void CliqueEnumerator::workonco(int numco,
			   Array<int> &firstedge,
			   Array<Edge> &edgelist
			   )
  /* works on the edgelists as generated by  getconnco
     it processes each component by computing its maximal cliques 
     pre : firstedge[1..numco], if nonzero, points to a connected component
     in edgelist
     post: all components are processed
  */
{
  int orignode1[MAXM];
  int orignode2[MAXN];
  bool connected[MAXM][MAXN];
  int m; int n;   // graph dimensions 
  
  int stk[STKSIZE];  // stack 
  int tos;  // top of stack 
  int clique1[MAXM], clique2[MAXN];
  // CLIQUE for first and second node class  
  
  int co;
  int countco = 0;
  
  for (co=1; co <= numco; co++) 
    if (firstedge[co]) {
      // found a nonzero component list 
      countco ++ ;
      //printf("\nConnected component %d:\n", countco) ;
      
      genincidence(firstedge[co], edgelist,
		   orignode1, orignode2, connected, &m, &n);
      
      /* compute the cliques of the component via  extend;
	 initialize stack with the full sets of nodes
	 and empty sets CAND and NOT  */
      tos = 0;
      {
	int i ;
	for (i=0; i<m; i++)  stk[tos++] = i;   // CAND1 = NODES1 
	for (i=0; i<n; i++)  stk[tos++] = i;   // CAND2 = NODES2 
      }
      extend(stk, connected, clique1, 0, clique2, 0,
	     0, 0, m, m, m, m+n, tos, orignode1, orignode2);
    }
}

}  // end namespace Gambit::Nash
}  // end namespace Gambit

