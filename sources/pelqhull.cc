/* qhull.c - For the Gambit Project

   This file contains the implementation code taken from qhull and 
incorporated into the Gambit source code.  We have placed all qhull
code in qhull.h and qhull.c to avoid adding to the number of files
in Gambit, and, more importantly, to express the idea that, from our
point of view, this is a blackbox.  We are unlikely to be able to 
answer questions concerning this code, and those who wish to modify
it should consider beginning with the version distributed by the 
Geometry Center.
*/

#include "pelqhull.h" 

/*************************************************************************/
/******************* implementation code from mem.c **********************/
/*************************************************************************/

/* mem.c - memory management routines for qhull

   This is a standalone program.
   
   To initialize memory:

     qh_meminit (stderr);  /
    qh_meminitbuffers (qh IStracing, qh_MEMalign, 7, qh_MEMbufsize,qh_MEMinitbuf);
    qh_memsize(sizeof(facetT));
    qh_memsize(sizeof(facetT));
    ...
    qh_memsetup();
    
   To free up all memory buffers:
    qh_memfreeshort (&curlong, &totlong);
         
   
   uses Quickfit algorithm (freelists for commonly allocated sizes)
   assumes small sizes for freelists (it discards the tail of memory buffers)
   
   see README and mem.h
   see global.c (qh_initbuffers) for an example of using mem.c 
   
   copyright (c) 1993-1994 The Geometry Center
*/

/* ============ -global data structure ==============
    see mem.h for definition
*/

qhmemT qhmem= {0};     /* remove "= {0}" if this causes a compiler error */

/* internal functions */
  
static int qh_intcompare(const void *i, const void *j);

/*========== functions in alphabetical order ======== */

/*-------------------------------------------------
-intcompare- used by qsort and bsearch to compare two integers
*/
static int qh_intcompare(const void *i, const void *j) {
  return(*((int *)i) - *((int *)j));
} /* intcompare */


/*-------------------------------------------------
-memalloc- allocates memory for object from qhmem
returns:
 pointer to allocated memory (errors if insufficient memory)
 outsize= actual size allocated, may be NULL
notes:
  use qh_memalloc_() for inline code for quick allocations
*/
void *qh_memalloc(int insize) {
  void **freelistp, *newbuffer;
  int index, size;
  int outsize, bufsize;
  void *object;

  if ((unsigned) insize <= (unsigned) qhmem.LASTsize) {
    index= qhmem.indextable[insize];
    freelistp= qhmem.freelists+index;
    if ((object= *freelistp)) {
      qhmem.cntquick++;  
      *freelistp= *((void **)*freelistp);  /* replace freelist with next object */
      return (object);
    }else {
      outsize= qhmem.sizetable[index];
      qhmem.cntshort++;
      if (outsize > qhmem .freesize) {
	if (!qhmem.curbuffer)
	  bufsize= qhmem.BUFinit;
        else
	  bufsize= qhmem.BUFsize;
        qhmem.totshort += bufsize;

	if (!(newbuffer= malloc(bufsize))) qhull_fatal(1);

	*((void **)newbuffer)= qhmem.curbuffer;  /* prepend newbuffer to curbuffer 
						    list */
	qhmem.curbuffer= newbuffer;
        size= (sizeof(void **) + qhmem.ALIGNmask) & ~qhmem.ALIGNmask;
	qhmem.freemem= (void *)((char *)newbuffer+size);
	qhmem.freesize= bufsize - size;
      }
      object= qhmem.freemem;
      qhmem.freemem= (void *)((char *)qhmem.freemem + outsize);
      qhmem.freesize -= outsize;
      return object;
    }
  }else {                     /* long allocation */

    if (!qhmem.indextable) qhull_fatal(2);

    outsize= insize;
    qhmem .cntlong++;
    qhmem .curlong++;
    qhmem .totlong += outsize;
    if (qhmem.maxlong < qhmem.totlong)
      qhmem.maxlong= qhmem.totlong;

    if (!(object= malloc(outsize))) qhull_fatal(3);

    if (qhmem.IStracing >= 5)
      fprintf (qhmem.ferr, "qh_memalloc long: %d bytes at %x\n", outsize, (int)object);
  }
  return (object);
} /* memalloc */


/*-------------------------------------------------
-memfree- frees memory object (may be NULL)
  size is either insize or outsize from qh_memalloc
  type checking warns if using (void **)object
  qh_memfree_()- in-line code for quick free's
*/
void qh_memfree(void *object, int size) {
  void **freelistp;

  if (!object)
    return;
  if (size <= qhmem.LASTsize) {
    qhmem .freeshort++;
    freelistp= qhmem.freelists + qhmem.indextable[size];
    *((void **)object)= *freelistp;
    *freelistp= object;
  }else {
    qhmem .freelong++;
    qhmem .totlong -= size;
    free (object);
    if (qhmem.IStracing >= 5)
      fprintf (qhmem.ferr, "qh_memfree long: %d bytes at %x\n", size, (int)object);
  }
} /* memfree */


/*-------------------------------------------------
-memfreeshort- frees up all short and qhmem memory allocations
returns: number and size of current long allocations
*/
void qh_memfreeshort (int *curlong, int *totlong) {
  void *buffer, *nextbuffer;

  *curlong= qhmem .cntlong - qhmem .freelong;
  *totlong= qhmem .totlong;
  for(buffer= qhmem.curbuffer; buffer; buffer= nextbuffer) {
    nextbuffer= *((void **) buffer);
    free(buffer);
  }
  qhmem.curbuffer= NULL;
  if (qhmem .LASTsize) {
    free (qhmem .indextable);
    free (qhmem .freelists);
    free (qhmem .sizetable);
  }
  memset((char *)&qhmem, 0, sizeof qhmem);  /* every field is 0, FALSE, NULL */
} /* memfreeshort */


/*-------------------------------------------------
-meminit- initialize memory (memalloc errors until memsetup)
*/
void qh_meminit (FILE *ferr) {
  
  memset((char *)&qhmem, 0, sizeof qhmem);  /* every field is 0, FALSE, NULL */
  qhmem.ferr= ferr;

#ifndef __BCC55__
  // This condition is always false under BCC55
  if (sizeof(void*) < sizeof(int)) qhull_fatal(4);
#endif  // __BCC55__
} /* meminit */

/*-------------------------------------------------
-meminitbuffers- initialize memory buffers
*/
void qh_meminitbuffers (int tracelevel, int alignment, int numsizes, int bufsize, int bufinit) {

  qhmem.IStracing= tracelevel;
  qhmem.NUMsizes= numsizes;
  qhmem.BUFsize= bufsize;
  qhmem.BUFinit= bufinit;
  qhmem.ALIGNmask= alignment-1;

  if (qhmem.ALIGNmask & ~qhmem.ALIGNmask) qhull_fatal(5);

  qhmem.sizetable= (int *) calloc (numsizes, sizeof(int));
  qhmem.freelists= (void **) calloc (numsizes, sizeof(void *));

  if (!qhmem.sizetable || !qhmem.freelists) qhull_fatal(6);

  if (qhmem.IStracing >= 1)
    fprintf (qhmem.ferr, "qh_meminitbuffers: memory initialized with alignment %d\n", alignment);
} /* meminitbuffers */

/*-------------------------------------------------
-memsetup- set up memory after running memsize()
*/
void qh_memsetup (void) {
  int k,i;

  qsort(qhmem.sizetable, qhmem.TABLEsize, sizeof(int), qh_intcompare);
  qhmem.LASTsize= qhmem.sizetable[qhmem.TABLEsize-1];

  if (qhmem .LASTsize >= qhmem .BUFsize || qhmem.LASTsize >= qhmem .BUFinit) 
    qhull_fatal(7);

  if (!(qhmem.indextable= (int *)malloc((qhmem.LASTsize+1) * sizeof(int)))) 
    qhull_fatal(8);

  for(k=qhmem.LASTsize+1; k--; )
    qhmem.indextable[k]= k;
  i= 0;
  for(k= 0; k <= qhmem.LASTsize; k++) {
    if (qhmem.indextable[k] <= qhmem.sizetable[i])
      qhmem.indextable[k]= i;
    else
      qhmem.indextable[k]= ++i;
  }
} /* memsetup */

/*-------------------------------------------------
-memsize- define a free list for this size
*/
void qh_memsize(int size) {
  int k;

  if (qhmem .LASTsize) qhull_fatal(9);

  size= (size + qhmem.ALIGNmask) & ~qhmem.ALIGNmask;
  for(k= qhmem.TABLEsize; k--; ) {
    if (qhmem.sizetable[k] == size)
      return;
  }
  if (qhmem.TABLEsize < qhmem.NUMsizes)
    qhmem.sizetable[qhmem.TABLEsize++]= size;
  else
    fprintf(qhmem.ferr, "qhull warning (memsize): free list table has room for only %d sizes\n", qhmem.NUMsizes);
} /* memsize */


/*-------------------------------------------------
-memstatistics-  print out memory statistics
*/
void qh_memstatistics (FILE *fp) {
  int i, count;
  void *object;
  
  fprintf (fp, "\nmemory statistics:\n\
%7d quick allocations\n\
%7d short allocations\n\
%7d long allocations\n\
%7d short frees\n\
%7d long frees\n\
%7d bytes of short memory in use or on freelists\n\
%7d bytes of long memory allocated (except for input)\n\
%7d bytes of long memory in use (in %d pieces)\n\
%7d bytes per memory buffer (initially %d bytes)\n",
	   qhmem .cntquick, qhmem.cntshort, qhmem.cntlong,
	   qhmem .freeshort, qhmem.freelong, 
	   qhmem .totshort - qhmem .freesize,
	   qhmem .maxlong, qhmem .totlong, qhmem .cntlong - qhmem .freelong,
	   qhmem .BUFsize, qhmem .BUFinit);
  if (qhmem.cntlarger) {
    fprintf (fp, "%7d calls to qh_setlarger\n%7.2g     average copy size\n",
	   qhmem.cntlarger, ((float) qhmem.totlarger)/ qhmem.cntlarger);
    fprintf (fp, "  freelists (bytes->count):");
  }
  for (i=0; i<qhmem.TABLEsize; i++) {
    count=0;
    for (object= qhmem .freelists[i]; object; object= *((void **)object))
      count++;
    fprintf (fp, " %d->%d", qhmem.sizetable[i], count);
  }
  fprintf (fp, "\n\n");
} /* memstatistics */


/*************************************************************************/
/******************* implementation code from set.c **********************/
/*************************************************************************/

/* set.c -- implements set manipulations needed for quickhull 

   see README and set.h
   
   copyright (c) 1993-1994 The Geometry Center        
*/

/*----------- internal macros -------------------
-SETsizeaddr_(set) - return pointer to actual size+1 of set (set CANNOT be NULL!!)
    *SETsizeaddr==NULL or e[*SETsizeaddr-1]==NULL
*/
#define SETsizeaddr_(set) ((int *)(&((set)->e[(set)->maxsize])))

/*============ functions in alphabetical order ===================*/
  
/*----------------------------------------
-setaddnth- adds newelem as n'th element of sorted or unsorted set
  setp and newelem must be defined
  set may be a temp set
  nth=0 is first element
  errors if nth is out of bounds
*/
void qh_setaddnth(setT **setp, int nth, void *newelem) {
  int *sizep, oldsize, i;
  void **oldp, **newp;

  if (!*setp || !*(sizep= SETsizeaddr_(*setp))) {
    qh_setlarger(setp);
    sizep= SETsizeaddr_(*setp);
  }
  oldsize= *sizep - 1;
  if (nth < 0 || nth > oldsize) qhull_fatal(10);

  (*sizep)++;
  oldp= SETelemaddr_(*setp, oldsize, void);   /* NULL */
  newp= oldp+1;
  for (i= oldsize-nth+1; i--; )  /* move at least NULL  */
    *(newp--)= *(oldp--);       /* may overwrite *sizep */
  *newp= newelem;
} /* setaddnth */


/*----------------------------------------
-setaddsorted- adds an element to a sorted set
  setp and newelem must be defined
  set may be a temp set
  nop if newelem already in set
*/
void qh_setaddsorted(setT **setp, void *newelem) {
  int newindex=0;
  void *elem, **elemp;

  FOREACHelem_(*setp) {          /* could use binary search instead */
    if (elem < newelem)
      newindex++;
    else if (elem == newelem)
      return;
    else
      break;
  }
  qh_setaddnth(setp, newindex, newelem);
} /* setaddsorted */


/*----------------------------------------
-setappend- appends an element to a set
  set may be a temp set
  *setp and newelem may be NULL
*/
void qh_setappend(setT **setp, void *newelem) {
  int *sizep;
  void **endp;

  if (!newelem)
    return;
  if (!*setp || !*(sizep= SETsizeaddr_(*setp))) {
    qh_setlarger(setp);
    sizep= SETsizeaddr_(*setp);
  }
  *(endp= &((*setp)->e[(*sizep)++ - 1]))= newelem;
  *(++endp)= NULL;
} /* setappend */

/*----------------------------------------
-setappend_set- appends a set to a set
  *setp and set may be NULL
  setp can not be a temp set
*/
void qh_setappend_set(setT **setp, setT *setA) {
  int *sizep, sizeA, size;
  setT *oldset;

  if (!setA)
    return;
  SETreturnsize_(setA, sizeA);
  if (!*setp)
    *setp= qh_setnew (sizeA);
  sizep= SETsizeaddr_(*setp);
  if (!(size= *sizep))
    size= (*setp)->maxsize;
  else
    size--;
  if (size + sizeA >(int) (*setp)->maxsize) {
    oldset= *setp;
    *setp= qh_setcopy (oldset, sizeA);
    qh_setfree (&oldset);
    sizep= SETsizeaddr_(*setp);
  }
  *sizep= size+sizeA+1;   /* memcpy may overwrite */
  if (sizeA > 0) 
    memcpy((char *)&((*setp)->e[size]), (char *)&(setA->e[0]), SETelemsize *(sizeA+1));
} /* setappend_set */


/*----------------------------------------
-setappend2ndlast- makes newelem the next to the last element in set
  set must have at least one element, newelem must be defined
  set may be a temp set
*/
void qh_setappend2ndlast(setT **setp, void *newelem) {
  int *sizep;
  void **endp, **lastp;
  
  if (!*setp || !*(sizep= SETsizeaddr_(*setp))) {
    qh_setlarger(setp);
    sizep= SETsizeaddr_(*setp);
  }
  endp= SETelemaddr_(*setp, (*sizep)++ -1, void); /* NULL */
  lastp= endp-1;
  *(endp++)= *lastp;
  *endp= NULL;    /* may overwrite *sizep */
  *lastp= newelem;
} /* setappend2ndlast */


/*----------------------------------------
-setcheck- check set for validity
*/
void qh_setcheck(setT *set, char *typenameNEW, int id) {
  int maxsize, size;
  int waserr= 0;

  if (!set)
    return;
  SETreturnsize_(set, size);
  maxsize= set->maxsize;
  if (size > maxsize || !maxsize) {
    fprintf (qhmem.ferr, "qhull internal error (setcheck): actual size %d of %s%d is greater than max size %d\n",
	     size, typenameNEW, id, maxsize);
    waserr= 1;
  }else if (set->e[size]) {
    fprintf (qhmem.ferr, "qhull internal error (setcheck): %s%d (size %d max %d) is not null terminated.\n",
	     typenameNEW, id, maxsize, size-1);
    waserr= 1;
  }
  if (waserr) qhull_fatal(11);

} /* setcheck */


/*----------------------------------------
-setcopy- copies a sorted or unsorted set into another
returns:
  new set is actual size of old set plus extra
*/
setT *qh_setcopy(setT *set, int extra) {
  setT *newset;
  int size;

  if (extra < 0)
    extra= 0;
  SETreturnsize_(set, size);
  newset= qh_setnew(size+extra);
  *SETsizeaddr_(newset)= size+1;    /* memcpy may overwrite */
  memcpy((char *)&(newset->e[0]), (char *)&(set->e[0]), SETelemsize *(size+1));
  return (newset);
} /* setcopy */


/*----------------------------------------
-setdel- deletes oldelem from unsorted set.
   if found, overwrites newlelem with lastelem
   set may be NULL, oldelem must not be NULL;
returns:
  returns oldelem if it was deleted
*/
void *qh_setdel(setT *set, void *oldelem) {
  void **elemp, **lastp;
  int *sizep;

  if (!set)
    return NULL;
  elemp= SETaddr_(set, void);
  while (*elemp != oldelem && *elemp)
    elemp++;
  if (*elemp) {
    sizep= SETsizeaddr_(set);
    if (!(*sizep)--)         /*  if was a full set */
      *sizep= set->maxsize;  /*     *sizep= (maxsize-1)+ 1 */
    lastp= SETelemaddr_(set, *sizep-1, void);
    *elemp= *lastp;      /* may overwrite itself */
    *lastp= NULL;
    return oldelem;
  }
  return NULL;
} /* setdel */


/*----------------------------------------
-setdellast- return last element of set or NULL
   delete element from set
   set may be NULL
*/
void *qh_setdellast(setT *set) {
  int setsize;
  void **last;
  void *returnvalue;
  
  if (!set || !(set->e[0]))
    return NULL;
  if ((setsize= (int)*(last= &(set->e[set->maxsize])))) {
    returnvalue= set->e[setsize - 2];
    set->e[setsize - 2]= NULL;
    *last= (void *)((int)*last - 1);
  }else {
    returnvalue= set->e[set->maxsize - 1];
    set->e[set->maxsize - 1]= NULL;
    *last= (void *)(set->maxsize);
  }
  return returnvalue;
} /* setdellast */


/*----------------------------------------
-setdelnth- deletes nth element from unsorted set 
  errors if nth invalid
  returns the element
*/
void *qh_setdelnth(setT *set, int nth) {
  void **elemp, **lastp, *elem;
  int *sizep;


  elemp= SETelemaddr_(set, nth, void);
  sizep= SETsizeaddr_(set);
  if (!(*sizep)--)         /*  if was a full set */
    *sizep= set->maxsize;  /*     *sizep= (maxsize-1)+ 1 */

  if (nth < 0 || nth >= *sizep) qhull_fatal(12);

  lastp= SETelemaddr_(set, *sizep-1, void);
  elem= *elemp;
  *elemp= *lastp;      /* may overwrite itself */
  *lastp= NULL;
  return elem;
} /* setdelnth */

/*----------------------------------------
-setdelnthsorted- deletes nth element from sorted set
  sort order is undefined
  errors if nth invalid
  see also: setnew_delnthsorted
*/
void *qh_setdelnthsorted(setT *set, int nth) {
  void **newp, **oldp, *elem;
  int *sizep;

  sizep= SETsizeaddr_(set);

  if ( nth < 0 || 
      (*sizep && nth >= *sizep-1) || 
      nth >= (int)set->maxsize ) 
    qhull_fatal(13);

  newp= SETelemaddr_(set, nth, void);
  elem= *newp;
  oldp= newp+1;
  while ((*(newp++)= *(oldp++)))
    ; /* copy remaining elements and NULL */
  if (!(*sizep)--)         /*  if was a full set */
    *sizep= set->maxsize;  /*     *sizep= (max size-1)+ 1 */
  return elem;
} /* setdelnthsorted */


/*----------------------------------------
-setdelsorted- deletes oldelem from sorted set
  sort order is undefined
  set may be NULL
  returns oldelem if it was deleted
*/
void *qh_setdelsorted(setT *set, void *oldelem) {
  void **newp, **oldp;
  int *sizep;

  if (!set)
    return NULL;
  newp= SETaddr_(set, void);
  while(*newp != oldelem && *newp)
    newp++;
  if (*newp) {
    oldp= newp+1;
    while ((*(newp++)= *(oldp++)))
      ; /* copy remaining elements */
    sizep= SETsizeaddr_(set);
    if (!(*sizep)--)    /*  if was a full set */
      *sizep= set->maxsize;  /*     *sizep= (max size-1)+ 1 */
    return oldelem;
  }
  return NULL;
} /* setdelsorted */


/*----------------------------------------
-setequal- returns 1 if two sorted sets are equal, otherwise returns 0
    either set may be NULL
*/
int qh_setequal(setT *setA, setT *setB) {
  void **elemAp, **elemBp;
  int sizeA, sizeB;
  
  SETreturnsize_(setA, sizeA);
  SETreturnsize_(setB, sizeB);
  if (sizeA != sizeB)
    return 0;
  if (!sizeA)
    return 1;
  elemAp= SETaddr_(setA, void);
  elemBp= SETaddr_(setB, void);
  if (!memcmp((char *)elemAp, (char *)elemBp, sizeA*SETelemsize))
    return 1;
  return 0;
} /* setequal */


/*----------------------------------------
-setequal_except- returns 1 if two sorted sets are equal except for 2 elements
  neither set may be NULL
  false if either skip is missing
  if second skip is NULL, 
     can skip any one element
*/
int qh_setequal_except (setT *setA, void *skipelemA, setT *setB, void *skipelemB) {
  void **elemA, **elemB;
  int skip=0;

  elemA= SETaddr_(setA, void);
  elemB= SETaddr_(setB, void);
  while (1) {
    if (*elemA == skipelemA) {
      skip++;
      elemA++;
    }
    if (skipelemB) {
      if (*elemB == skipelemB) {
        skip++;
        elemB++;
      }
    }else if (*elemA != *elemB) {
      skip++;
      if (!(skipelemB= *elemB++))
        return 0;
    }
    if (!*elemA)
      break;
    if (*elemA++ != *elemB++) 
      return 0;
  }
  if (skip != 2 || *elemB)
    return 0;
  return 1;
} /* setequal_except */
  

/*----------------------------------------
-setequal_skip- returns 1 if two sorted sets are equal except for skips
  neither set may be NULL
  false if different size
*/
int qh_setequal_skip (setT *setA, int skipA, setT *setB, int skipB) {
  void **elemA, **elemB, **skipAp, **skipBp;

  elemA= SETaddr_(setA, void);
  elemB= SETaddr_(setB, void);
  skipAp= SETelemaddr_(setA, skipA, void);
  skipBp= SETelemaddr_(setB, skipB, void);
  while (1) {
    if (elemA == skipAp)
      elemA++;
    if (elemB == skipBp)
      elemB++;
    if (!*elemA)
      break;
    if (*elemA++ != *elemB++) 
      return 0;
  }
  if (*elemB)
    return 0;
  return 1;
} /* setequal_skip */
  

/*----------------------------------------
-setfree- frees the space occupied by a sorted or unsorted set
  set may be NULL
*/
void qh_setfree(setT **setp) {
  int size;
  void **freelistp;
  
  if (*setp) {
    size= sizeof(setT) + ((*setp)->maxsize)*SETelemsize; 
    if (size <= qhmem.LASTsize) {
      qh_memfree_(*setp, size, freelistp);
    }else
      qh_memfree (*setp, size);
    *setp= NULL;
  }
} /* setfree */


/*----------------------------------------
-setfreelong- frees a set only if it's in long memory
  set may be NULL
*/
void qh_setfreelong(setT **setp) {
  int size;
  
  if (*setp) {
    size= sizeof(setT) + ((*setp)->maxsize)*SETelemsize; 
    if (size > qhmem.LASTsize) {
      qh_memfree (*setp, size);
      *setp= NULL;
    }
  }
} /* setfreelong */


/*----------------------------------------
-setin- returns 1 if setelem is in a set, 0 otherwise
  set may be NULL or unsorted
*/
int qh_setin(setT *set, void *setelem) {
  void *elem, **elemp;

  FOREACHelem_(set) {
    if (elem == setelem)
      return 1;
  }
  return 0;
} /* setin */


/*----------------------------------------
-setindex- returns the index of elem in set.   If none, returns -1
  set may be NULL and may contain nulls.
*/
int qh_setindex(setT *set, void *atelem) {
  void **elem;
  int size, i;

  SETreturnsize_(set, size);
  if (size > (int)set->maxsize)
    return -1;
  elem= SETaddr_(set, void);
  for (i=0; i<size; i++) {
    if (*elem++ == atelem)
      return i;
  }
  return -1;
} /* setindex */


/*----------------------------------------
-setlarger- returns a larger set that contains elements of *setp
  the set is at least twice as large
  updates qhmem.tempstack if needed
*/
void qh_setlarger(setT **oldsetp) {
  int size= 1, *sizep;
  setT *newset, *set, **setp, *oldset;
  void **oldp, **newp;

  if (*oldsetp) {
    oldset= *oldsetp;
    SETreturnsize_(oldset, size);
    qhmem.cntlarger++;
    qhmem.totlarger += size+1;
    newset= qh_setnew(2 * size);
    oldp= SETaddr_(oldset, void);
    newp= SETaddr_(newset, void);
    memcpy((char *)newp, (char *)oldp, (size+1) * SETelemsize);
    sizep= SETsizeaddr_(newset);
    *sizep= size+1;
    FOREACHset_((setT *)qhmem.tempstack) {
      if (set == oldset)
	*(setp-1)= newset;
    }
    qh_setfree(oldsetp);
  }else 
    newset= qh_setnew(3);
  *oldsetp= newset;
} /* setlarger */


/*----------------------------------------
-setlast- return last element of set or NULL
   set may be NULL
*/
void *qh_setlast(setT *set) {
  int size;

  if (set) {
    size= *SETsizeaddr_(set);
    if (!size) 
      return SETelem_(set, set->maxsize - 1);
    else if (size > 1)
      return SETelem_(set, size - 2);
  }
  return NULL;
} /* setlast */


/*----------------------------------------
-setnew- creates and allocates space for a set
    setsize means the number of elements (NOT including the NULL terminator)
    use qh_settemp/qh_setfreetemp if set is temporary
*/
setT *qh_setnew(int setsize) {
  setT *set;
  int sizereceived, size;
  void **freelistp;

  if (!setsize)
    setsize++;
  size= sizeof(setT) + setsize * SETelemsize;
  if ((unsigned) size <= (unsigned) qhmem.LASTsize) {
    qh_memalloc_(size, freelistp, set);
    sizereceived= qhmem.sizetable[ qhmem.indextable[size]];
    if (sizereceived > size) 
      setsize += (sizereceived - size)/SETelemsize;
  }else
    set= (setT *)qh_memalloc ((int)size);
  set->maxsize= setsize;
  set->e[setsize]= (void *) 1;
  set->e[0]= NULL;
  return (set);
} /* setnew */


/*----------------------------------------
-setnew_delnthsorted- creates a sorted set not containing nth element
  the new set may have prepended undefined entries
  set must be defined
  checks nth
  see also: setdelnthsorted
*/
setT *qh_setnew_delnthsorted(setT *set, int size, int nth, int prepend) {
  setT *newset;
  void **oldp, **newp;
  int tailsize= size - nth -1, newsize;

  if (tailsize < 0) qhull_fatal(14);

  newsize= size-1 + prepend;
  newset= qh_setnew(newsize);
  newset->e[newset->maxsize]= (void *)(newsize+1);  /* may be overwritten */
  oldp= SETaddr_(set, void);
  newp= SETaddr_(newset, void) + prepend;
  switch (nth) {
  case 0:
    break;
  case 1:
    *(newp++)= *oldp++;
    break;
  case 2:
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    break;
  case 3:
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    break;
  case 4:
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    break;
  default:
    memcpy((char *)newp, (char *)oldp, nth * SETelemsize);
    newp += nth;
    oldp += nth;
    break;
  }
  oldp++;
  switch (tailsize) {
  case 0:
    break;
  case 1:
    *(newp++)= *oldp++;
    break;
  case 2:
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    break;
  case 3:
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    break;
  case 4:
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    *(newp++)= *oldp++;
    break;
  default:
    memcpy((char *)newp, (char *)oldp, tailsize * SETelemsize);
    newp += tailsize;
  }
  *newp= NULL;
  return(newset);
} /* setnew_delnthsorted */


/*----------------------------------------
-setprint- print set elements to fp
notes:
  never errors
*/
void qh_setprint(FILE *fp, char* string, setT *set) {
  int size, k;

  if (!set)
    fprintf (fp, "%s set is null\n", string);
  else {
    SETreturnsize_(set, size);
    fprintf (fp, "%s set=%x maxsize=%d size=%d elems=",
	     string, (unsigned int)set, set->maxsize, size);
    if (size > (int)set->maxsize)
      size= set->maxsize+1;
    for (k=0; k<size; k++)
      fprintf(fp, " %x", (unsigned int)(set->e[k]));
    fprintf(fp, "\n");
  }
} /* setprint */

/*----------------------------------------
-setreplace- replaces oldelem in set with newelem
   errors if oldelem not in the set
   if newelem is NULL then FOREACH no longer works
*/
void qh_setreplace(setT *set, void *oldelem, void *newelem) {
  void **elemp;
  
  elemp= SETaddr_(set, void);
  while(*elemp != oldelem && *elemp)
    elemp++;
  if (*elemp)
    *elemp= newelem;

  else qhull_fatal(15);

} /* setreplace */


/*----------------------------------------
-setsize- returns the size of a set
  same as SETreturnsize_(set)
*/
int qh_setsize(setT *set) {
  int size, *sizep;
  
  if (!set)
    return (0);
  sizep= SETsizeaddr_(set);
  if ((size= *sizep)) {
    size--;

    if (size > (int)set->maxsize) qhull_fatal(16);

  }else
    size= set->maxsize;
  return size;
} /* setsize */

/*----------------------------------------
-settemp- return a stacked, temporary set
  use settempfree or settempfree_all to release from qhmem.tempstack
  see also qh_setnew
*/
setT *qh_settemp(int setsize) {
  setT *newset;
  
  newset= qh_setnew (setsize);
  qh_setappend ((setT **)&qhmem.tempstack, newset);
  if (qhmem.IStracing >= 5)
    fprintf (qhmem.ferr, "qh_settemp: temp set %x of %d elements, depth %d\n",
       (int)newset, newset->maxsize, qh_setsize ((setT *)qhmem.tempstack));
  return newset;
} /* settemp */

/*----------------------------------------
-settempfree- free temporary set at top of qhmem.tempstack
  nop if NULL
  errors if set not from previous qh_settemp
    locate source by T2 and find mis-matching qh_settemp
*/
void qh_settempfree(setT **set) {
  setT *stackedset;

  if (!*set)
    return;
  stackedset= qh_settemppop ();

  if (stackedset != *set) qhull_fatal(17);

  qh_setfree (set);
} /* settempfree */

/*----------------------------------------
-settempfree_all- free all temporary sets in qhmem.tempstack
*/
void qh_settempfree_all(void) {
  setT *set, **setp;

  FOREACHset_((setT *)qhmem.tempstack) 
    qh_setfree(&set);
  qh_setfree((setT **)&qhmem.tempstack);
} /* settempfree_all */

/*----------------------------------------
-settemppop- pop and return temporary set from qhmem.tempstack (makes it permanent)
*/
setT *qh_settemppop(void) {
  setT *stackedset;
  
  stackedset= (setT *)qh_setdellast((setT *)qhmem.tempstack);

  if (!stackedset) qhull_fatal(18);

  if (qhmem.IStracing >= 5)
    fprintf (qhmem.ferr, "qh_settemppop: depth %d temp set %x of %d elements\n",
       qh_setsize((setT *)qhmem.tempstack)+1, (int)stackedset, qh_setsize(stackedset));
  return stackedset;
} /* settemppop */

/*----------------------------------------
-settemppush- push temporary set unto qhmem.tempstack (makes it temporary)
  duplicates settemp() for tracing
*/
void qh_settemppush(setT *set) {
  
  qh_setappend ((setT**)&qhmem.tempstack, set);
  if (qhmem.IStracing >= 5)
    fprintf (qhmem.ferr, "qh_settemppush: depth %d temp set %x of %d elements\n",
    qh_setsize((setT *)qhmem.tempstack), (int)set, qh_setsize(set));
} /* settemppush */

 
/*----------------------------------------
-settruncate- truncate set to size elements
  set must be defined
*/
void qh_settruncate (setT *set, int size) {

  if (size < 0 || size > (int)set->maxsize) qhull_fatal(19);

  set->e[set->maxsize]= (void *) (size+1);   /* maybe overwritten */
  set->e[size]= NULL;
} /* setruncate */
    
/*----------------------------------------
-setunique- add element if it isn't already
  returns 1 if it's appended
*/
int qh_setunique (setT **set, void *elem) {

  if (!qh_setin (*set, elem)) {
    qh_setappend (set, elem);
    return 1;
  }
  return 0;
} /* setunique */
    
/*----------------------------------------
-setzero- zero remainder of set and set its size
  set must be defined
*/
void qh_setzero (setT *set, int index, int size) {
  int count;

  if (index < 0 || index >= size || size > (int)set->maxsize) qhull_fatal(20);

  (set->e[set->maxsize])=  (void *)(size+1);  /* may be overwritten */
  count= size - index + 1;   /* +1 for NULL terminator */
  memset ((char *)SETelemaddr_(set, index, void), 0, count * sizeof(void *));
} /* setzero */

    
/*************************************************************************/
/****************** implementation code from geom.c **********************/
/*************************************************************************/


/* geom.c -- geometric routines of qhull
   
   see README and geom.h

   copyright (c) 1993-1994 The Geometry Center        
*/
   
/*-------------------------------------------------
-backnormal- solve for normal x using back substitution over rows U
     solves Ux=b where Ax=b and PA=LU
     b= [0,...,0,sign or 0]  (-1 if sign, else +1)
     last row of A= [0,...,0,1]
     assumes numrow == numcol-1
returns:
     normal= x 
     if can't divzero() for later normalization (qh MINdenom_2 and qh MINdenom_1_2),
         sets tail of normal to [...,sign,0,...], i.e., solves for b= [0...]
	 sets nearzero, unless last row (i.e., hyperplane intersects [0,..,1])
notes:
     1) Ly=Pb == y=b since P only permutes the 0's of b
     see Golub & van Loan 4.4-9 for back substitution
*/
void qh_backnormal (realT **rows, int numrow, int numcol, boolT sign,
  	coordT *normal, boolT *nearzero) {
  int i, j;
  coordT *normalp, *normal_tail, *ai, *ak;
  realT diagonal;
  boolT waszero;
  int zerocol=-1;
  
  normalp= normal + numcol - 1;
  *normalp--= (sign ? -1.0 : 1.0);
  for(i= numrow; i--; ) {
    *normalp= 0.0;
    ai= rows[i] + i + 1;
    ak= normalp+1;
    for(j= i+1; j < numcol; j++)
      *normalp -= *ai++ * *ak++;
    diagonal= (rows[i])[i];
    if (fabs_(diagonal) > qh MINdenom_2)
      *(normalp--) /= diagonal;
    else {
      waszero= False;
      *normalp= qh_divzero (*normalp, diagonal, qh MINdenom_1_2, &waszero);
      if (waszero) {
        zerocol= i;
	*(normalp--)= (sign ? -1.0 : 1.0);
	for (normal_tail= normalp+2; normal_tail < normal + numcol; normal_tail++)
	  *normal_tail= 0.0;
      }else
	normalp--;
    }
  }
  if (zerocol != -1) {
    zzinc_(Zback0);
    *nearzero= True;
    trace4((qh ferr, "qh_backnormal: zero diagonal at column %d.\n", i));
  }
} /* backnormal */

/*-------------------------------------------------
-crossproduct- of 2 dim vectors, C= A x B
    from Glasner, Graphics Gems I, p. 639
    NOTE: only defined for dim==3
*/
void qh_crossproduct (int dim, realT vecA[3], realT vecB[3], realT vecC[3]){

  if (dim == 3) {
    vecC[0]=   det2_(vecA[1], vecA[2],
		     vecB[1], vecB[2]);
    vecC[1]= - det2_(vecA[0], vecA[2],
		     vecB[0], vecB[2]);
    vecC[2]=   det2_(vecA[0], vecA[1],
		     vecB[0], vecB[1]);
  }
} /* vcross */

/*-------------------------------------------------
-determinant- compute the determinant of a square matrix
  rows= row vectors
  uses qh NEARzero to test for degenerate matrices
    this does look right, probably no easy way of doing it
returns:
  determinant
  overwrites rows and the matrix
  nearzero set if degenerate
*/
realT qh_determinant (realT **rows, int dim, boolT *nearzero) {
  realT det=0;
  int i;
  boolT sign= False;

  *nearzero= False;
  if (dim < 2) { qhull_fatal(21); }
  else if (dim == 2) {
    det= det2_(rows[0][0], rows[0][1],
		 rows[1][0], rows[1][1]);
    if (fabs_(det) < qh NEARzero[1])  /* not really correct, what should this be? */
      *nearzero= True;
  }else if (dim == 3) {
    det= det3_(rows[0][0], rows[0][1], rows[0][2],
		 rows[1][0], rows[1][1], rows[1][2],
		 rows[2][0], rows[2][1], rows[2][2]);
    if (fabs_(det) < qh NEARzero[2])  /* not really correct, what should this be? */
      *nearzero= True;
  }else {	
    qh_gausselim(rows, dim, dim, &sign, nearzero);  /* if nearzero, diagonal still ok*/
    det= 1.0;
    for (i= dim; i--; )
      det *= (rows[i])[i];
    if (sign)
      det= -det;
  }
  return det;
} /* determinant */

/*-------------------------------------------------
-detsimplex- compute determinant of a simplex with point apex and base points
   uses qh gm_matrix/qh gm_row (assumes they're big enough)
   uses dim coordinates of point and vertex->point
returns:
   if dim == 2 or 3
     nearzero iff determinant < qh NEARzero[dim-1]  (not quite correct)
   if dim >= 4
     nearzero iff diagonal[k] < qh NEARzero[k]
*/
realT qh_detsimplex(pointT *apex, setT *points, int dim, boolT *nearzero) {
  pointT *coorda, *coordp, *gmcoord, *point, **pointp;
  coordT **rows;
  int k,  i=0;
  realT det= 0.0;

  zinc_(Zdetsimplex);
  gmcoord= qh gm_matrix;
  rows= qh gm_row;
  FOREACHpoint_(points) {
    if (i == dim)
      break;
    rows[i++]= gmcoord;
    coordp= point;
    coorda= apex;
    for(k= dim; k--; )
      *(gmcoord++)= *coordp++ - *coorda++;
  }
  if (i < dim) { qhull_fatal(22); }
  det= qh_determinant (rows, dim, nearzero);
  trace2((qh ferr, "qh_detsimplex: det=%2.2g for point p%d, dimension %d, nearzero? %d\n",
	  det, qh_pointid(apex), dim, *nearzero)); 
  return det;
} /* detsimplex */

/*-------------------------------------------
-distplane- get distance from point to facet
returns:
    positive if point is above facet (i.e., outside)
    can not qhull_fatal (for sortfacets)
*/
void qh_distplane (pointT *point, facetT *facet, realT *dist) {
  coordT *normal= facet->normal, *coordp, randr;
  int k;
  
  switch(qh hull_dim){
  case 2:
    *dist= facet->offset + point[0] * normal[0] + point[1] * normal[1];
    break;
  case 3:
    *dist= facet->offset + point[0] * normal[0] + point[1] * normal[1] + point[2] * normal[2];
    break;
  case 4:
    *dist= facet->offset+point[0]*normal[0]+point[1]*normal[1]+point[2]*normal[2]+point[3]*normal[3];
    break;
  case 5:
    *dist= facet->offset+point[0]*normal[0]+point[1]*normal[1]+point[2]*normal[2]+point[3]*normal[3]+point[4]*normal[4];
    break;
  case 6:
    *dist= facet->offset+point[0]*normal[0]+point[1]*normal[1]+point[2]*normal[2]+point[3]*normal[3]+point[4]*normal[4]+point[5]*normal[5];
    break;
  case 7:  
    *dist= facet->offset+point[0]*normal[0]+point[1]*normal[1]+point[2]*normal[2]+point[3]*normal[3]+point[4]*normal[4]+point[5]*normal[5]+point[6]*normal[6];
    break;
  case 8:
    *dist= facet->offset+point[0]*normal[0]+point[1]*normal[1]+point[2]*normal[2]+point[3]*normal[3]+point[4]*normal[4]+point[5]*normal[5]+point[6]*normal[6]+point[7]*normal[7];
    break;
  default:
    *dist= facet->offset;
    coordp= point;
    for (k= qh hull_dim; k--; )
      *dist += *coordp++ * *normal++;
    break;
  }
  zinc_(Zdistplane);
  if (!qh RANDOMdist && qh IStracing < 4)
    return;
  if (qh RANDOMdist) {
    randr= qh_RANDOMint;
    *dist += (2.0 * randr / qh_RANDOMmax - 1.0) *
      qh RANDOMfactor * qh maxmaxcoord;
  }
  /*
  if (qh IStracing >= 4) {
    fprintf (qh ferr, "qh_distplane: ");
    fprintf (qh ferr, qh_REAL_1, *dist);
    fprintf (qh ferr, "from p%d to f%d\n", qh_pointid(point), facet->id);
  }
  */
  return;
} /* distplane */


/*--------------------------------------------------
-divzero -- divide by a number that's nearly zero
  mindenom1= minimum denominator for dividing into 1.0
returns:
  zerodiv and 0.0 if it would overflow
*/
realT qh_divzero (realT numer, realT denom, realT mindenom1, boolT *zerodiv) {
  realT temp, numerx, denomx;
  

  if (numer < mindenom1 && numer > -mindenom1) {
    numerx= fabs_(numer);
    denomx= fabs_(denom);
    if (numerx < denomx) {
      *zerodiv= False;
      return numer/denom;
    }else {
      *zerodiv= True;
      return 0.0;
    }
  }
  temp= denom/numer;
  if (temp > mindenom1 || temp < -mindenom1) {
    *zerodiv= False;
    return numer/denom;
  }else {
    *zerodiv= True;
    return 0.0;
  }
} /* divzero */
  

/*-------------------------------------------------
-facetcenter- return Voronoi center for a facet's vertices
*/
pointT *qh_facetcenter (setT *vertices) {
  setT *points= qh_settemp (qh_setsize (vertices));
  vertexT *vertex, **vertexp;
  pointT *center;
  
  FOREACHvertex_(vertices) 
    qh_setappend (&points, vertex->point);
  center= qh_voronoi_center (qh hull_dim-1, points);
  qh_settempfree (&points);
  return center;
} /* facetcenter */

/*-------------------------------------------------
-findbest- find best facet for point starting at a facet (not flipped!)
  if bestoutside, searches all facets else stops at first outside
    MINoutside is DISTround in precise case
  if firstid, searches facets with ids >= firstid
    searches old facets if bestoutside || (not outside and imprecise)
  searches all neighbors of coplanar and flipped facets
    searchdist is arbitrarily set to min_vertex+max_outside+DISTround
       max_outside is needed for setting facet->maxoutside
returns:
  if !firstid, updates facet->maxoutside for good, visited facets
  distance to facet
  isoutside true if point is outside of facet
  bumps visit_id and seen flags
notes:
  uses visitid and seen
  statistics collected here for partitions, caller does outside/coplanar
  caller traces the results
  #2 after setfacetplane in D3, optimized for outside points and !bestoutside
  #1 when merging in D3
  see also partitionall()
notes on searchdist:
  searchdist needed since vertex neighbors can be geometric neighbors of facet 
  if searchdist=DISTround, gets stuck for rbox 50 W1e-3 D7 | qhull A-0.99 W0.2
  if !BESToutside and merging, gets stuck for rbox 1000 W8e-6 | qhull C-0
    because nearly coplanar widens when the point is outside of the facets
  searching all new facets does not prevent !BESToutside getting stuck
  check_maxoutside can also get stuck, should keep coplanars
*/
facetT *qh_findbest (pointT *point, facetT *facet, boolT bestoutside,
	   unsigned firstid, realT *dist, boolT *isoutside, int *numpart) {
  realT bestdist, searchdist;
  facetT *neighbor, **neighborp, *bestfacet;
  setT *search= NULL;
  int oldtrace= qh IStracing;
  boolT checkmax= (boolT)(bestoutside && !firstid && qh_MAXoutside
    && (qh MERGING || qh APPROXhull));

  if (qh TRACEpoint >= 0 && qh TRACEpoint == qh_pointid (point)) {
    qh IStracing= qh TRACElevel;
    /*
    fprintf (qh ferr, "qh_findbest: point p%d starting at f%d bestoutside? %d firstid %d\n",
	     qh TRACEpoint, facet->id, bestoutside, firstid);
    fprintf (qh ferr, "  Last point added to hull was p%d.", qh furthest_id);
    fprintf(qh ferr, "  Last merge was #%d.\n", zzval_(Ztotmerge));
    */
  }
  searchdist= - qh min_vertex + qh max_outside + 2* qh DISTround;
  *isoutside= True;
  *numpart= 1;
  qh_distplane (point, facet, dist);
  bestdist= *dist;
  bestfacet= facet;
  if (!bestoutside &&  *dist >= qh MINoutside) 
    goto LABELreturn_best;
#if qh_MAXoutside
  if (checkmax && (!qh ONLYgood || facet->good) && *dist > facet->maxoutside)
    facet->maxoutside= *dist;
#endif
  facet->visitid= ++qh visit_id;
  facet->seen= False;
  if (True) {                   /* directed search for bestfacet */
LABELrepeat:			   /* facet->seen if clearly worse */
    trace4((qh ferr, "qh_findbest: neighbors of f%d\n", facet->id));
    FOREACHneighbor_(facet) {
      if ((int)neighbor->visitid == qh visit_id)
        continue;
      if (neighbor->id < firstid) {
        neighbor->seen= True;
        continue;
      }
      neighbor->visitid= qh visit_id;
      neighbor->seen= False;
      if (neighbor->flipped)
        continue;
      (*numpart)++;
      qh_distplane (point, neighbor, dist);
      if (!bestoutside && *dist >= qh MINoutside) {
	bestfacet= neighbor;
	goto LABELreturn_best;
      }
#if qh_MAXoutside
      if (checkmax && (!qh ONLYgood || neighbor->good) 
          && *dist > neighbor->maxoutside)
        neighbor->maxoutside= *dist;
#endif
      if (*dist >= bestdist) {  /* >= for exact coplanar */
        bestdist= *dist;
        bestfacet= neighbor;
        if (*dist > bestdist + searchdist)
          facet->seen= True;
        facet= neighbor;
        goto LABELrepeat;
      }else if (*dist < bestdist - searchdist)
        neighbor->seen= True;
    }
  }
  do {                   /* search horizon of facet */
    FOREACHneighbor_(facet) {
      if ((int)neighbor->visitid == qh visit_id) {
        if (!neighbor->seen) {
          neighbor->seen= True;
          if (!search)
            search= qh_settemp (qh TEMPsize);
          qh_setappend (&search, neighbor);
        }
        continue;
      }
      neighbor->visitid= qh visit_id;
      neighbor->seen= True;
      if (neighbor->flipped) {
        if (!search)
          search= qh_settemp (qh TEMPsize);
        qh_setappend (&search, neighbor);
        continue;
      }
      if (neighbor->id < firstid) {
        if (!(bestoutside+qh APPROXhull+qh PREmerge))
          continue;
      }else
        zinc_(Zpartneighbor);
      (*numpart)++;
      qh_distplane (point, neighbor, dist);
      if (!bestoutside && *dist >= qh MINoutside) {
	bestfacet= neighbor;
	goto LABELreturn_best;
      }
#if qh_MAXoutside
      if (checkmax && *dist > neighbor->maxoutside)
        neighbor->maxoutside= *dist;
#endif
      if (*dist >= bestdist - searchdist) {
        if (!search)
          search= qh_settemp (qh TEMPsize);
        qh_setappend (&search, neighbor);
        if (*dist > bestdist) {
          bestdist= *dist;
          bestfacet= neighbor;
        }
      }
    }
  }while ((facet= (facetT *)qh_setdellast (search)));
  *dist= bestdist;
  if (!bestoutside || bestdist < qh MINoutside)
    *isoutside= False;
LABELreturn_best:
  if (search)
    qh_settempfree (&search);
  qh IStracing= oldtrace;
  return bestfacet;
}  /* findbest */


/*-------------------------------------------------
-findgooddist- find best good facet visible for point from facetA
  assumes facetA is visible from point
  uses qh visit_id and qh visible_list (but doesn't set visible)
returns:
  furthest distance to good facet, if any
  bumps visit_id and seen flags
*/
facetT *qh_findgooddist (pointT *point, facetT *facetA, realT *distp) {
  realT bestdist= -REALmax, dist;
  facetT *neighbor, **neighborp, *bestfacet=NULL, *facet;
  boolT goodseen= False;  

  if (facetA->good) {
    zinc_(Zverifypart);
    qh_distplane (point, facetA, &bestdist);
    bestfacet= facetA;
    goodseen= True;
  }
  qh_removefacet (facetA);
  qh_appendfacet (facetA);
  qh visible_list= facetA;
  facetA->visitid= ++qh visit_id;
  FORALLfacet_(qh visible_list) {
    FOREACHneighbor_(facet) {
      if ((int)neighbor->visitid == qh visit_id)
        continue;
      neighbor->visitid= qh visit_id;
      if (goodseen && !neighbor->good)
        continue;
      zinc_(Zverifypart);
      qh_distplane (point, neighbor, &dist);
      if (dist > 0) {
        qh_removefacet (neighbor);
        qh_appendfacet (neighbor);
        if (neighbor->good) {
          goodseen= True;
          if (dist > bestdist) {
            bestdist= dist;
            bestfacet= neighbor;
          }
        }
      }
    }
  }
  if (bestfacet) {
    *distp= bestdist;
    trace2((qh ferr, "qh_findgooddist: p%d is %2.2g above good facet f%d\n",
      qh_pointid(point), bestdist, bestfacet->id));
    return bestfacet;
  }
  trace4((qh ferr, "qh_findgooddist: no good facet for p%d above f%d\n", 
      qh_pointid(point), facetA->id));
  return NULL;
}  /* findgooddist */
    
/*-------------------------------------------------
-gausselim- Gaussian elimination with partial pivoting
  coordT data in rows
  assumes numrow <= numcol
returns:
  rows is upper triangular (includes row exchanges)
  flips sign for each row exchange
  sets nearzero if pivot[k] < qh NEARzero[k], else False.
    if nearzero, the determinant's sign may be incorrect.
*/
void qh_gausselim(realT **rows, int numrow, int numcol, boolT *sign, boolT *nearzero) {
  realT *ai, *ak, *rowp, *pivotrow;
  realT n, pivot, pivot_abs= 0.0, temp;
  int i, j, k, pivoti, flip=0, tempint;
  
  *nearzero= False;
  for(k= 0; k < numrow; k++) {
    pivot_abs= fabs_((rows[k])[k]);
    pivoti= k;
    for(i= k+1; i < numrow; i++) {
      if ((temp= fabs_((rows[i])[k])) > pivot_abs) {
	pivot_abs= temp;
	pivoti= i;
      }
    }
    if (pivoti != k) {
      rowp= rows[pivoti]; 
      rows[pivoti]= rows[k]; 
      rows[k]= rowp; 
      tempint = (int)*sign;
      tempint ^= 1;
      *sign = (boolT)tempint;
      flip ^= 1;
    }
    if (pivot_abs <= qh NEARzero[k]) {
      *nearzero= True;
      if (pivot_abs == 0.0) {   /* remainder of column == 0 */

	/*
	if (qh IStracing >= 4) {
	  fprintf (qh ferr, "qh_gausselim: 0 pivot at column %d. (%2.2g < %2.2g)\n", k, pivot_abs, qh DISTround);
	  qh_printmatrix (qh ferr, "Matrix:", rows, numrow, numcol);
	}
	*/

	zzinc_(Zgauss0);
	goto LABELnextcol;
      }
    }
    pivotrow= rows[k] + k;
    pivot= *pivotrow++;  /* signed value of pivot, and remainder of row */
    for(i= k+1; i < numrow; i++) {
      ai= rows[i] + k;
      ak= pivotrow;
      n= (*ai++)/pivot;   /* divzero() not needed since |pivot| >= |*ai| */
      for(j= numcol - (k+1); j--; )
	*ai++ -= n * *ak++;
    }
  LABELnextcol:
    ;
  }
  wmin_(Wmindenom, pivot_abs);  /* last pivot element */
  if (qh IStracing >= 5)
    qh_printmatrix (qh ferr, "qh_gausselem: result", rows, numrow, numcol);
} /* gausselim */


/*----------------------------------------------
-getangle- returns the dot product of two, qh hull_dim vectors
  may be > 1.0 or < -1.0
*/
realT qh_getangle(pointT *vect1, pointT *vect2) {
  realT angle= 0;
  int k;

  for(k= qh hull_dim; k--; )
    angle += *vect1++ * *vect2++;
  trace4((qh ferr, "qh_getangle: %2.2g\n", angle));
  return(angle);
} /* getangle */


/*----------------------------------------------
-getcenter-  gets arithmetic center of a set of vertices as a new point
  assumes normal_size is in short memory
*/
pointT *qh_getcenter(setT *vertices) {
  int k;
  pointT *center, *coord;
  vertexT *vertex, **vertexp;
  int count= qh_setsize(vertices);

  if (count < 2) qhull_fatal(23);

  center= (pointT *)qh_memalloc(qh normal_size);
  for (k=0; k < qh hull_dim; k++) {
    coord= center+k;
    *coord= 0.0;
    FOREACHvertex_(vertices)
      *coord += vertex->point[k];
    *coord /= count;
  }
  return(center);
} /* getcenter */


/*----------------------------------------------
-getcentrum- returns the centrum for a facet as a new point
  assumes normal_size is in short memory
*/
pointT *qh_getcentrum(facetT *facet) {
  realT dist;
  pointT *centrum, *point;

  point= qh_getcenter(facet->vertices);
  zinc_(Zcentrumtests);
  qh_distplane (point, facet, &dist);
  centrum= qh_projectpoint(point, facet, dist);
  qh_memfree(point, qh normal_size);
  trace4((qh ferr, "qh_getcentrum: for f%d, %d vertices dist= %2.2g\n",
	  facet->id, qh_setsize(facet->vertices), dist));
  return centrum;
} /* getcentrum */


/*-------------------------------------------------
-gram_schmidt- implements Gram-Schmidt orthogonalization by rows
   overwrites rows[dim][dim]
returns:
   false if gets a zero norm
notes:
   see Golub & van Loan Algorithm 6.2-2
   overflow due to small divisors not handled
*/
boolT qh_gram_schmidt(int dim, realT **row) {
  realT *rowi, *rowj, norm;
  int i, j, k;
  
  for(i=0; i < dim; i++) {
    rowi= row[i];
    for (norm= 0.0, k= dim; k--; rowi++)
      norm += *rowi * *rowi;
    norm= sqrt(norm);
    wmin_(Wmindenom, norm);
    if (norm == 0.0)  /* either 0 or overflow due to sqrt */
      return False;
    for(k= dim; k--; )
      *(--rowi) /= norm;  
    for(j= i+1; j < dim; j++) {
      rowj= row[j];
      for(norm= 0.0, k=dim; k--; )
	norm += *rowi++ * *rowj++;
      for(k=dim; k--; )
	*(--rowj) -= *(--rowi) * norm;
    }
  }
  return True;
} /* gram_schmidt */



	
/*--------------------------------------------------
-inthresholds- return True if normal within qh lower_/upper_threshold
returns:
  angle cos to a threshold border (may be NULL, invalid if qh SPLITthresholds)
*/
boolT qh_inthresholds (coordT *normal, realT *angle) {
  boolT within= True;
  int k;

  if (angle)
    *angle= 0.0;
  for(k= 0; k < qh hull_dim; k++) {
    if (qh lower_threshold[k] > -REALmax/2) {
      if (normal[k] < qh lower_threshold[k])
        within= False;
      if (angle)
        *angle += normal[k] * qh lower_threshold[k];
    }
    if (qh upper_threshold[k] < REALmax/2) {
      if (normal[k] > qh upper_threshold[k])
        within= False;
      if (angle)
        *angle += normal[k] * qh upper_threshold[k];
    }
  }
  return within;
} /* inthresholds */
    

/*--------------------------------------------------
-maxabsval -- return pointer to maximum absolute value of a dim vector
   returns NULL if dim==0
*/
realT *qh_maxabsval (realT *normal, int dim) {
  realT maxval= -REALmax;
  realT *maxp= NULL, *colp, absval;
  int k;

  for (k= dim, colp= normal; k--; colp++) {
    absval= fabs_(*colp);
    if (absval > maxval) {
      maxval= absval;
      maxp= colp;
    }
  }
  return maxp;
} /* maxabsval */


/*-------------------------------------------------
-maxmin- collects the maximum and minimum points of input into a set
  determines maximum roundoff errors
returns:
  returns a temporary set, without qh GOODpoint
  points are not unique
*/
setT *qh_maxmin(pointT *points, int numpoints, int dimension) {
  int k;
  realT maxsum= 0.0, maxcoord, temp, maxdistsum;
  realT maxneg= REALmax, maxpos= -REALmax;
  pointT *minimum, *maximum, *point, *pointtemp;
  setT *set;

  set= qh_settemp(2*dimension);
  for(k= 0; k < dimension; k++) {
    if (points == qh GOODpointp)
      minimum= maximum= points + qh hull_dim;
    else
      minimum= maximum= points;
    FORALLpoint_(points, numpoints) {
      if (point == qh GOODpointp)
	continue;
      if (maximum[k] < point[k])
	maximum= point;
      else if (minimum[k] > point[k])
	minimum= point;
    }
    maxcoord= fmax_(maximum[k], -minimum[k]);
    if (qh GOODpointp) {
      temp= fmax_(qh GOODpointp[k], -qh GOODpointp[k]);
      maximize_(maxcoord, temp);
    }
    maximize_(qh maxmaxcoord, maxcoord);
    maxsum += maxcoord;
    maximize_(maxpos, maximum[k]);
    minimize_(maxneg, minimum[k]);
    qh_setappend (&set, maximum);
    qh_setappend (&set, minimum);
    /* calculation of qh NEARzero is based on error formula 4.4-13 of
       Golub & van Loan, authors say n^3 can be ignored and 10 be used in
       place of rho */
    qh NEARzero[k]= 80 * maxsum * REALepsilon;
  }
  /* calculate roundoff error according to
     Lemma 3.2-1 of Golub and van Loan "Matrix Computation"
     use sqrt(dim) since one vector is normalized */
  maxdistsum= sqrt (qh hull_dim) * qh maxmaxcoord;
  if (!qh SETroundoff) {
    qh DISTround= REALepsilon * (qh hull_dim * maxdistsum * 1.01
			   	       + qh maxmaxcoord);  /* for offset */
    if (qh RANDOMdist)
      qh DISTround += qh RANDOMfactor * qh maxmaxcoord;
  }
  qh MINdenom= qh MINdenom_1 * qh maxmaxcoord;
  qh MINdenom_1_2= sqrt (qh MINdenom_1 * qh hull_dim) ;  /* if will be normalized */
  qh MINdenom_2= qh MINdenom_1_2 * qh maxmaxcoord;
  if (qh premerge_cos < REALmax/2)      /* for inner product */
    qh premerge_cos -= 1.01 * qh hull_dim * REALepsilon; 
  if (qh postmerge_cos < REALmax/2) 
    qh postmerge_cos -= 1.01 * qh hull_dim * REALepsilon;
  qh premerge_centrum += 2 * qh DISTround;    /*2 for centrum and distplane()*/
  qh postmerge_centrum += 2 * qh DISTround;
  { /* compute ONEmerge, max vertex offset for merging simplicial facets */
    realT maxangle= 1.0, maxrho;
    
    minimize_(maxangle, qh premerge_cos);
    minimize_(maxangle, qh postmerge_cos);
    /* max diameter * sin theta + DISTround for vertex to its hyperplane */
    qh ONEmerge= sqrt (qh hull_dim) * (maxpos - maxneg) *
      sqrt (1.0 - maxangle * maxangle) + qh DISTround;  
    maxrho= qh hull_dim * qh premerge_centrum + qh DISTround;
    maximize_(qh ONEmerge, maxrho);
    maxrho= qh hull_dim * qh postmerge_centrum + qh DISTround;
    maximize_(qh ONEmerge, maxrho);
  }
  if (!qh APPROXhull) {             /* user may specify qh MINoutside */
    qh MINoutside= qh premerge_centrum - qh DISTround;
    if (qh premerge_cos < REALmax/2) 
      maximize_(qh MINoutside, (1- qh premerge_cos) * qh maxmaxcoord);
  }
  if (qh MINvisible > REALmax/2)
    qh MINvisible= qh DISTround;

  /*
  if (qh MINvisible > qh MINoutside + 3*REALepsilon && !qh BESToutside &&
      !qh FORCEoutput)
    fprintf (qh ferr, "qhull input warning: minimum visibility V%.2g is greater than \nminimum outside W%.2g.  Flipped facets are likely.\n",
	     qh MINvisible, qh MINoutside);
	     */

  qh max_vertex= qh DISTround;
  qh min_vertex= -qh DISTround;

  /*
  if (qh IStracing >=1)
    qh_printpoints (qh ferr, "qh_maxmin: found the max and min points (by dim):", set);
    */

  /* numeric constants reported in printsummary */
  return(set);
} /* maxmin */


/*-------------------------------------------------
-maxsimplex- determines maximum simplex for a set of points 
  assumes at least pointsneeded points in points
  skips qh GOODpointp (assumes that it isn't in maxpoints)
  starts from points already in simplex
returns:
  temporary set of dim+1 points
notes:
  maximizes determinate for x,y,z,w, etc.
  uses maxpoints as long as determinate is clearly non-zero
*/
void qh_maxsimplex (int dim, setT *maxpoints, pointT *points, int numpoints, setT **simplex) {
  pointT *point, **pointp, *pointtemp, *maxpoint, *minx=NULL, *maxx=NULL;
  boolT nearzero, maxnearzero= False;
  int k, sizinit;
  realT maxdet= -REALmax, det, mincoord= REALmax, maxcoord= -REALmax;

  sizinit= qh_setsize (*simplex);
  if (sizinit < 2) {
    if (qh_setsize (maxpoints) >= 2) {
      FOREACHpoint_(maxpoints) {
	
        if (maxcoord < point[0]) {
          maxcoord= point[0];
          maxx= point;
        }
	if (mincoord > point[0]) {
          mincoord= point[0];
          minx= point;
        }
      }
    }else {
      FORALLpoint_(points, numpoints) {
	if (point == qh GOODpointp)
	  continue;
        if (maxcoord < point[0]) {
	  maxcoord= point[0];
          maxx= point;
        }
	if (mincoord > point[0]) {
          mincoord= point[0];
          minx= point;
	}
      }
    }
    qh_setunique (simplex, minx);
    if (qh_setsize (*simplex) < 2)
      qh_setunique (simplex, maxx);
    sizinit= qh_setsize (*simplex);

    if (sizinit < 2) qhull_fatal(24);
  }
  for(k= sizinit; k < dim+1; k++) {
    maxpoint= NULL;
    maxdet= -REALmax;
    FOREACHpoint_(maxpoints) {
      if (!qh_setin (*simplex, point)) {
        det= qh_detsimplex(point, *simplex, k, &nearzero);
        if ((det= fabs_(det)) > maxdet) {
	  maxdet= det;
          maxpoint= point;
	  maxnearzero= nearzero;
        }
      }
    }
    if (!maxpoint || maxnearzero) {
      zinc_(Zsearchpoints);
      if (!maxpoint) {
        trace0((qh ferr, "qh_maxsimplex: searching all points for %d-th initial vertex\n", k));
      }else {
        trace0((qh ferr, "qh_maxsimplex: searching all points for %d-th initial vertex, better than p%d det %2.2g\n",
		k+1, qh_pointid(maxpoint), maxdet));
      }
      FORALLpoint_(points, numpoints) {
	if (point == qh GOODpointp)
	  continue;
        if (!qh_setin (*simplex, point)) {
          det= qh_detsimplex(point, *simplex, k, &nearzero);
          if ((det= fabs_(det)) > maxdet) {
	    maxdet= det;
            maxpoint= point;
	    maxnearzero= nearzero;
	  }
        }
      }
    } /* !maxpoint */
    if (!maxpoint) qhull_fatal(25);

    qh_setappend(simplex, maxpoint);
    trace1((qh ferr, "qh_maxsimplex: selected point p%d for %d`th initial vertex, det=%2.2g\n",
	    qh_pointid(maxpoint), k, maxdet));
  } 
} /* maxsimplex */

/*--------------------------------------------------
-minabsval -- return min absolute value of a dim vector
*/
realT qh_minabsval (realT *normal, int dim) {
  realT minval= 0;
  realT maxval= 0;
  realT *colp;
  int k;

  for (k= dim, colp= normal; k--; colp++) {
    maximize_(maxval, *colp);
    minimize_(minval, *colp);
  }
  return fmax_(maxval, -minval);
} /* maxabsval */



/*--------------------------------------------------
-normalize -- normalize a vector
   qh MINdenom/MINdenom1 upper limits for divide overflow
returns:
    normalized vector
    flips sign if !toporient
    if zero norm
       sets all elements to sqrt(1.0/dim)
    if divide by zero (divzero ())
       sets largest element to +/-1
       bumps Znearlysingular
*/
void qh_normalize (coordT *normal, int dim, boolT toporient) {
  int k;
  realT *colp, *maxp, norm= 0, temp, *norm1, *norm2, *norm3;
  boolT zerodiv;

  norm1= normal+1;
  norm2= normal+2;
  norm3= normal+3;
  if (dim == 2)
    norm= sqrt((*normal)*(*normal) + (*norm1)*(*norm1));
  else if (dim == 3)
    norm= sqrt((*normal)*(*normal) + (*norm1)*(*norm1) + (*norm2)*(*norm2));
  else if (dim == 4) {
    norm= sqrt((*normal)*(*normal) + (*norm1)*(*norm1) + (*norm2)*(*norm2) 
               + (*norm3)*(*norm3));
  }else if (dim > 4) {
    norm= (*normal)*(*normal) + (*norm1)*(*norm1) + (*norm2)*(*norm2) 
               + (*norm3)*(*norm3);
    for (k= dim-4, colp= normal+4; k--; colp++)
      norm += (*colp) * (*colp);
    norm= sqrt(norm);
  }
  wmin_(Wmindenom, norm);
  if (norm > qh MINdenom) {
    if (!toporient)
      norm= -norm;
    *normal /= norm;
    *norm1 /= norm;
    if (dim == 2)
      ; /* all done */
    else if (dim == 3)
      *norm2 /= norm;
    else if (dim == 4) {
      *norm2 /= norm;
      *norm3 /= norm;
    }else if (dim >4) {
      *norm2 /= norm;
      *norm3 /= norm;
      for (k= dim-4, colp= normal+4; k--; )
        *colp++ /= norm;
    }
  }else if (norm == 0.0) {
    temp= sqrt (1.0/dim);
    for (k= dim, colp= normal; k--; )
      *colp++ = temp;
  }else {
    if (!toporient)
      norm= -norm;
    for (k= dim, colp= normal; k--; colp++) { /* k used below */
      temp= qh_divzero (*colp, norm, qh MINdenom_1, &zerodiv);
      if (!zerodiv)
	*colp= temp;
      else {
	maxp= qh_maxabsval(normal, dim);
	temp= ((*maxp * norm >= 0.0) ? 1.0 : -1.0);
	for (k= dim, colp= normal; k--; colp++)
	  *colp= 0.0;
	*maxp= temp;
	zzinc_(Znearlysingular);
	trace0((qh ferr, "qh_normalize: norm=%2.2g too small\n", norm));
	return;
      }
    }
  }
} /* normalize */


/*-------------------------------------------
-orientoutside- make facet outside oriented via qh interior_point
  returns True if reversed orientation.
*/
boolT qh_orientoutside (facetT *facet) {
  int k;
  realT dist;

  qh_distplane (qh interior_point, facet, &dist);
  if (dist > 0) {
    for (k= qh hull_dim; k--; )
      facet->normal[k]= -facet->normal[k];
    facet->offset= -facet->offset;
    return True;
  }
  return False;
} /* orientoutside */

/*-------------------------------------------
-pointdist- distance between points
*/
coordT qh_pointdist(pointT *point1, pointT *point2, int dim) {
  coordT dist, diff;
  int k;
  
  dist= 0.0;
  for (k= dim; k--; ) {
    diff= *point1++ - *point2++;
    dist += diff * diff;
  }
  return(sqrt(dist));
} /* pointdist */


/*-------------------------------------------------
-printmatrix- print matrix given by row vectors
  print a vector by (fp, "", &vect, 1, len)
*/
void qh_printmatrix (FILE *fp, char *string, realT **rows, int numrow, int numcol) {
  realT *rowp;
  int i,k;

  fprintf (fp, "%s\n", string);
  for (i= 0; i<numrow; i++) {
    rowp= rows[i];
    for (k= 0; k<numcol; k++)
      fprintf (fp, "%6.3g ", *rowp++);
    fprintf (fp, "\n");
  }
} /* printmatrix */

  
/*-------------------------------------------------
-printpoints- print pointids for a set of points starting at index 
  prints string and 'p' if defined
*/
void qh_printpoints (FILE *fp, char *string, setT *points) {
  pointT *point, **pointp;

  if (string) {
    fprintf (fp, "%s", string);
    FOREACHpoint_(points) 
      fprintf (fp, " p%d", qh_pointid(point));
    fprintf (fp, "\n");
  }else {
    FOREACHpoint_(points) 
      fprintf (fp, " %d", qh_pointid(point));
    fprintf (fp, "\n");
  }
} /* printpoints */

  
/*-------------------------------------------------
-projectinput- project input points using qh DELAUNAY and qh low_bound/high_bound
  input points in qh first_point, num_points, input_dim
     if POINTSmalloc, will free old point array
  if low[k]=high[k]= 0, removes dimension k 
     checks that hull_dim agrees with input_dim, PROJECTinput, and DELAUNAY
  if DELAUNAY 
    projects points to paraboloid
returns:
  new point array in first_point of qh hull_dim coordinates
  sets POINTSmalloc
  lowbound/highbound is also projected
*/
void qh_projectinput (void) {
  int k,i;
  int newdim= qh input_dim, newnum= qh num_points;
  signed char *project;
  int size= (qh input_dim+1)*sizeof(*project);
  pointT *newpoints, *coord, *infinity;
  realT paraboloid, maxboloid= 0;
  
  project= (signed char *)qh_memalloc (size);
  memset ((char*)project, 0, size);
  for (k= 0; k<qh input_dim; k++) {   /* skip Delaunay bound */
    if (qh lower_bound[k] == 0 && qh upper_bound[k] == 0) {
      project[k]= -1;
      newdim--;
    }
  }
  if (qh DELAUNAY) {
    project[k]= 1;
    newdim++;
    newnum++;
  }
  if (newdim != qh hull_dim) qhull_fatal(26);

  if (!(newpoints=(coordT*)malloc(newnum*newdim*sizeof(coordT))))
    qhull_fatal(27);

  qh_projectpoints (project, qh input_dim+1, qh first_point,
                    qh num_points, qh input_dim, newpoints, newdim);
  trace1((qh ferr, "qh_projectinput: updating lower and upper_bound\n"));
  qh_projectpoints (project, qh input_dim+1, qh lower_bound,
                    1, qh input_dim+1, qh lower_bound, newdim+1);
  qh_projectpoints (project, qh input_dim+1, qh upper_bound,
                    1, qh input_dim+1, qh upper_bound, newdim+1);
  qh_memfree(project, ((qh input_dim+1)*sizeof(*project)));
  if (qh POINTSmalloc)
    free (qh first_point);
  qh first_point= newpoints;
  qh POINTSmalloc= True;
  if (qh DELAUNAY) {
    coord= qh first_point;
    infinity= qh first_point + qh hull_dim * qh num_points;
    for (k=qh hull_dim-1; k--; )
      infinity[k]= 0.0;
    for (i=qh num_points; i--; ) {
      paraboloid= 0.0;
      for (k=qh hull_dim-1; k--; ) {
        paraboloid += *coord * *coord;
	infinity[k] += *coord;
        coord++;
      }
      *(coord++)= paraboloid;
      maximize_(maxboloid, paraboloid);
    }
    for (k=qh hull_dim-1; k--; )
      *(coord++) /= qh num_points;
    *(coord++)= maxboloid * 1.1;
    qh num_points++;
    trace0((qh ferr, "qh_projectinput: projected points to paraboloid for Delaunay\n"));
  }
} /* projectinput */

  
/*-------------------------------------------------
-projectpoint- project point onto a facet by dist
  projects point to hyperplane if dist= distplane(point,facet)
returns:
  returns a new point
  assumes normal_size is in short memory
*/

pointT *qh_projectpoint(pointT *point, facetT *facet, realT dist) {
  pointT *newpoint, *np, *normal;
  int normsize= qh normal_size,k;
  void **freelistp;
  
  float_qh_memalloc_(normsize, freelistp, newpoint);
  np= newpoint;
  normal= facet->normal;
  for(k= qh hull_dim; k--; )
    *(np++)= *point++ - dist * *normal++;
  return(newpoint);
} /* projectpoint */

  
/*-------------------------------------------------
-projectpoints- project along one or more dimensions
  delete dimension k if project[k] == -1
  add dimension k if project[k] == 1 
  n is size of project
  points, numpoints, dim is old points
  newpoints, newdim is buffer for new points (already allocated)
    newpoints may be points if only adding dimension at end
*/
void qh_projectpoints (signed char *project, int n, realT *points, 
        int numpoints, int dim, realT *newpoints, int newdim) {
  int testdim= dim, oldk=0, newk=0, i,j=0,k;
  realT *newp, *oldp;
  
  for (k= 0; k<n; k++)
    testdim += project[k];
  if (testdim != newdim) qhull_fatal(28);

  for (j= 0; j<n; j++) {
    if (project[j] == -1)
      oldk++;
    else {
      newp= newpoints+newk++;
      if (project[j] == +1) {
	if (oldk >= dim)
	  continue;
	oldp= points+oldk;
      }else 
	oldp= points+oldk++;
      for (i=numpoints; i--; ) {
        *newp= *oldp;
        newp += newdim;
        oldp += dim;
      }
    }
    if (oldk >= dim)
      break;
  }
  trace1((qh ferr, "qh_projectpoints: projected %d points from dim %d to dim %d\n", 
    numpoints, dim, newdim));
} /* projectpoints */
        

/*-------------------------------------------------
-randomfactor- return a random factor within qh RANDOMmax of 1.0
  RANDOMa/b definedin global.c
*/
realT qh_randomfactor (void) {
  realT randr;

  randr= qh_RANDOMint;
  return randr * qh RANDOMa + qh RANDOMb;
} /* randomfactor */

/*-------------------------------------------------
-randommatrix- generate a random dimXdim matrix in range (-1,1)
  assumes buffer is dim+1Xdim
returns:
  returns row vector for buffer
  plus row[dim] for scratch
*/
void qh_randommatrix (realT *buffer, int dim, realT **row) {
  int i, k;
  realT **rowi, *coord, realr;

  coord= buffer;
  rowi= row;
  for (i=0; i<dim; i++) {
    *(rowi++)= coord;
    for (k=0; k<dim; k++) {
      realr= qh_RANDOMint;
      *(coord++)= 2.0 * realr/(qh_RANDOMmax+1) - 1.0;
    }
  }
  *rowi= coord;
} /* randommatrix */

        
/*-------------------------------------------------
-rotateinput- rotate input using row matrix
  input points given by qh first_point, num_points, hull_dim
  if qh POINTSmalloc, overwrites input points, else mallocs a new array
  assumes rows[dim] is a scratch buffer
returns:
  sets qh POINTSmalloc
*/
void qh_rotateinput (realT **rows) {
  int size;
  pointT *newpoints;

  if (!qh POINTSmalloc) {
    size= qh num_points*qh hull_dim*sizeof(pointT);
    if (!(newpoints=(coordT*)malloc(size))) qhull_fatal(29);

    memcpy ((char *)newpoints, (char *)qh first_point, size);
    qh first_point= newpoints;
    qh POINTSmalloc= True;
  }
  qh_rotatepoints (qh first_point, qh num_points, qh hull_dim, rows);
}  /* rotateinput */

/*-------------------------------------------------
-rotatepoints- rotate numpoints points by a row matrix
  assumes rows[dim] is a scratch buffer
*/
void qh_rotatepoints (realT *points, int numpoints, int dim, realT **row) {
  realT *point, *rowi, *coord= NULL, sum, *newval;
  int i,j,k;

  for (point= points, j= numpoints; j--; point += dim) {
    newval= row[dim];
    for (i= 0; i<dim; i++) {
      rowi= row[i];
      coord= point;
      for (sum= 0.0, k= dim; k--; )
        sum += *rowi++ * *coord++;
      *(newval++)= sum;
    }
    for (k= dim; k--; )
      *(--coord)= *(--newval);
  }
} /* rotatepoints */  
  

/*-------------------------------------------------
-scaleinput- scale input points using qh low_bound/high_bound
  input points given by qh first_point, num_points, hull_dim
  if qh POINTSmalloc, overwrites input points, else mallocs a new array
returns:
  scales points to low[k], high[k]
  sets qh POINTSmalloc
*/
void qh_scaleinput (void) {
  int size;
  pointT *newpoints;

  if (!qh POINTSmalloc) {
    size= qh num_points*qh hull_dim*sizeof(pointT);

    if (!(newpoints=(coordT*)malloc(size))) qhull_fatal(30);

    memcpy ((char *)newpoints, (char *)qh first_point, size);
    qh first_point= newpoints;
    qh POINTSmalloc= True;
  }
  qh_scalepoints (qh first_point, qh num_points, qh hull_dim,
       qh lower_bound, qh upper_bound);
}  /* scaleinput */
  
/*-------------------------------------------------
-scalepoints- scale points to new lowbound and highbound
  retains old bound when newlow= -REALmax or newhigh= +REALmax
  overwrites old points
*/
void qh_scalepoints (pointT *points, int numpoints, int dim,
	realT *newlows, realT *newhighs) {
  int i,k;
  realT shift, scale, *coord, low, high, newlow, newhigh, mincoord, maxcoord;
  boolT nearzero= False;
     
  for (k= 0; k<dim; k++) {
    newhigh= newhighs[k];
    newlow= newlows[k];
    if (newhigh > REALmax/2 && newlow < -REALmax/2)
      continue;
    low= REALmax;
    high= -REALmax;
    for (i= numpoints, coord= points+k; i--; coord += dim) {
      minimize_(low, *coord);
      maximize_(high, *coord);
    }
    if (newhigh > REALmax/2)
      newhigh= high;
    if (newlow < -REALmax/2)
      newlow= low;
    scale= qh_divzero (newhigh - newlow, high - low,
                  qh MINdenom_1, &nearzero);
    if (nearzero) qhull_fatal(31);

    shift= (newlow * high - low * newhigh)/(high-low);
    coord= points+k;
    for (i= numpoints; i--; coord += dim)
      *coord= *coord * scale + shift;
    coord= points+k;
    if (newlow < newhigh) {
      mincoord= newlow;
      maxcoord= newhigh;
    }else {
      mincoord= newhigh;
      maxcoord= newlow;
    }
    for (i= numpoints; i--; coord += dim) {
      minimize_(*coord, maxcoord);  /* because of roundoff error */
      maximize_(*coord, mincoord);
    }
    trace0((qh ferr, "qh_scalepoints: scaled %d'th coordinate [%2.2g, %2.2g] to [%.2g, %.2g] for %d points by %2.2g and shifted %2.2g\n",
      k, low, high, newlow, newhigh, numpoints, scale, shift));
  }
} /* scalepoints */    

       
/*-------------------------------------------------
-setfacetplane- sets the hyperplane for a facet
   uses global buffers qh gm_matrix and qh gm_row
   overwrites facet->normal if already defined
   updates Wnewvertex if PRINTstatistics
*/
void qh_setfacetplane(facetT *facet) {
  pointT *point;
  vertexT *vertex, **vertexp;
  int k,i=0;
  int normsize= qh normal_size;
  int oldtrace = 0;
  realT dist;
  void **freelistp;
  coordT *coord, *gmcoord= qh gm_matrix;
  pointT *point0= ((vertexT*)SETfirst_(facet->vertices))->point;
  boolT nearzero;

  zzinc_(Zsetplane);
  if (!facet->normal)
    float_qh_memalloc_(normsize, freelistp, facet->normal);
  if (facet == qh tracefacet) {
    oldtrace= qh IStracing;
    qh IStracing= 5;

    /*
    fprintf (qh ferr, "qh_setfacetplane: facet f%d created.\n", facet->id);
    fprintf (qh ferr, "  Last point added to hull was p%d.", qh furthest_id);
    if (zzval_(Ztotmerge))
      fprintf(qh ferr, "  Last merge was #%d.", zzval_(Ztotmerge));
    fprintf (qh ferr, "\n\nSorry, qh_printsummary no longer supported:\n");
       fprintf (qh ferr, "\n\nCurrent summary is:\n");
	  qh_printsummary (qh ferr); */
  }
  if (qh hull_dim <= 4) {
    if (qh RANDOMdist) {
      FOREACHvertex_(facet->vertices) {
        qh gm_row[i++]= gmcoord;
	coord= vertex->point;
	for (k= qh hull_dim; k--; )
	  *(gmcoord++)= *coord++ * qh_randomfactor();
      }	  
    }else {
      FOREACHvertex_(facet->vertices)
       qh gm_row[i++]= vertex->point;
    }
    qh_sethyperplane_det(qh hull_dim, qh gm_row, point0, 
			 (boolT)facet->toporient,
			 facet->normal, &facet->offset);
  }else {
    FOREACHvertex_(facet->vertices) {
      if (vertex->point != point0) {
	qh gm_row[i++]= gmcoord;
	coord= vertex->point;
	point= point0;
	for(k= qh hull_dim; k--; )
	  *(gmcoord++)= *coord++ - *point++;
      }
    }
    qh gm_row[i]= gmcoord;  /* for areasimplex */
    if (qh RANDOMdist) {
      gmcoord= qh gm_matrix;
      for (i= qh hull_dim-1; i--; ) {
	for (k= qh hull_dim; k--; )
	  *(gmcoord++) *= qh_randomfactor();
      }
    }
    qh_sethyperplane_gauss(qh hull_dim, qh gm_row, point0, 
			   (boolT)facet->toporient,
			   facet->normal, &facet->offset, &nearzero);
    if (nearzero) { 
      if (qh_orientoutside (facet)) {
	trace0((qh ferr, "qh_setfacetplane: flipped orientation after testing interior_point\n"));
      /* this is part of using Gaussian Elimination.  For example in 5-d
	   1 1 1 1 0
	   1 1 1 1 1
	   0 0 0 1 0
	   0 1 0 0 0
	   1 0 0 0 0
	   norm= 0.38 0.38 -0.76 0.38 0
	 has a determinate of 1, but g.e. after subtracting pt. 0 has
	 0's in the diagonal, even with full pivoting.  It does work
	 if you subtract pt. 4 instead. */
      }
    }
  }
  if (qh PRINTstatistics) {
    FOREACHvertex_(facet->vertices) {
      if (vertex->point != point0) {
	zinc_(Zdiststat);
        qh_distplane(vertex->point, facet, &dist);
        dist= fabs_(dist);
        zinc_(Znewvertex);
        wadd_(Wnewvertex, dist);
        if (dist > wval_(Wnewvertexmax)) {
          wval_(Wnewvertexmax)= dist;
          maximize_(qh max_outside, dist);
        }
      }
    }
  }

  if (qh IStracing >= 3) {
    /*
    fprintf (qh ferr, "qh_setfacetplane: f%d offset %2.2g normal: ",
	     facet->id, facet->offset);
    for (k=0; k<qh hull_dim; k++)
      fprintf (qh ferr, "%2.2g ", facet->normal[k]);
    fprintf (qh ferr, "\n");
    */
  }

  if (facet == qh tracefacet)
    qh IStracing= oldtrace;
} /* setfacetplane */


/*-------------------------------------------------
-sethyperplane_det- set normalized hyperplane equation from oriented simplex
  dim X dim array indexed by rows[], one row per point, point0 is any row
  only defined for dim == 2..4
returns:
  offset, normal
  bumps Znearlysingular if normalization fails
  rows[] is not modified
notes:
  solves det(P-V_0, V_n-V_0, ..., V_1-V_0)=0, i.e. every point is on hyperplane
  offset places point0 on the hyperplane
  toporient just flips all signs, so orientation is correct
  see Bower & Woodworth, A programmer's geometry, Butterworths 1983.
*/
void qh_sethyperplane_det (int dim, coordT **rows, coordT *point0, 
          boolT toporient, coordT *normal, realT *offset) {

  if (dim == 2) {
    normal[0]= dY(1,0);
    normal[1]= dX(0,1);
    qh_normalize (normal, dim, toporient);
    *offset= -(point0[0]*normal[0]+point0[1]*normal[1]);
  }else if (dim == 3) {
    normal[0]= det2_(dY(2,0), dZ(2,0),
		     dY(1,0), dZ(1,0));
    normal[1]= det2_(dX(1,0), dZ(1,0),
		     dX(2,0), dZ(2,0));
    normal[2]= det2_(dX(2,0), dY(2,0),
		     dX(1,0), dY(1,0));
    qh_normalize (normal, dim, toporient);
    *offset= -(point0[0]*normal[0] + point0[1]*normal[1]
	       + point0[2]*normal[2]);
  }else if (dim == 4) {
    normal[0]= - det3_(dY(2,0), dZ(2,0), dW(2,0),
			dY(1,0), dZ(1,0), dW(1,0),
			dY(3,0), dZ(3,0), dW(3,0));
    normal[1]=   det3_(dX(2,0), dZ(2,0), dW(2,0),
		        dX(1,0), dZ(1,0), dW(1,0),
		        dX(3,0), dZ(3,0), dW(3,0));
    normal[2]= - det3_(dX(2,0), dY(2,0), dW(2,0),
			dX(1,0), dY(1,0), dW(1,0),
			dX(3,0), dY(3,0), dW(3,0));
    normal[3]=   det3_(dX(2,0), dY(2,0), dZ(2,0),
		        dX(1,0), dY(1,0), dZ(1,0),
		        dX(3,0), dY(3,0), dZ(3,0));
    qh_normalize (normal, dim, toporient);
    *offset= -(point0[0]*normal[0] + point0[1]*normal[1]
	       + point0[2]*normal[2] + point0[3]*normal[3]);
  }
} /* sethyperplane_det */


/*-------------------------------------------------
-sethyperplane_gauss- set normalized hyperplane equation from oriented simplex
    (dim-1) X dim array of rows[i]= V_{i+1} - V_0 (point0)
returns:
    offset, normal
    if nearzero, bumps Znearlysingular
      orientation may be incorrect because of incorrect sign flips in gausselim
notes:
    solves [V_n-V_0,...,V_1-V_0, 0 .. 0 1] * N == [0 .. 0 1] 
        or [V_n-V_0,...,V_1-V_0, 0 .. 0 1] * N == [0] 
    i.e., N is normal to the hyperplane, and the unnormalized
        distance to [0 .. 1] is either 1 or 0
    offset places point0 on the hyperplane
*/
void qh_sethyperplane_gauss (int dim, coordT **rows, pointT *point0, 
		boolT toporient, coordT *normal, coordT *offset, boolT *nearzero) {
  coordT *pointcoord, *normalcoef;
  int k, tempint;
  boolT sign= (boolT)!toporient, nearzero2= False;
  
  qh_gausselim(rows, dim-1, dim, &sign, nearzero);
  for(k= dim-1; k--; ) {
    if ((rows[k])[k] < 0) {
      tempint = (int)sign;
      tempint ^= 1;
      sign = (boolT)tempint;
    }
  }
  if (*nearzero) {
    zinc_(Znearlysingular);
    trace0((qh ferr, "qh_sethyperplane_gauss: nearly singular or axis parallel hyperplane.\n"));
    qh_backnormal(rows, dim-1, dim, sign, normal, &nearzero2);
  }else {
    qh_backnormal(rows, dim-1, dim, sign, normal, &nearzero2);
    if (nearzero2) {
      zinc_(Znearlysingular);
      trace0((qh ferr, "qh_sethyperplane_gauss: singular or axis parallel hyperplane at normalization\n"));
    }
  }
  if (nearzero2)
    *nearzero= True;
  qh_normalize(normal, dim, True);
  pointcoord= point0;
  normalcoef= normal;
  *offset= -(*pointcoord++ * *normalcoef++);
  for(k= dim-1; k--; )
    *offset -= *pointcoord++ * *normalcoef++;
} /* sethyperplane_gauss */

/*-------------------------------------------
-voronoi_center- return Voronoi center for a set of points
  dim is the orginal dimension of the points
notes:
  if non-simplicial, returns center for max simplex of points
  from Bowyer & Woodwark, A Programmer's Geometry, 1983, p. 65
*/
pointT *qh_voronoi_center (int dim, setT *points) {
  pointT *point, **pointp, *point0;
  pointT *center= (pointT *)qh_memalloc (qh center_size);
  setT *simplex;
  int i, j, k, num, size= qh_setsize(points);
  coordT *gmcoord;
  realT *diffp, sum2, *sum2row, *sum2p, det, factor;
  boolT nearzero, infinite;

  if (size == dim+1)
    simplex= points;
  else if (size < dim+1) qhull_fatal(32);

  else {
    simplex= qh_settemp (dim+1);
    qh_maxsimplex (dim, points, NULL, 0, &simplex);
  }
  num= qh_setsize (simplex);
  point0= (pointT *)SETfirst_(simplex);
  gmcoord= qh gm_matrix;
  for (k=0; k<dim; k++) {
    qh gm_row[k]= gmcoord;
    FOREACHpoint_(simplex) {
      if (point != point0)
        *(gmcoord++)= point[k] - point0[k];
    }
  }
  sum2row= gmcoord;
  for (i=0; i<dim; i++) {
    sum2= 0.0;
    for (k= 0; k<dim; k++) {
      diffp= qh gm_row[k] + i;
      sum2 += *diffp * *diffp;
    }
    *(gmcoord++)= sum2;
  }
  det= qh_determinant (qh gm_row, dim, &nearzero);
  factor= qh_divzero (0.5, det, qh MINdenom, &infinite);
  if (infinite) {
    for (k=dim; k--; )
      center[k]= qh_INFINITE;
    if (qh IStracing)
      qh_printpoints (qh ferr, "qh_voronoi_center: at infinity for ", simplex);
  }else {
    for (i=0; i<dim; i++) {
      gmcoord= qh gm_matrix;
      sum2p= sum2row;
      for (k=0; k<dim; k++) {
	qh gm_row[k]= gmcoord;
	if (k == i) {
	  for (j= dim; j--; )
	    *(gmcoord++)= *sum2p++;
	}else {
	  FOREACHpoint_(simplex) {
	    if (point != point0)
	      *(gmcoord++)= point[k] - point0[k];
	  }
	}
      }
      center[i]= qh_determinant (qh gm_row, dim, &nearzero)*factor + point0[i];
    }
    if (qh IStracing >= 3) {

      /*
      fprintf (qh ferr, "qh_voronoi_center: det %2.2g factor %2.2g ", det, factor);
      qh_printmatrix (qh ferr, "center:", &center, 1, dim);
      if (qh IStracing >= 5) {
	qh_printpoints (qh ferr, "points", simplex);
	FOREACHpoint_(simplex)
	  fprintf (qh ferr, "p%d dist %.2g, ", qh_pointid (point),
		   qh_pointdist (point, center, dim));
	fprintf (qh ferr, "\n");
      }
      */

    }
  }
  if (simplex != points)
    qh_settempfree (&simplex);
  return center;
} /* voronoi_center */


/*************************************************************************/
/****************** implementation code from stat.c **********************/
/*************************************************************************/

/* stat.c - contains all statistics that are collected for qhull

   see README and stat.h

   copyright (c) 1993-1994, The Geometry Center
*/

#if qh_QHpointer
qhstatT *qh_qhstat=NULL;  /* global data structure */
#else
qhstatT qh_qhstat ={0};   /* remove "={0}" if this causes a compiler error */
#endif


/*-------------------------------------------------
-allstatA -- define statistics in groups of 20
  (otherwise, 'gcc -O2' uses too much memory)
  uses qhstat next
*/
void qh_allstatA (void) {
  
   /* zdef_(type,name,doc,average) */
  zzdef_(zdoc, Zdoc2, "precision statistics", -1);
  zdef_(zinc, Znewvertex, NULL, -1);
  zdef_(wadd, Wnewvertex, "ave. distance of a new vertex to a facet (not 0s)", Znewvertex);
  zdef_(wmax, Wnewvertexmax, "max. distance of a new vertex to a facet", -1);
  zdef_(wmax, Wvertexmax, "max. distance of an output vertex to a facet", -1);
  zdef_(wmin, Wvertexmin, "min. distance of an output vertex to a facet", -1);
  zdef_(wmin, Wmindenom, "min. denominator in hyperplane computation", -1);

  qhstat precision= qhstat next;
  zzdef_(zdoc, Zdoc3, "precision problems", -1);
  zzdef_(zinc, Zcoplanarridges, "coplanar half ridges in output", -1);
  zzdef_(zinc, Zconcaveridges, "concave half ridges in output", -1);
  zzdef_(zinc, Zflippedfacets, "flipped facets", -1);
  zzdef_(zinc, Zcoplanarhorizon, "coplanar horizon facets for new vertices", -1);
  zzdef_(zinc, Zcoplanarpart, "coplanar points during partitioning", -1);
  zzdef_(zinc, Znearlysingular, "nearly singular or axis-parallel hyperplanes", -1);
  zzdef_(zinc, Zback0, "zero divisors during back substitute", -1);
  zzdef_(zinc, Zgauss0, "zero divisors during gaussian elimination", -1);
  zzdef_(zinc, Zmultimatch, "ridges with multiple neighbors (same flip)", -1);
  zzdef_(zinc, Zmultiflip, "ridges with multiple neighbors (diff flip)", -1);

}
void qh_allstatB (void) {
  zzdef_(zdoc, Zdoc1, "summary information", -1);
  zdef_(zinc, Zvertices, "number of vertices in output", -1);
  zdef_(zinc, Znumfacets, "number of facets in output", -1);
  zdef_(zinc, Znumridges, "number of ridges in output", -1);
  zdef_(zadd, Znumridges, "average number of ridges per facet", Znumfacets);
  zdef_(zmax, Zmaxridges, "maximum number of ridges", -1);
  zdef_(zadd, Znumneighbors, "average number of neighbors per facet", Znumfacets);
  zdef_(zmax, Zmaxneighbors, "maximum number of neighbors", -1);
  zdef_(zadd, Znumvertices, "average number of vertices per facet", Znumfacets);
  zdef_(zmax, Zmaxvertices, "maximum number of vertices", -1);
  zdef_(zadd, Znumvneighbors, "average number of neighbors per vertex", Zvertices);
  zdef_(zmax, Zmaxvneighbors, "maximum number of neighbors", -1);
  zdef_(wadd, Wcpu, "cpu seconds for qhull after input", -1);
  zdef_(zinc, Ztotvertices, "vertices created altogether", -1);
  zzdef_(zinc, Zsetplane, "facets created altogether", -1);
  zdef_(zinc, Ztotridges, "ridges created altogether", -1);
  zdef_(zinc, Zpostfacets, "facets before post merge", -1);
  zdef_(zinc, Zangle, NULL, -1);
  zdef_(wadd, Wangle, "average angle (cosine) for all ridges", Zangle);
  zdef_(wmax, Wanglemax, "maximum angle (cosine) of a ridge", -1);
  zdef_(wmin, Wanglemin, "minimum angle (cosine) of a ridge", -1);
  

  zdef_(zdoc, Zdoc9, "build hull statistics", -1);
  zdef_(zinc, Zdetsimplex, "determinates computed (initial hull)", -1);
  zzdef_(zinc, Zprocessed, "points processed", -1);
  zdef_(zinc, Ztotvisible, "visible facets for all new vertices", -1);
  zdef_(zinc, Zinsidevisible, "visible facets without an horizon neighbor", -1);
  zdef_(zinc, Ztothorizon, "horizon facets for all new vertices", -1);
  zdef_(wadd, Wnewbalance, "average new facet balance", Zprocessed);
  zdef_(wadd, Wnewbalance2, "  standard deviation", -1);
  zdef_(wadd, Wpbalance, "average partition balance", Zpbalance);
  zdef_(wadd, Wpbalance2, "  standard deviation", -1);
  zdef_(zinc, Zsearchpoints, "searches of all points for initial simplex", -1);
  zdef_(zinc, Znotmax, "points ignored (not above max_outside)", -1);
  zdef_(zinc, Znotgood, "points ignored (not above a good facet)", -1);
  zdef_(zinc, Znotgoodnew, "points ignored (didn't create a good new facet)", -1);
  zdef_(zinc, Zgoodfacet, "good facets found", -1);
  zzdef_(zinc, Znumvisibility, "distance tests for facet visibility", -1);
  zdef_(zinc, Ztotverify, "points verified", -1);
  zdef_(zinc, Zverifypart, "  ave. distance tests per verify", Ztotverify);
}
void qh_allstatC(void) {
  zdef_(zdoc, Zdoc4, "partitioning statistics", -1);
  zdef_(zinc, Zpartinside, "inside points", -1);
  zdef_(zinc, Zcoplanarinside, "  inside points that were coplanar with a facet", -1);
  zdef_(zinc, Ztotpartition, "partitions of a point", -1);
  
  zzdef_(zinc, Zpartitionall, "distance tests for initial partition", -1);
  zzdef_(zinc, Zpartition, "distance tests for other partitions", -1);
  zdef_(zinc, Zpartneighbor, "extra distance tests for coplanar and flipped neighbors", -1);
  zzdef_(zinc, Zdistcheck, "distance tests for checking flipped facets", -1); 
  zzdef_(zinc, Zdistconvex, "distance tests for checking convexity", -1); 
  zdef_(zinc, Zdistgood, "distance tests for checking good point", -1); 
  zdef_(zinc, Zdistio, "distance tests for output", -1); 
  zdef_(zinc, Zdiststat, "distance tests for statistics", -1); 
  zdef_(zinc, Zdistplane, "total number of distance tests", -1);
  zdef_(zinc, Ztotpartcoplanar, "partitions of coplanar points or deleted vertices", -1);
  zzdef_(zinc, Zpartcoplanar, "   distance tests for these partitions", -1);
  zdef_(zinc, Zcomputefurthest, "distance tests for computing furthest", -1);
  
}
void qh_allstatD(void) {
  zdef_(zdoc, Zdoc5, "statistics for matching ridges", -1);
  zdef_(zinc, Zhashlookup, "total lookups for matching ridges of new facets", -1);
  zdef_(zinc, Zhashtests, "average number of tests to match a ridge", Zhashlookup);
  zdef_(zinc, Zhashridge, "total lookups of subridges (duplicates and boundary)", -1);
  zdef_(zinc, Zhashridgetest, "average number of tests per subridge", Zhashridge);

  zdef_(zdoc, Zdoc6, "statistics for determining merges", -1);
  zdef_(zinc, Zangletests, "angles computed for ridge convexity", -1);
  zdef_(zinc, Zbestcentrum, "best merges used centrum instead of vertices", -1);
  zzdef_(zinc, Zbestdist, "distance tests for best merge", -1);
  zzdef_(zinc, Zcentrumtests, "distance tests for centrum convexity", -1);
  zdef_(zinc, Zcoplanarangle, "coplanar angles in getmergeset", -1);
  zdef_(zinc, Zcoplanarcentrum, "coplanar centrums in getmergeset", -1);
  zdef_(zinc, Zconcaveridge, "concave ridges in getmergeset", -1);
}
void qh_allstatE(void) {
  zdef_(zdoc, Zdoc7, "statistics for merging", -1);
  zdef_(wmax, Wmaxoutside, "max distance of merged vertex above facet", -1);
  zdef_(wmin, Wminvertex, "max distance of merged vertex below facet", -1);
  zzdef_(zinc, Ztotmerge, "total number of facets merged", -1);
  zdef_(zinc, Zmergenew, "new facets merged", -1);
  zdef_(zinc, Zmergehorizon, "horizon facets merged into new facets", -1);
  zdef_(zinc, Zmergeintohorizon, "new facets merged into horizon", -1);
  zdef_(zinc, Zmergevertex, "vertices deleted by merging", -1);
  zdef_(zinc, Zdegenvertex, "vertices deleted by degenerate facet", -1);
  zdef_(zinc, Zmergeflip, "merges due to flipped neighbors", -1);
  zdef_(zinc, Zmergeflipdup, "merges due to flipped facets in duplicated ridge", -1);
  zdef_(zinc, Zneighbor, "merges due to redundant neighbors", -1);
  zdef_(zinc, Zacoplanar, "merges due to angle coplanar facets", -1);
  zdef_(wadd, Wacoplanartot, "  average merge distance", Zacoplanar);
  zdef_(wmax, Wacoplanarmax, "  maximum merge distance", -1);
  zdef_(zinc, Zcoplanar, "merges due to coplanar facets", -1);
  zdef_(wadd, Wcoplanartot, "  average merge distance", Zcoplanar);
  zdef_(wmax, Wcoplanarmax, "  maximum merge distance", -1);
  zdef_(zinc, Zconcave, "merges due to concave facets", -1);
  zdef_(wadd, Wconcavetot, "  average merge distance", Zconcave);
  zdef_(wmax, Wconcavemax, "  maximum merge distance", -1);
  zdef_(zinc, Zavoidold, "coplanar/concave merges due to avoiding old merge", -1);
  zdef_(wadd, Wavoidoldtot, "  average merge distance", Zavoidold);
  zdef_(wmax, Wavoidoldmax, "  maximum merge distance", -1);
  zdef_(zinc, Zdegen, "merges due to degenerate facets", -1);
  zdef_(wadd, Wdegentot, "  average merge distance", Zdegen);
  zdef_(wmax, Wdegenmax, "  maximum merge distance", -1);
  zdef_(zinc, Zflipped, "merges due to removing flipped facets", -1);
  zdef_(wadd, Wflippedtot, "  average merge distance", Zflipped);
  zdef_(wmax, Wflippedmax, "  maximum merge distance", -1);
  zdef_(zinc, Zduplicate, "merges due to duplicated ridges", -1);
  zdef_(wadd, Wduplicatetot, "  average merge distance", Zduplicate);
  zdef_(wmax, Wduplicatemax, "  maximum merge distance", -1);

}
void qh_allstatF(void) {
  zdef_(zdoc, Zdoc8, "renamed vertex statistics", -1);
  zdef_(zinc, Zrenameshare, "renamed vertices shared by two facets", -1);
  zdef_(zinc, Zrenamepinch, "renamed vertices in a pinched facet", -1);
  zdef_(zinc, Zrenameall, "renamed vertices shared by multiple facets", -1);
  zdef_(zinc, Zfindfail, "rename failures due to duplicated ridges", -1);
  zdef_(zinc, Zdupridge, "  duplicate ridges detected", -1);
  zdef_(zinc, Zdelridge, "deleted ridges due to renamed vertices", -1);
  zdef_(zinc, Zdropneighbor, "dropped neighbors due to renamed vertices", -1);
  zdef_(zinc, Zdropdegen, "degenerate facets due to dropped neighbors", -1);
  zdef_(zinc, Zdelfacetdup, "  facets deleted because of no neighbors", -1);
  zdef_(zinc, Zremvertex, "vertices removed from facets due to no ridges", -1);
  zdef_(zinc, Zremvertexdel, "vertices deleted due to no ridges", -1);
  zdef_(zinc, Zintersectnum, "vertex intersections for locating redundant vertices", -1);
  zdef_(zinc, Zintersectfail, "intersections failed to find a redundant vertex", -1);
  zdef_(zinc, Zintersect, "intersections found redundant vertices", -1);
  zdef_(zadd, Zintersecttot, "   ave. number found per vertex", Zintersect);
  zdef_(zmax, Zintersectmax, "   max. found for a vertex", -1);
  zdef_(zinc, Zvertexridge, NULL, -1);
  zdef_(zadd, Zvertexridgetot, "  ave. number of ridges per tested vertex", Zvertexridge);
  zdef_(zmax, Zvertexridgemax, "  max. number of ridges per tested vertex", -1);

  zdef_(zdoc, Zdoc10, "memory usage statistics (in bytes)", -1);
  zdef_(zadd, Zmemfacets, "for facets and their normals, neighbor and vertex sets", -1);
  zdef_(zadd, Zmemvertices, "for vertices and their neighbor sets", -1);
  zdef_(zadd, Zmempoints, "for input points and outside and coplanar sets",-1);
  zdef_(zadd, Zmemridges, "for ridges and their vertex sets", -1);
} /* allstat */


#if qh_KEEPstatistics
/*-------------------------------------------------
-collectstatistics- collect statistics for qh facet_list
*/
void qh_collectstatistics (void) {
  facetT *facet, *neighbor, **neighborp;
  vertexT *vertex, **vertexp;
  realT dotproduct, dist;
  int sizneighbors, sizridges, sizvertices;
  
  zval_(Zmempoints)= qh num_points * qh normal_size + 
                             sizeof (qhT) + sizeof (qhstatT);
  zval_(Zmemfacets)= 0;
  zval_(Zmemridges)= 0;
  zval_(Zmemvertices)= 0;
  zval_(Zangle)= 0;
  wval_(Wangle)= 0.0;
  zval_(Znumridges)= 0;
  zval_(Znumfacets)= 0;
  zval_(Znumneighbors)= 0;
  zval_(Znumvertices)= 0;
  zval_(Znumvneighbors)= 0;
  zval_(Zvertices)= qh num_vertices - qh_setsize (qh del_vertices);
  if (qh MERGING || qh APPROXhull)
    wmax_(Wmaxoutside, qh max_outside);
  if (qh MERGING)
    wmin_(Wminvertex, qh min_vertex);
  FORALLfacets
    facet->seen= False;
  if (qh DELAUNAY) {
    FORALLfacets {
      if (facet->normal && facet->normal[qh hull_dim - 1] > 0.0)
        facet->seen= True;
    }
  }
  FORALLfacets {
    if (facet->visible && qh NEWfacets)
      continue;
    sizvertices= qh_setsize (facet->vertices);
    sizneighbors= qh_setsize (facet->neighbors);
    sizridges= qh_setsize (facet->ridges);
    zinc_(Znumfacets);
    zadd_(Znumvertices, sizvertices);
    zmax_(Zmaxvertices, sizvertices);
    zadd_(Znumneighbors, sizneighbors);
    zmax_(Zmaxneighbors, sizneighbors);
    if (sizridges) {
      zadd_(Znumridges, sizridges);
      zmax_(Zmaxridges, sizridges);
    }
    zadd_(Zmemfacets, sizeof (facetT) + qh normal_size + 2*sizeof (setT) 
       + SETelemsize * (sizneighbors + sizvertices));
    if (facet->ridges) {
      zadd_(Zmemridges,
	 sizeof (setT) + SETelemsize * sizridges + sizridges * 
         (sizeof (ridgeT) + sizeof (setT) + SETelemsize * (qh hull_dim-1))/2);
    }
    if (facet->outsideset)
      zadd_(Zmempoints, sizeof (setT) + SETelemsize * qh_setsize (facet->outsideset));
    if (facet->coplanarset)
      zadd_(Zmempoints, sizeof (setT) + SETelemsize * qh_setsize (facet->coplanarset));
    if (facet->seen) /* Delaunay upper envelope */
      continue;
    facet->seen= True;
    FOREACHneighbor_(facet) {
      if (neighbor == qh_DUPLICATEridge || neighbor == qh_MERGEridge
	  || neighbor->seen || !facet->normal || !neighbor->normal)
	continue;
      dotproduct= qh_getangle(facet->normal, neighbor->normal);
      zinc_(Zangle);
      wadd_(Wangle, dotproduct);
      wmax_(Wanglemax, dotproduct)
      wmin_(Wanglemin, dotproduct)
    }
    FOREACHvertex_(facet->vertices) {
      zinc_(Zdiststat);
      qh_distplane(vertex->point, facet, &dist);
      wmax_(Wvertexmax, dist);
      wmin_(Wvertexmin, dist);
    }
  }
  FORALLvertices {
    if (vertex->deleted)
      continue;
    zadd_(Zmemvertices, sizeof (vertexT));
    if (vertex->neighbors) {
      sizneighbors= qh_setsize (vertex->neighbors);
      zadd_(Znumvneighbors, sizneighbors);
      zmax_(Zmaxvneighbors, sizneighbors);
      zadd_(Zmemvertices, sizeof (vertexT) + SETelemsize * sizneighbors);
    }
  }
} /* collectstatistics */
#endif /* qh_KEEPstatistics */

/*-------------------------------------------------
-freestatistics- free memory used for statistics
*/
void qh_freestatistics (void) {

#if qh_QHpointer   /* duplicated if !KEEPstatistics */
  free (qh_qhstat);
  qh_qhstat= NULL;
#endif
} /* freestatistics */

/*-------------------------------------------------
-initstatistics- allocate and initialize statistics
  uses malloc instead of memalloc since mem.c not setup yet
*/
void qh_initstatistics (void) {
  int i;
  realT realx;
  int intx;

#if qh_QHpointer   /* duplicated if !KEEPstatistics */
  if (!(qh_qhstat= (qhstatT *)malloc (sizeof(qhstatT)))) qhull_fatal(33);
#endif
  
  qhstat next= 0;
  qh_allstatA();
  qh_allstatB();
  qh_allstatC();
  qh_allstatD();
  qh_allstatE();
  qh_allstatF();

  qhstat init[zinc].i= 0;
  qhstat init[zadd].i= 0;
  qhstat init[zmin].i= INT_MAX;
  qhstat init[zmax].i= INT_MIN;
  qhstat init[wadd].r= 0;
  qhstat init[wmin].r= REALmax;
  qhstat init[wmax].r= -REALmax;
  for (i=0; i<ZEND; i++) {
    if (qhstat type[i] > ZTYPEreal) {
      realx= qhstat init[(unsigned char)(qhstat type[i])].r;
      qhstat stats[i].r= realx;
    }else if (qhstat type[i] != zdoc) {
      intx= qhstat init[(unsigned char)(qhstat type[i])].i;
      qhstat stats[i].i= intx;
    }
  }
} /* initstatistics */

/*-------------------------------------------
-newstats- returns True if statistics for zdoc
returns:
  next zdoc
*/
boolT qh_newstats (int index, int *nextindex) {
  boolT isnew= False;
  int start, i;

  if (qhstat type[qhstat id[index]] == zdoc) 
    start= index+1;
  else
    start= index;
  for (i= start; i < qhstat next && qhstat type[qhstat id[i]] != zdoc; i++) {
    if (!qh_nostatistic(qhstat id[i]) && !qhstat printed[qhstat id[i]])
	isnew= True;
  }
  *nextindex= i;
  return isnew;
} /* newstats */

/*-------------------------------------------
-nostatistic- true if no statistic to print
*/
boolT qh_nostatistic (int i) {
  
  if ((qhstat type[i] > ZTYPEreal
       &&qhstat stats[i].r == qhstat init[(unsigned char)(qhstat type[i])].r)
      || (qhstat type[i] < ZTYPEreal
	  &&qhstat stats[i].i == qhstat init[(unsigned char)(qhstat type[i])].i))
    return True;
  return False;
} /* nostatistic */

#if qh_KEEPstatistics
/*-------------------------------------------
-qh_printallstatistics- print all statistics to a file
*/
void qh_printallstatistics (FILE *fp, char *string) {
  int i;
  
  for (i=ZEND; i--; ) 
    qhstat printed[i]= False;
  qh_printstatistics (fp, string);
}


/*-------------------------------------------
-printstatistics- print statistics to a file
*/
void qh_printstatistics (FILE *fp, char *string) {
  int i, k;
  realT ave;
  
  if (qh num_points != qh num_vertices) {
    wval_(Wpbalance)= 0;
    wval_(Wpbalance2)= 0;
  }else
    wval_(Wpbalance2)= qh_stddev (zval_(Zpbalance), wval_(Wpbalance), 
                                 wval_(Wpbalance2), &ave);
  wval_(Wnewbalance2)= qh_stddev (zval_(Zprocessed), wval_(Wnewbalance), 
                                 wval_(Wnewbalance2), &ave);
  fprintf (fp, "\n\
%s\n\
 qhull invoked by: %s | %s\n", string, qh rbox_command, qh qhull_command);
  fprintf (fp, "\nprecision constants:\n\
 %6.2g max. coordinate in the input\n\
 %6.2g max. roundoff error for distance computation\n\
 %6.2g min. distance for outside points\n", 
              qh maxmaxcoord, qh DISTround, qh MINoutside);
  if (qh premerge_cos < REALmax/2) fprintf (fp, "\
 %6.2g max. cosine for pre-merge angle\n", qh premerge_cos);
  if (qh PREmerge) fprintf (fp, "\
 %6.2g radius of pre-merge centrum\n", qh premerge_centrum);
  if (qh postmerge_cos < REALmax/2) fprintf (fp, "\
 %6.2g max. cosine for post-merge angle\n", qh postmerge_cos);
  if (qh POSTmerge) fprintf (fp, "\
 %6.2g radius of post-merge centrum\n", qh postmerge_centrum);
  fprintf (fp, "\
 %6.2g max. distance for merging two simplicial facets\n\
 %6.2g max. roundoff error for arithmetic operations\n\
 %6.2g min. denominator for divisions\n\
  zero diagonal for Gauss: ", qh ONEmerge, REALepsilon, qh MINdenom);
  for (k=0; k<qh hull_dim; k++)
    fprintf (fp, "%6.2e ", qh NEARzero[k]);
  fprintf (fp, "\n\n");
  for (i=0 ; i<qhstat next; ) 
    qh_printstats (fp, i, &i);
} /* printstatistics */
#endif /* qh_KEEPstatistics */

/*-------------------------------------------
-printstatlevel- print level information for a statistic
  start is 1st level to print
  nop if id >= ZEND, printed, or same as initial value
*/
void qh_printstatlevel (FILE *fp, int id, int start) {
#define NULLfield "       "

  if (id >= ZEND || qhstat printed[id])
    return;
  if (qhstat type[id] == zdoc) {
    fprintf (fp, "%s\n", qhstat doc[id]);
    return;
  }
  if (qh_nostatistic(id) || !qhstat doc[id])
    return;
  qhstat printed[id]= True;
  if (qhstat count[id] != -1 
      && qhstat stats[(unsigned char)(qhstat count[id])].i == 0)
    fprintf (fp, " *0 cnt*");
  else if (qhstat type[id] >= ZTYPEreal && qhstat count[id] == -1)
    fprintf (fp, " %6.2g", qhstat stats[id].r);
  else if (qhstat type[id] >= ZTYPEreal && qhstat count[id] != -1)
    fprintf (fp, " %6.2g", qhstat stats[id].r/ qhstat stats[(unsigned char)(qhstat count[id])].i);
  else if (qhstat type[id] < ZTYPEreal && qhstat count[id] == -1)
    fprintf (fp, " %6d", qhstat stats[id].i);
  else if (qhstat type[id] < ZTYPEreal && qhstat count[id] != -1)
    fprintf (fp, " %6.2g", (realT) qhstat stats[id].i / qhstat stats[(unsigned char)(qhstat count[id])].i);
  fprintf (fp, " %s\n", qhstat doc[id]);
} /* printstatlevel */


/*-------------------------------------------------------
-printstats- print statistics for a zdoc group
returns:
  next zdoc if non-null
*/
void qh_printstats (FILE *fp, int index, int *nextindex) {
  int j, nexti;

  if (qh_newstats (index, &nexti)) {
    for (j=index; j<nexti; j++)
      qh_printstatlevel (fp, qhstat id[j], 0);
    fprintf (fp, "\n");
  }
  if (nextindex)
    *nextindex= nexti;
} /* printstats */

#if qh_KEEPstatistics

/*-----------------------------------------
-stddev- compute the standard deviation and average from statistics
  tot2 is the sum of the squares
notes:
  computes r.m.s.: (x-ave)^2 == x^2 - 2x tot/num + (tot/num)^2
     == tot2 - 2 tot tot/num + tot tot/num == tot2 - tot ave
*/
realT qh_stddev (int num, realT tot, realT tot2, realT *ave) {
  realT stddev;

  *ave= tot/num;
  stddev= sqrt (tot2/num - *ave * *ave);
  return stddev;
} /* stddev */

#endif /* qh_KEEPstatistics */ 

#if !qh_KEEPstatistics
void    qh_collectstatistics (void) {}
void    qh_printallstatistics (FILE *fp, char *string) {}
void    qh_printstatistics (FILE *fp, char *string) {}
#endif


/*************************************************************************/
/****************** implementation code from poly.c **********************/
/*************************************************************************/

/* Ppoly.c -- implements polygons and simplices

   see README, poly.h and qhull.h
   
   copyright (c) 1993-1994, The Geometry Center
*/

/*======== functions in alphabetical order ==========*/

/*-------------------------------------------------
-appendfacet- appends facet to end of qh facet_list,
  updates qh facet_list, facet_tail, newfacet_list, facet_next
  increments qh numfacets
  assumes qh facet_list/facet_tail is defined (createsimplex)
*/
void qh_appendfacet(facetT *facet) {
  facetT *tail= qh facet_tail;

  if (tail == qh newfacet_list)
    qh newfacet_list= facet;
  if (tail == qh facet_next)
    qh facet_next= facet;
  facet->previous= tail->previous;
  facet->next= tail;
  if (tail->previous)
    tail->previous->next= facet;
  else
    qh facet_list= facet;
  tail->previous= facet;
  qh num_facets++;
  trace4((qh ferr, "qh_appendfacet: append f%d to facet_list\n", facet->id));
} /* appendfacet */


/*-------------------------------------------------
-appendvertex- appends vertex to end of qh vertex_list,
  updates qh vertex_list, vertex_tail, newvertex_list
  increments qh num_vertices
  assumes qh vertex_list/vertex_tail is defined (createsimplex)
*/
void qh_appendvertex (vertexT *vertex) {
  vertexT *tail= qh vertex_tail;

  if (tail == qh newvertex_list)
    qh newvertex_list= vertex;
  vertex->previous= tail->previous;
  vertex->next= tail;
  if (tail->previous)
    tail->previous->next= vertex;
  else
    qh vertex_list= vertex;
  tail->previous= vertex;
  qh num_vertices++;
  trace4((qh ferr, "qh_appendvertex: append v%d to vertex_list\n", vertex->id));
} /* appendvertex */


/*-------------------------------------------------
-attachnewfacets- attach horizon facets to new facets in qh newfacet_list
  if (qh ONLYgood)
    newfacets have neighbor and ridge links to horizon but not vice versa
  else
    newfacets attached to horizon and visible facets attached to new
  qh visible_list is all visible facets
returns:
  horizon facets linked to new facets 
     ridges changed from visible facets to new facets
     simplicial ridges deleted
     for non-simplicial horizon facets, the first neighbor is a new facet
  qh visible_list, no ridges valid, only newfacet neighbors (if any)
     sets qh NEWfacets
  if qh VERTEXneighbors, update neighbors for each vertex
  interior vertices added to qh del_vertices for later partitioning
*/
void qh_attachnewfacets (void ) {
  facetT *newfacet= NULL, *neighbor, **neighborp, *horizon, *visible;
  ridgeT *ridge, **ridgep;
  vertexT *vertex, **vertexp;

  qh NEWfacets= True;
  if (qh ONLYgood) {
    trace3((qh ferr, "qh_attachnewfacets: delete interior ridges\n"));
    qh visit_id++;
    FORALLvisible_facets {
      visible->visitid= qh visit_id;
      if (visible->ridges) {
        FOREACHridge_(visible->ridges) {
          neighbor= otherfacet_(ridge, visible);
          if ((int)neighbor->visitid == qh visit_id
          || (!neighbor->visible && neighbor->simplicial)) {
            if (!neighbor->visible)  /* delete ridge for simplicial horizon */
	      qh_setdel (neighbor->ridges, ridge);
	    qh_setfree (&(ridge->vertices)); /* delete on 2nd visit */
	    qh_memfree (ridge, sizeof(ridgeT));
	  }
        }
        SETfirst_(visible->ridges)= NULL;
      }
      SETfirst_(visible->neighbors)= NULL;
      SETsecond_(visible->neighbors)= NULL;
    }
    trace1((qh ferr, "qh_attachnewfacets: attach horizon facets to new facets\n"));
    FORALLnew_facets {
      horizon= (facetT *)SETfirst_(newfacet->neighbors);
      if (horizon->simplicial) {
        visible= NULL;
        FOREACHneighbor_(horizon) {   /* may have more than one horizon ridge */
	  if (neighbor->visible) {
	    if (visible) {
	      if (qh_setequal_skip (newfacet->vertices, 0, horizon->vertices,
		     SETindex_(horizon->neighbors, neighbor))) {
	        visible= neighbor;
	        break;
	      }
	    }else
	      visible= neighbor;
	  }
        }
        if (visible) {
          SETfirst_(visible->neighbors)= newfacet;
	  qh_setreplace (horizon->neighbors, visible, newfacet);
        }

	else qhull_fatal(34);

      }else { /* non-simplicial, with a ridge for newfacet */
        FOREACHneighbor_(horizon) {    /* may hold for many new facets */
	  if (neighbor->visible) {
	    SETfirst_(neighbor->neighbors)= newfacet;
	    qh_setdelnth (horizon->neighbors,
			SETindex_(horizon->neighbors, neighbor));
	    neighborp--; /* repeat */
	  }
        }
        qh_setappend (&horizon->neighbors, newfacet);
        ridge= (ridgeT *)SETfirst_(newfacet->ridges);
        if (ridge->top == horizon)
          ridge->bottom= newfacet;
        else
          ridge->top= newfacet;
      }
    } /* newfacets */
    if (qh PRINTstatistics) {
      FORALLvisible_facets {
        if (!SETfirst_(visible->neighbors))
          zinc_(Zinsidevisible);
      }
    }
  }
  trace3((qh ferr, "qh_attachnewfacets: delete interior vertices and update vertex->neighbors\n"));
  if (qh VERTEXneighbors) {
    FORALLvisible_facets {
      FOREACHvertex_(visible->vertices) {
        if (vertex->newlist)
          qh_setdel (vertex->neighbors, visible);
        else if (!vertex->deleted) {
  	  FOREACHneighbor_(vertex) { /* this can happen under merging,
				      see checkfacet() */
	    if (!neighbor->visible)
	      break;
	  }
	  if (neighbor)
	    qh_setdel (vertex->neighbors, visible);
	  else {
	    vertex->deleted= True;
	    qh_setappend (&qh del_vertices, vertex);
	    trace2((qh ferr, "qh_attachnewfacets: delete vertex p%d (v%d) in f%d\n",
		  qh_pointid(vertex->point), vertex->id, visible->id));
  	  }
        }
      }
    }
    FORALLnew_facets {
      FOREACHvertex_(newfacet->vertices)
        qh_setappend (&vertex->neighbors, newfacet);
    }
  }else {  /* !VERTEXneighbors */
    FORALLvisible_facets {
      FOREACHvertex_(visible->vertices) {
        if (!vertex->newlist && !vertex->deleted) {
          vertex->deleted= True;
	  qh_setappend (&qh del_vertices, vertex);
	  trace2((qh ferr, "qh_attachnewfacets: delete vertex p%d (v%d) in f%d\n",
		  qh_pointid(vertex->point), vertex->id, visible->id));
  	}
      }
    }
  }
} /* attachnewfacets */

/*-----------------------------------------------
-check_bestdist- check that points are within max_outside of the nearest facet
  if ONLYgood, ignores !good facets
  see: check_maxout
*/
void qh_check_bestdist (void) {
  boolT waserror= False, isoutside;
  facetT *facet, *bestfacet, *errfacet1= NULL, *errfacet2= NULL;
  realT dist, maxoutside;
  pointT *point;
  int numpart, facet_i, facet_n, notgood= 0;
  setT *facets;

  maxoutside= fmax_(qh max_outside, qh DISTround);
  maxoutside += 2 * qh DISTround;
  /* 1 DISTround to actual point and another DISTround to computed point */
  trace1((qh ferr, "qh_check_bestdist: check that all points are within %2.2g of best facet\n", maxoutside));
  facets= qh_pointfacet (/*qh facet_list*/);
  if (!qh_QUICKhelp && qh PRINTprecision)
    fprintf (qh ferr, "\n\
qhull output completed.  Verifying that %d points are\n\
below %2.2g of the nearest %sfacet.\n",
	     qh_setsize(facets), maxoutside, (qh ONLYgood ?  "good " : ""));
  FOREACHfacet_i_(facets) {
    zinc_(Ztotverify);
    if (!facet)
      facet= qh facet_list;
    point= qh_point(facet_i);
    if (point == qh GOODpointp)
      continue;
    bestfacet= qh_findbest (point, facet, qh_ALL, 0,
			    &dist, &isoutside, &numpart);
    zadd_(Zverifypart, numpart);
    if (dist > maxoutside) {
      if (qh ONLYgood && !bestfacet->good 
	  && !((bestfacet= qh_findgooddist (point, bestfacet, &dist))
	       && dist > maxoutside))
	notgood++;
      else {
	waserror= True;
	fprintf(qh ferr, "qhull precision error: point p%d is outside facet f%d, distance = %6.8g maxoutside= %6.8g\n", 
		facet_i, bestfacet->id, dist, maxoutside);
	errfacet2= errfacet1;
	errfacet1= bestfacet;		    
      }
    }
  }
  qh_settempfree (&facets);

  if (waserror) qhull_fatal(35);

} /* check_bestdist */

/*-----------------------------------------------
-check_maxout- updates max_outside by checking all points against bestfacet
  updates facet->maxoutside via findbest
  updates facet->coplanarset to qh min_vertex
  if PRINTnone, updates min_vertex for printsummary()
  if ONLYgood, ignores !good facets
  see check_bestdist
notes:
  may not need to check inside points if KEEPcoplanar 
     (since coplanar is now min_vertex instead of -DISTround)
*/
void qh_check_maxout (void) {
  boolT isoutside;
  facetT *facet, *bestfacet, *neighbor, **neighborp;
  realT dist, maxoutside, minvertex;
  pointT *point;
  int numpart, facet_i, facet_n, notgood= 0;
  setT *facets, *vertices= NULL;
  vertexT *vertex;

  maxoutside= minvertex= 0;
  trace1((qh ferr, "qh_check_maxout: determine actual maxoutside and minoutside\n"));
  facets= qh_pointfacet (/*qh facet_list*/);
  if (qh PRINTout[0] == qh_PRINTnone || qh PRINTsummary) {
    vertices= qh_pointvertex (/*qh facet_list*/);
    FORALLvertices {
      FOREACHneighbor_(vertex) {
	zinc_(Zverifypart);
	qh_distplane (vertex->point, neighbor, &dist);
	minimize_(minvertex, dist);
	if (-dist > qh TRACEdist
	    || neighbor == qh tracefacet || vertex == qh tracevertex)
	  fprintf (qh ferr, "qh_check_maxout: p%d (v%d) is %.2g below f%d\n",
		   qh_pointid (vertex->point), vertex->id, dist, neighbor->id);
      }
    }
    if (qh MERGING)
      wmin_(Wminvertex, qh min_vertex);
    qh min_vertex= minvertex;
  }
  FOREACHfacet_i_(facets) {
    if (True) { /* inside points can end up outside after merging */
      zinc_(Ztotverify);
      if (!facet)
        facet= qh facet_list;
      point= qh_point(facet_i);
      if (point == qh GOODpointp)
	continue;
      bestfacet= qh_findbest (point, facet, qh_ALL, 0,
			    &dist, &isoutside, &numpart);
      zadd_(Zverifypart, numpart);
      if (dist > maxoutside) {
        if (qh ONLYgood && !bestfacet->good 
        && !((bestfacet= qh_findgooddist (point, bestfacet, &dist))
             && dist > maxoutside))
          notgood++;
        else
	  maxoutside= dist;
      }
      if (!facet && dist > qh min_vertex) 
	qh_partitioncoplanar (point, bestfacet, &dist);
      if (dist > qh TRACEdist || bestfacet == qh tracefacet)
	  fprintf (qh ferr, "qh_check_maxout: p%d is %.2g above f%d\n",
		   qh_pointid (point), dist, bestfacet->id);
    }
  }
  qh_settempfree (&vertices);
  qh_settempfree (&facets);
  wmax_(Wmaxoutside, qh max_outside);
  qh max_outside= maxoutside;
  trace1((qh ferr, "qh_check_maxout: maxoutside %2.2g outside of not good %d\n",
       maxoutside, notgood));
} /* check_maxout */

/*----------------------------------------
-check_output- performs the checks at the end of qhull algorithm
  does not check points (may take a long time)
*/
void qh_check_output (void) {
  int i;

  if (qh STOPcone)
    return;
  if (qh VERIFYoutput | qh IStracing | qh CHECKfrequently) {
    qh_checkpolygon (qh facet_list);
    qh_checkflipped_all (qh facet_list);
    qh_checkconvex(qh facet_list, qh_ALGORITHMfault);
  }else if (!qh MERGING && qh_newstats (qhstat precision, &i)) {
    qh_checkflipped_all (qh facet_list);
    qh_checkconvex (qh facet_list, qh_ALGORITHMfault);
  }
} /* check_output */



/*-------------------------------------------------------------
-check_point- check that point is not outside facet
  if maxerror, doesn't report an error
*/
void qh_check_point (pointT *point, facetT *facet, realT *maxoutside, facetT **errfacet1, facetT **errfacet2) {
  realT dist;

  zinc_(Zverifypart);
  qh_distplane(point, facet, &dist);
  if (dist > *maxoutside) {
    *errfacet2= *errfacet1;
    *errfacet1= facet;
    fprintf(qh ferr, "qhull precision error: point p%d is outside facet f%d, distance = %6.8g maxoutside= %6.8g\n", 
	      qh_pointid(point), facet->id, dist, *maxoutside);
  }
} /* qh_check_point */


/*-------------------------------------------------
-check_points- checks that all points are inside all facets
     uses findbest if lots of points
     ignores flipped facets
*/
void qh_check_points (void) {
  facetT *facet, *errfacet1= NULL, *errfacet2= NULL;
  realT total, maxoutside;
  pointT *point, **pointp, *pointtemp;

  maxoutside= fmax_(qh max_outside, qh DISTround);
  maxoutside += 2* qh DISTround;
  /* 1 DISTround to actual point and another DISTround to computed point */
  if (qh RANDOMdist) /* repeated computations can differ by 2*distround */
    maxoutside += qh DISTround;
  trace1((qh ferr, "qh_check_points: check all points below %2.2g of all facet planes\n",
	  maxoutside));
  if (qh num_good)
     total= (float) qh num_good * qh num_points;
  else
     total= (float) qh num_facets * qh num_points;
  if (total >= qh_VERIFYdirect && !qh MERGING)  /* MERGING qh_check_maxout */
    qh_check_bestdist();
  else {
    if (!qh_QUICKhelp  && qh PRINTprecision) { /* miss counts other_points and !good facets */
      if (qh PREmerge && !qh KEEPcoplanar && zzval_(Zcoplanarpart) > 10)
	fprintf (qh ferr, "\n\
qhull input warning: pre-merging ('C-n' or 'A-n') without retaining coplanar\n\
points ('Qc').  Verify may report that an outer plane is incorrect.\n");
      fprintf (qh ferr, "\n\
Output completed.  Verifying that all points are below %2.2g of\n\
all %sfacets.  Will make %2.0f distance computations.\n", 
	       maxoutside, (qh ONLYgood ?  "good " : ""), total);
    }
    FORALLfacets {
      if (!facet->good && qh ONLYgood)
        continue;
      if (facet->flipped)
        continue;
#if qh_MAXoutside
      maxoutside= facet->maxoutside + 2* qh DISTround;
      /* 1 DISTround to actual point and another DISTround to computed point */
#endif
      FORALLpoints {
	if (point != qh GOODpointp)
	  qh_check_point (point, facet, &maxoutside, &errfacet1, &errfacet2);
      }
      FOREACHpoint_(qh other_points) {
	if (point != qh GOODpointp)
	  qh_check_point (point, facet, &maxoutside, &errfacet1, &errfacet2);
      }
    }

    if (errfacet1) qhull_fatal(36);

  }
} /* check_points */


/*-------------------------------------------------
-checkconvex- check that each ridge in facetlist is convex
returns:
    counts Zconcaveridges and Zcoplanarridges
    errors if concaveridge or if merging an coplanar ridge
note:
    optimized for simplicial facets
*/
void qh_checkconvex(facetT *facetlist, int fault) {
  facetT *facet, *neighbor, **neighborp, *errfacet1=NULL, *errfacet2=NULL;
  vertexT *vertex;
  realT dist;
  pointT *centrum;
  boolT waserror= False;
  int neighbor_i;

  trace1((qh ferr, "qh_checkconvex: check all ridges are convex\n"));
  zzval_(Zconcaveridges)= 0;
  zzval_(Zcoplanarridges)= 0;
  FORALLfacet_(facetlist) {
    if (facet->flipped) {
      fprintf (qh ferr, "qhull precision error: f%d is flipped (interior point is outside)\n",
	       facet->id);
      errfacet1= facet;
      waserror= True;
      continue;
    }
    if (!qh MERGING) {  /* simplicial facet */
      neighbor_i= 0;
      FOREACHneighbor_(facet) {
        vertex= (vertexT *)SETelem_(facet->vertices, neighbor_i++);
        zzinc_(Zdistconvex);
        qh_distplane (vertex->point, neighbor, &dist);
        if (dist > -qh DISTround) {

	  if (fault == qh_DATAfault) qhull_fatal(37);

          if (dist > qh DISTround) {
            zzinc_(Zconcaveridges);
            fprintf (qh ferr, "qhull precision error: f%d is concave to f%d, since p%d (v%d) is %6.4g above\n",
              facet->id, neighbor->id, qh_pointid(vertex->point), vertex->id, dist);
            errfacet1= facet;
            errfacet2= neighbor;
            waserror= True;
          }else {
            zzinc_(Zcoplanarridges);
            trace0((qh ferr, "qhull precision error: f%d may be coplanar to f%d, since p%d (v%d) is within %6.4g\n",
              facet->id, neighbor->id, qh_pointid(vertex->point), vertex->id, dist));
          }
        }
      }
    }else { /* qh MERGING */
      if (qh CENTERtype == qh_centrum) {
        if (!facet->center)
          facet->center= qh_getcentrum (facet);
        centrum= facet->center;
      }else 
        centrum= qh_getcentrum(facet);
      FOREACHneighbor_(facet) {
        zzinc_(Zdistconvex);
        qh_distplane (centrum, neighbor, &dist);
        if (dist > qh DISTround) {
          zzinc_(Zconcaveridges);
          fprintf (qh ferr, "qhull precision error: f%d is concave to f%d, since its centrum is %6.4g above\n",
            facet->id, neighbor->id, dist);
          errfacet1= facet;
          errfacet2= neighbor;
          waserror= True;
        }else if (dist >= 0.0) {   /* if arithmetic always rounds the same,
				     can test against centrum radius instead */
          zzinc_(Zcoplanarridges);
          fprintf (qh ferr, "qhull precision error: f%d is coplanar or concave to f%d, since its centrum is %6.4g above\n",
            facet->id, neighbor->id, dist);
	  errfacet1= facet;
	  errfacet2= neighbor;
	  waserror= True;
        }
      }
      if (qh CENTERtype != qh_centrum)
        qh_memfree(centrum, qh normal_size);
    }
  }

  if (waserror && !qh FORCEoutput) qhull_fatal(38);

} /* checkconvex */


/*-------------------------------------------------
-checkfacet- checks for consistency errors in facet
    vertex ids are inverse sorted
    unless newmerge, at least hull_dim neighbors and vertices (exactly if simplicial)
    if non-simplicial, at least as many ridges as neighbors
    neighbors are not duplicated
    ridges are not duplicated
    in 3-d, ridges=verticies
    (hull_dim-1) ridge vertices
    neighbors are reciprocated
    ridge neighbors are facet neighbors and a ridge for every neighbor
    simplicial neighbors match facetintersect
    vertex intersection matches vertices of common ridges 
    vertex neighbors and facet vertices agree
  sets waserror if any error occurs
  uses neighbor->seen
*/
void qh_checkfacet(facetT *facet, boolT newmerge, boolT *waserrorp) {
  facetT *neighbor, **neighborp, *errother=NULL;
  ridgeT *ridge, **ridgep, *errridge= NULL;
  vertexT *vertex, **vertexp;
  unsigned previousid= INT_MAX;
  int numneighbors, numvertices, numridges=0, numRvertices=0;
  boolT waserror= False;
  int skipA, skipB;
  setT *intersection;

  if (facet->visible) qhull_fatal(39);

  if (!facet->normal) {
    fprintf (qh ferr, "qhull internal error (checkfacet): facet f%d does not have  a normal\n",
      facet->id);
    waserror= True;
  }
  qh_setcheck (facet->vertices, "vertices for f", facet->id);
  qh_setcheck (facet->ridges, "ridges for f", facet->id);
  qh_setcheck (facet->outsideset, "outsideset for f", facet->id);
  qh_setcheck (facet->coplanarset, "coplanarset for f", facet->id);
  qh_setcheck (facet->neighbors, "neighbors for f", facet->id);
  FOREACHvertex_(facet->vertices) {
    if (vertex->deleted) {
      fprintf(qh ferr, "qhull internal error (checkfacet): deleted vertex v%d in f%d\n", vertex->id, facet->id);

      /*
      qh_errprint ("ERRONEOUS", NULL, NULL, NULL, vertex);
      NO LONGER IN SERVICE */

      waserror= True;
    }
    if (vertex->id >= previousid) {
      fprintf(qh ferr, "qhull internal error (checkfacet): vertices of f%d are not in descending id order at v%d\n", facet->id, vertex->id);
      waserror= True;
      break;
    }
    previousid= vertex->id;
  }
  numneighbors= qh_setsize(facet->neighbors);
  numvertices= qh_setsize(facet->vertices);
  numridges= qh_setsize(facet->ridges);
  if (facet->simplicial) {
    if (numvertices+numneighbors != 2*qh hull_dim && !qh_facetdegen(facet)) {
      fprintf(qh ferr, "qhull internal error (checkfacet): for simplicial facet f%d, #vertices %d + #neighbors %d != 2*qh hull_dim\n", 
                facet->id, numvertices, numneighbors);
      qh_setprint (qh ferr, "", facet->neighbors);
      waserror= True;
    }
  }else { /* non-simplicial */
    if (!newmerge 
     &&(numvertices < qh hull_dim || numneighbors < qh hull_dim)
     && !qh_facetdegen(facet)) {
      fprintf(qh ferr, "qhull internal error (checkfacet): for facet f%d, #vertices %d or #neighbors %d < qh hull_dim\n",
         facet->id, numvertices, numneighbors);
       waserror= True;
    }
    if (numridges < numneighbors
    ||(qh hull_dim == 3 && !qh NEWmerges && numvertices != numridges)
    ||(qh hull_dim == 2 && numridges + numvertices + numneighbors != 6)) {
      if (!qh_facetdegen(facet)) {
	fprintf(qh ferr, "qhull internal error (checkfacet): for facet f%d, #ridges %d < #neighbors %d or (3-d) != #vertices %d or (2-d) not all 2\n",
	    facet->id, numridges, numneighbors, numvertices);
	waserror= True;
      }
    }
  }
  FOREACHneighbor_(facet) {

    if (neighbor == qh_MERGEridge || neighbor == qh_DUPLICATEridge) 
      qhull_fatal(40);

    neighbor->seen= True;
  }
  FOREACHneighbor_(facet) {
    if (!qh_setin(neighbor->neighbors, facet)) {
      fprintf(qh ferr, "qhull internal error (checkfacet): facet f%d has neighbor f%d, but f%d does not have neighbor f%d\n",
	      facet->id, neighbor->id, neighbor->id, facet->id);
      errother= neighbor;
      waserror= True;
    }
    if (!neighbor->seen) {
      fprintf(qh ferr, "qhull internal error (checkfacet): facet f%d has a duplicate neighbor f%d\n",
	      facet->id, neighbor->id);
      errother= neighbor;
      waserror= True;
    }    
    neighbor->seen= False;
  }
  FOREACHridge_(facet->ridges) {
    qh_setcheck (ridge->vertices, "vertices for r", ridge->id);
    ridge->seen= False;
    if (ridge->mergeridge && !newmerge) {
      fprintf (qh ferr, "qhull internal error (checkfacet): mergeridge r%d still in facetlist\n", ridge->id);

      /*
      qh_errprint ("ERRONEOUS", NULL, NULL, ridge, NULL);
      NO LONGER IN SERVICE */

      waserror= True;
    }
  }
  FOREACHridge_(facet->ridges) {
    if (ridge->seen) {
      fprintf(qh ferr, "qhull internal error (checkfacet): facet f%d has a duplicate ridge r%d\n",
	      facet->id, ridge->id);
      errridge= ridge;
      waserror= True;
    }    
    ridge->seen= True;
    numRvertices= qh_setsize(ridge->vertices);
    if (numRvertices != qh hull_dim - 1) {
      fprintf(qh ferr, "qhull internal error (checkfacet): ridge between f%d and f%d has %d vertices\n", 
                ridge->top->id, ridge->bottom->id, numRvertices);
      errridge= ridge;
      waserror= True;
    }
    neighbor= otherfacet_(ridge, facet);
    neighbor->seen= True;
    if (!qh_setin(facet->neighbors, neighbor)) {
      fprintf(qh ferr, "qhull internal error (checkfacet): for facet f%d, neighbor f%d of ridge r%d not in facet\n",
           facet->id, neighbor->id, ridge->id);
      errridge= ridge;
      waserror= True;
    }
  }
  if (!facet->simplicial) {
    FOREACHneighbor_(facet) {
      if (!neighbor->seen) {
        fprintf(qh ferr, "qhull internal error (checkfacet): facet f%d does not have a ridge for neighbor f%d\n",
	      facet->id, neighbor->id);
	errother= neighbor;
        waserror= True;
      }
      intersection= qh_vertexintersect_new(facet->vertices, neighbor->vertices);
      qh_settemppush (intersection);
      FOREACHvertex_(facet->vertices) {
	vertex->seen= False;
	vertex->seen2= False;
      }
      FOREACHvertex_(intersection)
	vertex->seen= True;
      FOREACHridge_(facet->ridges) {
	if (neighbor != otherfacet_(ridge, facet))
	    continue;
	FOREACHvertex_(ridge->vertices) {

	  if (!vertex->seen) qhull_fatal(41);

	  vertex->seen2= True;
	}
      }
      if (!newmerge) {
	FOREACHvertex_(intersection) {
	  if (!vertex->seen2) {
	    if (qh IStracing >=3 || !qh MERGING) {
	      fprintf (qh ferr, "qhull precision error (checkfacet): vertex v%d in f%d intersect f%d but\n\
 not in a ridge.  This is ok under merging.  Last point was p%d\n",
		     vertex->id, facet->id, neighbor->id, qh furthest_id);

	      if (!qh FORCEoutput && !qh MERGING) qhull_fatal(42);

	    }
	  }
	}
      }      
      qh_settempfree (&intersection);
    }
  }else { /* simplicial */
    FOREACHneighbor_(facet) {
      if (neighbor->simplicial) {    
	skipA= SETindex_(facet->neighbors, neighbor);
	skipB= qh_setindex (neighbor->neighbors, facet);
	if (!qh_setequal_skip (facet->vertices, skipA, neighbor->vertices, skipB)) {
	  fprintf (qh ferr, "qhull internal error (checkfacet): facet f%d skip %d and neighbor f%d skip %d do not match \n",
		   facet->id, skipA, neighbor->id, skipB);
	  errother= neighbor;
	  waserror= True;
	}
      }
    }
  }
  
  if (waserror) {

    /*
    qh_errprint("ERRONEOUS", facet, errother, errridge, NULL);
    NO LONGER IN SERVICE */

    *waserrorp= True;
  }
} /* checkfacet */


/*-------------------------------------------------
-checkflipped- checks facet orientation to interior point
  tests against 0 if !allerror since tested against DISTround before
returns:
  False if flipped orientation (sets facet->flipped)
  distance if non-NULL
*/
boolT qh_checkflipped (facetT *facet, realT *distp, boolT allerror) {
  realT dist;

  if (facet->flipped && !distp)
    return False;
  zzinc_(Zdistcheck);
  qh_distplane(qh interior_point, facet, &dist);
  if (distp)
    *distp= dist;
  if ((allerror && dist > -qh DISTround)|| (!allerror && dist >= 0.0)) {
    facet->flipped= True;
    zzinc_(Zflippedfacets);
    trace0((qh ferr, "qh_checkflipped: facet f%d is flipped, distance= %6.12g\n",
              facet->id, dist));
    return False;
  }
  return True;
} /* checkflipped */

/*-------------------------------------------------
-checkflipped_all- checks orientation of facets in list against interior point
*/
void qh_checkflipped_all (facetT *facetlist) {
  facetT *facet;
  boolT waserror= False;
  realT dist;

  if (facetlist == qh facet_list)
    zzval_(Zflippedfacets)= 0;
  FORALLfacet_(facetlist) {
    if (!qh_checkflipped (facet, &dist, (boolT)!qh_ALL)) {
      fprintf(qh ferr, "qhull precision error: facet f%d is flipped, distance= %6.12g\n",
	      facet->id, dist);
      if (!qh FORCEoutput) {

	/*
	qh_errprint("ERRONEOUS", facet, NULL, NULL, NULL);
	NO LONGER IN SERVICE */

	waserror= True;
      }
    }
  }
  if (waserror) qhull_fatal(43);

} /* checkflipped_all */

/*-------------------------------------------------
-checkpolygon- checks the correctness of the structure
  check num_facets and num_vertices if qh facet_list
  call with either qh facet_list or qh newfacet_list
*/
void qh_checkpolygon(facetT *facetlist) {
  facetT *facet;
  vertexT *vertex, **vertexp, *vertexlist;
  int numfacets= 0, numvertices= 0, numridges= 0;
  boolT waserror= False, nextseen= False, visibleseen= False;
  
  trace1((qh ferr, "qh_checkpolygon: check all facets from f%d\n", facetlist->id));
  if (facetlist != qh facet_list || qh ONLYgood)
    nextseen= True;
  FORALLfacet_(facetlist) {
    if (facet == qh visible_list)
      visibleseen= True;
    if (!facet->visible) {
      if (!nextseen) {
	if (facet == qh facet_next)
	  nextseen= True;
	else 

	  if (qh_setsize (facet->outsideset)) qhull_fatal(44);

      }
      numfacets++;
      qh_checkfacet(facet, False, &waserror);
    }
  }

  if (qh visible_list && !visibleseen && facetlist == qh facet_list) 
    qhull_fatal(45);

  if (facetlist == qh facet_list)
    vertexlist= qh vertex_list;
  else if (facetlist == qh newfacet_list)
    vertexlist= qh newvertex_list;
  else
    vertexlist= NULL;
  FORALLvertex_(vertexlist) {
    vertex->seen= False;
    vertex->visitid= 0;
  }  
  FORALLfacet_(facetlist) {
    if (facet->visible)
      continue;
    if (facet->simplicial)
      numridges += qh hull_dim;
    else
      numridges += qh_setsize (facet->ridges);
    FOREACHvertex_(facet->vertices) {
      vertex->visitid++;
      if (!vertex->seen) {
	vertex->seen= True;
	numvertices++;
	if (qh_pointid (vertex->point) == -1) {
	  fprintf (qh ferr, "qhull internal error (checkpolygon): unknown point %x for vertex v%d first_point %x\n",
		   (int) vertex->point, vertex->id, (int) qh first_point);
	  waserror= True;
	}
      }
    }
  }
  qh vertex_visit += numfacets;
  if (facetlist == qh facet_list) {
    if (numfacets != qh num_facets - qh num_visible) {
      fprintf(qh ferr, "qhull internal error (checkpolygon): actual number of facets is %d, cumulative facet count is %d\n",
	      numfacets, qh num_facets- qh num_visible);
      waserror= True;
    }
    qh vertex_visit++;
    if (qh VERTEXneighbors) {
      FORALLvertices {
	qh_setcheck (vertex->neighbors, "neighbors for v", vertex->id);
        if ((int)vertex->visitid != qh vertex_visit && !vertex->deleted) {
          if ((int)vertex->visitid != qh_setsize (vertex->neighbors)) {
            fprintf (qh ferr, "qhull internal error (checkpolygon): v%d neighbors is wrong, actually in %d facets\n",
              vertex->id, vertex->visitid);

	    /*
            qh_errprint ("ERRONEOUS", NULL, NULL, NULL, vertex);
	    NO LONGER IN SERVICE */

	    waserror= True;
          }
        }
      }
    }
    if (numvertices != qh num_vertices - qh_setsize(qh del_vertices)) {
      fprintf(qh ferr, "qhull internal error (checkpolygon): actual number of vertices is %d, cumulative vertex count is %d\n",
	      numvertices, qh num_vertices - qh_setsize(qh del_vertices));
      waserror= True;
    }
    if (qh hull_dim == 2 && numvertices != numfacets) {
      fprintf (qh ferr, "qhull internal error (checkpolygon): #vertices %d != #facets %d\n",
        numvertices, numfacets);
      waserror= True;
    }
    if (qh hull_dim == 3 && numvertices + numfacets - numridges/2 != 2) {
      fprintf (qh ferr, "qhull internal error (checkpolygon): #vertices %d + #facets %d - #edges %d != 2\n",
        numvertices, numfacets, numridges/2);
      waserror= True;
    }
  }

  if (waserror) qhull_fatal(46);

} /* checkpolygon */


/*-------------------------------------------------
-checkvertex- check vertex for consistency
notes:
  neighbors checked efficiently in checkpolygon
*/
void qh_checkvertex (vertexT *vertex) {
  boolT waserror= False;
  facetT *neighbor, **neighborp, *errfacet=NULL;

  if (qh_pointid (vertex->point) == -1) {
    fprintf (qh ferr, "qhull internal error (checkvertex): unknown point id %x\n", (int) vertex->point);
    waserror= True;
  }
  if (vertex->id >= qh vertex_id) {
    fprintf (qh ferr, "qhull internal error (checkvertex): unknown vertex id %d\n", vertex->id);
    waserror= True;
  }
  if (!waserror && !vertex->deleted) {
    FOREACHneighbor_(vertex) {
      if (!qh_setin (neighbor->vertices, vertex)) {
	fprintf (qh ferr, "qhull internal error (checkvertex): neighbor f%d does not contain v%d\n", neighbor->id, vertex->id);
	errfacet= neighbor;
	waserror= True;
      }
    }
  }

  if (waserror) qhull_fatal(47);

} /* checkvertex */
  
/*-------------------------------------------------
-clearcenters- clear old data from facet->center
  sets new centertype
  nop if CENTERtype is the same
*/
void qh_clearcenters (int type) {
  facetT *facet;
  
  if (qh CENTERtype != type) {
    FORALLfacets {
      if (facet->center) {
        if (qh CENTERtype == qh_voronoi)
          qh_memfree (facet->center, qh center_size);
        else
          qh_memfree (facet->center, qh normal_size);
	facet->center= NULL;
      }
    }
    qh CENTERtype= type;
  }
  trace2((qh ferr, "clearcenters: switched to center type %d\n", type));
} /* clearcenters */

/*-------------------------------------------------
-clearnewvertices- clear vertices from newvertex_list
*/
void qh_clearnewvertices (void /*qh newvertex_list*/) {
  vertexT *vertex;
  
  FORALLvertex_(qh newvertex_list)
    vertex->newlist= False;
  qh newvertex_list= NULL;
} /* clearnewvertices */
    
/*-------------------------------------------------
-clearvisible- clear facets from visible list
  resets NEWfacets
*/
void qh_clearvisible (void /*qh visible_list*/) {
  facetT *visible;
  
  FORALLvisible_facets
    visible->visible= False;
  qh num_visible= 0;
  qh visible_list= NULL;
  qh NEWfacets= False;
} /* clearvisible */
    
/*----------------------------------------
-createsimplex- creates a simplex from a set of vertices
returns:
    initializes qh facet_list to the simplex
*/
void qh_createsimplex(setT *vertices) {
  facetT *facet= NULL, *newfacet;
  boolT toporient= True;
  int vertex_i, vertex_n, nth, tempint;
  setT *newfacets= qh_settemp (qh hull_dim+1);
  vertexT *vertex;
  
  qh facet_list= qh newfacet_list= qh facet_tail= qh_newfacet();
  qh vertex_list= qh newvertex_list= qh vertex_tail= qh_newvertex(NULL);
  FOREACHvertex_i_(vertices) {
    newfacet= qh_newfacet();
    newfacet->vertices= qh_setnew_delnthsorted (vertices, vertex_n,
						vertex_i, 0);
    newfacet->toporient= toporient;
    qh_appendfacet(newfacet);
    qh_appendvertex (vertex);
    qh_setappend (&newfacets, newfacet);
    tempint = (int)toporient;
    tempint ^= True;
    toporient = (boolT)tempint;
  }
  FORALLnew_facets {
    nth= 0;
    FORALLfacet_(qh newfacet_list) {
      if (facet != newfacet) 
        SETelem_(newfacet->neighbors, nth++)= facet;
    }
    qh_settruncate (newfacet->neighbors, qh hull_dim);
  }
  qh_settempfree (&newfacets);
  trace1((qh ferr, "qh_createsimplex: created simplex\n"));
} /* createsimplex */


/*-------------------------------------------------
-deletevisible- delete visible facets 
    ridges and vertices already deleted
    horizon facets do not reference facets on qh visible_list
    new facets in qh newfacet_list
returns:
    deletes each facet and removes from facetlist
    calls qh_clearvisible()
    uses qh visit_id;
*/
void qh_deletevisible (/*qh visible_list*/) {
  facetT *visible, *nextfacet;
  vertexT *vertex, **vertexp;
  int numvisible= 0;

  trace1((qh ferr, "qh_deletevisible: delete visible facets\n"));
  for (visible= qh visible_list; visible && visible->visible; 
                visible= nextfacet) { /* deleting current */
    nextfacet= visible->next;        
    numvisible++;
    qh_delfacet(visible);
  }

  if (numvisible != qh num_visible) qhull_fatal(48);

  FOREACHvertex_(qh del_vertices) 
    qh_delvertex (vertex);
  qh_settruncate (qh del_vertices, 0);
  qh_clearvisible();
} /* deletevisible */


/*-------------------------------------------------
-delfacet- removes facet from facet_list and frees up its memory
   assumes vertices and ridges already freed
*/
void qh_delfacet(facetT *facet) {
  void **freelistp;

  trace5((qh ferr, "qh_delfacet: delete f%d\n", facet->id));
  if (facet == qh tracefacet)
    qh tracefacet= NULL;
  qh_removefacet(facet);
  qh_memfree_(facet->normal, qh normal_size, freelistp);
  if (qh CENTERtype == qh_voronoi) {
    qh_memfree_(facet->center, qh center_size, freelistp);
  }else {
    qh_memfree_(facet->center, qh normal_size, freelistp);
  }
  qh_setfree(&(facet->neighbors));
  if (facet->ridges)
    qh_setfree(&(facet->ridges));
  qh_setfree(&(facet->vertices));
  if (facet->outsideset)
    qh_setfree(&(facet->outsideset));
  if (facet->coplanarset)
    qh_setfree(&(facet->coplanarset));
  qh_memfree_(facet, sizeof(facetT), freelistp);
} /* delfacet */


/*-------------------------------------------------
-delridge- deletes ridge from data structures it belongs to and frees up the
memory occupied by it
notes:
  in merge.c, caller sets vertex->delridge for each vertex
  also freed in qh_freeqhull
*/
void qh_delridge(ridgeT *ridge) {
  void **freelistp;
  
  qh_setdel(ridge->top->ridges, ridge);
  qh_setdel(ridge->bottom->ridges, ridge);
  qh_setfree(&(ridge->vertices));
  qh_memfree_(ridge, sizeof(ridgeT), freelistp);
} /* delridge */


/*-------------------------------------------------
-delvertex- deletes a vertex and frees its memory
  assumes vertex->adjacencies have been updated if needed
  unlinks for vertex_list
*/
void qh_delvertex (vertexT *vertex) {

  if (vertex == qh tracevertex)
    qh tracevertex= NULL;
  qh_removevertex (vertex);
  qh_setfree (&vertex->neighbors);
  qh_memfree(vertex, sizeof(vertexT));
} /* delvertex */


/*----------------------------------------
-facet3vertex- return temporary set of 3-d vertices
  in qh_ORIENTclock order
*/
setT *qh_facet3vertex (facetT *facet) {
  ridgeT *ridge, *firstridge;
  vertexT *vertex;
  int cntvertices, cntprojected=0;
  setT *vertices;

  cntvertices= qh_setsize(facet->vertices);
  vertices= qh_settemp (cntvertices);
  if (facet->simplicial) {

    if (cntvertices != 3) qhull_fatal(49);

    qh_setappend (&vertices, SETfirst_(facet->vertices));
    if (facet->toporient ^ qh_ORIENTclock)
      qh_setappend (&vertices, SETsecond_(facet->vertices));
    else
      qh_setaddnth (&vertices, 0, SETsecond_(facet->vertices));
    qh_setappend (&vertices, SETelem_(facet->vertices, 2));
  }else {
    ridge= firstridge= (ridgeT *)SETfirst_(facet->ridges);   /* no infinite */
    while ((ridge= qh_nextridge3d (ridge, facet, &vertex))) {
      qh_setappend (&vertices, vertex);
      if (++cntprojected > cntvertices || ridge == firstridge)
        break;
    }

    if (!ridge || cntprojected != cntvertices) qhull_fatal(50);

  }
  return vertices;
} /* facet3vertex */


/*-------------------------------------------------
-facetintersect- return vertices for intersection of two simplicial facets
  may include 1 prepended entry (if more, need to settemppush)
returns:
  returns set of hull_dim-1 + optional extra
  returns skipped index for each test and checks for exactly one
notes:
  does not need settemp since set in quick memory
  see also qh_vertexintersect and qh_vertexintersect_new
  use qh_setnew_delnthsorted to get nth ridge (no skip information)
*/
setT *qh_facetintersect (facetT *facetA, facetT *facetB,
			 int *skipA,int *skipB, int prepend) {
  setT *intersect;
  int dim= qh hull_dim, i, j;
  facetT **neighborsA, **neighborsB;

  neighborsA= SETaddr_(facetA->neighbors, facetT);
  neighborsB= SETaddr_(facetB->neighbors, facetT);
  i= j= 0;
  if (facetB == *neighborsA++)
    *skipA= 0;
  else if (facetB == *neighborsA++)
    *skipA= 1;
  else if (facetB == *neighborsA++)
    *skipA= 2;
  else {
    for (i= 3; i < dim; i++) {
      if (facetB == *neighborsA++) {
        *skipA= i;
        break;
      }
    }
  }
  if (facetA == *neighborsB++)
    *skipB= 0;
  else if (facetA == *neighborsB++)
    *skipB= 1;
  else if (facetA == *neighborsB++)
    *skipB= 2;
  else {
    for (j= 3; j < dim; j++) {
      if (facetA == *neighborsB++) {
        *skipB= j;
        break;
      }
    }
  }

  if (i >= dim || j >= dim) qhull_fatal(51);

  intersect= qh_setnew_delnthsorted (facetA->vertices, qh hull_dim, *skipA, prepend);
  trace4((qh ferr, "qh_facetintersect: f%d skip %d matches f%d skip %d\n",
	  facetA->id, *skipA, facetB->id, *skipB));
  return(intersect);
} /* facetintersect */


/*----------------------------------------
-gethash- return hashvalue for a set with firstindex and skipelem
  assumes at least firstindex+1 elements
  sum of elements does badly in high d
  assumes skipelem is NULL, in set, or part of hash
*/
unsigned qh_gethash (int hashsize, setT *set, int size, int firstindex, void *skipelem) {
  void **elemp= SETelemaddr_(set, firstindex, void);
  unsigned hash, elem;
  int i;

  switch (size-firstindex) {
  case 1:
    hash= (unsigned)(*elemp) - (unsigned) skipelem;
    break;
  case 2:
    hash= (unsigned)(*elemp) + (unsigned)elemp[1] - (unsigned) skipelem;
    break;
  case 3:
    hash= (unsigned)(*elemp) + (unsigned)elemp[1] + (unsigned)elemp[2]
      - (unsigned) skipelem;
    break;
  case 4:
    hash= (unsigned)(*elemp) + (unsigned)elemp[1] + (unsigned)elemp[2]
      + (unsigned)elemp[3] - (unsigned) skipelem;
    break;
  case 5:
    hash= (unsigned)(*elemp) + (unsigned)elemp[1] + (unsigned)elemp[2]
      + (unsigned)elemp[3] + (unsigned)elemp[4] - (unsigned) skipelem;
    break;
  case 6:
    hash= (unsigned)(*elemp) + (unsigned)elemp[1] + (unsigned)elemp[2]
      + (unsigned)elemp[3] + (unsigned)elemp[4]+ (unsigned)elemp[5]
      - (unsigned) skipelem;
    break;
  default:
    hash= 0;
    i= 3;
    do {     /* this is about 10% in 10-d */
      if ((elem= (unsigned)*elemp++) != (unsigned)skipelem) {
        hash ^= (elem << i) + (elem >> (32-i));
	i += 3;
	if (i >= 32)
	  i -= 32;
      }
    }while(*elemp);
    break;
  }
  hash %= (unsigned) hashsize;
  /* hash= 0;   for debugging purposes */
  return hash;
} /* gethash */

/*-------------------------------------------------
-isvertex- returns vertex if point is in vertex set, else returns NULL
*/
vertexT *qh_isvertex (pointT *point, setT *vertices) {
  vertexT *vertex, **vertexp;

  FOREACHvertex_(vertices) {
    if (vertex->point == point)
      return vertex;
  }
  return NULL;
} /* isvertex */


/*-------------------------------------------------
-makenewfacet- creates a toporient? facet from vertices and apex
   modifies vertices 
returns:
    adds newfacet to qh facet_list 
       facet->neighbor= horizon, but not vice versa
    facet->vertices= vertices= apex+vertices
    facet->hyperplane defined
    newvertex_list updated
*/
facetT *qh_makenewfacet(setT *vertices, boolT toporient,facetT *horizon) {
  facetT *newfacet;
  vertexT *vertex, **vertexp;

  FOREACHvertex_(vertices) {
    if (!vertex->newlist) {
      vertex->newlist= True;
      qh_removevertex (vertex);
      qh_appendvertex (vertex);
    }
  }
  newfacet= qh_newfacet();
  newfacet->vertices= vertices;
  newfacet->toporient= toporient;
  qh_setfacetplane (newfacet);
  qh_setappend(&(newfacet->neighbors), horizon);
  qh_appendfacet(newfacet);
  return(newfacet);
} /* makenewfacet */


/*-------------------------------------------------
-makenewfacets- make new facets from point and qh visible_list
returns:
  qh newfacet_list= list of new facets with hyperplanes and id >= newfacet_id
  qh newvertex_list= list of vertices in new facets with 'new' set
  if (qh ONLYgood)
    newfacets reference horizon facets, but not vice versa
    ridges reference non-simplicial horizon ridges, but not vice versa
    does not change existing facets
  otherwise
    newfacets attached to horizon facets and ridges
    first neighbor of visible facet is corresponding new facet
*/
vertexT *qh_makenewfacets (pointT *point /*visible_list*/) {
  facetT *visible, *newfacet= NULL, *newfacet2= NULL, *neighbor, **neighborp;
  vertexT *apex;
  int numnew=0;

  qh newfacet_list= qh facet_tail;
  qh newvertex_list= qh vertex_tail;
  qh newfacet_id= qh facet_id;
  apex= qh_newvertex(point);
  apex->newlist= True;
  qh_appendvertex (apex);  
  qh visit_id++;
  if (!qh ONLYgood)
    qh NEWfacets= True;
  FORALLvisible_facets {
    FOREACHneighbor_(visible) 
      neighbor->seen= False;
    if (visible->ridges) {
      visible->visitid= qh visit_id;
      newfacet2= qh_makenew_nonsimplicial (visible, apex, &numnew);
    }
    if (visible->simplicial)
      newfacet= qh_makenew_simplicial (visible, apex, &numnew);
    if (!qh ONLYgood) {
      if (newfacet2)
        newfacet= newfacet2;
      if (!newfacet)
        zinc_(Zinsidevisible);
      SETfirst_(visible->neighbors)= newfacet;
      SETsecond_(visible->neighbors)= NULL;
    }
  }
  trace1((qh ferr, "qh_makenewfacets: created %d new facets from point p%d to horizon\n",
	  numnew, qh_pointid(point)));

  /*
  if (qh IStracing >= 4)
    qh_printfacetlist (qh newfacet_list, NULL, qh_ALL);
    THIS IS NO LONGER IN SERVICE */

  return apex;
} /* makenewfacets */

/*---------------------------------------------
-makenew_nonsimplicial- make new facets for ridges of visible facets
  qh visit_id if visible has already been seen
  attaches new facets if !qh ONLY good
  assumes all 'seen' flags false
returns:
  newfacet or NULL, bumps numnew as needed
  marks ridge neighbors for simplicial visible
  if (qh ONLYgood)
    ridges on newfacet, horizon, and visible
  else
    ridge and neighbors between newfacet and horizon
    visible facet's ridges are deleted    
*/
facetT *qh_makenew_nonsimplicial (facetT *visible, vertexT *apex, int *numnew) {
  void **freelistp;
  ridgeT *ridge, **ridgep;
  facetT *neighbor, *newfacet= NULL;
  setT *vertices;
  boolT toporient = False;

  FOREACHridge_(visible->ridges) {
    neighbor= otherfacet_(ridge, visible);
    if (neighbor->visible) {
      if (!qh ONLYgood) {
        if ((int)neighbor->visitid == qh visit_id) {
          qh_setfree (&(ridge->vertices));  /* delete on 2nd visit */
	  qh_memfree_(ridge, sizeof(ridgeT), freelistp);
	}
      }
    }else {  /* neighbor is an horizon facet */
      toporient= (boolT)(ridge->top == visible);
      vertices= qh_setnew (qh hull_dim); /* makes sure this is quick */
      qh_setappend (&vertices, apex);
      qh_setappend_set (&vertices, ridge->vertices);
      newfacet= qh_makenewfacet(vertices, toporient, neighbor);
      (*numnew)++;
      if (qh ONLYgood) {
        if (!neighbor->simplicial)
 	  qh_setappend(&(newfacet->ridges), ridge);
      }else {  /* qh_attachnewfacets */
        if (neighbor->seen) {

	  if (neighbor->simplicial) qhull_fatal(52);

	  qh_setappend (&(neighbor->neighbors), newfacet);
	}else
          qh_setreplace (neighbor->neighbors, visible, newfacet);
        if (neighbor->simplicial) {
          qh_setdel (neighbor->ridges, ridge);
          qh_setfree (&(ridge->vertices)); 
	  qh_memfree (ridge, sizeof(ridgeT));
	}else {
 	  qh_setappend(&(newfacet->ridges), ridge);
 	  if (toporient)
 	    ridge->top= newfacet;
 	  else
 	    ridge->bottom= newfacet;
 	}
      trace4((qh ferr, "qh_makenew_nonsimplicial: created facet f%d from v%d and r%d of horizon f%d\n",
	    newfacet->id, apex->id, ridge->id, neighbor->id));
      }
    }
    neighbor->seen= True;        
  } /* for each ridge */
  if (!qh ONLYgood)
    SETfirst_(visible->ridges)= NULL;
  return newfacet;
} /* makenew_nonsimplicial */

/*---------------------------------------------
-makenew_simplicial- make new facets for simplicial facet
  uses 'seen' flag
  attaches new facets if !qh ONLY good
returns:
  newfacet or NULL, bumps numnew as needed
  if (!qh ONLYgood)
    neighbors between newfacet and horizon
*/
facetT *qh_makenew_simplicial (facetT *visible, vertexT *apex, int *numnew) {
  facetT *neighbor, **neighborp, *newfacet= NULL;
  setT *vertices;
  boolT flip, toporient;
  int horizonskip, visibleskip;

  FOREACHneighbor_(visible) {
    if (!neighbor->seen && !neighbor->visible) {
      vertices= qh_facetintersect(neighbor,visible, &horizonskip, &visibleskip, 1);
      SETfirst_(vertices)= apex;
      flip= (boolT) ((horizonskip & 0x1) ^ (visibleskip & 0x1));    
      toporient=(boolT)( neighbor->toporient ^ !(horizonskip & 0x1) );
      newfacet= qh_makenewfacet(vertices, toporient, neighbor);
      (*numnew)++;
      if (!qh ONLYgood)
        SETelem_(neighbor->neighbors, horizonskip)= newfacet;
      trace4((qh ferr, "qh_makenew_simplicial: create facet f%d top %d from v%d and horizon f%d skip %d top %d and visible f%d skip %d, flip? %d\n",
	    newfacet->id, toporient, apex->id, neighbor->id, horizonskip,
	      neighbor->toporient, visible->id, visibleskip, flip));
    }
  }
  return newfacet;
} /* makenew_simplicial */

/*-------------------------------------------------
-matchduplicate- try to match an unmatched duplicated ridge
returns:
  True if a match was made
notes:
  found matching facet for a duplicate ridge.
     1) if keep already found, merge this pair
     2) if !flip&!flip, keep this
     3) if flip & flip, merge this pair
     4) otherwise, return False and continue
*/
boolT qh_matchduplicate (facetT *facet, int skip, facetT *newfacet, int newskip, boolT keepfound) {
  boolT ismatched= False;
           
  trace2((qh ferr, "qh_matchduplicate: duplicated f%d skip %d matches new f%d skip %d.  ",
           facet->id, skip, newfacet->id, newskip));
  if (keepfound) {
    SETelem_(facet->neighbors, skip)= newfacet;
    if (qh PREmerge)
      SETelem_(newfacet->neighbors, newskip)= qh_MERGEridge;
    else
      SETelem_(newfacet->neighbors, newskip)= facet;
    trace2((qh ferr, "Merge this pair.\n"));
    ismatched= True;
  }else if (!facet->flipped && !newfacet->flipped) {
    SETelem_(facet->neighbors, skip)= newfacet;
    SETelem_(newfacet->neighbors, newskip)= facet;
    trace2((qh ferr, "Match good pair\n"));
    ismatched= True;
  }else if (facet->flipped && newfacet->flipped) {
    SETelem_(facet->neighbors, skip)= newfacet;
    if (qh PREmerge)
      SETelem_(newfacet->neighbors, newskip)= qh_MERGEridge;
    else
      SETelem_(newfacet->neighbors, newskip)= facet;
    trace2((qh ferr, "Merge flipped pair\n"));
    ismatched= True;
  }else
    trace2((qh ferr, "No match, continue\n"));
  if (qh IStracing >= 4) {

    /*
    qh_errprint ("OLD/NEW", facet, newfacet, NULL, NULL);
    NO LONGER IN SERVICE */

    qh_setprint (qh ferr, "facet's neighbors", facet->neighbors);
    qh_setprint (qh ferr, "newfacet's", newfacet->neighbors);
  }
  return ismatched;
} /* matchduplicate */

/*--------------------------------------------
-matchmatch- try to match duplicate matching pair and newfacet
returns:
  True if existing match is ok and continue with newfacet
  False if need to place matchfacet
    newmatched set if newfacet matched
notes:        
  1) if pair is !flip&!flip, keep is found and continue
  2) if pair is flip&flip, merge them and continue
  3) otherwise, replace matchfacet
       if new pair is !flip&!flip, keep is found
       if new pair is flip&flip, merge them
       otherwise, mark duplicated and continue
*/
boolT qh_matchmatch (facetT *facet, int skip, facetT *matchfacet, facetT *newfacet, 
     int newskip, boolT ismatch, boolT *keepfound, boolT *newmatched) {
  int matchskip;
  boolT ismatched= False;

  matchskip= qh_setindex (matchfacet->neighbors, facet);
  trace2((qh ferr, "qh_matchmatch: duplicated f%d skip %d matches f%d skip %d and new f%d skip %d.  ",
    facet->id, skip, matchfacet->id, matchskip, newfacet->id, newskip));
  if (!facet->flipped && !matchfacet->flipped) {
    *keepfound= True;
    trace2((qh ferr, "Keep good match\n"));
    ismatched= True;
  }else if (facet->flipped && matchfacet->flipped) {
    if (matchskip >= 0 && qh PREmerge)
      SETelem_(matchfacet->neighbors, matchskip)= qh_MERGEridge;
    trace2((qh ferr, "Both flipped, merge\n"));
    ismatched= True;
  }else {
    /* matched facets have opposite orientations, undo matchfacet */
    SETelem_(matchfacet->neighbors, matchskip)= qh_DUPLICATEridge;
    if (ismatch && (facet->flipped == newfacet->flipped)) {
      SETelem_(facet->neighbors, skip)= newfacet;
      if (newfacet->flipped && qh PREmerge)
	SETelem_(newfacet->neighbors, newskip)= qh_MERGEridge;
      else
	SETelem_(newfacet->neighbors, newskip)= facet;
      trace2((qh ferr, "Substitute new\n"));
      *newmatched= True;
    }else {
      SETelem_(facet->neighbors, skip)= qh_DUPLICATEridge;
      *newmatched= False;
      trace2((qh ferr, "Undo match\n"));
    }
  }
  if (qh IStracing >= 4) {

    /*
    qh_errprint ("OLD/MATCH", facet, matchfacet, NULL, NULL);
    qh_errprint ("NEW", newfacet, NULL, NULL, NULL);
    NO LONGER IN SERVICE */

    qh_setprint (qh ferr, "facet's neighbors", facet->neighbors);
    qh_setprint (qh ferr, "matchfacet's", matchfacet->neighbors);
    qh_setprint (qh ferr, "newfacet's", newfacet->neighbors);
  }
  return ismatched;
} /* qh_matchmatch */
      
/*-------------------------------------------------
-matchneighbor- match subridge of newfacet with neighbor or add to hash_table
  ridge is newfacet->vertices w/o newskip vertex
  flipped tested if PREmerge or FORCEoutput
  use matchall to match unmatched duplicates
returns:
  at end of matching duplicate ridges,
    one !flipped,!flipped matched
    flipped,flipped matched with MERGEridge back pointers
    rest are unmatched, with a DUPLICATEridge link
    all facets involved have seen set and flip tested
notes:
  do not allocate memory (need to free hash_table cleanly)
  matches flipped,flipped since these get merged anyway
  uses linear hash chains
*/
void qh_matchneighbor (facetT *newfacet, int newskip, int hashsize, int *hashcount, boolT matchall) {
  boolT keepfound= False;  /* True, if !flip,!flip duplicate ridge found */
  boolT duplicated= False; /* True, if duplicate ridge detected */
  boolT newfound= False;   /* True, if new facet is already in hash chain */
  boolT same, ismatch, newmatched;
  unsigned hash, scan;
  facetT *facet, *matchfacet;
  int skip;

  hash= qh_gethash (hashsize, newfacet->vertices, qh hull_dim, 1, 
                     SETelem_(newfacet->vertices, newskip));
  trace4((qh ferr, "qh_matchneighbor: newfacet f%d skip %d hash %d hashcount %d\n",
	  newfacet->id, newskip, hash, *hashcount));
  zinc_(Zhashlookup);
  while ((facet= (facetT *)SETelem_(qh hash_table, hash))) {
    if (facet == newfacet) {
      newfound= True;
      goto LABELnexthash;
    }
    zinc_(Zhashtests);
    if (qh_matchvertices (1, newfacet->vertices, newskip, facet->vertices, &skip, &same)) {

      if (SETelem_(newfacet->vertices, newskip) == 
          SETelem_(facet->vertices, skip)) 
	qhull_fatal(53);

      ismatch=
	(boolT) (same == (int)(newfacet->toporient ^ facet->toporient));
      matchfacet= (facetT *)SETelem_(facet->neighbors, skip);
      if (matchfacet == qh_MERGEridge) 
        goto LABELnexthash;
      else if (matchfacet == qh_DUPLICATEridge) {
        duplicated= True;
        matchfacet= NULL;
      }else if (ismatch && !matchfacet && !duplicated) {
        SETelem_(facet->neighbors, skip)= newfacet;
        SETelem_(newfacet->neighbors, newskip)= facet;
        (*hashcount)--;
        trace4((qh ferr, "qh_matchneighbor: f%d skip %d matched with new f%d skip %d\n",
           facet->id, skip, newfacet->id, newskip));
        return;
      }

      if (!qh PREmerge) qhull_fatal(54);

      if (matchall) {
	if (matchfacet || !ismatch)
	  goto LABELnexthash;
	SETelem_(facet->neighbors, skip)= newfacet;
	SETelem_(newfacet->neighbors, newskip)= qh_MERGEridge;
	/* this may merge one more ridge than necessary, but hard to detect */
	(*hashcount) -= 2;  /* removed two unmatched facets */
        zzinc_(Zmultiflip);
	trace2((qh ferr, "qh_matchneighbor: duplicate f%d skip %d matched with new f%d skip %d\n",
             facet->id, skip, newfacet->id, newskip));
	return;
      }
      duplicated= True;
      newfacet->seen= True;
      facet->seen= True;
      if (matchfacet)
        matchfacet->seen= True;
      if (!matchfacet && ismatch) { 
        if (qh_matchduplicate (facet, skip, newfacet, newskip, keepfound)) {
          zzinc_(Zmultimatch);
          (*hashcount)--;
          return;
        }
      }else if (matchfacet && !keepfound) {
        if (!qh_matchmatch (facet, skip, matchfacet, newfacet, newskip, 
                   ismatch, &keepfound, &newmatched)) {
          scan= hash;
          while ((facet= (facetT *)SETelem_(qh hash_table, scan))) {
            if (facet == matchfacet)
              break;
            if ((int)(++scan) >= hashsize)
              scan= 0;
          }
          if (!facet)
            SETelem_(qh hash_table, scan)= matchfacet;
	  (*hashcount)++;
          if (newmatched)
            return;
          else
            (*hashcount)++; 
        }
      }
    }
LABELnexthash:
    if ((int)(++hash) >= hashsize)  
      hash= 0;
  /* loop terminates because qh_HASHfactor >= 1.1 by qh_initbuffers */
  }
  (*hashcount)++;
  if (!newfound) 
    SETelem_(qh hash_table, hash)= newfacet;
  if (duplicated) {
    SETelem_(newfacet->neighbors, newskip)= qh_DUPLICATEridge;
    trace4((qh ferr, "qh_matchneighbor: no match for duplicated f%d skip %d at hash %d\n",
           newfacet->id, newskip, hash));
  }else
    trace4((qh ferr, "qh_matchneighbor: no match for f%d skip %d at hash %d\n",
           newfacet->id, newskip, hash));
} /* matchneighbor */


/*-------------------------------------------------
-matchnewfacets- match newfacets in qh newfacet_list to their newfacet neighbors
  newfacets already have neighbor[0] (horizon facet)
  assumes qh hash_table is NULL
returns:
  qh newfacet_list with full neighbor sets
    get vertices with nth neighbor by deleting nth vertex
  if PREmerge or FORCEoutput 
    all facets check for flipped (also prevents point partitioning)
  if duplicate ridges and PREmerge
    facet->seen set
    missing neighbor links identifies extra ridges to be merging
notes:
  do not allocate memory after hash_table (need to free it cleanly)
*/
void qh_matchnewfacets (void) {
  int numnew=0, numfree= 0, hashcount=0, newskip, nth=0;
  facetT *newfacet, *neighbor, **neighborp, *facet;
  int facet_i, facet_n, dim= qh hull_dim, hashsize;
  setT *neighbors;
  
  trace1((qh ferr, "qh_matchnewfacets: match neighbors for new facets.\n"));
  FORALLnew_facets {
    numnew++;
    {  /* qh_setzero (newfacet->neighbors, 1, qh hull_dim); */
      neighbors= newfacet->neighbors;
      neighbors->e[neighbors->maxsize]= (void *)(dim+1); /*may be overwritten*/
      memset ((char *)&neighbors->e[1], 0, dim * sizeof(void *));
    }    
    if (qh MERGING)
      qh_checkflipped (newfacet, NULL, qh_ALL);
  }
  if (qh FORCEoutput && !qh MERGING)
    qh_checkflipped_all (qh newfacet_list);  /* prints warnings for flipped */
  qh_newhashtable (numnew*(qh hull_dim-1)); /* twice what is normally needed,
                                     but every ridge could be DUPLICATEridge */
  hashsize= qh_setsize (qh hash_table);
  FORALLnew_facets {
    newfacet->seen= False;
    for (newskip=1; newskip<qh hull_dim; newskip++) /* furthest/horizon already matched */
      qh_matchneighbor (newfacet, newskip, hashsize, &hashcount, 
			(boolT)!qh_ALL);
#if 0   /* use the following to trap hashcount errors */
    {
      int count= 0, k;
      facetT *facet, *neighbor;

      count= 0;
      FORALLfacet_(qh newfacet_list) {  /* newfacet already in use */
	for (k=1; k<qh hull_dim; k++) {
	  neighbor= SETelem_(facet->neighbors, k);
	  if (!neighbor || neighbor == qh_DUPLICATEridge)
	    count++;
	}
	if (facet == newfacet)
	  break;
      }

      if (count != hashcount) qhull_fatal(55);

    }
#endif  /* end of trap code */
  }
  if (hashcount) {
    FORALLnew_facets {
      if (newfacet->seen) {
        nth= 0;
        FOREACHneighbor_(newfacet) {
          if (neighbor == qh_DUPLICATEridge) {
	    trace2((qh ferr, "qh_matchnewfacets: find good/flip match for duplicated f%d skip %d\n",
		    newfacet->id, nth));
            qh_matchneighbor (newfacet, nth, hashsize, &hashcount, qh_ALL);

	    /*
	    if (qh IStracing >= 4)
	      qh_errprint ("DUPLICATED/MATCH", newfacet,
		  (facetT*)SETelem_(newfacet->neighbors, nth), NULL, NULL);
	    NO LONGER IN SERVICE */
         	    /* this may report MERGEfacet */
	  }
          nth++;
        }
      }
    }
  }

  if (hashcount) qhull_fatal(56);

  if (qh IStracing >= 2) {
    FOREACHfacet_i_(qh hash_table) {
      if (!facet)
        numfree++;
    }
    fprintf (qh ferr, "qh_matchnewfacets: %d new facets, %d unused hash entries .  hashsize %d\n",
	     numnew, numfree, qh_setsize (qh hash_table));
  }
  qh_setfree (&qh hash_table);

  /*
  if (qh IStracing >= 4 && qh PREmerge)
    qh_printfacetlist (qh newfacet_list, NULL, qh_ALL);
    THIS IS NO LONGER IN SERVICE */

} /* matchnewfacets */

    
/*----------------------------------------
-matchvertices- tests whether vertices match with a single skip
  starts match at firstindex since all new facets have a common vertex
  assumes skipA is in A and both sets are the same size
returns:
  skip index
  sets same iff vertices have the same orientation
*/
boolT qh_matchvertices (int firstindex, setT *verticesA, int skipA, 
       setT *verticesB, int *skipB, boolT *same) {
  vertexT **elemAp, **elemBp, **skipBp=NULL, **skipAp;

  elemAp= SETelemaddr_(verticesA, firstindex, vertexT);
  elemBp= SETelemaddr_(verticesB, firstindex, vertexT);
  skipAp= SETelemaddr_(verticesA, skipA, vertexT);
  do if (elemAp != skipAp) {
    while (*elemAp != *elemBp++) {
      if (skipBp)
        return False;
      skipBp= elemBp;  /* one extra like FOREACH */
    }
  }while(*(++elemAp));
  if (!skipBp)
    skipBp= ++elemBp;
  *skipB= (int)SETindex_(verticesB, skipB);
  *same= (boolT)(!(((unsigned)skipA & 0x1) ^ ((unsigned)*skipB & 0x1)));
  trace4((qh ferr, "qh_matchvertices: matched by skip %d (v%d) and skip %d (v%d) same? %d\n",
	  skipA, (*skipAp)->id, *skipB, (*(skipBp-1))->id, *same));
  return (True);
} /* matchvertices */

/*----------------------------------------
-nextridge3d- return next ridge and vertex for a 3d facet
  in qh_ORIENTclock order
  n^2 implementation to trace all ridges
  be sure to stop on any 2nd visit
*/
ridgeT *qh_nextridge3d (ridgeT *atridge, facetT *facet, vertexT **vertexp) {
  vertexT *atvertex, *vertex, *othervertex;
  ridgeT *ridge, **ridgep;

  if ((atridge->top == facet) ^ qh_ORIENTclock)
    atvertex= (vertexT *)SETsecond_(atridge->vertices);
  else
    atvertex= (vertexT *)SETfirst_(atridge->vertices);
  FOREACHridge_(facet->ridges) {
    if (ridge == atridge)
      continue;
    if ((ridge->top == facet) ^ qh_ORIENTclock) {
      othervertex= (vertexT *)SETsecond_(ridge->vertices);
      vertex= (vertexT *)SETfirst_(ridge->vertices);
    }else {
      vertex= (vertexT *)SETsecond_(ridge->vertices);
      othervertex= (vertexT *)SETfirst_(ridge->vertices);
    }
    if (vertex == atvertex) {
      if (vertexp)
        *vertexp= othervertex;
      return ridge;
    }
  }
  return NULL;
} /* nextridge3d */

  
/*----------------------------------------
-newfacet- creates and allocates space for a facet
returns:
    all fields initialized or cleared (NULL)
    preallocates neighbors
*/
facetT *qh_newfacet(void) {
  facetT *facet = NULL;
  void **freelistp;
  
  facetT_qh_memalloc_(sizeof(facetT), freelistp, facet);
  memset ((char *)facet, 0, sizeof(facetT));
  if (qh facet_id == 0xFFFFFF) {
    fprintf(qh ferr, "\
qhull warning: more than %d facets.  Id field overflows and two facets\n\
may have the same identifier.  Otherwise output ok.\n", 0xFFFFFF);
  }
  if (qh facet_id == (int)qh tracefacet_id)
    qh tracefacet= facet;
  facet->id= qh facet_id++;
  facet->neighbors= qh_setnew(qh hull_dim);
#if !qh_COMPUTEfurthest
  facet->furthestdist= 0.0;
#endif
#if qh_MAXoutside
  if (qh FORCEoutput && qh APPROXhull)
    facet->maxoutside= qh MINoutside;
  else
    facet->maxoutside= qh DISTround;
#endif
  facet->simplicial= True;
  facet->good= True;
  trace4((qh ferr, "qh_newfacet: created facet f%d\n", facet->id));
  return (facet);
} /* newfacet */


/*-------------------------------------------------
-newhashtable- returns size of qh hash_table of at least newsize slots
  assumes qh hash_table is NULL
  qh_HASHfactor determines the number of extra slots
*/
int qh_newhashtable(int newsize) {
  int size;

  size= ((newsize+1)*qh_HASHfactor) | 0x1;  /* odd number */
  while (True) { 
    if ((size%3) && (size%5))
      break;
    size += 2;
    /* loop terminates because there is an infinite number of primes */
  }
  qh hash_table= qh_setnew (size);
  qh_setzero (qh hash_table, 0, size);
  return size;
} /* newhashtable */

/*----------------------------------------
-newridge- creates and allocates space for a ridge
*/
ridgeT *qh_newridge(void) {
  ridgeT *ridge = NULL;
  void **freelistp;

  ridgeT_qh_memalloc_(sizeof(ridgeT), freelistp, ridge);
  memset ((char *)ridge, 0, sizeof(ridgeT));
  zinc_(Ztotridges);
  if (qh ridge_id == 0xFFFFFF) {
    fprintf(qh ferr, "\
qhull warning: more than %d ridges.  Id field overflows and two ridges\n\
may have the same identifier.  Otherwise output ok.\n", 0xFFFFFF);
  }
  ridge->id= qh ridge_id++;     
  trace4((qh ferr, "qh_newridge: created ridge r%d\n", ridge->id));
  return (ridge);
} /* newridge */


/*----------------------------------------
-newvertex- creates and allocates space for a vertex
*/
vertexT *qh_newvertex(pointT *point) {
  vertexT *vertex;

  zinc_(Ztotvertices);
  vertex= (vertexT *)qh_memalloc(sizeof(vertexT));
  memset ((char *) vertex, 0, sizeof (vertexT));

  if (qh vertex_id == 0xFFFFFF) qhull_fatal(57);

  if (qh vertex_id == qh tracevertex_id)
    qh tracevertex= vertex;
  vertex->id= qh vertex_id++;
  vertex->point= point;
  trace4((qh ferr, "qh_newvertex: vertex p%d (v%d) created\n", qh_pointid(vertex->point), 
	  vertex->id));
  return (vertex);
} /* newvertex */

/*-------------------------------------------------
-point- return point for a point id, or NULL if unknown
*/
pointT *qh_point (int id) {

  if (id < 0)
    return NULL;
  if (id < qh num_points)
    return ((pointT *)((unsigned)qh first_point+(unsigned)((id)*qh normal_size)));
  id -= qh num_points;
  if (id < qh_setsize (qh other_points))
    return (pointT *)SETelem_(qh other_points, id);
  return NULL;
} /* point */
  
/*-------------------------------------------------
-point_add- access function for pointfacet and pointvertex
*/
void qh_point_add (setT *set, pointT *point, void *elem) {
  int id;
  
  if ((id= qh_pointid(point)) == -1)
    fprintf (qh ferr, "qhull internal warning (pointfacet,pointvertex): unknown point %ux\n", 
      (unsigned) point);
  else 
    SETelem_(set, id)= elem;
} /* point_add */


/*-------------------------------------------------
-pointfacet- return temporary set of facets indexed by point id
  for vertices, coplanarset, and outsideset
  access with FOREACHfacet_i_(facets) and SETelem_(facets, i)
  NULL if no facet for point (inside)
     this will include qh GOODpointp
*/
setT *qh_pointfacet (void /*qh facet_list*/) {
  int numpoints= qh num_points + qh_setsize (qh other_points);
  setT *facets;
  facetT *facet;
  vertexT *vertex, **vertexp;
  pointT *point, **pointp;
  
  facets= qh_settemp (numpoints);
  qh_setzero (facets, 0, numpoints);
  qh vertex_visit++;
  FORALLfacets {
    FOREACHvertex_(facet->vertices) {
      if ((int)vertex->visitid != qh vertex_visit) {
        vertex->visitid= qh vertex_visit;
        qh_point_add (facets, vertex->point, facet);
      }
    }
    FOREACHpoint_(facet->coplanarset) 
      qh_point_add (facets, point, facet);
    FOREACHpoint_(facet->outsideset) 
      qh_point_add (facets, point, facet);
  }
  return facets;
} /* pointfacet */

/*-------------------------------------------------
-pointid- return id for a point, -3 if null, -2 if interior, or -1 if not known
*/
int qh_pointid (pointT *point) {
  unsigned id;

  if (!point)
    return -3;
  id= ((unsigned) point - (unsigned) qh first_point)/qh normal_size;
  if ((int)id >= qh num_points) {
    if (point == qh interior_point)
      id= -2;
    else if ((int)(id= qh_setindex (qh other_points, point)) != -1)
      id += qh num_points;
  }
  return (int) id;
} /* pointid */
  
/*-------------------------------------------------
-pointvertex- return temporary set of vertices indexed by point id
  access with FOREACHvertex_i_(vertices) and SETelem_(vertices, i)
  NULL if no vertex for point
     this will include qh GOODpointp
*/
setT *qh_pointvertex (void /*qh facet_list*/) {
  int numpoints= qh num_points + qh_setsize (qh other_points);
  setT *vertices;
  vertexT *vertex;
  
  vertices= qh_settemp (numpoints);
  qh_setzero (vertices, 0, numpoints);
  FORALLvertices 
    qh_point_add (vertices, vertex->point, vertex);
  return vertices;
} /* pointvertex */


/*-------------------------------------------------
-prependfacet- prepend facet to the start of a facetlist
  increments qh numfacets
  updates facetlist, qh facet_list, facet_next
notes:
  be careful of prepending since it can lose a pointer.
  e.g., can lose _next by deleting and then prepending before _next
*/
void qh_prependfacet(facetT *facet, facetT **facetlist) {
  facetT *prevfacet, *list= *facetlist;
  

  trace4((qh ferr, "qh_prependfacet: prepend f%d before f%d\n",
	  facet->id, list->id));
  prevfacet= list->previous;
  facet->previous= prevfacet;
  if (prevfacet)
    prevfacet->next= facet;
  list->previous= facet;
  facet->next= *facetlist;
  if (qh facet_list == list)  /* this may change *facetlist */
    qh facet_list= facet;
  if (qh facet_next == list)
    qh facet_next= facet;
  *facetlist= facet;
  qh num_facets++;
} /* prependfacet */


/*-----------------------------------------
-printhashtable- print hash table
  not in I/O to avoid bringing io.c in
*/
void qh_printhashtable(FILE *fp) {
  facetT *facet, *neighbor;
  int id, facet_i, facet_n, neighbor_i= 0, neighbor_n= 0;
  vertexT *vertex, **vertexp;

  FOREACHfacet_i_(qh hash_table) {
    if (facet) {
      FOREACHneighbor_i_(facet) {
        if (!neighbor || neighbor == qh_MERGEridge || neighbor == qh_DUPLICATEridge) 
          break;
      }
      if (neighbor_i == neighbor_n)
        continue;
      fprintf (fp, "hash %d f%d ", facet_i, facet->id);
      FOREACHvertex_(facet->vertices)
        fprintf (fp, "v%d ", vertex->id);
      fprintf (fp, "\n neighbors:");
      FOREACHneighbor_i_(facet) {
	if (neighbor == qh_MERGEridge)
	  id= -3;
	else if (neighbor == qh_DUPLICATEridge)
	  id= -2;
	else
	  id= getid_(neighbor);
        fprintf (fp, " %d", id);
      }
      fprintf (fp, "\n");
    }
  }
} /* printhashtable */
     

/*-------------------------------------------------
-printlists- print out facet and vertex list for debugging (without 'f/v' tags)
*/
void qh_printlists (void) {
  facetT *facet;
  vertexT *vertex;
  
  fprintf (qh ferr, "qh_printlists: facets:");
  FORALLfacets 
    fprintf (qh ferr, " %d", facet->id);
  fprintf (qh ferr, "\n  new facets %d visible facets %d next facet for addpoint %d\n  vertices (new %d):",
     getid_(qh newfacet_list), getid_(qh visible_list), getid_(qh facet_next),
     getid_(qh newvertex_list));
  FORALLvertices
    fprintf (qh ferr, " %d", vertex->id);
  fprintf (qh ferr, "\n");
} /* printlists */
  
/*-------------------------------------------------
-removefacet- unlinks facet from qh facet_list,
updates qh facet_list .newfacet_list .facet_next visible_list

decrements qh num_facets
*/
void qh_removefacet(facetT *facet) {
  facetT *next= facet->next, *previous= facet->previous;
  
  if (facet == qh newfacet_list)
    qh newfacet_list= next;
  if (facet == qh facet_next)
    qh facet_next= next;
  if (facet == qh visible_list)
    qh visible_list= next; 
  if (previous) {
    previous->next= next;
    next->previous= previous;
  }else {  /* 1st facet in qh facet_list */
    qh facet_list= next;
    qh facet_list->previous= NULL;
  }
  qh num_facets--;
  trace4((qh ferr, "qh_removefacet: remove f%d from facet_list\n", facet->id));
} /* removefacet */


/*-------------------------------------------------
-removevertex- unlinks vertex from qh vertex_list,
updates qh vertex_list .newvertex_list 

decrements qh num_vertices
*/
void qh_removevertex(vertexT *vertex) {
  vertexT *next= vertex->next, *previous= vertex->previous;
  
  if (vertex == qh newvertex_list)
    qh newvertex_list= next;
  if (previous) {
    previous->next= next;
    next->previous= previous;
  }else {  /* 1st vertex in qh vertex_list */
    qh vertex_list= vertex->next;
    qh vertex_list->previous= NULL;
  }
  qh num_vertices--;
  trace4((qh ferr, "qh_removevertex: remove v%d from vertex_list\n", vertex->id));
} /* removevertex */


/*-------------------------------------------------
-vertexintersect- intersects two vertex sets (inverse id ordered)
  temporary set vertexsetA is replaced by the intersection
     must be at top of stack
  could overwrite vertexsetA if currently too slow
*/
void qh_vertexintersect(setT **vertexsetA,setT *vertexsetB) {
  setT *intersection;

  intersection= qh_vertexintersect_new (*vertexsetA, vertexsetB);
  qh_settempfree (vertexsetA);
  *vertexsetA= intersection;
  qh_settemppush (intersection);
} /* vertexintersect */

/*-------------------------------------------------
-vertexintersect_new- intersects two vertex sets (inverse id ordered)
returns:
  a new set
*/
setT *qh_vertexintersect_new (setT *vertexsetA,setT *vertexsetB) {
  setT *intersection= qh_setnew (qh hull_dim - 1);
  vertexT **vertexA= SETaddr_(vertexsetA, vertexT); 
  vertexT **vertexB= SETaddr_(vertexsetB, vertexT); 

  while (*vertexA && *vertexB) {
    if (*vertexA  == *vertexB) {
      qh_setappend(&intersection, *vertexA);
      vertexA++; vertexB++;
    }else {
      if ((*vertexA)->id > (*vertexB)->id)
        vertexA++;
      else
        vertexB++;
    }
  }
  return intersection;
} /* vertexintersect_new */


/*-------------------------------------------
-vertexneighhbors- for each vertex in hull, determine facet neighbors
  nop if VERTEXneighbors
  assumes all vertex->neighbors are NULL
returns:
  sets qh VERTEXneighbors, qh_addpoint() will maintain them
*/
void qh_vertexneighbors (void /*qh facet_list*/) {
  facetT *facet;
  vertexT *vertex, **vertexp;

  if (qh VERTEXneighbors)
    return;
  trace1((qh ferr, "qh_vertexneighbors: determing neighboring facets for each vertex\n"));
  qh vertex_visit++;
  FORALLfacets {
    FOREACHvertex_(facet->vertices) {
      if ((int)vertex->visitid != qh vertex_visit) {
        vertex->visitid= qh vertex_visit;
        vertex->neighbors= qh_setnew (qh hull_dim);
      }
      qh_setappend (&vertex->neighbors, facet);
    }
  }
  qh VERTEXneighbors= True;
} /* vertexneighbors */

/*-------------------------------------------------
-vertexsubset- returns True if vertexsetA is a subset of vertexsetB, False
  otherwise; relies on vertexsets being sorted;
an empty set is a subset of any other set
  */
boolT qh_vertexsubset(setT *vertexsetA, setT *vertexsetB) {
  vertexT **vertexA= (vertexT **) SETaddr_(vertexsetA, vertexT);
  vertexT **vertexB= (vertexT **) SETaddr_(vertexsetB, vertexT);
  
  while (True) {
    if (!*vertexA)
      return True;
    if (!*vertexB)
      return False;
    if ((*vertexA)->id > (*vertexB)->id)
      return False;
    if (*vertexA  == *vertexB)
      vertexA++;
    vertexB++; 
  }
} /* vertexsubset */


/*************************************************************************/
/****************** implementation code from merge.c *********************/
/*************************************************************************/

/* merge.c - merges non-convex facets

   see README and merge.h
   
   other modules call qh_merge_nonconvex() and facetdegen()
   to avoid loading merge.o, redefine them to null procedures (user.c)

   assumes neighbor sets for each vertex (qh VERTEXneighbors)

   assumes qh CENTERtype= centrum
      
   copyright (c) 1993-1994 The Geometry Center        
*/

static int qh_compareangle(const void *p1, const void *p2);
static int qh_comparevisit (const void *p1, const void *p2);

/*-------------------------------------------------
-appendmergeset- appends an entry to facet_mergeset, angle is optional
  all other fields 0
returns:
  merge
notes:
  see test_appendmerge()
*/
mergeT *qh_appendmergeset(facetT *facet, facetT *neighbor, realT *angle) {
  mergeT *merge;
  void **freelistp;

  mergeT_qh_memalloc_(sizeof(mergeT), freelistp, merge);
  memset ((char*) merge, 0, sizeof(mergeT));
  merge->facet1= facet;
  merge->facet2= neighbor;
  if (angle)
    merge->angle= *angle;
  qh_setappend(&(qh facet_mergeset), merge);
  return merge;
} /* appendmergeset */


/*-------------------------------------------------
-checkridge_boundary- checks that ridges of a facet are boundaryless,
  nop if qh hull_dim < 3 or simplicial
  may miss a few errors if pinched facets
*/
void qh_checkridge_boundary (facetT *facet) {
#if 0
  ridgeT *ridge, **ridgep, *ridgeA, *errridge= NULL;
  vertexT *vertex, **vertexp;
  int unmatched= 0, matches=0, hashsize, hashslot;

  /* this code doesn't work because hash buckets can coalesce */
  if (qh hull_dim < 3 || facet->simplicial)
    return;
  trace3((qh ferr, "qh_checkridge_boundary: check subridges for f%d\n", facet->id));
  hashsize= qh_newhashtable (qh_setsize (facet->ridges));
  FOREACHridge_(facet->ridges)
    ridge->seen= False;
  FOREACHridge_(facet->ridges) {
    FOREACHvertex_(ridge->vertices) {
      if ((ridgeA= qh_hashridge_find (qh hash_table, hashsize, 
             ridge, vertex, NULL, &hashslot))) {
        matches++;
        ridgeA->seen ^= True;
      }else {
        unmatched++;
        ridge->seen ^= True;
        if (hashslot != -1)
          SETelem_(qh hash_table, hashslot)= ridge; 
      }
    }
  }
  FOREACHridge_(facet->ridges) {
    if (ridge->seen) {
      fprintf(qh ferr, "qhull internal error (checkridge_boundary): subridges of r%d in f%d don't match up\n", 
               ridge->id, facet->id);
      errridge= ridge;
    }
  }

  if (errridge || unmatched > matches || ((matches-unmatched) ^ 0x1)) 
    qhull_fatal(58);

  qh_settempfree (&qh hash_table);
#endif
} /* checkridge_boundary */


/*-------------------------------------------------
-compareangle- used by qsort() to order merges by the angle between
 them
*/
static int qh_compareangle(const void *p1, const void *p2) {
  mergeT *a= *((mergeT **)p1), *b= *((mergeT **)p2);
 
  return ((a->angle > b->angle) ? 1 : -1);
} /* compareangle */

/*-------------------------------------------------
-comparevisit- used by qsort() to order vertices by their visitid
*/
static int qh_comparevisit (const void *p1, const void *p2) {
  vertexT *a= *((vertexT **)p1), *b= *((vertexT **)p2);
 
  return (a->visitid - b->visitid);
} /* comparevisit */

/*------------------------------------------------
-copynonconvex- copy non-convex flag to all ridges between same neighbors
*/
void qh_copynonconvex (ridgeT *atridge) {
  boolT nonconvex = False;
  facetT *facet, *otherfacet;
  ridgeT *ridge, **ridgep;

  nonconvex= (boolT) atridge->nonconvex;
  facet= atridge->top;
  otherfacet= atridge->bottom;
  FOREACHridge_(facet->ridges) {
    if (otherfacet == otherfacet_(ridge, facet))
      ridge->nonconvex= nonconvex;
  }
} /* copynonconvex */

/*------------------------------------------------
-degen_redundant_neighbors- append degen. and redundant neighbors to facet_mergeset
  also checks current facet for degeneracy
  bumps visitid
  called for each mergefacet(), merge and statistics occur in merge_nonconvex
    redundant facets will be merged before degenerate ones
notes:
  a degenerate facet doesn't have enough neighbors
  a redundant facet's vertices is a subset of its neighbor's vertices
*/
void qh_degen_redundant_neighbors (facetT *facet) {
  vertexT *vertex, **vertexp;
  facetT *neighbor, **neighborp;
  int size;
  realT angledegen= qh_ANGLEdegen, angleredundant= qh_ANGLEredundant;

  trace3((qh ferr, "qh_degen_redundant_neighbors: test neighbors of f%d\n", facet->id));
  if ((size= qh_setsize (facet->neighbors)) < qh hull_dim) {
    qh_appendmergeset (facet, facet, &angledegen);
    trace2((qh ferr, "qh_degen_redundant_neighbors: f%d is degenerate.\n", facet->id));
  }
  FOREACHneighbor_(facet) {   /* first so that redundant merges occur first */
    if ((size= qh_setsize (neighbor->neighbors)) < qh hull_dim) {
      qh_appendmergeset (neighbor, neighbor, &angledegen);
      trace2((qh ferr, "qh_degen_redundant_neighbors: f%d is degenerate.  Neighbor of f%d.\n", neighbor->id, facet->id)); 
    }
  }
  qh vertex_visit++;
  FOREACHvertex_(facet->vertices)
    vertex->visitid= qh vertex_visit;
  FOREACHneighbor_(facet) {
    FOREACHvertex_(neighbor->vertices) {
      if ((int)vertex->visitid != qh vertex_visit)
        break;
    }
    if (!vertex) {
      qh_appendmergeset (neighbor, facet, &angleredundant);
      trace2((qh ferr, "qh_degen_redundant_neighbors: f%d is contained in f%d.  merge\n", neighbor->id, facet->id)); 
    }
  }
} /* degen_redundant_neighbors */


/*-------------------------------------------------
-facetdegen- true if facet already in mergeset as a degenerate
*/
boolT qh_facetdegen (facetT *facet) {
  mergeT *merge, **mergep;

  FOREACHmerge_(qh facet_mergeset) {
    if (merge->facet1 == facet && merge->angle == qh_ANGLEdegen) 
        return True;
  }
  return False;
} /* facetdegen */


/*-----------------------------------------
-find_newvertex - locate new vertex for renaming old vertex
  each ridge includes oldvertex
  vertices consists of possible new vertices
returns:
  newvertex or NULL
  vertices sorted by number of deleted ridges
notes:
  new vertex is in one of the ridges
  renaming will not cause a duplicate ridge
  renaming will minimize the number of deleted ridges
  newvertex may not be adjacent in the dual (though unlikely)
*/
vertexT *qh_find_newvertex (vertexT *oldvertex, setT *vertices, setT *ridges) {
  vertexT *vertex, **vertexp;
  setT *newridges;
  ridgeT *ridge, **ridgep, *dupridge;
  int size, hashsize;
  int hash;

  if (qh IStracing >= 4) {
    fprintf (qh ferr, "qh_find_newvertex: find new vertex for v%d from ",
	     oldvertex->id);
    FOREACHvertex_(vertices) 
      fprintf (qh ferr, "v%d ", vertex->id);
    FOREACHridge_(ridges)
      fprintf (qh ferr, "r%d ", ridge->id);
    fprintf (qh ferr, "\n");
  }
  FOREACHvertex_(vertices) 
    vertex->visitid= 0;
  FOREACHridge_(ridges) {
    FOREACHvertex_(ridge->vertices) 
      vertex->visitid++;
  }
  FOREACHvertex_(vertices) {
    if (!vertex->visitid) {
      qh_setdelnth (vertices, SETindex_(vertices,vertex));
      vertexp--; /* repeat since deleted this vertex */
    }
  }
  qh vertex_visit += qh_setsize (ridges);
  if (!qh_setsize (vertices)) {
    trace4((qh ferr, "qh_find_newvertex: vertices not in ridges for v%d\n",
	    oldvertex->id));
    return NULL;
  }
  qsort (SETaddr_(vertices, vertexT), qh_setsize (vertices),
	        sizeof (vertexT *), qh_comparevisit);
  /* can now use qh vertex_visit */
  if (qh PRINTstatistics) {
    size= qh_setsize (vertices);
    zinc_(Zintersect);
    zadd_(Zintersecttot, size);
    zmax_(Zintersectmax, size);
  }
  hashsize= qh_newhashtable (qh_setsize (ridges));
  FOREACHridge_(ridges)
    qh_hashridge (qh hash_table, hashsize, ridge, oldvertex);
  FOREACHvertex_(vertices) {
    newridges= qh_vertexridges (vertex);
    FOREACHridge_(newridges) {
      if ((dupridge= qh_hashridge_find (qh hash_table, hashsize, ridge, vertex, oldvertex, &hash))) {
	zinc_(Zdupridge);
	break;
      }
    }
    qh_settempfree (&newridges);
    if (!ridge)
      break;  /* found a rename */
  }
  if (vertex) {
    zinc_(Zfindvertex);
    trace2((qh ferr, "qh_find_newvertex: found v%d for old v%d from %d vertices and %d ridges.\n",
      vertex->id, oldvertex->id, qh_setsize (vertices), qh_setsize (ridges)));
  }else {
    zinc_(Zfindfail);
    trace0((qh ferr, "qh_find_newvertex: no vertex for renaming v%d (all duplicated ridges)\n",
      oldvertex->id));
  }
  qh_setfree (&qh hash_table);
  return vertex;
} /* find_newvertex */

/*-------------------------------------------------
-findbest_test- test neighbor for findbestneighbor()
  either test centrum or vertices
*/
void qh_findbest_test (boolT testcentrum, facetT *facet, facetT *neighbor,
      facetT **bestfacet, realT *distp, realT *mindistp, realT *maxdistp) {
  realT dist, mindist, maxdist;

  if (testcentrum) {
    zzinc_(Zbestdist);
    qh_distplane(facet->center, neighbor, &dist);
    dist *= qh hull_dim; /* estimate furthest vertex */
    if (dist < 0) {
      maxdist= 0;
      mindist= dist;
      dist= -dist;
    }else
      maxdist= dist;
  }else
    dist= qh_getdistance (facet, neighbor, &mindist, &maxdist);
  if (dist < *distp) {
    *bestfacet= neighbor;
    *mindistp= mindist;
    *maxdistp= maxdist;
    *distp= dist;
  }
} /* findbest_test */

/*-------------------------------------------------
-findbestneighbor- finds best neighbor (least dist) of a facet for merging
  returns min and max distances and their max absolute value
  avoids merging old into new
*/
facetT *qh_findbestneighbor(facetT *facet, realT *distp, realT *mindistp, realT *maxdistp) {
  facetT *neighbor, **neighborp, *bestfacet= NULL;
  ridgeT *ridge, **ridgep;
  boolT nonconvex= True, testcentrum= False;
  int size= qh_setsize (facet->vertices);   

  *distp= REALmax;
  if (size > qh hull_dim + qh_BESTcentrum) {
    testcentrum= True;
    zinc_(Zbestcentrum);
    if (!facet->center)
       facet->center= qh_getcentrum (facet);
  }
  if (size > qh hull_dim * qh_BESTnonconvex) {
    FOREACHridge_(facet->ridges) {
      if (ridge->nonconvex) {
        neighbor= otherfacet_(ridge, facet);
	qh_findbest_test (testcentrum, facet, neighbor,
			  &bestfacet, distp, mindistp, maxdistp);
      }
    }
  }
  if (!bestfacet) {     
    nonconvex= False;
    FOREACHneighbor_(facet)
      qh_findbest_test (testcentrum, facet, neighbor,
			&bestfacet, distp, mindistp, maxdistp);
  }

  if (!bestfacet) qhull_fatal(59);

  trace4((qh ferr, "qh_findbestneighbor: f%d is best neighbor for f%d testcentrum? %d nonconvex? %d dist %2.2g min %2.2g max %2.2g\n",
     bestfacet->id, facet->id, testcentrum, nonconvex, *distp, *mindistp, *maxdistp));
  return(bestfacet);
} /* findbestneighbor */


/*-------------------------------------------------
-flippedmerges- merge flipped facets into best neighbor
  facet_mergeset may have degen/redundant from forced merges
  assumes facet_mergeset at top of temp stack
returns:
  no flipped facets on facetlist
  degen/redundant merges passed through
*/
void qh_flippedmerges(facetT *facetlist) {
  facetT *facet, *neighbor, *facet1, *facet2;
  realT dist, mindist, maxdist;
  mergeT *merge;
  setT *othermerges= qh_settemp (qh TEMPsize);
  int nummerge=0;

  trace2((qh ferr, "qh_flippedmerges: begin\n"));
  FORALLfacet_(facetlist) {
    if (facet->flipped) 
      qh_appendmergeset (facet, facet, NULL);
  }
  while ((merge= (mergeT *)qh_setdellast (qh facet_mergeset))) {
    if (merge->angle >= qh_ANGLEdegen) {  /* and qh_ANGLEredundant */
      qh_setappend (&othermerges, merge);
      continue;
    }
    facet1= merge->facet1;
    facet2= merge->facet2;
    qh_memfree (merge, sizeof(mergeT));
    if (facet1->visible || facet2->visible)
      continue;
    if (qh TRACEmerge-1 == zzval_(Ztotmerge))
      qhmem.IStracing= qh IStracing= qh TRACElevel;
    neighbor= qh_findbestneighbor (facet1, &dist, &mindist, &maxdist);
    trace0((qh ferr, "qh_flippedmerges: merge flipped f%d into f%d dist %2.2g\n",
      facet1->id, neighbor->id, dist));
    qh_mergefacet (facet1, neighbor, &mindist, &maxdist, NULL);
    nummerge++;
    if (qh PRINTstatistics) {
      zinc_(Zflipped);
      wadd_(Wflippedtot, dist);
      wmax_(Wflippedmax, dist);
    }
  }
  if (qh_setsize (othermerges)) {
    qh_settemppop();  /* othermerges, errors here will leave memory */
    qh_settempfree (&qh facet_mergeset);
    qh facet_mergeset= othermerges;
    qh_settemppush (othermerges);
  }else
    qh_settempfree (&othermerges);
  trace1((qh ferr, "qh_flippedmerges: merged %d flipped facets into a good neighbor\n", nummerge));
} /* flippedmerges */


/*-------------------------------------------------
-forcedmerges- merge across duplicated ridges and mutually flipped facets
  duplicate ridges marked by MERGEridge and both sides facet->seen
  bumps visit_id
  assumes no merge has merge->newmerge
returns:
  all neighboring, flipped facets merged together
  no duplicate ridges
  facet_mergeset includes any degen/redundant merges
  uses facet-tested
notes: 
  duplicate ridges occur when the horizon is pinched,
        i.e. a subridge occurs in more than two horizon ridges.
     could rename vertices that pinch the horizon
*/
void qh_forcedmerges(facetT *facetlist) {
  facetT *facet, *neighbor, **neighborp, *facet1, *facet2;
  mergeT *merge, **mergep;
  realT dist1, dist2, mindist1, mindist2, maxdist1, maxdist2;
  int nummerge=0, numflip=0;

  if (qh TRACEmerge-1 == zzval_(Ztotmerge))
    qhmem.IStracing= qh IStracing= qh TRACElevel;
  trace2((qh ferr, "qh_forcedmerges: begin\n"));
  qh visit_id++;
  FORALLfacet_(facetlist) {
    facet->tested= True;
    if (facet->seen || facet->flipped) {
      facet->visitid= qh visit_id;
      FOREACHneighbor_(facet) {
        if (neighbor == qh_MERGEridge) {
	  facet->tested= False;
	  continue;
	}
        if (neighbor->seen && facet->seen
	&& !qh_setin (neighbor->neighbors, facet)) {  /* qh_MERGEridge */
	  merge= qh_appendmergeset (facet, neighbor, NULL);
	  merge->mergeridge= True;
	}
	if ((int)neighbor->visitid == qh visit_id)
          continue;
	if (neighbor->flipped && facet->flipped)
          qh_appendmergeset (facet, neighbor, NULL);
      }
    }
  }
  FORALLfacet_(facetlist) {            /* gets rid of qh_MERGEridge */
    if (!facet->tested)   
      qh_makeridges (facet);
  }
  FOREACHmerge_(qh facet_mergeset) {   /* restore the missing neighbors */
    if (merge->mergeridge) {
      qh_setappend (&merge->facet2->neighbors, merge->facet1);
      qh_makeridges (merge->facet1);   /* and the missing ridges */
    }
  }
 LABELrestart_merges:
  FOREACHmerge_(qh facet_mergeset) {   /* do duplicates first */
    if (!merge->mergeridge)
      continue;
    facet1= merge->facet1;
    facet2= merge->facet2;
    while (facet1->visible)
      facet1= (facetT *)SETfirst_(facet1->neighbors);
    while (facet2->visible)
      facet2= (facetT *)SETfirst_(facet2->neighbors);
    if (facet1 == facet2) 
      continue;
    if (!qh_setin (facet2->neighbors, facet1)) qhull_fatal(60);

    if (qh TRACEmerge-1 == zzval_(Ztotmerge))
      qhmem.IStracing= qh IStracing= qh TRACElevel;
    if (facet1->flipped || facet2->flipped) {
      zinc_(Zmergeflipdup);
      numflip++;
      trace0((qh ferr, "qh_forcedmerges: duplicate ridge with a flipped facet for f%d and f%d\n",
	      facet1->id, facet2->id));
      if (facet1->flipped)     /* delay until qh_flippedmerges */
	qh_mergefacet (facet2, facet1, NULL, NULL, NULL);
      else
	qh_mergefacet (facet1, facet2, NULL, NULL, NULL);
      zinc_(Zmergeflip);
    }else {
      dist1= qh_getdistance (facet1, facet2, &mindist1, &maxdist1);
      dist2= qh_getdistance (facet2, facet1, &mindist2, &maxdist2);
      trace0((qh ferr, "qh_forcedmerges: duplicate ridge between f%d and f%d, dist %2.2g and reverse dist %2.2g\n",
	      facet1->id, facet2->id, dist1, dist2));
      if (dist1 < dist2) 
	qh_mergefacet (facet1, facet2, &mindist1, &maxdist1, NULL);
      else {
	dist1= dist2;
	qh_mergefacet (facet2, facet1, &mindist2, &maxdist2, NULL);
      }
      nummerge++;
      if (qh PRINTstatistics) {
	zinc_(Zduplicate);
	wadd_(Wduplicatetot, dist1);
	wmax_(Wduplicatemax, dist1);
      }
    }
    goto LABELrestart_merges;  /* facet_mergeset may have changed */
  }
  while ((merge= (mergeT *)qh_setdellast (qh facet_mergeset))) {
    if (merge->newmerge) 
      break;
    facet1= merge->facet1;
    facet2= merge->facet2;
    if (merge->mergeridge || facet1->visible || facet2->visible) 
      qh_memfree (merge, sizeof(mergeT));
    else if (facet1 == facet2 || !facet1->flipped || !facet2->flipped) {
      merge->newmerge= True;
      qh_setaddnth (&qh facet_mergeset, 0, merge);
    }else {
      if (qh TRACEmerge-1 == zzval_(Ztotmerge))
	qhmem.IStracing= qh IStracing= qh TRACElevel;
      qh_memfree (merge, sizeof(mergeT));
      zinc_(Zmergeflip);
      trace3((qh ferr, "qh_forcedmerges: merge flipped facets f%d and f%d\n",
	      facet1->id, facet2->id));
      qh_mergefacet (facet1, facet2, NULL, NULL, NULL);
      numflip++;
    }
  }
  trace1((qh ferr, "qh_forcedmerges: merged %d facets across duplicated ridges and %d flipped facets\n", nummerge, numflip));
} /* forcedmerges */


/*-------------------------------------------------
-getdistance- returns the max and min distance of any vertex from neighbor
  returns the max absolute value
*/
realT qh_getdistance(facetT *facet, facetT *neighbor, realT *mindist, realT *maxdist) {
  vertexT *vertex, **vertexp;
  realT dist;
  
  FOREACHvertex_(facet->vertices)
    vertex->seen= False;
  FOREACHvertex_(neighbor->vertices)
    vertex->seen= True;
  *mindist= 0.0;
  *maxdist= 0.0;
  FOREACHvertex_(facet->vertices) {
    if (!vertex->seen) {
      zzinc_(Zbestdist);
      qh_distplane(vertex->point, neighbor, &dist);
      if (dist > *maxdist)
        *maxdist= dist;
      else if (dist < *mindist)
        *mindist= dist;
    }
  }
  return fmax_(*maxdist, -(*mindist));
} /* getdistance */


/*-------------------------------------------------
-getmergeset- returns facet_mergeset of facet-neighbor pairs to be merged
    only tests !tested ridges of !tested facets
returns:
    sorted mergeset
    all ridges tested
notes:
  uses ridge->tested to prevent duplicate tests
  resets ridge->tested if !facet->center
  can not limit tests to modified ridges since the centrum changed
*/
void qh_getmergeset(facetT *facetlist) {
  facetT *facet, *neighbor, **neighborp;
  ridgeT *ridge, **ridgep;
  int nummerges;
  
  nummerges= qh_setsize (qh facet_mergeset);
  trace2((qh ferr, "qh_getmergeset: started.\n"));
  FORALLfacet_(facetlist) {
    if (!facet->center) {
      facet->tested= False;
      FOREACHridge_(facet->ridges)
	ridge->tested= False;
    }
    if (facet->tested)
      continue;
    facet->tested= True;  /* must be non-simplicial */
    FOREACHneighbor_(facet) 
      neighbor->seen= False;
    FOREACHridge_(facet->ridges) {
      if (ridge->tested && !ridge->nonconvex)
	continue;
      ridge->tested= True;
      ridge->nonconvex= False;
      neighbor= otherfacet_(ridge, facet);
      if (!neighbor->seen) {
	neighbor->seen= True;
	if (qh_test_appendmerge (facet, neighbor))
	  ridge->nonconvex= True;
      }
    }
  }
  nummerges= qh_setsize (qh facet_mergeset);
  qsort(SETaddr_(qh facet_mergeset, mergeT), nummerges,sizeof(mergeT *),qh_compareangle);
  trace2((qh ferr, "qh_getmergeset: %d merges found\n", nummerges));
} /* getmergeset */


/*------------------------------------------------
-getmergeset_initial- initial mergeset for facets
  tests all facet/neighbor pairs on facetlist
  uses visit_id
  facet_mergeset may have degen/redundant from flipped and forced merges
returns:
  sorted mergeset
  sets facet->tested and ridge->tested
*/
void qh_getmergeset_initial (facetT *facetlist) {
  facetT *facet, *neighbor, **neighborp;
  ridgeT *ridge, **ridgep;
  int nummerges;

  qh visit_id++;
  FORALLfacet_(facetlist) {
    facet->visitid= qh visit_id;
    facet->tested= True;
    FOREACHneighbor_(facet) {
      if ((int)neighbor->visitid != qh visit_id) {
        if (qh_test_appendmerge (facet, neighbor)) {
          if (!neighbor->simplicial) {
            FOREACHridge_(neighbor->ridges) {
              if (facet == otherfacet_(ridge, neighbor)) {
                ridge->nonconvex= True;
                break;
              }
            }
          }
        }
      }
    }
    FOREACHridge_(facet->ridges)
      ridge->tested= True;
  }
  nummerges= qh_setsize (qh facet_mergeset);
  qsort(SETaddr_(qh facet_mergeset, mergeT), nummerges,sizeof(mergeT *),qh_compareangle);
  trace2((qh ferr, "qh_getmergeset_initial: %d merges found\n", nummerges));
} /* getmergeset_initial */


/*-----------------------------------------
-hashridge- add ridge to hashtable without oldvertex
  assumes hashtable is large enough
*/
void qh_hashridge (setT *hashtable, int hashsize, ridgeT *ridge, vertexT *oldvertex) {
  unsigned hash;
  ridgeT *ridgeA;

  hash= qh_gethash (hashsize, ridge->vertices, qh hull_dim-1, 0, oldvertex);
  while (True) {
    if (!(ridgeA= (ridgeT *)SETelem_(hashtable, hash))) {
      SETelem_(hashtable, hash)= ridge;
      break;
    }else if (ridgeA == ridge)
      break;
    if ((int)(++hash) == hashsize)
      hash= 0;
  }
} /* hashridge */


/*-----------------------------------------
-hashridge_find- returns matching ridge in hashtable without oldvertex
  assumes hashtable is large enough
  can't match ridge to itself
  if oldvertex is NULL matches with one skip
returns:
  returns matching ridge;
  if no match,
    hashslot= -1 if ridge already in table
      else next NULL index
*/
ridgeT *qh_hashridge_find (setT *hashtable, int hashsize, ridgeT *ridge, 
              vertexT *vertex, vertexT *oldvertex, int *hashslot) {
  unsigned hash;
  ridgeT *ridgeA;

  *hashslot= 0;
  zinc_(Zhashridge);
  hash= qh_gethash (hashsize, ridge->vertices, qh hull_dim-1, 0, vertex);
  while ((ridgeA= (ridgeT *)SETelem_(hashtable, hash))) {
    if (ridgeA == ridge)
      *hashslot= -1;      
    else {
      zinc_(Zhashridgetest);
      if (qh_setequal_except (ridge->vertices, vertex, ridgeA->vertices, oldvertex))
        return ridgeA;
    }
    if ((int)(++hash) == hashsize)
      hash= 0;
  }
  if (!*hashslot)
    *hashslot= hash;
  return NULL;
} /* hashridge_find */


/*-------------------------------------------------
-makeridges- creates explicit ridges between simplicial facets
  allows qh_MERGEridge flag
  uses existing ridges
returns:
  facet with ridges and without qh_MERGEridge
*/
void qh_makeridges(facetT *facet) {
  facetT *neighbor, **neighborp;
  ridgeT *ridge, **ridgep;
  int neighbor_i, neighbor_n;
  boolT toporient=False, mergeridge= False;
  
  if (!facet->simplicial)
    return;
  trace4((qh ferr, "qh_makeridges: make ridges for f%d\n", facet->id));
  facet->simplicial= False;
  FOREACHneighbor_(facet) {
    if (neighbor == qh_MERGEridge)
      mergeridge= True;
    else
      neighbor->seen= False;
  }
  FOREACHridge_(facet->ridges)
    otherfacet_(ridge, facet)->seen= True;
  FOREACHneighbor_i_(facet) {
    if (neighbor == qh_MERGEridge) {
      FOREACHridge_(facet->ridges) {
	if (!qh_setin (facet->neighbors, otherfacet_(ridge, facet)))
	  ridge->mergeridge= True;
      }
    }else if (!neighbor->seen) {
      ridge= qh_newridge();
      if (!neighbor->simplicial)   /* only in forced merges, checkfacet */
	ridge->mergeridge= True;  
      ridge->vertices= qh_setnew_delnthsorted (facet->vertices, qh hull_dim,
					                  neighbor_i, 0);
      toporient= (boolT)( facet->toporient ^ (neighbor_i & 0x1) );
      if (toporient) {
        ridge->top= facet;
        ridge->bottom= neighbor;
      }else {
        ridge->top= neighbor;
        ridge->bottom= facet;
      }
#if 0 /* this also works */
      flip= (facet->toporient ^ neighbor->toporient)^(skip1 & 0x1) ^ (skip2 & 0x1);
      if (facet->toporient ^ (skip1 & 0x1) ^ flip) {
        ridge->top= neighbor;
        ridge->bottom= facet;
      }else {
        ridge->top= facet;
        ridge->bottom= neighbor;
      }
#endif
      qh_setappend(&(facet->ridges), ridge);
      qh_setappend(&(neighbor->ridges), ridge);
    }
  }
  if (mergeridge) {
    while (qh_setdel (facet->neighbors, qh_MERGEridge))
      ; /* delete each one */
  }
} /* makeridges */


/*-------------------------------------------
-maydropneighbor -- drop neighbor relationship if no ridge between facet and neighbor
  bumps qh visit_id
returns:
  appends degenerate facets to facet_mergeset
  won't cause redundant facets since vertex inclusion is the same
  may drop vertex and neighbor if no ridge
*/
void qh_maydropneighbor (facetT *facet) {
  ridgeT *ridge, **ridgep;
  realT angledegen= qh_ANGLEdegen;
  facetT *neighbor, **neighborp;

  qh visit_id++;
  trace4((qh ferr, "qh_maydropneighbor: test f%d for no ridges to a neighbor\n",
	  facet->id));
  FOREACHridge_(facet->ridges) {
    ridge->top->visitid= qh visit_id;
    ridge->bottom->visitid= qh visit_id;
  }
  FOREACHneighbor_(facet) {
    if ((int)neighbor->visitid != qh visit_id) {
      trace0((qh ferr, "qh_maydropneighbor: facets f%d and f%d are no longer neighbors\n",
	    facet->id, neighbor->id));
      zinc_(Zdropneighbor);
      qh_setdel (facet->neighbors, neighbor);
      neighborp--;  /* repeat, deleted a neighbor */
      qh_setdel (neighbor->neighbors, facet);
      if (qh_setsize (neighbor->neighbors) < qh hull_dim) {
        zinc_(Zdropdegen);
        qh_appendmergeset (neighbor, neighbor, &angledegen);
        trace2((qh ferr, "qh_maydropneighbors: f%d is degenerate.\n", neighbor->id));
      }
    }
  }
  if (qh_setsize (facet->neighbors) < qh hull_dim) {
    zinc_(Zdropdegen);
    qh_appendmergeset (facet, facet, &angledegen);
    trace2((qh ferr, "qh_maydropneighbors: f%d is degenerate.\n", facet->id));
  }
} /* maydropneighbor */


/*---------------------------------------
-merge_degenredundant- merge degenerate and redundant facets
  if facet1 undefined, tests last merge on facet_mergeset and sets angle
returns:
  True if merge performed or degen merge not needed
notes:
  need this since renaming vertices can result in degen/redundant facets
  any merge can also result in degen/redundant facets
*/
boolT qh_merge_degenredundant (facetT *facet1, facetT *facet2, realT *angle) {
  int numnewmerges= 0, size;
  mergeT *merge;
  facetT *bestneighbor;
  realT dist, mindist, maxdist;
  vertexT *vertex, **vertexp;

  if (qh TRACEmerge-1 == zzval_(Ztotmerge))
    qhmem.IStracing= qh IStracing= qh TRACElevel;
  while (!facet1 && (merge= (mergeT *)qh_setlast (qh facet_mergeset))
	 && merge->angle >= qh_ANGLEdegen) {
    merge= (mergeT *)qh_setdellast (qh facet_mergeset);
    facet1= merge->facet1;
    facet2= merge->facet2;
    *angle= merge->angle;
    qh_memfree (merge, sizeof(mergeT));
    if (facet1->visible || facet2->visible)
      facet1= NULL;
  }
  if (!facet1) 
    return False;
  if (*angle == qh_ANGLEredundant) {
    trace2((qh ferr, "qh_merge_degenredundant: facet f%d is contained in f%d, will merge\n",
	    facet1->id, facet2->id));
    qh_mergefacet(facet1, facet2, NULL, NULL, angle);
    qh_newmerge_(facet2);
    zinc_(Zneighbor);
  }else if (*angle == qh_ANGLEdegen) {   /* other merges may have fixed */
    if (!(size= qh_setsize (facet1->neighbors))) {
      trace2((qh ferr, "qh_merge_degenredundant: facet f%d has no neighbors.  Deleted\n", facet1->id));
      zinc_(Zdelfacetdup);
      qh_removefacet(facet1);
      qh_prependfacet (facet1, &qh visible_list);
      qh num_visible++;
      facet1->visible= True;
      /* SETfirst_(facet1->neighbors) == NULL */
      FOREACHvertex_(facet1->vertices) {
	qh_setdel (vertex->neighbors, facet1);
	if (!SETfirst_(vertex->neighbors)) {
	  zinc_(Zdegenvertex);
	  trace2((qh ferr, "qh_merge_degenredundant: deleted v%d because f%d has no neighbors\n",
          vertex->id, facet1->id));
	  vertex->deleted= True;
	  qh_setappend (&qh del_vertices, vertex);
	}
      }
      numnewmerges++;  /* needed since cleared this merge */
    }else if (size < qh hull_dim) {
      bestneighbor= qh_findbestneighbor(facet1, &dist, &mindist, &maxdist);
      trace2((qh ferr, "qh_merge_degenredundant: facet f%d has %d neighbors, merge into f%d dist %2.2g\n",
	      facet1->id, size, bestneighbor->id, dist));
      qh_mergefacet(facet1, bestneighbor, &mindist, &maxdist, angle);
      qh_newmerge_(bestneighbor);
      if (qh PRINTstatistics) {
	zinc_(Zdegen);
	wadd_(Wdegentot, dist);
	wmax_(Wdegenmax, dist);
      }
    }else 
      numnewmerges++;  /* needed since cleared this merge */
  }
  return (boolT)numnewmerges;
} /* merge_degenredundant */

/*-------------------------------------------------
-merge_nonconvex- merges all nonconvex facets
  assumes qh_vertexneighbors() already called
  need to use qh newfacet_list since merge calls removefacet()
returns:
  deleted facets added to visible_list
notes:
  numdegenredun also counts degen facets that become ok
*/
void qh_merge_nonconvex (void /*qh newfacet_list*/) {
  facetT *bestfacet, *neighbor, *facet1, *facet2;
  facetT *bestneighbor;
  mergeT *merge;
  realT dist, dist2, mindist, mindist2, maxdist, maxdist2, angle;
  boolT wasmerge= True, ismodified=False, anglecoplanar = False;
  void **freelistp;
  vertexT *vertex;
  int nummerge=0, numconcave=0, numdegenredun= 0, numnewmerges= 0;
  
  trace2((qh ferr, "qh_merge_nonconvex: starting to merge facets beginning from f%d\n",
	    getid_(qh newfacet_list)));
  if (qh IStracing >= 4 && qh num_facets < 50)
    qh_printlists();
  qh facet_mergeset= qh_settemp (qh TEMPsize);
  qh_forcedmerges (qh newfacet_list);    /* also sets tested */
  qh_flippedmerges (qh newfacet_list);
  qh NEWmerges= True;
  if (qh POSTmerging)
    qh_tracemerging ("after flipped merges");
  qh_getmergeset_initial (qh newfacet_list); /* facet_mergeset */
  while (wasmerge) {
    wasmerge= False;
    while (qh_setsize (qh facet_mergeset)) {
      while ((merge= (mergeT *)qh_setdellast(qh facet_mergeset))) {
	facet1= merge->facet1;
	facet2= merge->facet2;
	angle= merge->angle;
	anglecoplanar= (boolT)merge->anglecoplanar;
	qh_memfree_(merge, sizeof(mergeT), freelistp);
	if ((facet1->id >= qh newfacet_id && !facet1->tested)
	    || (facet2->id >= qh newfacet_id && !facet2->tested))
	  ismodified= True;
	else
	  ismodified= False;
	if (facet1->visible || facet2->visible) /*deleted facet*/
	  continue;  
	if (ismodified && angle < qh_ANGLEconcave)
	  continue;
	if (qh TRACEmerge-1 == zzval_(Ztotmerge))
	  qhmem.IStracing= qh IStracing= qh TRACElevel;
	trace4((qh ferr, "qh_merge_nonconvex: merge #%d for f%d and f%d angle %2.2g modified? %d\n",
	    zzval_(Ztotmerge)+1, facet1->id, facet2->id, angle, ismodified));
	if (qh_merge_degenredundant (facet1, facet2, &angle)) {
	  numdegenredun++;
	  wasmerge= True;
	  continue;
	}else {  /* ANGLEconcave or coplanar */
	  if (facet1->id < qh newfacet_id) {
	    bestfacet= facet2;   /* avoid merging old facet if new is ok */
	    facet2= facet1;
	    facet1= bestfacet;
	  }else
 	    bestfacet= facet1;
	  bestneighbor= qh_findbestneighbor(bestfacet, &dist, &mindist, &maxdist);
	  neighbor= qh_findbestneighbor(facet2, &dist2, &mindist2, &maxdist2);
	  wasmerge= True;
	  if (dist < dist2) {
	    qh_mergefacet(bestfacet, bestneighbor, &mindist, &maxdist, &angle);
	    qh_newmerge_(bestneighbor);
	  }else if (facet2->id < qh newfacet_id
	  && ((mindist >= qh min_vertex && maxdist <= qh max_outside)
	       || dist * 1.5 < dist2)) {
	    zinc_(Zavoidold);
	    wadd_(Wavoidoldtot, dist);
	    wmax_(Wavoidoldmax, dist);
	    trace2((qh ferr, "qh_merge_nonconvex: avoid merging old facet f%d dist %2.2g.  Use f%d dist %2.2g insted\n",
		    facet2->id, dist2, facet1->id, dist2));
	    qh_mergefacet(bestfacet, bestneighbor, &mindist, &maxdist, &angle);
	    qh_newmerge_(bestneighbor);
	  }else {
	    qh_mergefacet(facet2, neighbor, &mindist2, &maxdist2, &angle);
	    qh_newmerge_(neighbor);
	    dist= dist2;
	  }
	  if (qh PRINTstatistics) {
	    if (anglecoplanar) {
	      nummerge++;
	      zinc_(Zacoplanar);
	      wadd_(Wacoplanartot, dist);
	      wmax_(Wacoplanarmax, dist);
	    }else if (angle > qh_ANGLEconcave) {
	      numconcave++;
	      zinc_(Zconcave);
	      wadd_(Wconcavetot, dist);
	      wmax_(Wconcavemax, dist);
	    }else {
	      nummerge++;
	      zinc_(Zcoplanar);
	      wadd_(Wcoplanartot, dist);
	      wmax_(Wcoplanarmax, dist);
	    }
	  }else if (angle > qh_ANGLEconcave)
	    numconcave++;
	  else
	    nummerge++;
	}
	/* reducing vertices here in 5-d, 50% more time, maybe fewer vertices,
	               and better bounds */
      } /* while setdellast */
      if (qh POSTmerging)
	qh_tracemerging ("after a merge set");
      if (numnewmerges > qh_MAXnewmerges) {  /* needed for large post merges */
	numnewmerges= 0;
	qh_reducevertices_centrums();
      }
      qh_getmergeset (qh newfacet_list); /* facet_mergeset */
    } /* while mergeset */
    if (wasmerge || (qh hull_dim >=4 && qh POSTmerging)) {
      if (qh hull_dim >=4 && qh POSTmerging) { /*duplicate ridges may changed*/
	FORALLvertices    
	  vertex->delridge= True;
      }
      if (qh_reducevertices_centrums()) 
	qh_getmergeset (qh newfacet_list); /* facet_mergeset */
      else
	wasmerge= False;
    }
  } /* while (wasmerge) */
  qh NEWmerges= False;
  if (qh CHECKfrequently)
    qh_checkconvex (qh newfacet_list, qh_ALGORITHMfault);
  qh_settempfree(&qh facet_mergeset);
  trace1((qh ferr, "qh_merge_nonconvex: merged %d coplanar facets %d concave facets and %d degen or redundant facets.\n",
    nummerge, numconcave, numdegenredun));
  if (qh IStracing >= 4 && qh num_facets < 50)
    qh_printlists ();
} /* merge_nonconvex */


/*-------------------------------------------------
-mergefacet- merges facet1 into facet2
  traces merge if fmax_(maxdist,-mindist) > TRACEdist
  mindist/maxdist and angle may be NULL
  max_outside and min_vertex updated
returns:
  facet1 prepended to visible_list for later deletion and partitioning
     qh num_visible updated.
     SETfirst_(facet1->neighbors) == facet2
  facet2 moved to end of qh facet_list
    facet2 is new (get's facet1->id if it was old)
  adds neighboring facets to facet_mergeset if redundant or degenerate
  clears facet->tested and ridge->tested ffrom facet1
*/
void qh_mergefacet(facetT *facet1, facetT *facet2, realT *mindist, realT *maxdist, realT *angle) {
  boolT traceonce= False, waserror= False;
  vertexT *vertex, **vertexp;
  ridgeT *ridge, **ridgep;
  int tracerestore=0;

  zzinc_(Ztotmerge);
  if ((mindist && (-*mindist > qh TRACEdist || *maxdist > qh TRACEdist))
  || facet1 == qh tracefacet || facet2 == qh tracefacet) {
    if (facet1 == qh tracefacet || facet2 == qh tracefacet) {
      tracerestore= qh IStracing;
      qh IStracing= 4;
      fprintf (qh ferr, "qh_mergefacet: ========= trace merge %d involving f%d, furthest is p%d\n",
	       zzval_(Ztotmerge), qh tracefacet_id,  qh furthest_id);
    }else {
      tracerestore= 0;
      qh IStracing= qh TRACElevel;
      fprintf (qh ferr, "qh_mergefacet: ========= trace wide merge %d (%2.2g) for f%d into f%d, last point was p%d\n", zzval_(Ztotmerge),
           fmax_(-*mindist, *maxdist), facet1->id, facet2->id, qh furthest_id);
    }
    traceonce= True;
  }
  if (qh IStracing >= 2) {
    realT mergemin= -2;
    realT mergemax= -2;
    realT mergeangle= -2;
    
    if (mindist) {
      mergemin= *mindist;
      mergemax= *maxdist;
    }
    if (angle)
      mergeangle= *angle;
    fprintf (qh ferr, "qh_mergefacet: #%d merge f%d into f%d, mindist= %2.2g, maxdist= %2.2g, angle = %2.2g\n", 
    zzval_(Ztotmerge), facet1->id, facet2->id, mergemin, mergemax, mergeangle);
  }

  if (facet1 == facet2 || facet1->visible || facet2->visible) 
    qhull_fatal(61);

  if (qh num_facets -qh num_visible <= qh hull_dim + 1) qhull_fatal(62);

  if (mindist) {
    maximize_(qh max_outside, *maxdist);
    maximize_(qh max_vertex, *maxdist);
#if qh_MAXoutside
    maximize_(facet2->maxoutside, *maxdist);
#endif
    minimize_(qh min_vertex, *mindist);
  }
  qh_makeridges(facet1);
  qh_makeridges(facet2);

  /*
  if (qh IStracing >=4)
    qh_errprint ("MERGING", facet1, facet2, NULL, NULL);
    NO LONGER IN SERVICE */

  if (facet2->center && qh hull_dim == 2) {  /* only two vertices */
    qh_memfree (facet2->center, qh center_size);
    facet2->center= NULL;
  }
  qh_mergeneighbors(facet1, facet2);
  FOREACHridge_(facet1->ridges)
    ridge->tested= False;
  qh_mergeridges(facet1, facet2);
  qh vertex_visit++;
  FOREACHvertex_(facet2->vertices)
    vertex->visitid= qh vertex_visit;
  if (qh hull_dim == 2) 
    qh_mergevertices2d(facet1->vertices, facet2->vertices);
  else 
    qh_mergevertices(facet1->vertices, &facet2->vertices);
  qh_mergevertex_neighbors(facet1, facet2);
  if (facet1->id < qh newfacet_id && facet2->id >= qh newfacet_id) {
    zinc_(Zmergehorizon);
  }else if (facet2->id >= qh newfacet_id)
    zinc_(Zmergenew);
  qh_removefacet(facet1); 
  qh_prependfacet (facet1, &qh visible_list);
  qh num_visible++;
  facet1->visible= True;
  SETfirst_(facet1->neighbors)= facet2;
  qh_settruncate (facet1->neighbors, 1);
  qh_removefacet(facet2);  /* append as a newfacet to end of qh facet_list */
  qh_appendfacet(facet2);
  if (facet2->id < qh newfacet_id) {
    zinc_(Zmergeintohorizon);
    trace3((qh ferr, "qh_mergefacet: RENAME - f%d as new facet f%d\n",
      facet2->id, qh facet_id));
    if (qh facet_id == (int)qh tracefacet_id) {
      qh tracefacet= facet2;
      fprintf (qh ferr, "qh_mergefacet: RENAME f%d as trace facet f%d.  Current furthest is p%d\n",
	       facet2->id, qh facet_id, qh furthest_id);
    }
    facet2->id= qh facet_id++;
  }
  qh_degen_redundant_neighbors (facet2);
  facet2->tested= False;

  /*
  if (qh IStracing >= 4) 
    qh_errprint ("MERGED", facet2, NULL, NULL, NULL);
    NO LONGER IN SERVICE */

  if (facet2 == qh tracefacet || (qh tracevertex && qh tracevertex->newlist)) {
    fprintf (qh ferr, "qh_mergefacets: trace facet and vertex after merge of f%d and f%d, furthest p%d\n", facet1->id, facet2->id, qh furthest_id);

    /*
    if (facet2 != qh tracefacet)
      qh_errprint ("TRACE", qh tracefacet, 
        (qh tracevertex ? (facetT *)SETfirst_(qh tracevertex->neighbors):NULL),
        NULL, qh tracevertex);      
	NO LONGER IN SERVICE */

  }
  if (qh CHECKfrequently || qh IStracing >= 4) { /* can't check polygon here */
    qh_checkfacet (facet2, True, &waserror);

    if (waserror) qhull_fatal(63);

    qh_checkridge_boundary (facet2);
  }
  if (qh tracevertex) {
    if (qh tracevertex->deleted)
      fprintf (qh ferr, "qh_mergefacet: trace vertex deleted at furthest p%d\n",
	    qh furthest_id);
    else
      qh_checkvertex (qh tracevertex);
  }
  if (qh tracefacet) {
    qh_checkfacet (qh tracefacet, True, &waserror);

    if (waserror) qhull_fatal(64);

  }
  if (traceonce) {
    fprintf (qh ferr, "qh_mergefacet: end of wide tracing\n");
    qh IStracing= tracerestore;
  }
} /* mergefacet */


/*-------------------------------------------------
-mergeneighbors- merges the neighbors of facet1 into facet2
*/
void qh_mergeneighbors(facetT *facet1, facetT *facet2) {
  facetT *neighbor, **neighborp;

  trace4((qh ferr, "qh_mergeneighbors: merge neighbors of f%d and f%d\n",
	  facet1->id, facet2->id));
  qh visit_id++;
  FOREACHneighbor_(facet2) {
    neighbor->visitid= qh visit_id;
  }
  FOREACHneighbor_(facet1) {
    if ((int)neighbor->visitid == qh visit_id) {
      if (neighbor->simplicial)    /* is degen, needs ridges */
	qh_makeridges (neighbor);
      if (SETfirst_(neighbor->neighbors) != facet1) /*keep horizon->newfacet*/
	 qh_setdel (neighbor->neighbors, facet1);
      else {
        qh_setdel(neighbor->neighbors, facet2);
        qh_setreplace(neighbor->neighbors, facet1, facet2);
      }
    }else if (neighbor != facet2) {
      qh_setappend(&(facet2->neighbors), neighbor);
      qh_setreplace(neighbor->neighbors, facet1, facet2);
    }
  }
  qh_setdel(facet1->neighbors, facet2);  /* here for makeridges */
  qh_setdel(facet2->neighbors, facet1);
} /* mergeneighbors */


/*-------------------------------------------------
-mergeridges- merges the ridge set of facet1 into facet2
  may delete all ridges for a vertex
*/
void qh_mergeridges(facetT *facet1, facetT *facet2) {
  ridgeT *ridge, **ridgep;
  vertexT *vertex, **vertexp;

  trace4((qh ferr, "qh_mergeridges: merge ridges of f%d and f%d\n",
	  facet1->id, facet2->id));
  FOREACHridge_(facet2->ridges) {
    if ((ridge->top == facet1) || (ridge->bottom == facet1)) {
      FOREACHvertex_(ridge->vertices)
        vertex->delridge= True;
      qh_delridge(ridge);
      ridgep--; /*repeat*/
    }
  }
  FOREACHridge_(facet1->ridges) {
    if (ridge->top == facet1)
      ridge->top= facet2;
    else
      ridge->bottom= facet2;
    qh_setappend(&(facet2->ridges), ridge);
  }
} /* mergeridges */


/*-------------------------------------------------
-mergevertex_neighbors- merge the vertex neighbors of facet1 to facet2
  deletes vertices if only one neighbor
  assumes neighbor sets are good
*/
void qh_mergevertex_neighbors(facetT *facet1, facetT *facet2) {
  vertexT *vertex, **vertexp;

  trace4((qh ferr, "qh_mergevertex_neighbors: merge vertex neighbors of f%d and f%d\n",
	  facet1->id, facet2->id));
  if (qh tracevertex) {
    fprintf (qh ferr, "qh_mergevertex_neighbors: of f%d and f%d at furthest p%d f0= %x\n",
	     facet1->id, facet2->id, qh furthest_id, (int)qh tracevertex->neighbors->e[0]);

    /*
    qh_errprint ("TRACE", NULL, NULL, NULL, qh tracevertex);
    NO LONGER IN SERVICE */

  }
  FOREACHvertex_(facet1->vertices) {
    if ((int)vertex->visitid != qh vertex_visit) {
      qh_setreplace(vertex->neighbors, facet1, facet2);
    }else {
      qh_setdel(vertex->neighbors, facet1);
      if (!SETsecond_(vertex->neighbors)) {
        zinc_(Zmergevertex);
        trace2((qh ferr, "qh_mergevertex_neighbors: deleted v%d when merging f%d into f%d\n",
          vertex->id, facet1->id, facet2->id));
        qh_setdelsorted (facet2->vertices, vertex);
        vertex->deleted= True;
        qh_setappend (&qh del_vertices, vertex);
      }
    }
  }

  /*
  if (qh tracevertex) 
    qh_errprint ("TRACE", NULL, NULL, NULL, qh tracevertex);
    NO LONGER IN SERVICE */

} /* mergevertex_neighbors */


/*-------------------------------------------------
-mergevertices- merges the vertex set of facet1 into facet2
  preserves vertex_visit for qh_mergevertex_neighbors
  updates qh newvertex_list
*/
void qh_mergevertices(setT *vertices1, setT **vertices2) {
  int newsize= qh_setsize(vertices1)+qh_setsize(*vertices2) - qh hull_dim + 1;
  setT *mergedvertices;
  vertexT *vertex, **vertexp, **vertex2= SETaddr_(*vertices2, vertexT);

  mergedvertices= qh_settemp (newsize);
  FOREACHvertex_(vertices1) {
    if (!*vertex2 || vertex->id > (*vertex2)->id)
      qh_setappend (&mergedvertices, vertex);
    else {
      while (*vertex2 && (*vertex2)->id > vertex->id)
	qh_setappend (&mergedvertices, *vertex2++);
      if (!*vertex2 || (*vertex2)->id < vertex->id)
	qh_setappend (&mergedvertices, vertex);
      else
	qh_setappend (&mergedvertices, *vertex2++);
    }
  }
  while (*vertex2)
    qh_setappend (&mergedvertices, *vertex2++);
  FOREACHvertex_(mergedvertices) {
    if (!vertex->newlist) {
      vertex->newlist= True;
      qh_removevertex (vertex);
      qh_appendvertex (vertex);
    }
  }

  if (newsize < qh_setsize (mergedvertices)) qhull_fatal(65);

  qh_setfree(vertices2);
  *vertices2= mergedvertices;
  qh_settemppop ();
} /* mergevertices */


/*-------------------------------------------------
-mergevertices2d- merges vertices1 into vertices2 in 2-d case
  preserves vertex_visit for qh_mergevertex_neighbors
*/
void qh_mergevertices2d(setT *vertices1, setT *vertices2) {
  vertexT *vertex1A, *vertex1B, *vertex2A, *vertex2B, *vertexA, *vertexB;

  vertex1A= (vertexT *)SETfirst_(vertices1);
  vertex1B= (vertexT *)SETsecond_(vertices1);
  vertex2A= (vertexT *)SETfirst_(vertices2);
  vertex2B= (vertexT *)SETsecond_(vertices2);
  if (vertex1A == vertex2A) {
    vertexA= vertex1B;
    vertexB= vertex2B;
  }else if (vertex1A == vertex2B) {
    vertexA= vertex1B;
    vertexB= vertex2A;
  }else if (vertex1B == vertex2A) {
    vertexA= vertex1A;
    vertexB= vertex2B;
  }else { /* 1B == 2B */
    vertexA= vertex1A;
    vertexB= vertex2A;
  }
  if (vertexA->id > vertexB->id) {
    SETfirst_(vertices2)= vertexA;
    SETsecond_(vertices2)= vertexB;
  }else {
    SETfirst_(vertices2)= vertexB;
    SETsecond_(vertices2)= vertexA;
  }
} /* mergevertices2d */

/*-------------------------------------------------
-neighbor_intersections- return intersection for vertex->neighbors
  returns temporary set of vertices
  does not include vertex
  NULL if an neighbor is simplicial
  NULL if empty set
*/
setT *qh_neighbor_intersections (vertexT *vertex) {
  facetT *neighbor, **neighborp, *neighborA, *neighborB;
  setT *intersect;
  int neighbor_i, neighbor_n;

  FOREACHneighbor_(vertex) {
    if (neighbor->simplicial)
      return NULL;
  }
  neighborA= (facetT *)SETfirst_(vertex->neighbors);
  neighborB= (facetT *)SETsecond_(vertex->neighbors);
  zinc_(Zintersectnum);
  if (!neighborA)
    return NULL;
  if (!neighborB)
    intersect= qh_setcopy (neighborA->vertices, 0);
  else
    intersect= qh_vertexintersect_new (neighborA->vertices, neighborB->vertices);
  qh_settemppush (intersect);
  qh_setdelsorted (intersect, vertex);
  FOREACHneighbor_i_(vertex) {
    if (neighbor_i >= 2) {
      zinc_(Zintersectnum);
      qh_vertexintersect (&intersect, neighbor->vertices);
      if (!SETfirst_(intersect)) {
        zinc_(Zintersectfail);
        qh_settempfree (&intersect);
        return NULL;
      }
    }
  }
  trace3((qh ferr, "qh_neighbor_intersections: %d vertices in neighbor intersection of v%d\n", 
          qh_setsize (intersect), vertex->id));
  return intersect;
} /* neighbor_intersections */

/*-------------------------------------------------
-reducevertices_centrums reduce vertex sets and reset centrums
  qh_isnewmerge_(facet) [i.e., flipped] true if merged since last call
  if 2-d, just clears newmerge flags
returns:
  True if degen_redundant facets or redefined centrums
  vertices are renamed if possible
  centrum's reset for small (qh_MAXnewcentrum), newly merged or renamed facets
*/
boolT qh_reducevertices_centrums (void) {
  int numshare=0, numrename= 0, numcentrums= 0;
  int numdegenredun= 0;
  facetT *newfacet;
  vertexT *vertex, **vertexp;
  realT angle;
  ridgeT *ridge, **ridgep;

  if (qh hull_dim == 2) {
    FORALLnew_facets 
      qh_clearnewmerge_(newfacet);
    return False;
  }
 LABELrestart:
  while (qh_merge_degenredundant (NULL, NULL, &angle))
    numdegenredun++;
  FORALLnew_facets {
    if (qh_isnewmerge_(newfacet)) /* this is seldom needed */
      qh_remove_extravertices (newfacet);
  }
  FORALLnew_facets {
    if (qh_isnewmerge_(newfacet)) {
      FOREACHvertex_(newfacet->vertices) {
	if (vertex->delridge) {
	  if (qh_rename_sharedvertex (vertex, newfacet)) {
	    numshare++;
	    vertexp--; /* repeat since deleted vertex */
	  }
        }
      }
    }
  }
  FORALLvertex_(qh newvertex_list) {
    if (vertex->delridge && !vertex->deleted) {
      vertex->delridge= False;
      if (qh hull_dim >= 4 && qh_redundant_vertex (vertex)) {
	numrename++;
	if (qh_merge_degenredundant (NULL, NULL, &angle)) {
	  numdegenredun++;
	  goto LABELrestart;
	}
      }
    }
  }
  FORALLnew_facets {
    if (qh_isnewmerge_(newfacet)) {
      qh_clearnewmerge_(newfacet);
      if (newfacet->center
      && qh_setsize (newfacet->vertices) <= qh hull_dim + qh_MAXnewcentrum) {
	qh_memfree (newfacet->center, qh center_size);
	newfacet->center= NULL;
	newfacet->tested= False;
	FOREACHridge_(newfacet->ridges)
	  ridge->tested= False;
	numcentrums++;
      }
    }
  }
  trace1((qh ferr, "qh_reducevertices_centrums: renamed %d shared vertices and %d redundant\nvertices.  Redefined %d centrums and merged %d degen, redundant facets\n",
	  numshare, numrename, numcentrums, numdegenredun));
  if (numdegenredun + numcentrums)
    return True;
  return False;
} /* reducevertices_centrums */
      
/*-------------------------------------------------
-redundant_vertex- returns true if detect and rename redundant vertex
  vertices have full ->neighbors 
  only needed if vertex->delridge and hull_dim >= 4
returns:
  may add degen facets to facet_mergeset
  doesn't change vertex->neighbors or create redundant facets
*/
vertexT *qh_redundant_vertex (vertexT *vertex) {
  vertexT *newvertex= NULL;
  setT *vertices, *ridges;

  trace3((qh ferr, "qh_redundant_vertex: check if v%d can be renamed\n", vertex->id));  
  if ((vertices= qh_neighbor_intersections (vertex))) {
    ridges= qh_vertexridges (vertex);
    if ((newvertex= qh_find_newvertex (vertex, vertices, ridges)))
      qh_renamevertex (vertex, newvertex, ridges, NULL, NULL);
    qh_settempfree (&ridges);
    qh_settempfree (&vertices);
  }
  return newvertex;
} /* redundant_vertex */

/*-------------------------------------------------
-remove_extravertices in non-simplicial facets
  returns True if it finds them
*/
boolT qh_remove_extravertices (facetT *facet) {
  ridgeT *ridge, **ridgep;
  vertexT *vertex, **vertexp, *vertexA, **vertexAp;
  boolT foundrem= False;

  trace4((qh ferr, "qh_remove_extravertices: test f%d for extra vertices\n",
	  facet->id));
  FOREACHvertex_(facet->vertices)
    vertex->seen= False;
  FOREACHridge_(facet->ridges) {
    FOREACHvertexA_(ridge->vertices)
      vertexA->seen= True;
  }
  FOREACHvertex_(facet->vertices) {
    if (!vertex->seen) {
      foundrem= True;
      zinc_(Zremvertex);
      qh_setdelsorted (facet->vertices, vertex);
      qh_setdel (vertex->neighbors, facet);
      if (!qh_setsize (vertex->neighbors)) {
	vertex->deleted= True;
	qh_setappend (&qh del_vertices, vertex);
	zinc_(Zremvertexdel);
	trace2((qh ferr, "qh_remove_extravertices: v%d deleted because it's lost all ridges\n", vertex->id));
      }else
	trace3((qh ferr, "qh_remove_extravertices: v%d removed from f%d because it's lost all ridges\n", vertex->id, facet->id));
      vertexp--; /*repeat*/
    }
  }
  return foundrem;
} /* remove_extravertices */

/*-------------------------------------------------
-rename_sharedvertex- detect and rename if shared vertex in facet
  vertices have full ->neighbors
returns:
  newvertex or NULL
  the vertex may still exist in other facets (i.e., a neighbor was pinched)
  does not change facet->neighbors, updates vertex->neighbors
notes:
  a shared vertex for a facet is only in ridges to one neighbor
  this may undo a pinched facet
 
  it does not catch pinches involving multiple facets.  These appear
  to be difficult to detect, since an exhaustive search is too expensive.
*/
vertexT *qh_rename_sharedvertex (vertexT *vertex, facetT *facet) {
  facetT *neighbor, **neighborp, *neighborA= NULL;
  setT *vertices, *ridges;
  vertexT *newvertex;

  if (qh_setsize (vertex->neighbors) == 2) {
    neighborA= (facetT *)SETfirst_(vertex->neighbors);
    if (neighborA == facet)
      neighborA= (facetT *)SETsecond_(vertex->neighbors);
  }else if (qh hull_dim == 3)
    return NULL;
  else {
    qh visit_id++;
    FOREACHneighbor_(facet)
      neighbor->visitid= qh visit_id;
    FOREACHneighbor_(vertex) {
      if ((int)neighbor->visitid == qh visit_id) {
        if (neighborA)
          return NULL;
        neighborA= neighbor;
      }
    }

    if (!neighborA) qhull_fatal(66);

  }
  /* the vertex is shared by facet and neighborA */
  ridges= qh_settemp (qh TEMPsize);
  neighborA->visitid= ++qh visit_id;
  qh_vertexridges_facet (vertex, facet, &ridges);
  trace2((qh ferr, "qh_rename_sharedvertex: p%d (v%d) is shared by f%d (%d ridges) and f%d\n",
    qh_pointid(vertex->point), vertex->id, facet->id, qh_setsize (ridges), neighborA->id));
  zinc_(Zintersectnum);
  vertices= qh_vertexintersect_new (facet->vertices, neighborA->vertices);
  qh_setdel (vertices, vertex);
  qh_settemppush (vertices);
  if ((newvertex= qh_find_newvertex (vertex, vertices, ridges))) 
    qh_renamevertex (vertex, newvertex, ridges, facet, neighborA);
  qh_settempfree (&vertices);
  qh_settempfree (&ridges);
  return newvertex;
} /* rename_sharedvertex */

/*-------------------------------------------------
-renameridgevertex- renames oldvertex as newvertex in ridge
*/
void qh_renameridgevertex(ridgeT *ridge, vertexT *oldvertex, vertexT *newvertex) {
  int nth= 0, oldnth;
  facetT *temp;
  vertexT *vertex, **vertexp;

  oldnth= qh_setindex (ridge->vertices, oldvertex);
  qh_setdelnthsorted (ridge->vertices, oldnth);
  FOREACHvertex_(ridge->vertices) {
    if (vertex == newvertex) {
      zinc_(Zdelridge);
      if (ridge->tested && ridge->nonconvex)
	qh_copynonconvex (ridge);
      qh_delridge (ridge);
      trace2((qh ferr, "qh_renameridgevertex: ridge r%d deleted.  It contained both v%d and v%d\n",
        ridge->id, oldvertex->id, newvertex->id));
      return;
    }
    if (vertex->id < newvertex->id)
      break;
    nth++;
  }
  qh_setaddnth(&ridge->vertices, nth, newvertex);
  if (abs(oldnth - nth)%2) {
    trace3((qh ferr, "qh_renameridgevertex: swapped the top and bottom of ridge r%d\n", 
	    ridge->id));
    temp= ridge->top;
    ridge->top= ridge->bottom;
    ridge->bottom= temp;
  }
} /* renameridgevertex */


/*-------------------------------------------------
-renamevertex- renames oldvertex as newvertex in ridges 
  oldvertex may still exist afterwards
  gives oldfacet/neighborA if shared between facets
notes:
  can not change neighbors of newvertex (since it's a subset)
*/
void qh_renamevertex(vertexT *oldvertex, vertexT *newvertex, setT *ridges, facetT *oldfacet, facetT *neighborA) {
  facetT *neighbor, **neighborp;
  ridgeT *ridge, **ridgep;
  boolT istrace= False;

  if (qh IStracing >= 2 || oldvertex->id == qh tracevertex_id ||
	newvertex->id == qh tracevertex_id)
    istrace= True;
  FOREACHridge_(ridges) 
    qh_renameridgevertex (ridge, oldvertex, newvertex);
  if (qh CHECKfrequently) {
    FOREACHneighbor_(oldvertex) {
      qh_checkridge_boundary (neighbor);
    }
  }    
  if (!oldfacet) {
    zinc_(Zrenameall);
    if (istrace)
      fprintf (qh ferr, "qh_renamevertex: renamed v%d to v%d in several facets\n",
               oldvertex->id, newvertex->id);
    FOREACHneighbor_(oldvertex) {
      qh_maydropneighbor (neighbor);
      qh_setdelsorted (neighbor->vertices, oldvertex);
      if (qh_remove_extravertices (neighbor))
        neighborp--; /* neighbor may be deleted */
    }
    if (!oldvertex->deleted) {
      oldvertex->deleted= True;
      qh_setappend (&qh del_vertices, oldvertex);
    }
  }else if (qh_setsize (oldvertex->neighbors) == 2) {
    zinc_(Zrenameshare);
    if (istrace)
      fprintf (qh ferr, "qh_renamevertex: renamed v%d to v%d in oldfacet f%d\n", 
               oldvertex->id, newvertex->id, oldfacet->id);
    FOREACHneighbor_(oldvertex)
      qh_setdelsorted (neighbor->vertices, oldvertex);
    oldvertex->deleted= True;
    qh_setappend (&qh del_vertices, oldvertex);
  }else {
    zinc_(Zrenamepinch);
    if (istrace || qh IStracing)
      fprintf (qh ferr, "qh_renamevertex: renamed pinched v%d to v%d between f%d and f%d\n", 
               oldvertex->id, newvertex->id, oldfacet->id, neighborA->id);
    qh_setdelsorted (oldfacet->vertices, oldvertex);
    qh_setdel (oldvertex->neighbors, oldfacet);
    qh_remove_extravertices (neighborA);
  }
} /* renamevertex */


/*-----------------------------------------
-test_appendmerge- tests facet/neighbor and appends to mergeset if nonconvex
  sets facet->center as needed
returns:
  true if appends to mergeset
    sets 'acoplanar' if angle coplanar
*/
boolT qh_test_appendmerge (facetT *facet, facetT *neighbor) {
  realT dist, dist2= -REALmax, angle;
  boolT isconcave= False, iscoplanar= False;
  mergeT *merge;

  angle= qh_getangle(facet->normal, neighbor->normal);
  zinc_(Zangletests);
  if (angle > qh cos_max) {
    zinc_(Zcoplanarangle);
    merge= qh_appendmergeset(facet, neighbor, &angle);
    merge->anglecoplanar= True;
    trace2((qh ferr, "qh_test_appendmerge: coplanar angle %4.4g between f%d and f%d\n",
       angle, facet->id, neighbor->id));
    return True;
  }else {
    if (!facet->center)
      facet->center= qh_getcentrum (facet);
    zzinc_(Zcentrumtests);
    qh_distplane(facet->center, neighbor, &dist);
    if (dist > qh centrum_radius)
      isconcave= True;
    else {
      if (dist > -qh centrum_radius)
	iscoplanar= True;
      if (!neighbor->center)
        neighbor->center= qh_getcentrum (neighbor);
      zinc_(Zcentrumtests);
      qh_distplane(neighbor->center, facet, &dist2);
      if (dist2 > qh centrum_radius)
        isconcave= True;
      else if (dist2 > -qh centrum_radius)
        iscoplanar= True;
    }
    if (isconcave) {
      zinc_(Zconcaveridge);
      angle += qh_ANGLEconcave + 0.5;
      qh_appendmergeset(facet, neighbor, &angle);
      trace0((qh ferr, "qh_test_appendmerge: concave f%d to f%d dist %4.4g and reverse dist %4.4g angle %4.4g\n",
	      facet->id, neighbor->id, dist, dist2, angle));
      return True;
    }else if (iscoplanar) {
      zinc_(Zcoplanarcentrum);
      qh_appendmergeset(facet, neighbor, &angle);
      trace2((qh ferr, "qh_test_appendmerge: coplanar f%d to f%d dist %4.4g, reverse dist %4.4g angle %4.4g\n",
	      facet->id, neighbor->id, dist, dist2, angle));
      return True;
    }
  }
  return False;
} /* test_appendmerge */


/*------------------------------------------
-tracemerging- print trace message if active
*/
void qh_tracemerging (char *string) {
  realT cpu;
  time_t timedata;
  struct tm *tp;

  if (qh REPORTfreq && (zzval_(Ztotmerge) > qh mergereport+qh REPORTfreq/2)) {
    qh mergereport= zzval_(Ztotmerge);
    time (&timedata);
    tp= localtime (&timedata);
    cpu= clock();
    cpu /= qh_SECticks;
    zinc_(Zdistio);
    fprintf (qh ferr, "\n\
At %d:%d:%d & %2.5g CPU secs, qhull has merged %d facets.  The hull\n\
  contains %d facets and %d vertices. p%d was the last point.\n",
      tp->tm_hour, tp->tm_min, tp->tm_sec, cpu,
      zzval_(Ztotmerge), qh num_facets - qh num_visible,
      qh num_vertices-qh_setsize (qh del_vertices), qh furthest_id);
  }
} /* tracemerging */

/*------------------------------------------
-vertexridges- return temporary set of ridges adjacent to a vertex
  vertex->neighbors defined
  uses qh visit_id
  does not include implicit ridges for simplicial facets
*/
setT *qh_vertexridges (vertexT *vertex) {
  facetT *neighbor, **neighborp;
  setT *ridges= qh_settemp (qh TEMPsize);
  int size;

  qh visit_id++;
  FOREACHneighbor_(vertex)
    neighbor->visitid= qh visit_id;
  FOREACHneighbor_(vertex) {
    if (*neighborp)   /* no new ridges in last neighbor */
      qh_vertexridges_facet (vertex, neighbor, &ridges);
  }
  if (qh PRINTstatistics || qh IStracing) {
    size= qh_setsize (ridges);
    zinc_(Zvertexridge);
    zadd_(Zvertexridgetot, size);
    zmax_(Zvertexridgemax, size);
    trace3((qh ferr, "qh_vertexridges: found %d ridges for v%d\n",
             size, vertex->id));
  }
  return ridges;
} /* vertexridges */

/*------------------------------------------
-vertexridges_facet- add adjacent ridges for vertex in facet
  skips ridges if neighbor->visitid< visit_id 
returns:
  sets facet->visitid to visit_id-1
*/
void qh_vertexridges_facet (vertexT *vertex, facetT *facet, setT **ridges) {
  ridgeT *ridge, **ridgep;
  facetT *neighbor;

  FOREACHridge_(facet->ridges) {
    neighbor= otherfacet_(ridge, facet);
    if ((int)neighbor->visitid == qh visit_id 
    && qh_setin (ridge->vertices, vertex))
      qh_setappend (ridges, ridge);
  }
  facet->visitid= qh visit_id-1;
} /* vertexridges_facet */




/*************************************************************************/
/****************** implementation code from global.c ********************/
/*************************************************************************/

/* global.c -- contains all the globals of the qhull application

   see README 
   
   see qhull.h for qh globals and function prototypes

   see qhull_a.h for internal functions

   copyright (c) 1993-1994, The Geometry Center
*/

#if qh_QHpointer
qhT *qh_qh=  NULL;
#else
qhT qh_qh; /*= {0};*/ /* remove "= {0}" if this causes a compiler error.  Also
		     qh_qhstat in stat.c and qhmem in mem.c.  */
#endif

/* ------------Simple all purpose error report-------------*/

#ifdef GAMBIT_EXCEPTIONS
ErrorInQhull::~ErrorInQhull() { }

gText ErrorInQhull::Description(void) const
{
  return "Error somewhere in Qhull";
}
#endif

void qhull_fatal(int errorno)
{
#ifdef GAMBIT_EXCEPTIONS
  throw ErrorInQhull();
#endif

  printf("\nError number %d in qhull.\n", errorno);
  exit(0);
}


/*-------------------------------------------
-appendprint- append output format to qh PRINTout unless already defined
*/
void qh_appendprint (int format) {
  int i;

  for (i=0; i < qh_PRINTEND; i++) {
    if (qh PRINTout[i] == format)
      break;
    if (!qh PRINTout[i]) {
      qh PRINTout[i]= format;
      break;
    }
  }
} /* appendprint */
     
/*-------------------------------------------
-freebuffers- free up global memory buffers
  must match initbuffers()
*/

void qh_freebuffers (void) {

  trace5((qh ferr, "qh_freebuffers: freeing up global memory buffers\n"));
  qh_memfree (qh NEARzero, qh hull_dim * sizeof(realT));
  qh_memfree (qh lower_threshold, (qh input_dim+1) * sizeof(realT));
  qh_memfree (qh upper_threshold, (qh input_dim+1) * sizeof(realT));
  qh_memfree (qh lower_bound, (qh input_dim+1) * sizeof(realT));
  qh_memfree (qh upper_bound, (qh input_dim+1) * sizeof(realT));
  qh_memfree (qh gm_matrix, (qh hull_dim+1) * qh hull_dim * sizeof(coordT));
  qh_memfree (qh gm_row, (qh hull_dim+1) * sizeof(coordT *));
  qh_setfree (&qh hash_table);
  qh_setfree (&qh other_points);
  qh_setfree (&qh del_vertices);
  /* qh facet_mergeset is a temp */
  qh NEARzero= qh lower_threshold= qh upper_threshold= NULL;
  qh lower_bound= qh upper_bound= NULL;
  qh gm_matrix= NULL;
  qh gm_row= NULL;
  if (qh line)
    free (qh line);
  if (qh first_point && qh POINTSmalloc)
    free(qh first_point);
  trace5((qh ferr, "qh_freebuffers: finished\n"));
} /* freebuffers */


/*-------------------------------------------
-freeqhull- free global memory
  if allmem, frees all allocated data structures
  else, frees all long memory
    rest of memory freed by qh_memfreeshort();
*/
void qh_freeqhull (boolT allmem) {
  facetT *facet;
  vertexT *vertex;
  ridgeT *ridge, **ridgep;
  mergeT *merge, **mergep;

  trace1((qh ferr, "qh_freeqhull: free global memory\n"));
  qh NOerrexit= True;  /* no more setjmp */
  if (allmem) {
    qh_clearcenters (qh_none);
    while ((vertex= qh vertex_list)) {
      if (vertex->next)
        qh_delvertex (vertex);
      else {
        qh_memfree (vertex, sizeof(vertexT));
        qh vertex_list= NULL;
      }
    }
  }else if (qh VERTEXneighbors) {
    FORALLvertices 
      qh_setfreelong (&(vertex->neighbors));
  }
  if (allmem) {
    FORALLfacets {
      if (!facet->visible) {
	FOREACHridge_(facet->ridges)
	  ridge->seen= False;
      }
    }
    FORALLfacets {
      FOREACHridge_(facet->ridges)
        ridge->seen ^= True;
    }
    while ((facet= qh facet_list)) {
      FOREACHridge_(facet->ridges) {
        if (ridge->seen) {
          qh_setfree(&(ridge->vertices));
          qh_memfree(ridge, sizeof(ridgeT));
        }else
          ridge->seen= True;
      }
      if (facet->next)
        qh_delfacet (facet);
      else {
        qh_memfree (facet, sizeof(facetT));
        qh facet_list= NULL;
      }
    }
  }else {
    FORALLfacets {
      qh_setfreelong (&(facet->outsideset));
      qh_setfreelong (&(facet->coplanarset));
      if (!facet->simplicial) {
        qh_setfreelong (&(facet->neighbors));
        qh_setfreelong (&(facet->ridges));
        qh_setfreelong (&(facet->vertices));
      }
    }
  }
  qh_setfree (&qh hash_table);
  FOREACHmerge_(qh facet_mergeset)  /* usually empty */
    qh_memfree (merge, sizeof(mergeT));
  qh_freebuffers();
  qh_freestatistics();
  qh_settempfree_all();
#if qh_QHpointer
  free (qh_qh);
  qh_qh= NULL;
#endif
} /* freeqhull */


/*---------------------------------------------
-init_qhull_command- build qhull_command from argc/argv
*/
void qh_init_qhull_command(int argc, char *argv[]) {
  int i;

  strcpy (qh qhull_command, argv[0]);
  for (i=1; i<argc; i++) {
    if (strlen (qh qhull_command) + strlen(argv[i]) + 1 < sizeof(qh qhull_command)) {
      strcat (qh qhull_command, " ");
      strcat (qh qhull_command, argv[i]);
    }else qhull_fatal(67);

  }
} /* init_qhull_command */

/*---------------------------------------------
-initflags- set flags and initialized constants from command line
  strips off first word
  see 'prompt' in unix.c for documentation
  see also initthresholds
  strtol/strtod may or may not skip trailing spaces
  does not reset any flags
    use the qh structure to change flags during execution of qhull
  
*/

void qh_initflags(char *command) {
  int k;
  char *s= command, *t, *prev_s;

  while (*s && !isspace(*s))  /* need a better convention */
    s++;
  while (*s) {
    while (*s && isspace(*s))
      s++;
    if (*s == '-')
      s++;
    prev_s= s;
    switch (*s++) {
    case 'd':
      qh DELAUNAY= True;
      break;
    case 'f':
      qh_appendprint (qh_PRINTfacets);
      break;
    case 'i':
      qh_appendprint (qh_PRINTincidences);
      break;
    case 'm':
      qh_appendprint (qh_PRINTmathematica);
      break;
    case 'n':
      qh_appendprint (qh_PRINTnormals);
      break;
    case 'o':
      qh_appendprint (qh_PRINToff);
      break;
    case 'p':
      qh_appendprint (qh_PRINTpoints);
      break;
    case 's':
      qh PRINTsummary= True;
      break;
    case 'v':
      qh VORONOI= True;
      qh DELAUNAY= True;
      break;
    case 'A':
      if (!isdigit(*s) && *s != '.' && *s != '-') 
	fprintf(qh ferr, "qhull warning: no maximum cosine angle given for option A.  Ignored.\n");
      else {
	if (*s == '-') {
	  qh premerge_cos= -qh_strtod (s, &s);
	  qh PREmerge= True;
	}else {
	  qh postmerge_cos= qh_strtod (s, &s);
	  qh POSTmerge= True;
	}
	qh MERGING= True; 
      }
      break;
    case 'C':
      if (!isdigit(*s) && *s != '.' && *s != '-')
	fprintf(qh ferr, "qhull warning: no centrum radius given for option C.  Ignored.\n");
      else {
	if (*s == '-') {
	  qh premerge_centrum= -qh_strtod (s, &s);
	  qh PREmerge= True;
	}else {
	  qh postmerge_centrum= qh_strtod (s, &s);
	  qh POSTmerge= True;
	}
	qh MERGING= True; 
      }
      break;
    case 'E':
      if (*s == '-')
	fprintf(qh ferr, "qhull warning: negative maximum roundoff given for option A.  Ignored.\n");
      else if (!isdigit(*s))
	fprintf(qh ferr, "qhull warning: no maximum roundoff given for option E.  Ignored.\n");
      else {
	qh DISTround= qh_strtod (s, &s);
	qh SETroundoff= True;
      }
      break;
    case 'R':
      if (!isdigit(*s))
	fprintf(qh ferr, "qhull warning: no random perturbation given for option R.  Ignored\n");
      else {
	qh RANDOMfactor= qh_strtod (s, &s);
        qh RANDOMdist= True;
      }
      break;
    case 'V':
      if (!isdigit(*s) && *s != '-')
	fprintf(qh ferr, "qhull warning: no distance given for option V.  Ignored\n");
      else 
	qh MINvisible= qh_strtod (s, &s);
      break;
    case 'W':
      if (*s == '-')
	fprintf(qh ferr, "qhull warning: negative width for option W.  Ignored.\n");
      else if (!isdigit(*s))
	fprintf(qh ferr, "qhull warning: no hull width given for option W.  Ignored\n");
      else {
	qh MINoutside= qh_strtod (s, &s);
        qh APPROXhull= True;
      }
      break;
    /************  sub menus ***************/
    case 'G':
      qh_appendprint (qh_PRINTgeom);
      while (*s && !isspace(*s)) {
	switch(*s++) {
        case 'a':
          qh PRINTdots= True;
          break;
        case 'c':
          qh PRINTcentrums= True;
          break;
	case 'h':
	  qh DOintersections= True;
	  break;
	case 'i':
	  qh PRINTinner= True;
	  break;
	case 'n':
	  qh PRINTnoplanes= True;
	  break;
	case 'o':
	  qh PRINTouter= True;
	  break;
	case 'p':
	  qh PRINTcoplanar= True;
	  break;
	case 'r':
	  qh PRINTridges= True;
	  break;
	case 'v':
	  qh PRINTspheres= True;
	  break;
	case 'D':
	  if (!isdigit (*s))
	    fprintf (qh ferr, "qhull input error: missing dimension for 'GD' option\n");
	  else
  	    qh DROPdim= qh_strtol (s, &s);
	  break;
	default:
	  s--;
	  fprintf (qh ferr, "qhull warning: unknown 'G' print option %c, rest ignored\n", (int)s[0]);
	  while (*++s && !isspace(*s));
	  break;
	}
      }
      break;
    case 'P':
      while (*s && !isspace(*s)) {
	switch(*s++) {
	case 'd': case 'D':
	  qh_strtol (s, &s);
	  if (*s == ':')
	    qh_strtod (++s, &s);
	  break;
        case 'g':
          qh PRINTgood= True;
          break;
        case 'G':
          qh PRINTneighbors= True;
          break;
        case 'o':
          qh FORCEoutput= True;
          break;
        case 'p':
          qh PRINTprecision= False;
          break;
	default:
	  s--;
	  fprintf (qh ferr, "qhull warning: unknown 'P' print option %c, rest ignored\n", (int)s[-1]);
	  while (*++s && !isspace(*s));
	  break;
	}
      }
      break;
    case 'Q':
      while (*s && !isspace(*s)) {
	switch(*s++) {
	case 'b': case 'B':
	  k= qh_strtol (s, &s);
	  t= NULL;
	  if (*s == ':' && qh_strtod(++s, &s) == 0.0) {
	    t= s;            /* need true dimension for memory allocation */
	    while (*t && !isspace(*t)) {
	      if (toupper(*t++) == 'B' 
	       && k == qh_strtol (t, &t)
	       && *t++ == ':'
	       && qh_strtod(t, &t) == 0.0) {
	        qh PROJECTinput++;
	        trace2((qh ferr, "qh_initflags: project dimension %d\n", k));
	        break;
	      }
	    }
  	  }
	  if (!t || !isspace(*t))
	    qh SCALEinput= True;
	  break;
	case 'c':
	  qh KEEPcoplanar= True;
	  break;
	case 'f':
	  qh BESToutside= True;
	  break;
	case 'g':
	  qh ONLYgood= True;
	  break;
	case 'i':
	  qh KEEPinside= True;
	  break;
	case 'm':
	  qh ONLYmax= True;
	  break;
	case 'r':
	  qh RANDOMoutside= True;
	  break;
	case 's':
	  qh ALLpoints= True;
	  break;
	case 'v':
	  qh VIRTUALmemory= True;
	  break;
	case 'G':
	  if (qh GOODpoint) {
	    fprintf (qh ferr, "qhull warning: good point already defined for QGn.  Ignored\n");
            qh_strtol(s, &s);
          }else if (!isdigit(*s) && (*s != '-'))
	    fprintf (qh ferr, "qhull warning: no good point id given for option QGn.  Ignored\n");
	  else if (*s == '-') 
 	    qh GOODpoint= qh_strtol(s, &s)-1;
	  else
 	    qh GOODpoint= qh_strtol(s, &s)+1;
	  break;
	case 'R':
          if (!isdigit(*s) && *s != '-')
	    fprintf (qh ferr, "qhull warning: missing random seed for option QRn.  Ignored\n");
	  else
 	    qh ROTATErandom= qh_strtol(s, &s);
	  break;
	case 'V':
	  if (qh GOODvertex) {
	    fprintf (qh ferr, "qhull warning: good vertex already defined for QV.  Ignored\n");
            qh_strtol(s, &s);
          }else if (!isdigit(*s) && (*s != '-'))
	    fprintf (qh ferr, "qhull warning: no good point id given for QV.  Ignored\n");
	  else if (*s == '-') 
 	    qh GOODvertex= qh_strtol(s, &s)-1;
	  else
 	    qh GOODvertex= qh_strtol(s, &s)+1;
	  break;
	default:
	  fprintf (qh ferr, "qhull warning: unknown 'Q' qhull option %c, rest ignored\n", (int)s[-1]);
	  while (*++s && !isspace(*s));
	  break;
	}
      }
      break;
    case 'T':
      while (*s && !isspace(*s)) {
	if (isdigit(*s) || *s == '-')
	  qh IStracing= qh_strtol(s, &s);
	else switch(*s++) {
	case 'c':
	  qh CHECKfrequently= True;
	  break;
	case 's':
	  qh PRINTstatistics= True;
	  break;
	case 'v':
	  qh VERIFYoutput= True;
	  break;
	case 'C':
	  if (!isdigit(*s))
	    fprintf (qh ferr, "qhull warning: no point given for trace option C.  Ignored\n");
	  else
	    qh STOPcone= qh_strtol (s, &s)+1;
	  break;
	case 'F':
	  if (!isdigit(*s))
	    fprintf (qh ferr, "qhull warning: no count of new facets for trace option P.  Ignored\n");
	  else
	    qh REPORTfreq= qh_strtol (s, &s);
	  break;
	case 'P':
	  if (!isdigit(*s))
	    fprintf (qh ferr, "qhull warning: no point given for trace option P.  Ignored\n");
	  else
	    qh TRACEpoint= qh_strtol (s, &s);
	  break;
	case 'M':
	  if (!isdigit(*s))
	    fprintf (qh ferr, "qhull warning: no merge given for trace option M.  Ignored\n");
	  else
	    qh TRACEmerge= qh_strtol (s, &s);
	  break;
	case 'V':
	  if (!isdigit(*s) && *s != '-')
	    fprintf (qh ferr, "qhull warning: no point given for trace option V.  Ignored\n");
	  else if (*s == '-')
	    qh STOPpoint= qh_strtol (s, &s)-1;
	  else
	    qh STOPpoint= qh_strtol (s, &s)+1;
	  break;
	case 'W':
	  if (!isdigit(*s))
	    fprintf (qh ferr, "qhull warning: no max width given for trace option D.  Ignored\n");
	  else 
 	    qh TRACEdist= (realT) qh_strtod (s, &s);
	  break;
	default:
	  fprintf (qh ferr, "qhull warning: unknown 'T' trace option %c, rest ignored\n", (int)s[-1]);
	  while (*++s && !isspace(*s));
	  break;
	}
      }
      break;
    default:
      fprintf (qh ferr, "qhull warning: unknown flag %c (%x)\n", (int)s[-1],
	       (int)s[-1]);
      break;
    }
    if (s-1 == prev_s && *s && !isspace(*s)) {
      fprintf (qh ferr, "qhull warning: missing space after flag %c (%x); reserved for menu. Skipped.\n",
	       (int)*prev_s, (int)*prev_s);
      while (*s && !isspace(*s))
	s++;
    }
  }
} /* initflags */


/*-------------------------------------------
-initqhull_buffers- initialize global memory buffers
  must match freebuffers()
*/
void qh_initqhull_buffers (void) {
  int k;

  qh TEMPsize= (qhmem.LASTsize - sizeof (setT))/SETelemsize;
  qh other_points= qh_setnew (qh TEMPsize);
  qh del_vertices= qh_setnew (qh TEMPsize);
  qh NEARzero= (realT *)qh_memalloc(qh hull_dim * sizeof(realT));
  qh lower_threshold= (realT *)qh_memalloc((qh input_dim+1) * sizeof(realT));
  qh upper_threshold= (realT *)qh_memalloc((qh input_dim+1) * sizeof(realT));
  qh lower_bound= (realT *)qh_memalloc((qh input_dim+1) * sizeof(realT));
  qh upper_bound= (realT *)qh_memalloc((qh input_dim+1) * sizeof(realT));
  for(k= qh input_dim+1; k--; ) {
    qh lower_threshold[k]= -REALmax;
    qh upper_threshold[k]= REALmax;
    qh lower_bound[k]= -REALmax;
    qh upper_bound[k]= REALmax;
  }
  qh gm_matrix= (coordT *)qh_memalloc((qh hull_dim+1) * qh hull_dim * sizeof(coordT));
  qh gm_row= (coordT **)qh_memalloc((qh hull_dim+1) * sizeof(coordT *));
} /* initqhull_buffers */

/*---------------------------------------------
-initqhull_globals- initialize globals
  ismalloc set if points were malloc'd and qhull should free at end
returns:
  sets qh first_point, num_points, input_dim, hull_dim and others
  modifies hull_dim if ((DELAUNAY and PROJECTdelaunay) or PROJECTinput)
  seeds random number generator (seed=1 if tracing)
  adjust user flags as needed
  also checks hull_dim dependencies and constants
*/
void qh_initqhull_globals (coordT *points, int numpoints, int dim, boolT ismalloc) {
  int seed, pointsneeded, extra= 0, i, randi;
  boolT printgeom= False, printother= False, printmath= False;
  realT randr;
  
  time_t timedata;

  trace0((qh ferr, "qh_initqhull_globals: for %s | %s\n", qh rbox_command, 
      qh qhull_command));
  qh POINTSmalloc= ismalloc;
  qh first_point= points;
  qh num_points= numpoints;
  qh hull_dim= qh input_dim= dim;
  if (qh DELAUNAY) {
    qh KEEPcoplanar= True;
    qh KEEPinside= True;
  }
  if (qh PREmerge) {
    /* e.g., rbox 1000 t W8e-6 | qhull C-0, can drop an outside point, since 
       qh_findbest doesn't detect nearly coplanar when point beyond facets */
    qh BESToutside= True;
  }
  if (qh MERGING)
    qh CENTERtype= qh_centrum;
  else if (qh VORONOI)
    qh CENTERtype= qh_voronoi;
  if (qh PROJECTinput || (qh DELAUNAY && qh PROJECTdelaunay)) {
    qh hull_dim -= qh PROJECTinput;
    if (qh DELAUNAY) {
      qh hull_dim++;
      extra= 1;
    }
  }

  if (qh hull_dim <= 1) qhull_fatal(68);

  trace2((qh ferr, "qh_initqhull_globals: initialize globals.  dim %d numpoints %d malloc? %d projected %d to hull_dim %d\n",
	dim, numpoints, ismalloc, qh PROJECTinput, qh hull_dim));
  qh normal_size= qh hull_dim * sizeof(coordT);
  qh center_size= qh normal_size - sizeof(coordT);
  pointsneeded= qh hull_dim+1;
  if (qh GOODpoint)
    pointsneeded++;
  if (qh GOODpoint > 0) 
    qh GOODpointp= qh_point (qh GOODpoint-1);
  else if (qh GOODpoint < 0) 
    qh GOODpointp= qh_point (-qh GOODpoint-1);
  if (qh GOODvertex > 0)
    qh GOODvertexp= qh_point (qh GOODvertex-1);
  else if (qh GOODvertex < 0) 
    qh GOODvertexp= qh_point (-qh GOODvertex-1);

  if ((qh GOODpointp
       && (qh GOODpointp < qh first_point
	   || qh GOODpointp > qh_point (qh num_points-1)))
    || (qh GOODvertexp
	&& (qh GOODvertexp < qh first_point
	    || qh GOODvertexp > qh_point (qh num_points-1)))) qhull_fatal(69);

  if (qh TRACEpoint != -1 || qh TRACEdist < REALmax/2 || qh TRACEmerge) {
    qh TRACElevel= (qh IStracing? qh IStracing : 3);
    qh IStracing= 0;
  }
  if (qh ROTATErandom == 0 || qh ROTATErandom == -1) {
    seed= time (&timedata);
    qh ROTATErandom= seed;
  }else if (qh ROTATErandom > 0)
    seed= qh ROTATErandom;
  else
    seed= 1;
  qh_RANDOMseed_(seed);
  randr= 0.0;
  for (i= 1000; i--; ) {
    randi= qh_RANDOMint;
    randr += randi;

    /* DEBUG */
    /* - I reset qh_RANDOMmax in qhull.h, but not intelligently.
       Expect more trouble inn the future. 
    printf("qh_RANDOMmax is %d.\n", qh_RANDOMmax);
    printf("randi is %d.\n", randi); */
    

    if (randi > qh_RANDOMmax) qhull_fatal(70);

  }
  if (randr/1000 < qh_RANDOMmax/10)
    fprintf (qh ferr, "qhull configuration warning (initqhull_globals): average of 1000 randoms %.2g much less than expected (%.2g).  Is qh_RANDOMmax wrong?\n",
	     randr/1000, qh_RANDOMmax/2);
  qh RANDOMa= 2.0 * qh RANDOMfactor/qh_RANDOMmax;
  qh RANDOMb= 1.0 - qh RANDOMfactor;

#ifndef __BCC55__
  // This condition is always false under BCC55
  if (qh_HASHfactor < 1.1) qhull_fatal(71);
#endif  // __BCC55__

  if (numpoints+extra < pointsneeded) qhull_fatal(72);

  for (i= qh_PRINTEND; i--; ) {
    if (qh PRINTout[i] == qh_PRINTmathematica)
      printmath= True;
    if (qh PRINTout[i] == qh_PRINTgeom)
      printgeom= True;
    else if (qh PRINTout[i])
      printother= True;
  }
  if (printmath && qh hull_dim > 3) qhull_fatal(73);

  if (printgeom) {

    if (qh hull_dim > 4) qhull_fatal(74);

    if (qh PRINTnoplanes && !(qh PRINTcoplanar + qh PRINTcentrums
     + qh PRINTdots + qh PRINTspheres + qh DOintersections + qh PRINTridges)) 
      qhull_fatal(75);

    if (qh VORONOI && (qh hull_dim > 3 || qh DROPdim >= 0)) qhull_fatal(76);

    if (qh hull_dim == 4 && qh DROPdim == -1 &&
	(qh PRINTcoplanar || qh PRINTspheres || qh PRINTcentrums)) {
      fprintf (qh ferr, "qhull input warning: coplanars, vertices, and centrums output not\n\
available for 4-d output (ignored).  Could use 'GDn' instead.\n");
      qh PRINTcoplanar= qh PRINTspheres= qh PRINTcentrums= False;
    }
  }
  qh PRINTdim= qh hull_dim;
  if (qh DROPdim >=0) {    /* after Geomview checks */
    if (qh DROPdim < qh hull_dim) {
      qh PRINTdim--;
      if (printother || qh hull_dim < 3) 
        fprintf (qh ferr, "qhull input warning: drop dimension 'GD%d' is only available for 3-d/4-d Geomview\n", qh DROPdim);
    }else
      qh DROPdim= -1;
  }else if (qh VORONOI) {
    qh DROPdim= qh hull_dim-1;
    qh PRINTdim= qh hull_dim-1; 
  }
} /* initqhull_globals */
 
/*-----------------------------------------------------
-initqhull_mem- initialize mem.c for qhull
  qh.hull_dim and normal_size determines some of the allocation sizes
  if qh MERGING, includes ridgeT
returns:
  mem.c already for memalloc/memfree (errors if called beforehand)
notes:
  the user can add up to 10 additional sizes for quick allocation (increase numsizes)
*/
void qh_initqhull_mem (void) {
  int numsizes;
  int i;

  numsizes= 7+10;
  qh_meminitbuffers (qh IStracing, qh_MEMalign, numsizes, 
                     qh_MEMbufsize,qh_MEMinitbuf);
  qh_memsize(sizeof(vertexT));
  if (qh MERGING)
    qh_memsize(sizeof(ridgeT));
  qh_memsize(sizeof(facetT));
  qh_memsize(sizeof(hashentryT));
  i= sizeof(setT) + (qh hull_dim - 1) * SETelemsize;  /* ridge.vertices */
  qh_memsize(i);
  qh_memsize(qh normal_size);        /* normal */
  i += SETelemsize;                 /* facet.vertices, .ridges, .neighbors */
  qh_memsize(i);
  /*  qh_user_memsizes(); - THIS DID NOTHING */
  qh_memsetup();
} /* initqhull_mem */

/*-------------------------------------------
-initqhull_start -- start initialization of qhull
  inits statistics
*/ 
void qh_initqhull_start (FILE *infile, FILE *outfile, FILE *errfile) {

  clock(); /* start the clock */
#if qh_QHpointer
  if (!(qh_qh= (qhT *)malloc (sizeof(qhT)))) {
    fprintf (errfile, "qhull error (initqhull_globals): insufficient memory\n");
    exit (qh_ERRmem);  /* no error handler */
  }
  memset((char *)qh_qh, 0, sizeof(qhT));   /* every field is 0, FALSE, NULL */
#else
  memset((char *)&qh_qh, 0, sizeof(qhT));
#endif
  strcat (qh qhull, "qhull");
  qh_initstatistics();
  qh ferr= errfile;
  qh fin= infile;
  qh fout= outfile;
  qh lastreport= INT_MIN;
  qh mergereport= INT_MIN;
  qh max_outside= 0.0;
  qh maxmaxcoord= 0.0;
  qh max_vertex= 0.0;
  qh min_vertex= 0.0;
  qh MINdenom_1= fmax_(1.0/REALmax, REALmin);
  qh MINoutside= 0.0;
  qh MINvisible= REALmax;
  qh premerge_centrum= 0.0;
  qh premerge_cos= REALmax;
  qh PRINTprecision= True;
  qh PRINTradius= 0.0;
  qh postmerge_cos= REALmax;
  qh postmerge_centrum= 0.0;
  qh ROTATErandom= INT_MIN;
  qh DROPdim= -1;
  qh TRACEdist= REALmax;
  qh TRACEpoint= -1;
  qh tracefacet_id= -1;  /* stderr set these to id for tracefacet/tracevertex */
  qh tracevertex_id= -1;
} /* initqhull_start */

/*---------------------------------------------
-initthresholds	set thresholds for printing and scaling from command line
  see 'prompt' in unix.c for documentation
  see also initflags()
  sets qh GOODthreshold or qh SPLITthreshold if 'Pd0D1' used
*/
void qh_initthresholds(char *command) {
  realT value;
  int index, maxdim, k;
  char *s= command;
  char key;
  
  maxdim= qh input_dim;
  if (qh DELAUNAY && (qh PROJECTdelaunay || qh PROJECTinput))
    maxdim++;
  while (*s) {
    if (*s == '-')
      s++;
    if (*s == 'P') {
      s++;
      while (*s && !isspace(key= *s++)) {
	if (key == 'd' || key == 'D') {
	  if (!isdigit(*s)) {
	    fprintf(qh ferr, "qhull warning: no dimension given for Print option %c.  Ignored\n",
		    key);
	    continue;
	  }
	  index= qh_strtol (s, &s);
	  if (index >= qh hull_dim) {
	    fprintf(qh ferr, "qhull warning: dimension %d for Print option %c is >= %d.  Ignored\n", 
	        index, key, qh hull_dim);
	    continue;
	  }
	  if (*s == ':') {
	    value= qh_strtod(++s, &s);
	    if (fabs((double)value) > 1.0) {
	      fprintf(qh ferr, "qhull warning: value %2.4g for Print option %c is > +1 or < -1.  Ignored\n", 
	              value, key);
	      continue;
	    }
	  }else
	    value= 0.0;
	  if (key == 'd')
	    qh lower_threshold[index]= value;
	  else
	    qh upper_threshold[index]= value;
	}
      }
    }else if (*s == 'Q') {
      s++;
      while (*s && !isspace(key= *s++)) {
	if (key == 'b' && *s == 'B') {
	  s++;
	  for (k=maxdim; k--; ) {
	    qh lower_bound[k]= -qh_DEFAULTbox;
	    qh upper_bound[k]= qh_DEFAULTbox;
	  }
	}else if (key == 'b' || key == 'B') {
	  if (!isdigit(*s)) {
	    fprintf(qh ferr, "qhull warning: no dimension given for Qhull option %c.  Ignored\n",
		    key);
	    continue;
	  }
	  index= qh_strtol (s, &s);
	  if (index >= maxdim) {
	    fprintf(qh ferr, "qhull warning: dimension %d for Qhull option %c is >= %d.  Ignored\n", 
	        index, key, maxdim);
	    continue;
	  }
	  if (*s == ':')
	    value= qh_strtod(++s, &s);
	  else if (key == 'b')
	    value= -qh_DEFAULTbox;
	  else
	    value= qh_DEFAULTbox;
	  if (key == 'b')
	    qh lower_bound[index]= value;
	  else
	    qh upper_bound[index]= value;
	}
      }
    }else {
      while (!isspace (*s))
        s++;
    }
    while (isspace (*s))
      s++;
  }
  for (k= qh hull_dim; k--; ) {
    if (qh lower_threshold[k] > -REALmax/2) {
      qh GOODthreshold= True;
      if (qh upper_threshold[k] < REALmax/2) {
        qh SPLITthresholds= True;
        qh GOODthreshold= False;
        break;
      }
    }else if (qh upper_threshold[k] < REALmax/2)
      qh GOODthreshold= True;
  }
} /* initthresholds */

#if qh_QHpointer
/*------------------------------------------
-restore_qhull- restores a previously saved qhull
  also restores qh_qhstat and qhmem.tempstack
  errors if current qhull hasn't been saved or freed
  uses qhmem for error reporting
*/
void qh_restore_qhull (qhT **oldqh) {

  if (*oldqh && strcmp ((*oldqh)->qhull, "qhull")) qhull_fatal(77);

  if (qh_qh) qhull_fatal(78);

  if (!*oldqh || !(*oldqh)->old_qhstat) qhull_fatal(79);

  qh_qh= *oldqh;
  *oldqh= NULL;
  qh_qhstat= qh old_qhstat;
  qhmem.tempstack= qh old_tempstack;
  trace1((qh ferr, "qh_restore_qhull: restored qhull from %x\n", (int) *oldqh));
} /* restore_qhull */

/*------------------------------------------
-save_qhull- saves qhull for a later qh_restore_qhull
  also saves qh_qhstat and qhmem.tempstack
returns:
  qhull for a later restore_qhull
  qh_qh=NULL
notes:
  need to initialize qhull or call qh_restore_qhull before continuing
*/
qhT *qh_save_qhull (void) {
  qhT *oldqh;

  if (!qh_qh) qhull_fatal(80);

  qh old_qhstat= qh_qhstat;
  qh_qhstat= NULL;
  qh old_tempstack= qhmem.tempstack;
  qhmem.tempstack= NULL;
  oldqh= qh_qh;
  qh_qh= NULL;
  trace1((qhmem.ferr, "qh_save_qhull: saved qhull %x\n", (int) oldqh));
  return oldqh;
} /* save_qhull */

#endif

/*-----------------------------------------
-strtol/tod -- internal versions that don't skip trailing spaces
*/
double qh_strtod (const char *s, char **endp) {
  double result;

  result= strtod (s, endp);
  if (s < (*endp) && (*endp)[-1] == ' ')
    (*endp)--;
  return result;
} /* strtod */

int qh_strtol (const char *s, char **endp) {
  int result;

  result= (int) strtol (s, endp, 10);
  if (s< (*endp) && (*endp)[-1] == ' ')
    (*endp)--;
  return result;
} /* strtol */



/*************************************************************************/
/****************** implementation code from qhull.c *********************/
/*************************************************************************/

/* qhull - Quickhull algorithm for convex hulls

   qhull() and top-level routines

   see README, qhull.h, unix.c and mac.c

   see qhull_a.h for internal functions
   
   copyright (c) 1993-1994 The Geometry Center        
*/


/*-------------------------------------------------
-qhull- hull_dim convex hull of num_points starting at first_point
returns:
  returns facet_list, numfacets, etc. 
*/
void qh_qhull (void) {
  setT *maxpoints, *vertices;
  facetT *facet;
  int numpart, numoutside, i;
  vertexT *vertex;
  realT dist;
  boolT isoutside;

  qh hulltime= clock();
  if (qh DELAUNAY && qh upper_threshold[qh hull_dim-1] > REALmax/2
                  && qh lower_threshold[qh hull_dim-1] < -REALmax/2) {
    for (i= qh_PRINTEND; i--; ) {
      if (qh PRINTout[i] == qh_PRINTgeom && qh DROPdim < 0 
 	  && !qh GOODthreshold && !qh SPLITthresholds)
	break;  /* in this case, don't set upper_threshold */
    }
    if (i < 0) {
      qh upper_threshold[qh hull_dim-1]= 0.0;
      if (!qh GOODthreshold)
	qh SPLITthresholds= True;
    }
  }
  maxpoints= qh_maxmin(qh first_point, qh num_points, qh hull_dim);
  /* qh_maxmin sets DISTround and other precision constants */
  vertices= qh_initialvertices(qh hull_dim, maxpoints, qh first_point, qh num_points); 
  qh_initialhull (vertices);  /* initial qh facet_list */
  qh_partitionall (vertices, qh first_point, qh num_points);
  if (qh PREmerge) {
    qh cos_max= qh premerge_cos;
    qh centrum_radius= qh premerge_centrum;
  }
  if (qh ONLYgood) {

    if (!(qh GOODthreshold || qh GOODpoint
	  || (qh GOODvertex > 0 && !qh MERGING))) qhull_fatal(81);

    if (qh GOODvertex > 0  && !qh MERGING  /* matches qh_partitionall */
	&& !qh_isvertex (qh GOODvertexp, vertices)) {
      facet= qh_findbest (qh GOODvertexp, qh facet_list, False, 0,
			  &dist, &isoutside, &numpart);
      zadd_(Zdistgood, numpart);

      if (!isoutside) qhull_fatal(82);

      if (!qh_addpoint (qh GOODvertexp, facet, False)) {
	qh_settempfree(&vertices);
	qh_settempfree(&maxpoints);
	return;
      }
    }
    qh_findgood (qh facet_list, 0);
  }
  qh_settempfree(&vertices);
  qh_settempfree(&maxpoints);
  qh_buildhull();
  if (qh POSTmerge && !qh STOPpoint && !qh STOPcone) {
    qh POSTmerging= True;
    qh cos_max= qh postmerge_cos;
    qh centrum_radius= qh postmerge_centrum;
    FORALLfacets
      zinc_(Zpostfacets);
    qh newfacet_list= qh facet_list;
    qh visible_list= qh facet_list;
    qh newfacet_id= 0;
    qh newvertex_list= qh vertex_list;
    FORALLvertices
      vertex->newlist= True;
    qh_vertexneighbors (/*qh facet_list*/);
    qh_merge_nonconvex(/*qh newfacet_list*/);
    qh_partitionvisible (/*visible_list, newfacet_list*/ (boolT)!qh_ALL, &numoutside);
    qh_deletevisible (/*qh visible_list*/);
    qh POSTmerging= False;
    qh cos_max= qh premerge_cos;
    qh centrum_radius= qh premerge_centrum;
  }
  if (!qh FORCEoutput && (qh MERGING || qh APPROXhull))
    qh_check_maxout ();

  if (qh_setsize ((setT *)qhmem.tempstack) != 0) qhull_fatal(83);

  qh hulltime= clock() - qh hulltime;
  qh QHULLfinished= True;
  trace1((qh ferr, "qh_qhull: algorithm completed\n"));
} /* qhull */

/*-------------------------------------------------
-addpoint-  add point to hull above a facet
  if checkdist or !facet, locates a facet for the point
  if !checkdist and facet, assumes point is above facet (major damage if below)
returns:
  if unknown point, adds it to qh other_points
  False if user requested break
*/
boolT qh_addpoint (pointT *furthest, facetT *facet, boolT checkdist) {
  int goodvisible, goodhorizon;
  vertexT *vertex;
  facetT *newfacet;
  realT dist, newbalance, pbalance;
  boolT isoutside= False;
  int numpart, numpoints;

  if (qh_pointid (furthest) == -1)
    qh_setappend (&qh other_points, furthest);
  if (checkdist || !facet) {
    if (!facet)
      facet= qh facet_list;
    facet= qh_findbest (furthest, facet, False, 0, &dist, &isoutside, &numpart);
    zzadd_(Zpartition, numpart);
    if (!isoutside) {
      zinc_(Znotmax);  /* last point of outsideset is no longer furthest. */
      qh_partitioncoplanar (furthest, facet, &dist);
      return True;
    }
  }
  qh_buildtracing (furthest, facet);
  if (qh STOPpoint < 0 && qh furthest_id == -qh STOPpoint-1)
    return False;
  qh_findhorizon (furthest, facet, &goodvisible, &goodhorizon); 
  if (qh ONLYgood && !(goodvisible+goodhorizon)) {
    zinc_(Znotgood);  
    /* last point of outsideset is no longer furthest.  This is ok
       since all points of the outside are likely to be bad */
    qh_clearvisible (/*qh visible_list*/);
    return True;
  }
  zzinc_(Zprocessed);
  vertex= qh_makenewfacets (furthest /*visible_list, attaches if !ONLYgood */);
  newbalance= qh facet_id - qh newfacet_id;
  newbalance -= (realT) (qh num_facets-qh num_visible)*qh hull_dim/qh num_vertices;
  wadd_(Wnewbalance, newbalance);
  wadd_(Wnewbalance2, newbalance * newbalance);
  if (qh ONLYgood && !qh_findgood (qh newfacet_list, goodhorizon)) {
    FORALLnew_facets 
      qh_delfacet (newfacet);
    qh_delvertex (vertex);
    qh_clearvisible (/*qh visible_list*/);
    qh_clearnewvertices (/* qh newvertex_list*/);
    zinc_(Znotgoodnew);
    return True;
  }
  qh_attachnewfacets(/*visible_list*/);
  qh_matchnewfacets();
  if (qh STOPcone && qh furthest_id == qh STOPcone-1)
    return False;
  if (qh PREmerge)
    qh_merge_nonconvex(/*qh newfacet_list*/);
  qh_partitionvisible (/*visible_list, newfacet_list*/ (boolT)!qh_ALL, &numpoints);
  zinc_(Zpbalance);
  pbalance= numpoints - (realT) qh hull_dim /* assumes all points extreme */
                * (qh num_points - qh num_vertices)/qh num_vertices;
  wadd_(Wpbalance, pbalance);
  wadd_(Wpbalance2, pbalance * pbalance);
  qh_deletevisible (/*qh visible_list*/);
  qh_clearnewvertices (/* qh newvertex_list*/);

  /*
  if (qh IStracing >= 4)
    qh_printfacetlist (qh newfacet_list, NULL, True);
    NO LONGER IN SERVICE */

  if (qh CHECKfrequently) {
    if (qh num_facets < 50)
      qh_checkpolygon (qh facet_list);
    else
      qh_checkpolygon (qh newfacet_list);
  }
  if (qh STOPpoint > 0 && qh furthest_id == qh STOPpoint-1)
    return False;
  trace2((qh ferr, "qh_addpoint: added p%d new facets %d new balance %2.2g point balance %2.2g\n",
    qh_pointid (furthest), qh facet_id - qh newfacet_id, newbalance, pbalance));
  qh newfacet_id= qh facet_id;
  qh newfacet_list= NULL;
  return True;
} /* addpoint */

/*-------------------------------------------------
-buildhull- constructs a hull by adding outside points one at a time
  may be called multiple times
  checks facet and vertex lists for 'visible' and 'newlist'
notes:
  to recover from STOPcone, call qh_deletevisible and qh_clearnewvertices
*/
void qh_buildhull(void) {
  facetT *facet;
  pointT *furthest;
  vertexT *vertex;
  int id;
  
  trace1((qh ferr, "qh_buildhull: start build hull\n"));
  FORALLfacets {

    if (facet->visible) qhull_fatal(84);

  }
  FORALLvertices {

    if (vertex->newlist) qhull_fatal(85);

    id= qh_pointid (vertex->point);
    if ((qh STOPpoint>0 && id == qh STOPpoint-1) ||
	(qh STOPpoint<0 && id == -qh STOPpoint-1) ||
	(qh STOPcone>0 && id == qh STOPcone-1)) {
      trace1((qh ferr,"qh_buildhull: stop point or cone P%d in initial hull\n", id));
      return;
    }
  }
  qh facet_next= qh facet_list;      /* advance facet when processed */
  while ((furthest= qh_nextfurthest (&facet))) {
    qh num_outside--;
    if (!qh_addpoint (furthest, facet, qh ONLYmax))
      break;
  }
  if (qh num_outside && !furthest) qhull_fatal(86);

  trace1((qh ferr, "qh_buildhull: completed the hull construction\n"));
} /* buildhull */
  

/*-------------------------------------------
-buildtracing- for tracing execution of buildhull
  also resets visit_id, vertext_visit on wrap around
*/
void qh_buildtracing (pointT *furthest, facetT *facet) {
  realT cpu, dist;
  time_t timedata;
  struct tm *tp;
  vertexT *vertex;

  qh furthest_id= qh_pointid(furthest);
  if (qh TRACEpoint == qh furthest_id) {
    qh IStracing= qh TRACElevel;
    qhmem.IStracing= qh TRACElevel;
  }
  if (qh REPORTfreq && (zzval_(Zsetplane) > qh lastreport+qh REPORTfreq)) {
    qh lastreport= zzval_(Zsetplane);
    time (&timedata);
    tp= localtime (&timedata);
    cpu= clock();
    cpu /= qh_SECticks;
    zinc_(Zdistio);
    qh_distplane (furthest, facet, &dist);
    fprintf (qh ferr, "\n\
At %d:%d:%d & %2.5g CPU secs, qhull has created %d facets and merged %d.\n\
 The current hull contains %d facets and %d vertices.  There are %d\n\
 outside points.  Next is point p%d (v%d), %2.2g above f%d.\n",
      tp->tm_hour, tp->tm_min, tp->tm_sec, cpu, zzval_(Zsetplane),
      zzval_(Ztotmerge), qh num_facets, qh num_vertices, qh num_outside+1,
      qh furthest_id, qh vertex_id, dist, facet->id);
  }else if (qh IStracing >=1) {
    cpu= clock();
    cpu /= qh_SECticks;
    qh_distplane (furthest, facet, &dist);
    fprintf (qh ferr, "qh_buildhull: add p%d (v%d) to hull of %d facets (%2.2g above f%d) and %d outside.  %4.4g CPU secs.\n",
      qh furthest_id, qh vertex_id, qh num_facets, dist,
      facet->id, qh num_outside+1, cpu);
  }
#ifndef __BCC55__
  // This condition is always false under BCC55
  if (qh visit_id > (unsigned) INT_MAX) {
    qh visit_id= 0;
    FORALLfacets
      facet->visitid= qh visit_id;
  }
#endif  // __BCC55__
#ifndef __BCC55__
  // This condition is always false under BCC55
  if (qh vertex_visit > (unsigned) INT_MAX) {
    qh vertex_visit= 0;
    FORALLvertices
      vertex->visitid= qh vertex_visit;
  }
#endif  // __BCC55__
} /* buildtracing */

/*-------------------------------------------
-errexit2- return exitcode to system after an error
  assumes exitcode non-zero
  for two facets, see qh_errexit() in user.c
*/
void qh_errexit2(int exitcode, facetT *facet, facetT *otherfacet) {
  qhull_fatal(87);
} /* errexit2 */


/*-------------------------------------------------
-findgood- identify good facets for qh ONLYgood
  GOODvertex>0 - facet includes point as vertex
    if !match, returns goodhorizon
    inactive if qh MERGING
  GOODpoint - facet is visible or coplanar (>0) or not visible (<0) 
  GOODthreshold - facet->normal matches threshold
    if !goodhorizon and !match, selects facet with closest angle
       and sets GOODclosest
returns:
  number of new, good facets found
  determins facet->good
  may update GOODclosest
notes:
  findgood_all further reduces the good region
*/
int qh_findgood (facetT *facetlist, int goodhorizon) {
  facetT *facet, *bestfacet;
  realT angle, bestangle, dist;
  int  numgood=0;

  if (qh GOODclosest) {
    bestfacet= qh GOODclosest;
    qh_inthresholds (bestfacet->normal, &bestangle);
  }else {
    bestfacet= NULL;
    bestangle= -REALmax;
  }
  FORALLfacet_(facetlist) {
    if (facet->good)
      numgood++;
  }
  if (qh GOODvertex>0 && !qh MERGING) {
    FORALLfacet_(facetlist) {
      if (!qh_isvertex (qh GOODvertexp, facet->vertices)) {
        facet->good= False;
        numgood--;
      }
    }
  }
  if (qh GOODpoint && numgood) {
    FORALLfacet_(facetlist) {
      if (facet->good) {
        zinc_(Zdistgood);
        qh_distplane (qh GOODpointp, facet, &dist);
        if ((qh GOODpoint > 0) ^ (dist > 0.0)) {
          facet->good= False;
          numgood--;
        }
      }
    }
  }
  if (qh GOODthreshold && (numgood || goodhorizon)) {
    FORALLfacet_(facetlist) {
      if (facet->good) {
        if (!qh_inthresholds (facet->normal, &angle)) {
          facet->good= False;
          numgood--;
          angle= fabs_(angle);
          if (angle > bestangle) {
            bestangle= angle;
            bestfacet= facet;
          }
        }
      }
    }
    if (!numgood && bestfacet && bestfacet != qh GOODclosest) {
      if (qh GOODclosest)
	qh GOODclosest->good= False;
      qh GOODclosest= bestfacet;
      bestfacet->good= True;
      numgood++;
      trace2((qh ferr, "qh_findgood: f%d is closest (%2.2g) to thresholds\n", 
           bestfacet->id, bestangle));
      return numgood;
    }else if (numgood && qh GOODclosest)
      qh GOODclosest->good= False;
  }
  zadd_(Zgoodfacet, numgood);
  trace2((qh ferr, "qh_findgood: found %d good facets\n", numgood));
  if (!numgood && qh GOODvertex>0 && !qh MERGING) 
    return goodhorizon;
  return numgood;
} /* findgood */

/*-------------------------------------------------
-findgood_all- apply other constraints for good facets (used by qh PRINTgood)
  GOODvertex - facet includes (>0) or doesn't include (<0) point as vertex
    if last good facet, prints warning and continues
  SPLITthreshold - facet->normal matches threshold, or if none, the closest one
  calls findgood if !ONLYgood
returns:
  clears facet->good if not good
  sets qh num_good
notes:
  this is like findgood but more restrictive
*/
void qh_findgood_all (facetT *facetlist) {
  facetT *facet, *bestfacet=NULL;
  realT angle, bestangle= REALmax;
  int  numgood=0, startgood;

  if (!qh ONLYgood)
    qh_findgood (qh facet_list, 0);
  FORALLfacet_(facetlist) {
    if (facet->good)
      numgood++;
  }
  if (qh GOODvertex <0 || (qh GOODvertex > 0 && qh MERGING)) {
    FORALLfacet_(facetlist) {
      if ((qh GOODvertex > 0) ^ !!qh_isvertex (qh GOODvertexp, facet->vertices)) {
        if (!--numgood) {
          fprintf (qh ferr, "qhull warning: good vertex p%d does not match last good facet f%d.  Ignored.\n",
             qh_pointid(qh GOODvertexp), facet->id);
          return;
        }
        facet->good= False;
      }
    }
  }
  startgood= numgood;
  if (qh SPLITthresholds) {
    FORALLfacet_(facetlist) {
      if (facet->good) {
        if (!qh_inthresholds (facet->normal, &angle)) {
          facet->good= False;
          numgood--;
          angle= fabs_(angle);
          if (angle < bestangle) {
            bestangle= angle;
            bestfacet= facet;
          }
        }
      }
    }
    if (!numgood) {
      bestfacet->good= True;
      numgood++;
      trace0((qh ferr, "qh_findgood_all: f%d is closest (%2.2g) to thresholds\n", 
           bestfacet->id, bestangle));
      return;
    }
  }
  qh num_good= numgood;
  trace0((qh ferr, "qh_findgood_all: %d good facets remain out of %d facets\n",
        numgood, startgood));
} /* findgood_all */

/*-------------------------------------------------
-findhorizon- given a visible facet, find the point's horizon and visible facets
returns:
  qh visible_list to all visible facets 
    marks visible facets with ->visible 
    goodvisible counts visible->good
    initializes num_visible
notes:
  similar to delpoint()
*/
void qh_findhorizon(pointT *point, facetT *facet, int *goodvisible, int *goodhorizon) {
  facetT *neighbor, **neighborp, *visible;
  int numhorizon= 0;
  realT dist;
#if qh_MAXoutside
  boolT ckminvis= (boolT)(qh MINvisible > qh DISTround), isoutside;
#endif
  
  trace1((qh ferr,"qh_findhorizon: find horizon for point p%d facet f%d\n",qh_pointid(point),facet->id));
  *goodvisible= *goodhorizon= 0;
  zinc_(Ztotvisible);
  qh_removefacet(facet);  /* visible_list at end of qh facet_list */
  qh_appendfacet(facet);
  qh num_visible= 1;
  if (facet->good)
    (*goodvisible)++;
  qh visible_list= facet;
  facet->visible= True;

  /*
  if (qh IStracing >=4)
    qh_errprint ("visible", facet, NULL, NULL, NULL);
    NO LONGER IN SERVICE */

  qh visit_id++;
  FORALLvisible_facets {
    visible->visitid= qh visit_id;
    FOREACHneighbor_(visible) {
      if ((int)neighbor->visitid == qh visit_id) 
        continue;
      neighbor->visitid= qh visit_id;
      zzinc_(Znumvisibility);
      qh_distplane(point, neighbor, &dist);
#if qh_MAXoutside
      isoutside= False;
      if (ckminvis) {
        if (dist >= qh MINvisible)
          isoutside= True;
      }else if (qh ONLYmax) {  /* furthest points are at least maxoutside above */
        if (dist >= neighbor->maxoutside || dist >= qh max_vertex)
          isoutside= True;
      }else if (qh PREmerge) {           
        if (dist >= qh MINoutside)
          isoutside= True;
      }else if (dist > qh DISTround)
        isoutside= True;
      if (isoutside) {
#else
      if (dist > qh MINvisible) {
#endif
        zinc_(Ztotvisible);
	qh_removefacet(neighbor);  /* append to end of qh visible_list */
	qh_appendfacet(neighbor);
	neighbor->visible= True;
	qh num_visible++;
	if (neighbor->good)
	  (*goodvisible)++;

	/*
        if (qh IStracing >=4)
          qh_errprint ("visible", neighbor, NULL, NULL, NULL);
	  NO LONGER IN SERVICE */

      }else {
    	if (dist > -qh DISTround) {
          zzinc_(Zcoplanarhorizon);
      	  trace0((qh ferr, "qh_findhorizon: point p%d is coplanar to horizon f%d, dist=%2.7g < qh MINvisible (%2.7g)\n",
	      qh_pointid(point), neighbor->id, dist, qh MINvisible));
	} 
    	zinc_(Ztothorizon);
        numhorizon++;
	if (neighbor->good)
	  (*goodhorizon)++;

	/*
        if (qh IStracing >=4)
          qh_errprint ("horizon", neighbor, NULL, NULL, NULL);
	  NO LONGER IN SERVICE */

      }
    }
  }

  if (!numhorizon) qhull_fatal(88);

  trace1((qh ferr, "qh_findhorizon: %d horizon facets (good %d), %d visible (good %d) min visible %2.2g\n", 
       numhorizon, *goodhorizon, qh num_visible, *goodvisible, qh MINvisible));
  if (qh IStracing >= 4 && qh num_facets < 50) 
    qh_printlists ();
} /* findhorizon */


/*--------------------------------------------------
-initialhull- constructs the initial hull as a qh hull_dim simplex of vertices
*/
void qh_initialhull(setT *vertices) {
  facetT *facet, *firstfacet;
  int k;
  realT dist;

  qh_createsimplex(vertices);  /* qh facet_list */
  qh interior_point= qh_getcenter(vertices);
  firstfacet= qh facet_list;
  qh_setfacetplane(firstfacet);
  zinc_(Znumvisibility); /* needs to be in printsummary */
  qh_distplane(qh interior_point, firstfacet, &dist);
  if (dist > 0) {  
    FORALLfacets
      facet->toporient ^= True;
  }
  FORALLfacets
    qh_setfacetplane(facet);
  FORALLfacets {
    if (!qh_checkflipped (facet, NULL, qh_ALL)) {/* due to axis-parallel facet */
      trace1((qh ferr, "qh_initialhull: initial orientation incorrect.  Correct all facets\n"));
      facet->flipped= False;
      FORALLfacets {
	facet->toporient ^= True;
	qh_orientoutside (facet);
      }
      break;
    }
  }
  FORALLfacets {

    if (!qh_checkflipped (facet, NULL, (boolT)!qh_ALL)) qhull_fatal(89);

  }
  zzval_(Zprocessed)= qh hull_dim+1;
  qh_checkpolygon (qh facet_list);
  qh_checkconvex(qh facet_list,   qh_DATAfault);
  if (qh IStracing >= 1) {
    fprintf(qh ferr, "qh_initialhull: simplex constructed, interior point:");
    for (k=0; k<qh hull_dim; k++) 
      fprintf (qh ferr, " %6.4g", qh interior_point[k]);
    fprintf (qh ferr, "\n");
  }
  if (qh PREmerge)
    qh_vertexneighbors (/*qh facet_list*/);
} /* initialhull */

/*-------------------------------------------------
-initialvertices- determines a non-singular set of initial vertices
  picks random points if qh RANDOMoutside && !ALLpoints
  all maxpoints are unique
returns:
  temporary set of dim+1 vertices in descending order by vertex id
notes:
  unless qh ALLpoints, uses maxpoints as long as determinate is non-zero
*/
setT *qh_initialvertices(int dim, setT *maxpoints, pointT *points, int numpoints) {
  pointT *point, **pointp;
  setT *vertices, *simplex;
  realT randr;
  int index, point_i, point_n;
  
  vertices= qh_settemp (dim + 1);
  simplex= qh_settemp (dim+1);
  if (qh ALLpoints) 
    qh_maxsimplex (dim, NULL, points, numpoints, &simplex);
  else if (qh RANDOMoutside) {
    while (qh_setsize (simplex) != dim+1) {
      randr= qh_RANDOMint;
      randr= randr/(qh_RANDOMmax+1);
      index= (int)floor(qh num_points * randr);
      point= qh_point (index);
      qh_setunique (&simplex, point);
    }
  }else if (qh hull_dim >= 8) {
    qh_setunique (&simplex, SETfirst_(maxpoints)); 
    FOREACHpoint_i_(maxpoints) {  
      if (point_i & 0x1) {     /* first pick up max/min x and max points */
        qh_setunique (&simplex, point);
        if (qh_setsize (simplex) == dim)  /* search for last point */
	  break;
      }
    }
    if (qh_setsize (simplex) != dim) {
      while ((point= (pointT *)qh_setdellast (maxpoints))) {
        qh_setunique (&simplex, point);
        if (qh_setsize (simplex) == dim)
	  break;
      }
    }
    index= 0;
    while (qh_setsize (simplex) != dim) {
      point= qh_point (index++);
      qh_setunique (&simplex, point);
    }
    qh_maxsimplex (dim, maxpoints, points, numpoints, &simplex);
  }else
    qh_maxsimplex (dim, maxpoints, points, numpoints, &simplex);
  FOREACHpoint_(simplex) 
    qh_setaddnth (&vertices, 0, qh_newvertex(point)); /* descending order */
  qh_settempfree (&simplex);
  return vertices;
} /* initialvertices */


/*------------------------------------------------
-nextfurthest- returns next furthest point for processing
returns:
  NULL if none available
  visible facet for furthest
  removes empty outside sets  
*/
pointT *qh_nextfurthest (facetT **visible) {
  facetT *facet;
  int size; /* , index; UNUSED */
  /*  realT randr; UNUSED */
  pointT *furthest;

  while ((facet= qh facet_next) != qh facet_tail) {
    if (!facet->outsideset) {
      qh facet_next= facet->next;
      continue;
    }
    SETreturnsize_(facet->outsideset, size);
    if (!size) {
      qh_setfree (&facet->outsideset);
      qh facet_next= facet->next;
      continue;
    }
    if (!qh RANDOMoutside && !qh VIRTUALmemory) {
      *visible= facet;
      return (pointT *)(qh_setdellast (facet->outsideset));
    }
    if (qh RANDOMoutside) qhull_fatal(90);

    else { /* VIRTUALmemory */
      facet= qh facet_tail->previous;
      if (!(furthest= (pointT *)qh_setdellast(facet->outsideset))) {
        if (facet->outsideset)
          qh_setfree (&facet->outsideset);
        qh_removefacet (facet);
        qh_prependfacet (facet, &qh facet_list);
        continue;
      }
      *visible= facet;
      return furthest;
    }
  }
  return NULL;
} /* nextfurthest */

/*-------------------------------------------------
-partitionall- partitions all points into the outsidesets of facets
   vertices= set of vertices used by qh facet_list
     does not partition qh GOODpoint
     if ONLYgood && !MERGING, does not partition GOODvertex
   qh newfacet_id=0 for qh_findbest
notes:
   faster if qh facet_list sorted by anticipated size of outside set
*/
void qh_partitionall(setT *vertices, pointT *points, int numpoints){
  setT *pointset;
  vertexT *vertex, **vertexp;
  pointT *point, **pointp, *bestpoint;
  int size, point_i, point_n, point_end, remaining, i, id;
  facetT *facet;
  realT bestdist= -REALmax, dist;
    
  trace1((qh ferr, "qh_partitionall: partition all points into outside sets\n"));
  pointset= qh_settemp (numpoints);
  pointp= SETaddr_(pointset, pointT);
  for (i=numpoints, point= points; i--; point += qh hull_dim)
    *(pointp++)= point;
  qh_settruncate (pointset, numpoints);
  FOREACHvertex_(vertices) {
    if ((id= qh_pointid(vertex->point)) >= 0)
      SETelem_(pointset, id)= NULL;
  }
  id= qh_pointid (qh GOODpointp);
  if (id >=0 && qh STOPcone-1 != id && -qh STOPpoint-1 != id)
    SETelem_(pointset, id)= NULL;
  if (qh GOODvertexp && qh ONLYgood && !qh MERGING) { /* matches qhull()*/
    if ((id= qh_pointid(qh GOODvertexp)) >= 0)
      SETelem_(pointset, id)= NULL;
  }
  if (!qh BESToutside) {
    zval_(Ztotpartition)= qh num_points - qh hull_dim - 1; /*misses GOOD... */
    remaining= qh num_facets;
    point_end= numpoints;
    FORALLfacets {
      size= point_end/(remaining--) + 100;
      facet->outsideset= qh_setnew (size);
      bestpoint= NULL;
      point_end= 0;
      FOREACHpoint_i_(pointset) {
        if (point) {
          zzinc_(Zpartitionall);
          qh_distplane (point, facet, &dist);
          if (dist < qh MINoutside)
            SETelem_(pointset, point_end++)= point;
          else {
	    qh num_outside++;
            if (!bestpoint) {
              bestpoint= point;
              bestdist= dist;
            }else if (dist > bestdist) {
              qh_setappend (&facet->outsideset, bestpoint);
              bestpoint= point;
              bestdist= dist;
            }else 
              qh_setappend (&facet->outsideset, point);
          }
        }
      }
      if (bestpoint) {
        qh_setappend (&facet->outsideset, bestpoint);
#if !qh_COMPUTEfurthest
	facet->furthestdist= bestdist;
#endif
      }else
        qh_setfree (&facet->outsideset);
      qh_settruncate (pointset, point_end);
    }
  }
  FOREACHpoint_i_(pointset) {
    if (point)
      qh_partitionpoint(point, qh facet_list);
  }
  qh_settempfree(&pointset);

  /*
  if (qh IStracing >= 4)
    qh_printfacetlist (qh facet_list, NULL, True);
    NO LONGER IN SERVICE */

} /* partitionall */


/*-------------------------------------------------
-partitioncoplanar- partition coplanar point to a facet
  if dist NULL, searches from bestfacet, and does nothing if inside
returns:
  max_ouside, num_coplanar updated
  if KEEPcoplanar or KEEPinside
    point assigned to best coplanarset
*/
void qh_partitioncoplanar (pointT *point, facetT *facet, realT *dist) {
  facetT *bestfacet;
  pointT *oldfurthest;
  realT bestdist, dist2;
  int numpart= 0;
  boolT isoutside;

  if (!dist) {
    bestfacet= qh_findbest (point, facet, True, 0, &bestdist, &isoutside, &numpart);
    zinc_(Ztotpartcoplanar);
    zzadd_(Zpartcoplanar, numpart);
    if (bestdist < qh min_vertex) {
      zinc_(Zcoplanarinside);
      if (!qh KEEPinside)
        return;
    }else
      qh num_coplanar++;
  }else {
    bestfacet= facet;
    bestdist= *dist;
    if (!qh KEEPinside || bestdist >= qh min_vertex)
      qh num_coplanar++;
  }
  if (qh KEEPcoplanar + qh KEEPinside) {
    oldfurthest= (pointT *)qh_setlast (bestfacet->coplanarset);
    if (oldfurthest) {
      zinc_(Zcomputefurthest);
      qh_distplane (oldfurthest, bestfacet, &dist2);
    }
    if (!oldfurthest || dist2 < bestdist) {
      qh_setappend(&bestfacet->coplanarset, point);
      maximize_(qh max_outside, bestdist);
    }else
      qh_setappend2ndlast(&bestfacet->coplanarset, point);
  }else
    maximize_(qh max_outside, bestdist);
  trace2((qh ferr, "qh_partitioncoplanar: point p%d is coplanar with facet f%d (or inside) dist %2.2g\n",
	  qh_pointid(point), bestfacet->id, bestdist));
} /* partitioncoplanar */


/*-------------------------------------------------
-partitionpoint- assigns point to a visible facet 
    !BESToutside stops search when point is outside or new facets
    findbest does not search !newfacet_id if precise and !BESToutside
*/
void qh_partitionpoint (pointT *point, facetT *facet) {
  realT bestdist;
  pointT *oldfurthest;
  boolT isoutside;
  facetT *bestfacet;
  int numpart;

  bestfacet= qh_findbest (point, facet, qh BESToutside, qh newfacet_id,
			  &bestdist, &isoutside, &numpart);
  zinc_(Ztotpartition);
  zzadd_(Zpartition, numpart);
  if (isoutside) {
    if (!bestfacet->outsideset 
    || !(oldfurthest= (pointT *)qh_setlast (bestfacet->outsideset))) {
      qh_setappend(&(bestfacet->outsideset), point);
      if (bestfacet->id < qh newfacet_id) {
        qh_removefacet (bestfacet);  /* move after qh facet_next */
        qh_appendfacet (bestfacet);
      }
    }else {
#if qh_COMPUTEfurthest
      zinc_(Zcomputefurthest);
      qh_distplane (oldfurthest, bestfacet, &dist);
      if (dist < bestdist) 
	qh_setappend(&(bestfacet->outsideset), point);
      else
	qh_setappend2ndlast(&(bestfacet->outsideset), point);
#else
      if (bestfacet->furthestdist < bestdist) {
	qh_setappend(&(bestfacet->outsideset), point);
	bestfacet->furthestdist= bestdist;
      }else
	qh_setappend2ndlast(&(bestfacet->outsideset), point);
#endif
    }
    qh num_outside++;
    trace4((qh ferr, "qh_partitionpoint: point p%d is outside facet f%d\n",
	  qh_pointid(point), bestfacet->id));
  }else if (bestdist < qh min_vertex) {
    zinc_(Zpartinside);
    trace4((qh ferr, "qh_partitionpoint: point p%d is inside all facets, closest to f%d dist %2.2g\n",
	  qh_pointid(point), bestfacet->id, bestdist));
    if (qh KEEPinside)	  
      qh_partitioncoplanar (point, bestfacet, &bestdist);
  }else {
    zzinc_(Zcoplanarpart);
    if (qh KEEPcoplanar || bestdist > qh max_outside 
	|| (!qh BESToutside && qh newfacet_id && !qh MERGING)) 
      qh_partitioncoplanar (point, bestfacet, &bestdist);
  }
} /* partitionpoint */

/*-------------------------------------------------
-partitionvisible- partitions points in visible_list to newfacet_list
  1st neighbor (if any) points to a horizon facet or a new facet
  repartitions coplanar points if allpoints
*/
void qh_partitionvisible(/*visible_list*/ boolT allpoints, int *numoutside) {
  facetT *visible, *newfacet;
  pointT *point, **pointp;
  int coplanar=0, size;
  vertexT *vertex, **vertexp;
  
  if (qh ONLYmax)
    maximize_(qh MINoutside, qh max_vertex);
  *numoutside= 0;
  FORALLvisible_facets {
    if (!visible->outsideset && !visible->coplanarset)
      continue;
    newfacet= (facetT *)SETfirst_(visible->neighbors);
    while (newfacet && newfacet->visible)
      newfacet= (facetT *)SETfirst_(newfacet->neighbors);
    if (!newfacet)
      newfacet= qh newfacet_list;
    if (visible->outsideset) {
      size= qh_setsize (visible->outsideset);
      *numoutside += size;
      qh num_outside -= size;
      FOREACHpoint_(visible->outsideset) 
        qh_partitionpoint (point, newfacet);
    }
    if (visible->coplanarset && (qh KEEPcoplanar || qh KEEPinside)) {
      size= qh_setsize (visible->coplanarset);
      coplanar += size;
      qh num_coplanar -= size;
      FOREACHpoint_(visible->coplanarset) {
        if (allpoints)
          qh_partitionpoint (point, newfacet);
        else
          qh_partitioncoplanar (point, newfacet, NULL);
      }
    }
  }
  FOREACHvertex_(qh del_vertices) {
    if (vertex->point) {
      if (qh DELAUNAY && !allpoints && !(qh APPROXhull | qh MERGING))
        fprintf (qh ferr, "qhull precision warning: point p%d (v%d) deleted due to roundoff errors\n", 
           qh_pointid(vertex->point), vertex->id);
      if (allpoints)
        qh_partitionpoint (vertex->point, qh newfacet_list);
      else
        qh_partitioncoplanar (vertex->point, qh newfacet_list, NULL);
    }
  }
  trace1((qh ferr,"qh_partitionvisible: partitioned %d points from outsidesets and %d points from coplanarsets\n", *numoutside, coplanar));
} /* partitionvisible */


