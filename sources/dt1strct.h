//
// FILE: dt1strct.h -- Old Gambit game-representation data structures
//
// $Id$
//

#define NAMESIZE                   15

struct pblm {	/* problem */
  int pblmnumber;
  char title[80];
  char filename[80];
  struct game *firstgame, *whichgame;
  struct node *firstnode;
  struct plyr *firstplayer;
  struct outc *firstoutcome;
  struct iset *firstinfoset;
  int nplayers, noutcomes, nnodes, nisets, ngames;
};

struct game {	/* extensive form game */
  int gamenumber;
  char gamename[NAMESIZE];
  struct game *nextgame;
  struct node *rootnode;
};

struct node {	/* nodes in extensive form tree	*/
  int nodenumber;
// these next four values are used for the renumbering into new-style notation 
  int game, player, iset, index;
  char nodename[NAMESIZE + 1];
  char branchname[NAMESIZE + 1];
  struct node *nextnode;
  struct node *parent, *firstbranch, *nextbranch;
  struct iset *infoset;
  struct outc *outcome;
  struct node *whichbranch;
  struct node *nextmember;
  struct game *nextgame;
  double probability;
};

struct iset {	/* info sets in extensive form tree */
  int isetnumber;
  int isetplyrnumber;
  char isetname[NAMESIZE + 1];
  struct iset *nextiset, *nextagent;
  struct iset *nextplyriset;
  struct node *firstmember;
  struct plyr *playr;
  int branches;
};

struct plyr {	/* player */
  int plyrnumber;
  char plyrname[NAMESIZE + 1];
  struct plyr *nextplayer;
  struct iset *firstiset;
  int length, nisets;
};

struct outc {	/* outcome */
  int outnumber;
  char outname[NAMESIZE + 1];
  struct outc *nextoutcome;
  float *component;
};

