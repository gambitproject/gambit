//
// FILE: dt1util.cc -- Implementation of various operations on old-style
//                     data structures
//
// $Id$
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dt1strct.h"
#include "dt1util.h"

extern struct pblm *NULLpblm;
extern struct game *NULLgame;
extern struct node *NULLnode;
extern struct iset *NULLiset;
extern struct outc *NULLoutc;
extern struct plyr *NULLplyr;

extern struct pblm *whichpblm;


void nrerror(char *s)
{
  fprintf(stderr, "%s\n", s);
  exit(1);
}

float *vector(int nl, int nh)
{
  float *v;
  int i;
  
  if ((v = (float *) malloc((nh - nl + 1) * sizeof(float))) == NULL)  {
    fprintf(stderr, "Allocation failure in vector()\n");
    exit(1);
  }
  return v - nl;
}

void free_vector(float *v, int nl, int nh)
{
  free((char *) (v + nl));
}

/*
 * Memory allocation routines
 */

struct pblm *newpblm(void)
{
  struct pblm *v, *vv;
  int i;
  
  if ((vv = (struct pblm *) malloc(sizeof(struct pblm))) == NULL)  {
    fprintf(stderr, "Memory allocation failure in newpblm()");
    exit(1);
  }

  whichpblm = vv;

  vv->title[0] = (char) '\0';
  vv->filename[0] = (char) '\0';

  vv->firstnode = (struct node *) NULLnode;
  vv->firstinfoset = (struct iset *) NULLiset;
  vv->firstoutcome = (struct outc *) NULLoutc;
  vv->firstplayer = (struct plyr *) NULLplyr;
  vv->firstgame = (struct game *) NULLgame;

  vv->nplayers = 0;
  vv->firstoutcome = newoutcome();
  vv->noutcomes = 0;	/* first outcome not counted */
  vv->ngames = 0;
  vv->nisets = 0;
  vv->nnodes = 0;

  vv->firstgame=newgame();
  vv->firstgame->rootnode = newnode(NULLnode);

  strncpy(vv->firstgame->rootnode->nodename, "ROOT", NAMESIZE);

  return vv;
}

struct game *newgame(void)
{
  struct game *g, *gg;
  int i, j;
  
  if ((gg = (struct game *) malloc(sizeof(struct game))) == NULL)  {
    fprintf(stderr, "Memory allocation failure in newgame()\n");
    exit(1);
  }

  gg->gamename[0] = (char) '\0';

  if (whichpblm->firstgame != NULLgame) {
    for (g = whichpblm->firstgame, i = 1; g->nextgame != NULLgame; g = g->nextgame, i++)
      g->gamenumber = i;
    g->gamenumber = i;
    g->nextgame = gg;
    gg->gamenumber = i + 1;
  }
  else {
    whichpblm->firstgame = gg;
    gg->gamenumber = 1;
  }
  gg->nextgame = (struct game *) NULLgame;


  gg->rootnode = (struct node *) NULLnode;

  (whichpblm->ngames)++;
  return gg;
}

struct node *newnode(struct node * parent)
{
  struct node *nn;
  struct node *b;

  int i, j;

  if ((nn = (struct node *) malloc(sizeof(struct node))) == NULL)
    nrerror("Memory allocation failure in newnode()\n");

  nn->branchname[0] = (char) '\0';
  nn->nodename[0] = (char) '\0';

  if (whichpblm->firstnode != NULLnode) {
    for (b = whichpblm->firstnode, i = 1; b->nextnode != NULLnode; b = b->nextnode, i++)
      b->nodenumber = i;
    b->nodenumber = i;
    b->nextnode = nn;
    nn->nodenumber = i + 1;
  }
  else {
    whichpblm->firstnode = nn;
    nn->nodenumber = 1;
  }

  nn->nextnode = (struct node *) NULLnode;

  nn->parent = parent;	/* init node.parent  */
  nn->firstbranch = (struct node *) NULLnode;	/* init node.firstbranch  */
  nn->nextbranch = (struct node *) NULLnode;
  if (parent != NULLnode) {
    if (parent->firstbranch == NULLnode)
      parent->firstbranch = nn;
    else {
      for (b = parent->firstbranch; b->nextbranch != NULLnode; b = b->nextbranch);
      b->nextbranch = nn;
    }
  }
  nn->infoset = NULLiset;	/* init node.infoset  */
  nn->whichbranch = NULLnode;
  nn->nextmember = NULLnode;
  nn->nextgame = NULLgame;
  nn->probability = -1;	/* uninitialized >>> negative  */
  nn->outcome = whichpblm->firstoutcome;	/* default >>> no score  */

  nn->game = nn->player = nn->iset = nn->index = 0;

  whichpblm->nnodes++;
  return nn;
}

struct iset *newiset(struct node * nn, struct plyr * p, int branches)
{
  struct iset *a, *b;
  int i;

  if ((a = (struct iset *) malloc(sizeof(struct iset))) == NULL)
    nrerror("Memory allocation failure in newiset()\n");

  a->isetname[0] = (char) '\0';

  if (whichpblm->firstinfoset != NULLiset) {
    for (b = whichpblm->firstinfoset, i = 1; b->nextiset != NULLiset; b = b->nextiset, i++)
      b->isetnumber = i;
    b->isetnumber = i;
    b->nextiset = a;
    a->isetnumber = i + 1;
  }
  else {
    whichpblm->firstinfoset = a;
    a->isetnumber = 1;
  }

  a->nextiset = (struct iset *) NULLiset;
  a->firstmember = nn;
  a->playr = p;
  a->branches = branches;

  whichpblm->nisets++;
  return a;
}

struct plyr *newplayer(void)
{
  struct plyr *pp, *p;
  struct node *nn;
  struct outc *oo;
  struct game *gg;
  float *v;
  int i;

  if ((pp = (struct plyr *) malloc(sizeof(struct plyr))) == NULL)
    nrerror("Memory allocation failure in newplayer()");

  if (whichpblm->firstplayer != NULLplyr) {
    for (p = whichpblm->firstplayer, i = 1; p->nextplayer != NULLplyr; p = p->nextplayer, i++)
      p->plyrnumber = i;
    p->plyrnumber = i;
    p->nextplayer = pp;
    pp->plyrnumber = i + 1;
  }
  else {
    whichpblm->firstplayer = pp;
    pp->plyrnumber = 1;
  }

  pp->nextplayer = (struct plyr *) NULLplyr;
  pp->plyrname[0] = (char) '\0';


  for (oo = whichpblm->firstoutcome; oo != NULLoutc; oo = oo->nextoutcome) {
    v = oo->component;
    oo->component = vector(1, whichpblm->nplayers + 1);
    for (i = 1; i <= whichpblm->nplayers; i++)
      oo->component[i] = v[i];
    oo->component[i] = 0.0;
    if (whichpblm->nplayers > 0)
      free_vector(v, 1, whichpblm->nplayers);
    else
      free_vector(v, 1, 1);
  }

  whichpblm->nplayers++;
  return pp;
}

struct outc *newoutcome(void)
{
  struct outc *o, *oo;
  int i, j;

  if ((oo = (struct outc *) malloc(sizeof(struct outc))) == NULL)
    nrerror("Memory allocation failure in newoutcome()");

  oo->outname[0] = (char) '\0';

  if (whichpblm->firstoutcome != NULLoutc) {
    for (o = whichpblm->firstoutcome, i = 0; o->nextoutcome != NULLoutc; o = o->nextoutcome, i++)
      o->outnumber = i;
    o->nextoutcome = oo;
    oo->outnumber = i + 1;
  }
  else {
    whichpblm->firstoutcome = oo;
    oo->outnumber = 0;
  }
  oo->nextoutcome = (struct outc *) NULLoutc;

  j = whichpblm->nplayers;
  if (j < 1)
    j = 1;
  oo->component = vector(1, j);
  for (i = 1; i <= j; i++)
    oo->component[i] = 0.0;

  whichpblm->noutcomes++;
  return oo;
}


void freepblms(void)
{
  freepblm();
  whichpblm = NULLpblm;
}

void freepblm(void)
{
  struct pblm *v, *vv;

  vv = whichpblm;
  if (vv == NULLpblm)
    return;

  freenodes();
  freeisets();
  freeplyrs();
  freeoutcomes();
  freegames();


  (void) free(vv);
}

void freegames(void)
{
  struct game *gg;

  while (whichpblm->firstgame != NULLgame) {
    gg = whichpblm->firstgame;
    whichpblm->firstgame = gg->nextgame;
    (void) free(gg);
  }
  whichpblm->ngames = 0;
}

void freegame(struct game * gg)
{
  struct game *g;

  if (gg == NULLgame)
    return;
  if (gg == whichpblm->firstgame) {
    whichpblm->firstgame = gg->nextgame;
    whichpblm->firstgame = whichpblm->firstgame;
  }
  else {
    for (g = whichpblm->firstgame; g != NULLgame && g->nextgame != gg; g = g->nextgame)
      g->gamenumber--;
    g->gamenumber--;
    g->nextgame = gg->nextgame;
  }
  (void) free(gg);
  whichpblm->ngames--;
}

void freenodes(void)
{
  struct node *n;

  while (whichpblm->firstnode != NULLnode) {
    n = whichpblm->firstnode;
    whichpblm->firstnode = whichpblm->firstnode->nextnode;
    (void) free(n);
  }

  whichpblm->nnodes = 0;
}

void freenode(struct node * nn)
{
  struct node *n;

  freegame(findgame1(nn));
  if (whichpblm->firstnode == nn)
    whichpblm->firstnode = nn->nextnode;
  else {
    for (n = whichpblm->firstnode; n != NULLnode && n->nextnode != nn; n = n->nextnode)
      n->nodenumber--;
    n->nodenumber--;
    n->nextnode = nn->nextnode;
  }
  (void) free(nn);
  whichpblm->nnodes--;
}

void freeisets(void)
{
  struct iset *a;

  while (whichpblm->firstinfoset != NULLiset) {
    a = whichpblm->firstinfoset;
    whichpblm->firstinfoset = whichpblm->firstinfoset->nextiset;
    (void) free(a);
  }
  whichpblm->nisets = 0;
}

void freeiset(struct iset * ii)
{
  if (whichpblm->firstinfoset == ii)
    whichpblm->firstinfoset = ii->nextiset;
  else
    (prioriset(ii))->nextiset = ii->nextiset;

  ii->playr->nisets--;

  if (ii->playr->nisets == 0)
    delete_plyr(ii->playr);

  (void) free(ii);
  whichpblm->nisets--;
}

/* use only after all isets have been freed */
void delete_plyr(struct plyr * pp)
{
  int i;
  struct plyr *p;
  struct outc *oo;

  if (whichpblm->firstplayer == pp)
    whichpblm->firstplayer = pp->nextplayer;
  else
    (priorplyr(pp))->nextplayer = pp->nextplayer;


  for (oo = whichpblm->firstoutcome; oo != NULLoutc; oo = oo->nextoutcome)
    for (i = pp->plyrnumber; i < whichpblm->nplayers; i++)
      oo->component[i] = oo->component[i + 1];

  for (p = whichpblm->firstplayer, i = 1; p != NULLplyr; p = p->nextplayer, i++) 
    p->plyrnumber = i;

  (void) free(pp);
  whichpblm->nplayers--;
}

void freeplyrs(void)
{
  struct plyr *p;

  while (whichpblm->firstplayer != NULLplyr) {
    p = whichpblm->firstplayer;
    whichpblm->firstplayer = whichpblm->firstplayer->nextplayer;
    (void) free(p);
  }

  whichpblm->nplayers = 0;
}

void freeoutcomes()
{
  struct outc *oo;

  while (whichpblm->firstoutcome != NULLoutc) {
    oo = whichpblm->firstoutcome;
    whichpblm->firstoutcome = oo->nextoutcome;
    freeoutcome(oo);
  }
  whichpblm->noutcomes = 0;
}

void freeoutcome(struct outc * oo)
{
  free_vector(oo->component, 1, whichpblm->nplayers);
  (void) free(oo);

  whichpblm->noutcomes--;

}

/*
 * Find and locate routines
 */

struct game *findgame(int i)
{/* find game number i */
  struct game *g;

  for (g = whichpblm->firstgame; g->gamenumber != i && g != NULLgame; g = g->nextgame);
  return g;
}

struct game *findgame1(struct node * nn)
{/* find game with root at nn */
  struct game *gg;

  gg = whichpblm->firstgame;
  while (gg != NULLgame && gg->rootnode != nn)
    gg = gg->nextgame;
  return gg;
}

struct node *findnode(int i)
{/* find node number i */
  struct node *n;

  for (n = whichpblm->firstnode; n->nodenumber != i && n != NULLnode; n = n->nextnode);
  return n;
}

struct node *findbranch(struct node * nn, int j)
/* find branch j of node nn */
{
  struct node *n;
  int i;

  for (n = nn->firstbranch, i = 0; i < j; n = n->nextbranch, i++);
  return n;
}

struct node *findlastmember(struct iset * aa)
/* find last member of iset aa */
{
  struct node *n;

  n = aa->firstmember;
  while (n != NULLnode && n->nextmember != NULLnode)
    n = n->nextmember;
  return n;
}

struct iset *findiset(int i)
{/* find iset number i */
  struct iset *n;

  for (n = whichpblm->firstinfoset; n->isetnumber != i && n != NULLiset; n = n->nextiset);
  return n;
}

struct plyr *findplyr(int i)
{/* find player number i */
  struct plyr *p;

  for (p = whichpblm->firstplayer; p->plyrnumber != i && p != NULLplyr; p = p->nextplayer);
  return p;
}

struct outc *findoutcome(int i)
{/* find outcome number i */
  struct outc *o;

  for (o = whichpblm->firstoutcome; o->outnumber != i && o != NULLoutc; o = o->nextoutcome);
  return o;
}


struct plyr *priorplyr(struct plyr * pp)
{/* find previous plyr to pp */
  struct plyr *p;

  for (p = whichpblm->firstplayer; p != NULLplyr && p->nextplayer != pp;
       p = p->nextplayer);
  return p;
}

struct iset *prioriset(struct iset * ii)
{/* find previous iset to ii */
  struct iset *i;

  for (i = whichpblm->firstinfoset; i->nextiset != ii; i = i->nextiset);
  return i;
}

void newtree(void)
{
/* set up new tree */
  struct pblm *vv;

  vv = newpblm();
  whichpblm = vv;
  vv->whichgame = whichpblm->firstgame;
}
