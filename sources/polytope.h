/* // polytope.h // */

#include "glist.h"
#include "glpsolv1.h"
#include "gpoly.h"

#ifndef POLYTOPE_H
#define POLYTOPE_H

/*

   A _polytope_ is represented by a finite collection of _vertices_
in the integer lattice of a certain dimension.  The main computation in
this class is is_face, which determines, for any subset of the vertices, 
whether they span a face of the polytope.  In particular, we can
apply this to singletons, and since conceptually we are identifying the 
polytope with the convex hull of the vertices, nonextreme vertices are
redundant, and we use is_face to detect them.  The code is simplified by
the assumption that no nonextreme vertices are present, and each 
constructor must guarantee that it will not construct the first
polytope not satisfying this.

*/

struct index_pair {  // For recording the endpoints of an edge
    int head;
    int tail;
  };

template <class T> class gPolytope {

private:
  const gSpace* Space;
  gList<gVector<T> > vertices;

// Getting rid on nonextreme points
  bool is_extreme_point(const int vertex_no) const;
  void eliminate_nonextreme_vertices();

public:
    // Constructors and Destructor
  gPolytope(const gSpace* space);   // Empty set
  gPolytope(const gPolytope<T>& p);    // Copy
  gPolytope(const gSpace* space, const gList<gVector<T> >&);  // Conversion
  gPolytope(const gPolytope<T>& p, const gArray<bool>& selection);   // Subset 
  gPolytope(const gSpace*, const int D, const int N, gFileInput& data);
  virtual ~gPolytope();

  // Operators
  gPolytope&   operator =  (const gPolytope& p);
  bool         operator == (const gPolytope& p) const;
  bool         operator != (const gPolytope& p) const;
  gVector<T>   operator[]  (const int i)        const;

  // Information
  const gSpace*      AmbientSpace()                         const;
  bool               IsEmptySet()                           const;
  int                number_of_vertices()                   const;
  bool               is_face(const gArray<bool>& selection) const; // See below
  bool               is_edge(const index_pair&)             const;
  gList<index_pair*> index_pairs_of_edges()                 const;

  // Input-Output 
  void print_edge(const index_pair edge) const;
  void list_edges(const int no_edges, const index_pair* edges) const;
       // Before using, consider rewriting to use gList<index_pair>

//friend gFileInput& operator >> (gFileInput& input, gPolytope& p);
friend gOutput& operator << (gOutput& output, const gPolytope<T>& p);

};

//gFileInput& operator >> (gFileInput& input, gPolytope& p);
template <class T> 
gOutput& operator << (gOutput& output, const gPolytope<T>& p);

template <class T> void 
print_polytope_list(const gPolytope<T>* const poly_list, const int length);


/*                       How is_face Works

     We are given a polytope P, as a finite collection of vertices, and
a subset of these vertices, represented in the input by a number_of_vertices()
vector of {IN=1,OUT=0}'s.  The question we must answer is whether the
convex hull of the subset is a face of the polytope.
	Let a1,..,ak be the vertices of the putative face, and let
b1,..,bl be the other vertices of P.  Geometrically, we wish to know
whether there is a hyperplane containing a1,..,ak such that b1,..,bl
are contained in one of the two associated open half spaces.  Consider
the following linear program: choose c in [-1,1]^D and d in [-1, 1] to

	max  a1*c + d  subject to
		bj*c + d <= 0      (j = 1,..,l)
		(ai - a1)*c = 0    (i = 2,..,k)

(Here a1*c is the inner product of a1 and c, etc.)  Then a1,..,ak will
span a face of P precisely when the value of this problem is positive.
Note that it is always feasible to set c = 0, d = 0, so the value is
always nonnegative.
	Let e = (1,..,1) be the vector of ones in R^D.  In order to
bring this to the standard formulation of linear programming we
replace c with f = c + e: choose f in R_+^D  and g in R_+ to

	max  a1*f + g  subject to
		f_h <= 2                 (h = 1,..,D)
		g <= 2
		bj*f + g <= bj*e + 1       (j = 2,..,l)
		(ai - a1)*f = (ai - a1)*e  (i = 2,..,k)

We now wish to determine whether the value of this problem is greater
than a1*e + 1.  Ugly beyond question, but basically we're stuck with
it.  The code simply prepares this problem to be passed to the linear
programming routine in glpsolve1.h, which is of the form 

	min Cx  subject to  Ax <= B, Dx = E, x >= 0.

 */

#endif //# POLYTOPE_H
