//
// FILE: efgfile.cc -- Write out a .efg file from the old-style structures
//
// $Id$
//

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



void write_outcomes(FILE *f)
{
  fprintf(f, "{ ");
  
  for (struct outc *oc = whichpblm->firstoutcome->nextoutcome; oc != NULLoutc;
       oc = oc->nextoutcome)   {
    if (oc != whichpblm->firstoutcome->nextoutcome)
      fprintf(f, "\n  ");
    fprintf(f, "{ %d { ", oc->outnumber);
    for (struct plyr *pp = whichpblm->firstplayer; pp != NULLplyr;
	 pp = pp->nextplayer)
      fprintf(f, "%f ", oc->component[pp->plyrnumber]);
    fprintf(f, "} \"%s\" }", oc->outname);
  }
  
  fprintf(f, " }\n");
}

// This labels the game number and player number for the subtree in a game
// rooted at node n
void part_nodes_into_games(struct node *n, int game)
{
  n->game = game;
  if (n->infoset != NULLiset)
    n->player = n->infoset->playr->plyrnumber;
  else if (n->firstbranch != NULLnode)
    n->player = 0;
  else
    n->player = -1;

  for (struct node *m = n->firstbranch; m != NULLnode; m = m->nextbranch)  {
    for (struct game *g = whichpblm->firstgame;
	 g != NULLgame && g->rootnode != m;
	 g = g->nextgame);
    if (g == NULLgame)   part_nodes_into_games(m, game);
  }
}

void mark_nodes(void)
{
  for (struct game *g = whichpblm->firstgame; g != NULLgame; g = g->nextgame) {
	// first, label all nodes belonging to this game with the
	// correct game number and player number
    part_nodes_into_games(g->rootnode, g->gamenumber);

	// second, go through infosets for "real" players
    for (struct plyr *p = whichpblm->firstplayer; p != NULLplyr;
	 p = p->nextplayer)  {
      int iset = 1;

      for (struct iset *s = whichpblm->firstinfoset; s != NULLiset;
	   s = s->nextiset)  {
	if (s->playr == p && s->firstmember->game == g->gamenumber)  {
	  int member = 1;
	  for (struct node *n = s->firstmember; n != NULLnode;
	       n = n->nextmember, member++)  {
	    n->iset = iset;
	    n->index = member;
	  }
	  iset++;
	}
      }
    }


    int dummyct = 1, chancect = 1;
	// now, label the infosets for dummy player
    for (struct node *n = whichpblm->firstnode; n != NULLnode;
	 n = n->nextnode)  {
      if (n->game == g->gamenumber && n->player == -1)   {
	n->iset = dummyct++;
	n->index = 1;
      }
      else if (n->game == g->gamenumber && n->player == 0)  {
	n->iset = chancect++;
	n->index = 1;
      }
    }
  }
}

void write_infosets(FILE *f, int game)
{
  fprintf(f, "  { ");

      // first, we make the infosets belonging to the chance player
  fprintf(f, "{ ");
  for (struct node *n = whichpblm->firstnode; n != NULLnode; n = n->nextnode)
    if (n->game == game && n->player == 0)  {
      if (n->iset != 1)
	fprintf(f, "\n      ");
      fprintf(f, "{ \"\" { ");
      for (struct node *m = n->firstbranch; m != NULLnode;
	   m = m->nextbranch)
	fprintf(f, "\"%s\" ", m->branchname);
      fprintf(f, "} { ");
      for (m = n->firstbranch; m != NULLnode; m = m->nextbranch)
	fprintf(f, "%lf ", m->probability);
      fprintf(f, "} }");
    }
  fprintf(f, " }\n");

  for (struct plyr *p = whichpblm->firstplayer; p != NULLplyr; 
       p = p->nextplayer)  {
    int flag = 0;
    fprintf(f, "    {");
    for (struct iset *s = p->firstiset; s != NULLiset; s = s->nextplyriset)  {
      if (s->firstmember->game == game)  {
	if (flag)
	  fprintf(f, "\n     ");
	else
	  flag = 1;
	fprintf(f, " { \"%s\" ", s->isetname);
	fprintf(f, "{ ");
	for (struct node *m = s->firstmember->firstbranch; m != NULLnode;
	     m = m->nextbranch)
	  fprintf(f, "\"%s\" ", m->branchname);
	fprintf(f, "} }");
      }
    }    
    fprintf(f, " }\n");
  } 
  
  fprintf(f, "  }\n");
}

void write_node(FILE *f, struct node *n)
{
  fprintf(f, "    (%d,%d,%d): ", n->player, n->iset, n->index);
  for (struct game *g = whichpblm->firstgame;
       g != NULLgame && g->rootnode != n;
       g = g->nextgame);
  if (n->parent == NULLnode || g != NULLgame)  
    fprintf(f, "(0,0,0) 0 ");
  else  {
    int foo = 1;
    for (struct node *m = n->parent->firstbranch; m != n;
	 m = m->nextbranch, foo++);
    fprintf(f, "(%d,%d,%d) %d ", n->parent->player, n->parent->iset,
	    n->parent->index, foo);
  }

  if (n->nextgame == NULLgame)
    fprintf(f, "0 ");
  else
    fprintf(f, "%d ", n->nextgame->gamenumber);

  if (n->outcome == NULLoutc)
    fprintf(f, "0 ");
  else
    fprintf(f, "%d ", n->outcome->outnumber);

  fprintf(f, "\"%s\"\n", n->nodename);
}

void write_nodes(FILE *f, int game)
{
  fprintf(f, "  {\n");
  
      // first, the dummy nodes
  for (struct node *n = whichpblm->firstnode; n != NULLnode; n = n->nextnode) 
    if (n->game == game && n->player == -1)   
      write_node(f, n);

      // then, the chance nodes
  for (n = whichpblm->firstnode; n != NULLnode; n = n->nextnode)  
    if (n->game == game && n->player == 0)
      write_node(f, n);

      // finally, the "real" player nodes
  for (struct plyr *p = whichpblm->firstplayer; p != NULLplyr;
       p = p->nextplayer) 
    for (struct iset *s = whichpblm->firstinfoset; s != NULLiset;
	 s = s->nextiset) 
      if (s->playr == p && s->firstmember->game == game)
	for (n = s->firstmember; n != NULLnode; n = n->nextmember)
	  write_node(f, n);
  
  fprintf(f, "  }\n");
}


void write_efg_file(FILE *f)
{
  fprintf(f, "{ \"%s\"\n\n", whichpblm->title);

  write_player_names(f);
  fprintf(f, "\n");
  write_outcomes(f);
  fprintf(f, "\n");

  mark_nodes();

  for (struct game *g = whichpblm->firstgame; g != NULLgame; g = g->nextgame) {
    fprintf(f, "{ %d \"%s\"\n", g->gamenumber, whichpblm->title);
    write_infosets(f, g->gamenumber);
    fprintf(f, "\n");
    write_nodes(f, g->gamenumber);
    fprintf(f, "}\n\n");
  }

  fprintf(f, "}\n");
}


void dt1toefg(FILE *in, FILE *out)
{
  initialize();
  if (read_dt1_file(in))  {
    printf("Input file formatted incorrectly.  Operation aborted.\n");
    cleanup();
    return;
  }

  split_subgame_roots();
  write_efg_file(out);
  cleanup();
}
