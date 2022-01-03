/*******************************************************/
/* lrsnashlib is a library of routines for computing   */
/* computing all nash equilibria for two person games  */
/* given by mxn payoff matrices A,B                    */
/*                                                     */
/*                                                     */
/* Main user callable function is                      */
/*         lrs_solve_nash(game *g)                     */
/*                                                     */
/* Sample driver: lrsnash.c                            */
/* Derived from nash.c in lrslib-060                   */
/* by Terje Lensberg, October 26, 2015:                */
/*******************************************************/

/*************/
/* Games     */
/*************/

#define MAXSTRAT 200
#define ROW 0
#define COL 1

typedef struct {
        long num;
        long den;
} ratnum;

typedef struct {
  long nstrats[2];
  ratnum payoff[MAXSTRAT][MAXSTRAT][2];
  // For auxiliary information
        void *aux;
} game;

typedef struct {
        char name[100];
        int fwidth[MAXSTRAT][2]; // Column field widths (for output)
} gInfo;


int lrs_solve_nash(game * g);

long nash2_main(lrs_dic * P1, lrs_dat * Q1, lrs_dic * P2orig,
                lrs_dat * Q2, long *numequilib, lrs_mp_vector output, long linindex[]);
        /* lrs driver, argv[2]= 2nd input file for nash equilibria */

long lrs_getfirstbasis2(lrs_dic ** D_p, lrs_dat * Q, lrs_dic * P2orig, lrs_mp_matrix * Lin, long no_output,
                        long linindex[]);

long getabasis2(lrs_dic * P, lrs_dat * Q, lrs_dic * P2orig, long order[], long linindex[]);

long lrs_nashoutput(lrs_dat * Q, lrs_mp_vector output, long player);
                  /* returns TRUE and prints output if not the origin */

int lrs_solve_nash_legacy (int argc, char *argv[]);

void BuildRep(lrs_dic * P, lrs_dat * Q, const game * g, int p1, int p2);
void FillFirstRow(lrs_dic * P, lrs_dat * Q, int n);
void FillLinearityRow(lrs_dic * P, lrs_dat * Q, int m, int n);
void FillConstraintRows(lrs_dic * P, lrs_dat * Q, const game * g, int p1, int p2, int firstRow);
void FillNonnegativityRows(lrs_dic * P, lrs_dat * Q, int firstRow, int lastRow, int n);
void printGame(game * g);
void setFwidth(game *g, int len);
void initFwidth(game *g);
void updateFwidth(game *g, int col, int pos, char *str);


long FirstTime;          /* set this to true for every new game to be solved */
static long Debug_flag;
static long Verbose_flag;

