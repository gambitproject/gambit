//#
//# FILE: efgfile.cc -- Write out a .efg file from the old-style structures
//#
//# $Id$
//#

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dt1strct.h"
#include "dt1util.h"

struct pblm *whichpblm;
struct pblm *NULLpblm;
struct game *NULLgame;
struct node *NULLnode;
struct iset *NULLiset;
struct plyr *NULLplyr;
struct outc *NULLoutc;

char string[256];	/* generic string buffer  */

//
// Initialization and cleanup routines for old-style data structures
// 

void cleanup(void)
{
  freepblms();

  free(NULLpblm);
  free(NULLgame);
  free(NULLnode);
  free(NULLplyr);
  free(NULLiset);
  free(NULLoutc);
}

void initialize(void)
{
  NULLpblm = (struct pblm *) malloc(sizeof(struct pblm));
  NULLgame = (struct game *) malloc(sizeof(struct game));
  NULLnode = (struct node *) malloc(sizeof(struct node));
  NULLplyr = (struct plyr *) malloc(sizeof(struct plyr));
  NULLiset = (struct iset *) malloc(sizeof(struct iset));
  NULLoutc = (struct outc *) malloc(sizeof(struct outc));

  NULLpblm->firstgame = NULLgame;
  NULLpblm->firstnode = NULLnode;
  NULLpblm->firstoutcome = NULLoutc;
  NULLpblm->firstinfoset = NULLiset;
  NULLpblm->firstplayer = NULLplyr;
  NULLgame->nextgame = NULLgame;
  NULLgame->rootnode = NULLnode;
  NULLnode->nextnode = NULLnode;
  NULLnode->parent = NULLnode;
  NULLnode->firstbranch = NULLnode;
  NULLnode->nextbranch = NULLnode;
  NULLnode->nextmember = NULLnode;

  NULLnode->whichbranch = NULLnode;
  NULLnode->infoset = NULLiset;
  NULLnode->outcome = NULLoutc;
  NULLnode->nextgame = NULLgame;
  NULLiset->firstmember = NULLnode;
  NULLiset->nextiset = NULLiset;

  NULLiset->nextplyriset = NULLiset;
  NULLiset->playr = NULLplyr;
  NULLplyr->nextplayer = NULLplyr;
  NULLplyr->firstiset = NULLiset;
  NULLplyr->plyrnumber = 0;
  NULLplyr->nisets = 1;
  NULLoutc->nextoutcome = NULLoutc;
  NULLpblm->pblmnumber = 0;
  NULLgame->gamenumber = 0;
  NULLnode->nodenumber = 0;
  NULLiset->isetnumber = 0;
  NULLoutc->outnumber = 0;

  newtree();
}

//
// Reading in a .dt1 file into the old structures
//

extern int yyparse(void);
extern void dt1_set_input(FILE *), dt1_close_input(void);

int read_dt1_file(FILE *f)
{
  int errorNo;

  dt1_set_input(f);
  errorNo = yyparse();
  if (!errorNo) return 0;
  dt1_close_input();
  return 1;
}


//
// Data conversion and writing .efg files
//

void split_subgame_roots(void)
{
  for (struct game *g = whichpblm->firstgame; g != NULLgame;
       g = g->nextgame)  {
    if (g->rootnode->parent != g->rootnode)  {
      struct node *n = newnode(NULLnode);
      n->parent = g->rootnode->parent;
      if (n->parent->firstbranch == g->rootnode)   
	n->parent->firstbranch = n;
      else   {
	for (struct node *m = g->rootnode->parent->firstbranch;
	     m->nextbranch != g->rootnode;
	     m = m->nextbranch); 
	m->nextbranch = n;
      }
      n->nextbranch = g->rootnode->nextbranch;
      n->nextgame = g;
      n->probability = g->rootnode->probability;
      strncpy(n->nodename, g->rootnode->nodename, NAMESIZE);
      strncpy(n->branchname, g->rootnode->branchname, NAMESIZE);
      g->rootnode->branchname[0] = '\0';
      g->rootnode->nextbranch = NULLnode;
      g->rootnode->parent = NULLnode;
      g->rootnode->probability = -1.0;
    }
  }
}


void write_player_names(FILE *f)
{
  fprintf(f, "{ \"CHANCE\" ");

  for (struct plyr *pp = whichpblm->firstplayer; pp != NULLplyr;
       pp = pp->nextplayer)
    fprintf(f, "\"%s\" ", pp->plyrname);
  
  fprintf(f, "}\n");
}

int cisets;

void write_node(FILE *f, struct node *n)
{
  if (n->infoset != NULLiset)   {
    fprintf(f, "p \"%s\" %d %d \"%s\" {", n->nodename,
	    n->infoset->playr->plyrnumber, n->infoset->isetplyrnumber,
	    n->infoset->isetname);
    int br = 1;
    for (struct node *m = n->firstbranch; m != NULLnode; 
	 m = m->nextbranch, br++)
      if (!strcmp(m->branchname, ""))
	fprintf(f, " \"%d\"", br);
      else
	fprintf(f, " \"%s\"", m->branchname);
    
    fprintf(f, " } ");
    
    if (n->outcome != whichpblm->firstoutcome)  {
      fprintf(f, "%d \"%s\" { ", n->outcome->outnumber, n->outcome->outname);
      for (int pl = 1; pl <= whichpblm->nplayers; pl++)
	fprintf(f, "%lf ", n->outcome->component[pl]);
      fprintf(f, "}");
    }
    else
      fprintf(f, "0");

    fprintf(f, "\n");
  }
  else if (n->firstbranch != NULLnode)   {
    fprintf(f, "c \"%s\" %d \"\" {", n->nodename, cisets++);
    int br = 1;
    
    for (struct node *m = n->firstbranch; m != NULLnode; 
	 m = m->nextbranch, br++)  {
      if (!strcmp(m->branchname, ""))  
	fprintf(f, " \"%d\"", br);
      else
	fprintf(f, " \"%s\"", m->branchname);
      fprintf(f, " %lf ", m->probability);
    }
    
    fprintf(f, " } ");

    if (n->outcome != whichpblm->firstoutcome)  {
      fprintf(f, "%d \"%s\" { ", n->outcome->outnumber, n->outcome->outname);
      for (int pl = 1; pl <= whichpblm->nplayers; pl++)
	fprintf(f, "%lf ", n->outcome->component[pl]);
      fprintf(f, "}");
    }
    else
      fprintf(f, "0");

    fprintf(f, "\n");
  }
  else   {    // terminal node
    fprintf(f, "t \"%s\" ", n->nodename);
    if (n->outcome != whichpblm->firstoutcome)  {
      fprintf(f, "%d \"%s\" { ", n->outcome->outnumber, n->outcome->outname);
      for (int pl = 1; pl <= whichpblm->nplayers; pl++)
	fprintf(f, "%lf ", n->outcome->component[pl]);
      fprintf(f, "}");
    }
    else
      fprintf(f, "0");
    fprintf(f, "\n");
  }


  for (struct node *m = n->firstbranch; m != NULLnode; m = m->nextbranch)
    write_node(f, m);
}
  
void write_efg_file(FILE *f)
{
  fprintf(f, "EFG 2 D \"%s\" {", whichpblm->title);

  for (struct plyr *p = whichpblm->firstplayer; p != NULLplyr;
       p = p->nextplayer)   {
    fprintf(f, " \"Player %d\"", p->plyrnumber);
    int iset = 1;
    for (struct iset *s = whichpblm->firstinfoset; s != NULLiset;
	 s = s->nextiset) 
      if (s->playr == p)   s->isetplyrnumber = iset++;
  }

  fprintf(f, " }\n");

  cisets = 1;

  write_node(f, whichpblm->firstgame->rootnode);
}


void dt1toefg(FILE *in, FILE *out)
{
  initialize();
  if (read_dt1_file(in))  {
    printf("Input file formatted incorrectly.  Operation aborted.\n");
    cleanup();
    return;
  }

  
  if (whichpblm->ngames != 1)   {
    printf("Sorry, .efg files do not support multiple game elements currently.\n");
    return;
  }

  write_efg_file(out);
  cleanup();
}
