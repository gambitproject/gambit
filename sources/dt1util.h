//
// FILE: dt1util.h -- Utility functions for old-style data structures
//
// $Id$
//

/* Memory allocation routines */
struct pblm *newpblm(void);
struct game *newgame(void);
struct node *newnode(struct node *parent);
struct iset *newiset(struct node *nn, struct plyr *p, int branches);
struct plyr *newplayer(void);
struct outc *newoutcome(void);

void newtree(void);

void freepblms(void);
void freepblm(void);
void freegames(void);
void freegame(struct game *gg);
void freenodes(void);
void freenode(struct node *nn);
void freeisets(void);
void freeiset(struct iset *ii);
void delete_plyr(struct plyr *pp);
void freeplyrs(void);
void freeoutcomes(void);
void freeoutcome(struct outc *oo);

/* Find and locate routines */
struct game *findgame(int i);
struct game *findgame1(struct node *nn);
struct node *findnode(int i);
struct node *findbranch(struct node *nn, int j);
struct node *findlastmember(struct iset *aa);
struct iset *findiset(int i);
struct plyr *findplyr(int i);
struct outc *findoutcome(int i);
struct iset *prioriset(struct iset *ii);
struct plyr *priorplyr(struct plyr *pp);
