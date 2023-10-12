/*
** Call_Qhull.h 
**      Old code calling qhull to find a mixed subdivision 
**
**    copyright (c) 1995  Birk Huber
*/

#include "pelclqhl.h"
#include "pelqhull.h"

/*
** node pcfg_facets(node PC, Imatrix Controll)
**  Return a list of the inner-normals of the facets of a point
**   configuaration PC. Uses Control as input to pcfg_good
**   facets (see bellow) to screen for facets whoose normals
**   obey certain conditions.
*/

#define FORALLfacet_(facetlist) if (facetlist) for(facet=(facetlist);facet && facet->next;facet=facet->next)

node pcfg_facets(node PC, Imatrix Controll)
{
    int curlong, totlong, exitcode, i, j, k;
    int D, N;
    coordT *points, *make_cay(node, boolT *, node **);
    boolT ismalloc;
    facetT *facet;
    vertexT *vertex, **vertexp;
    node NormList = nullptr, *pt_table;
    Imatrix M, Norm, P0, P1;
    LOCS(2);


    PUSH_LOC(PC);
    PUSH_LOC(NormList);

    N = pcfg_npts(PC);
    D = pcfg_dim(PC);
    M = Imatrix_new(N, D);
    Norm = Ivector_new(D);
    M = pcfg_M(PC, M);


    j = Imatrix_rref(M, &i);

   

    if (j <= D - 1) {
	if (j == D - 1) {
	  /* printf("point config is D-1 dim\n"); */



	    Imatrix_backsolve(M, Norm);
            Imatrix_gcd_reduce(Norm);
	    /* printf("Imatrix norm : ");
	       Imatrix_print(Norm); 
	       printf("\n"); */
	    if (is_normal_good(Norm, Controll) != True) {
		for (i = 1; i <= D; i++)
		    *IVref(Norm, i) *= -1;
	    }
	    if (is_normal_good(Norm, Controll) == True)
		NormList = Cons(atom_new((char *) Norm, IMTX), nullptr);
	    else
		Imatrix_free(Norm);
	} else {
	  bad_error("point config is low dim in pcfg_facets");
	  printf("point config is low dim \n");
	  Imatrix_free(Norm);
	}
	Imatrix_free(M);
	POP_LOCS();
	/* printf("leaving qhull with low D:");
	node_print(NormList);
	printf("\n"); */
	return NormList;
    }
    qh_meminit(stderr);
    qh_initqhull_start(stdin, stdout, stderr);
    if (!(exitcode = setjmp(qh errexit))) {
	strcpy(qh qhull_command, "qhull i Qs Pg Pp");
	qh_initflags(qh qhull_command);
	points = make_cay(PC, &ismalloc, &pt_table);
	qh_initqhull_globals(points, N, D, ismalloc);
	qh_initqhull_mem();

	/* mem.c and set.c are initialized */
	qh_initqhull_buffers();
	/* setting up treshold to ensure that good facets will be those
	   on the lower hull with respecto to the last coordinate */

	qh_qhull();
	qh_findgood_all(qh facet_list);

	FORALLfacet_(qh facet_list) {
	    if (facet->good) {
		k = 0;
		FOREACHvertex_(facet->vertices) {
		    if (k == 0)
			P0 = pnt_coords(pt_table[qh_pointid(vertex->point)]);
		    else {
			P1 = pnt_coords(pt_table[qh_pointid(vertex->point)]);
			for (i = 1; i <= pcfg_dim(PC); i++) {
			    *IMref(M, k, i) = *IVref(P1, i) - *IVref(P0, i);
			}
		    }
		    k++;

		}
		Imatrix_submat(M, k - 1, pcfg_dim(PC));
		Imatrix_rref(M, &i);


		Imatrix_backsolve(M, Norm);
		for (i = 1; (*IVref(Norm, i) == 0) && i <= IVlength(Norm); i++);
		if ((facet->normal[i - 1] * (*IVref(Norm, i))) > 0.0) {
		    for (i = 1; i <= IVlength(Norm); i++) {
			*(IVref(Norm, i)) = -1 * (*IVref(Norm, i));
		    }
		}
		if (is_normal_good(Norm, Controll) == True) {
		    list_insert(atom_new((char *) Norm, IMTX),
				    &NormList, &(list_Imatrix_comp),FALSE);
		    Norm = Ivector_new(pcfg_dim(PC));
		}
	    }
	}

	exitcode = qh_ERRnone;
    }
    qh NOerrexit = True;	/* no more setjmp */
    qh_freeqhull(False);
    qh_memfreeshort(&curlong, &totlong);
    if (curlong || totlong)
#ifdef LOG_PRINT
	fprintf(stderr,
		"qhull internal warning (main): did not free %d bytes of long memory (%d pieces)\n",
		totlong, curlong)
#endif
;
    mem_free((char *) pt_table);
    Imatrix_free(M);
    Imatrix_free(Norm);
    POP_LOCS();
    return NormList;
}


coordT *make_cay(node PC, boolT * ismalloc, node ** table)
{
    int i, j;
    coordT *points, *coords;
    node ptr = PC, *lptr;

    coords = points = (coordT *) malloc(pcfg_npts(PC) *
					pcfg_dim(PC) * sizeof(coordT));
    *table = lptr = (node *) mem_malloc(pcfg_npts(PC) * sizeof(node));
    if (coords == nullptr) {
	*ismalloc = False;
	return nullptr;
    }
    *ismalloc = True;
    for (i = 1; i <= pcfg_npts(PC); i++) {
	ptr = Cdr(ptr);
	*(lptr++) = Car(ptr);
	for (j = 1; j <= pcfg_dim(PC); j++) {
	    *(coords++) = *(IVref(pnt_coords(Car(ptr)), j));
	}
    }
    return points;
}


node aset_cayley(node A,int addlift)
{
    node res = nullptr, ptr = A, ptp, ptc;
    int r = 0, R, D, i;
    char *lab;
    char *amm_string;
    Imatrix C;
    LOCS(2);
    PUSH_LOC(A);
    PUSH_LOC(res);
    D = aset_dim(A);
    if (addlift==1)addlift=1; else addlift=0;
    R = aset_r(A);
    ptr = aset_start_cfg(A);
    res = pcfg_new();
    while ((ptc = aset_next_cfg(&ptr)) != nullptr) {
        r++;
        while ((ptp = aset_next_pnt(&ptc)) != nullptr) {
            C = Ivector_new(D + R - 1 + addlift);
            if(addlift==1) *IVref(C,D+R)=0;
            for (i = 1; i < D; i++)
                *IVref(C, i) = aset_pnt_get(ptp, i);
            for (i = 1; i <= R - 1; i++)
                if (r - 1 == i)                               
                    *IVref(C, D + i-1) = 1;
                else
                    *IVref(C, D + i-1) = 0;
            *IVref(C,D+R-1)=aset_pnt_get(ptp,D);
            lab = (char *)mem_malloc(6 * sizeof(char));
	    amm_string = pnt_label(ptp);
	    /*            strncpy(lab, amm_string,6); - ERROR */
	    /* Therefore I fake it as follows - AMM */
	    for (i = 0; i <= 5; i++)
	      if (i < (int)strlen(amm_string))
		lab[i] = amm_string[i];
	      else
		lab[i] = '\0';
            pcfg_add(pnt_new(lab, C), res);
        }
    }
    POP_LOCS();
    return res;
}

node aset_lower_facets(node A)
{
    node res = nullptr, cay = nullptr;
    Imatrix Normfilter;
    int i;
    LOCS(3);
    PUSH_LOC(A);
    PUSH_LOC(res);
    PUSH_LOC(cay);
                                cay = aset_cayley(A,0);
    Normfilter = Ivector_new(aset_dim(A) + aset_r(A)-1);
    for (i = 1; i < aset_dim(A) + aset_r(A)-1; i++)
        *IVref(Normfilter, i) = 0;
    *IVref(Normfilter, aset_r(A)+aset_dim(A)-1) = 2;

    res = pcfg_facets(cay, Normfilter);

    cay = res;
    while (cay != nullptr) {
      *IVref((Imatrix) Car(Car(cay)),aset_dim(A))
      = *IVref((Imatrix) Car(Car(cay)),
               aset_dim(A)+aset_r(A)-1);
        Imatrix_resize((Imatrix) Car(Car(cay)), 1, aset_dim(A));
        cay = Cdr(cay);
    }
    Imatrix_free(Normfilter);
    POP_LOCS();
    return res;
}
                        
/* #define ACTUALLY_PRINT */
node aset_print_subdiv(node A, node norms, Imatrix T)
{
  //  int i;
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
#ifdef ACTUALLY_PRINT
    printf("\n");
#endif
    while (ptr != nullptr) {
        /*     Imatrix_gcd_reduce((Imatrix)Car((Imatrix)Car(ptr))); */
        ptc = aset_face(A, (Imatrix) Car((node) Car(ptr)));
#ifdef ACTUALLY_PRINT
	aset_print_short(ptc);
        printf("   ");
#endif
	Tp = aset_type(ptc, Tp);
#ifdef ACTUALLY_PRINT
	printf(" < %d",*IVref(Tp,1));
        for(i=2;i<=IVlength(Tp);i++) printf(", %d",*IVref(Tp,i));
        printf(" >  ");
        printf("   ");
#endif
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
#ifdef ACTUALLY_PRINT
   printf("   vol = %d", abs(v));
#endif
  if (t == 1)
                mv += abs(v);
        }
#ifdef ACTUALLY_PRINT
     printf("\n");
#endif
     ptr = Cdr(ptr);
    }
    if (T != nullptr)
#ifdef ACTUALLY_PRINT
        printf("MV =%d\n", mv);
#endif
   Imatrix_free(Tp);
    Imatrix_free(M);
    POP_LOCS();
    return res;

}

