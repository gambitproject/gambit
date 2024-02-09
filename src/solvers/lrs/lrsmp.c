/* lrsmp.c     library code for lrs extended precision arithmetic */
/* Version 4.0c, August 26, 2009                          */
/* minor change to check result of fscanf */
/* Copyright: David Avis 1999, avis@cs.mcgill.ca          */

/* This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
 */

#ifdef PLRS
#include <sstream>
#include <iostream>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lrsmp.h"

long lrs_digits;        /* max permitted no. of digits   */
long lrs_record_digits; /* this is the biggest acheived so far.     */

/******************************************************************/
/* digit overflow is caught by digits_overflow at the end of this */
/* file, make sure it is either user supplied or uncomment        */
/*  the define below                                              */
/******************************************************************/

#define digits_overflow() lrs_default_digits_overflow()

/*********************************************************/
/* Initialization and allocation procedures - must use!  */
/******************************************************* */

long lrs_mp_init(long dec_digits, FILE *fpin, FILE *fpout)
/* max number of decimal digits for the computation */
{
  /* global variables lrs_ifp and lrs_ofp are file pointers for input and output   */

  lrs_ifp = fpin;
  lrs_ofp = fpout;

  lrs_record_digits = 0;
  if (dec_digits <= 0) {
    dec_digits = DEFAULT_DIGITS;
  }

  lrs_digits = DEC2DIG(dec_digits); /* max permitted no. of digits   */

  if (lrs_digits > MAX_DIGITS) {
#ifdef PLRS
    cout << "Digits must be at most " << DIG2DEC(MAX_DIGITS) << endl;
    cout << "Change MAX_DIGITS and recompile" << endl;
    exit(1);
#else
    fprintf(lrs_ofp, "\nDigits must be at most %ld\nChange MAX_DIGITS and recompile\n",
            DIG2DEC(MAX_DIGITS));
#endif
    lrs_digits = MAX_DIGITS;
    return FALSE;
  }

  return TRUE;
}

lrs_mp_t lrs_alloc_mp_t()
/* dynamic allocation of lrs_mp number */
{
  lrs_mp_t p;
  p = (long *)calloc(lrs_digits + 1, sizeof(long));
  return p;
}

lrs_mp_vector lrs_alloc_mp_vector(long n)
/* allocate lrs_mp_vector for n+1 lrs_mp numbers */
{
  lrs_mp_vector p;
  long i;

  p = (lrs_mp_vector)CALLOC((n + 1), sizeof(lrs_mp *));
  for (i = 0; i <= n; i++) {
    p[i] = (long int *)CALLOC(1, sizeof(lrs_mp));
  }

  return p;
}

void lrs_clear_mp_vector(lrs_mp_vector p, long n)
/* free space allocated to p */
{
  long i;
  for (i = 0; i <= n; i++) {
    free(p[i]);
  }
  free(p);
}

lrs_mp_matrix lrs_alloc_mp_matrix(long m, long n)
/* allocate lrs_mp_matrix for m+1 x n+1 lrs_mp numbers */
{
  lrs_mp_matrix a;
  long *araw;
  int mp_width, row_width;
  int i, j;

  mp_width = lrs_digits + 1;
  row_width = (n + 1) * mp_width;

  araw = (long int *)calloc((m + 1) * row_width, sizeof(long));
  a = (lrs_mp_matrix)calloc((m + 1), sizeof(lrs_mp_vector));

  for (i = 0; i < m + 1; i++) {
    a[i] = (long int **)calloc((n + 1), sizeof(lrs_mp *));

    for (j = 0; j < n + 1; j++) {
      a[i][j] = (araw + i * row_width + j * mp_width);
    }
  }
  return a;
}

void lrs_clear_mp_matrix(lrs_mp_matrix p, long m, long n)
/* free space allocated to lrs_mp_matrix p */
{
  long i;

  /* p[0][0] is araw, the actual matrix storage address */

  free(p[0][0]);

  for (i = 0; i < m + 1; i++) {
    free(p[i]);
  }
  free(p);
}

/*********************************************************/
/* Core library functions - depend on mp implementation  */
/******************************************************* */

void copy(lrs_mp a, lrs_mp b) /* assigns a=b  */
{
  long i;
  for (i = 0; i <= length(b); i++) {
    a[i] = b[i];
  }
}

/********************************************************/
/* Divide two multiple precision integers (c=a/b).      */
/* a is destroyed and contains the remainder on return. */
/* From Knuth Vol.2 SemiNumerical Algorithms            */
/* coded by J. Quinn                                    */
/********************************************************/
void divint(lrs_mp a, lrs_mp b, lrs_mp c) /* c=a/b, a contains remainder on return */
{
  long cy, la, lb, lc, d1, s, t, sig;
  long i, j, qh;

  /*  figure out and save sign, do everything with positive numbers */
  sig = sign(a) * sign(b);

  la = length(a);
  lb = length(b);
  lc = la - lb + 2;
  if (la < lb) {
    storelength(c, TWO);
    storesign(c, POS);
    c[1] = 0;
    normalize(c);
    return;
  }
  for (i = 1; i < lc; i++) {
    c[i] = 0;
  }
  storelength(c, lc);
  storesign(c, (sign(a) == sign(b)) ? POS : NEG);

  /******************************/
  /* division by a single word: */
  /*  do it directly            */
  /******************************/

  if (lb == 2) {
    cy = 0;
    t = b[1];
    for (i = la - 1; i > 0; i--) {
      cy = cy * BASE + a[i];
      a[i] = 0;
      cy -= (c[i] = cy / t) * t;
    }
    a[1] = cy;
    storesign(a, (cy == 0) ? POS : sign(a));
    storelength(a, TWO);
    /*      set sign of c to sig  (**mod**)            */
    storesign(c, sig);
    normalize(c);
    return;
  }
  else {
    /* mp's are actually DIGITS+1 in length, so if length of a or b = */
    /* DIGITS, there will still be room after normalization. */
    /****************************************************/
    /* Step D1 - normalize numbers so b > floor(BASE/2) */
    d1 = BASE / (b[lb - 1] + 1);
    if (d1 > 1) {
      cy = 0;
      for (i = 1; i < la; i++) {
        cy = (a[i] = a[i] * d1 + cy) / BASE;
        a[i] %= BASE;
      }
      a[i] = cy;
      cy = 0;
      for (i = 1; i < lb; i++) {
        cy = (b[i] = b[i] * d1 + cy) / BASE;
        b[i] %= BASE;
      }
      b[i] = cy;
    }
    else {
      a[la] = 0; /* if la or lb = DIGITS this won't work */
      b[lb] = 0;
    }
    /*********************************************/
    /* Steps D2 & D7 - start and end of the loop */
    for (j = 0; j <= la - lb; j++) {
      /*************************************/
      /* Step D3 - determine trial divisor */
      if (a[la - j] == b[lb - 1]) {
        qh = BASE - 1;
      }
      else {
        s = (a[la - j] * BASE + a[la - j - 1]);
        qh = s / b[lb - 1];
        while (qh * b[lb - 2] > (s - qh * b[lb - 1]) * BASE + a[la - j - 2]) {
          qh--;
        }
      }
      /*******************************************************/
      /* Step D4 - divide through using qh as quotient digit */
      cy = 0;
      for (i = 1; i <= lb; i++) {
        s = qh * b[i] + cy;
        a[la - j - lb + i] -= s % BASE;
        cy = s / BASE;
        if (a[la - j - lb + i] < 0) {
          a[la - j - lb + i] += BASE;
          cy++;
        }
      }
      /*****************************************************/
      /* Step D6 - adjust previous step if qh is 1 too big */
      if (cy) {
        qh--;
        cy = 0;
        for (i = 1; i <= lb; i++) /* add a back in */
        {
          a[la - j - lb + i] += b[i] + cy;
          cy = a[la - j - lb + i] / BASE;
          a[la - j - lb + i] %= BASE;
        }
      }
      /***********************************************************************/
      /* Step D5 - write final value of qh.  Saves calculating array indices */
      /* to do it here instead of before D6 */

      c[la - lb - j + 1] = qh;
    }
    /**********************************************************************/
    /* Step D8 - unnormalize a and b to get correct remainder and divisor */

    for (i = lc; c[i - 1] == 0 && i > 2; i--)
      ; /* strip excess 0's from quotient */
    storelength(c, i);
    if (i == 2 && c[1] == 0) {
      storesign(c, POS);
    }
    cy = 0;
    for (i = lb - 1; i >= 1; i--) {
      cy = (a[i] += cy * BASE) % d1;
      a[i] /= d1;
    }
    for (i = la; a[i - 1] == 0 && i > 2; i--)
      ; /* strip excess 0's from quotient */
    storelength(a, i);
    if (i == 2 && a[1] == 0) {
      storesign(a, POS);
    }
    if (cy) {
      fprintf(stdout, "divide error");
      exit(1);
    }
    for (i = lb - 1; i >= 1; i--) {
      cy = (b[i] += cy * BASE) % d1;
      b[i] /= d1;
    }
  }
}
/* end of divint */

void gcd(lrs_mp u, lrs_mp v) /*returns u=gcd(u,v) destroying v */
                             /*Euclid's algorithm.  Knuth, II, p.320
                                modified to avoid copies r=u,u=v,v=r
                                Switches to single precision when possible for greater speed */
{
  lrs_mp r;
  unsigned long ul, vl;
  long i;
  static unsigned long maxspval = MAXD; /* Max value for the last digit to guarantee */
  /* fitting into a single long integer. */

  static long maxsplen; /* Maximum digits for a number that will fit */
  /* into a single long integer. */

  static long firstime = TRUE;

  if (firstime) /* initialize constants */
  {
    for (maxsplen = 2; maxspval >= BASE; maxsplen++) {
      maxspval /= BASE;
    }
    firstime = FALSE;
  }
  if (mp_greater(v, u)) {
    goto bigv;
  }
bigu:
  if (zero(v)) {
    return;
  }
  if ((i = length(u)) < maxsplen || (i == maxsplen && u[maxsplen - 1] < maxspval)) {
    goto quickfinish;
  }
  divint(u, v, r);
  normalize(u);

bigv:
  if (zero(u)) {
    copy(u, v);
    return;
  }
  if ((i = length(v)) < maxsplen || (i == maxsplen && v[maxsplen - 1] < maxspval)) {
    goto quickfinish;
  }
  divint(v, u, r);
  normalize(v);
  goto bigu;
  /* Base 10000 only at the moment */
  /* when u and v are small enough, transfer to single precision integers */
  /* and finish with euclid's algorithm, then transfer back to lrs_mp */
quickfinish:
  ul = vl = 0;
  for (i = length(u) - 1; i > 0; i--) {
    ul = BASE * ul + u[i];
  }
  for (i = length(v) - 1; i > 0; i--) {
    vl = BASE * vl + v[i];
  }
  if (ul > vl) {
    goto qv;
  }
qu:
  if (!vl) {
    for (i = 1; ul; i++) {
      u[i] = ul % BASE;
      ul = ul / BASE;
    }
    storelength(u, i);
    return;
  }
  ul %= vl;
qv:
  if (!ul) {
    for (i = 1; vl; i++) {
      u[i] = vl % BASE;
      vl = vl / BASE;
    }
    storelength(u, i);
    return;
  }
  vl %= ul;
  goto qu;
}

long compare(lrs_mp a, lrs_mp b) /* a ? b and returns -1,0,1 for <,=,> */
{
  long i;

  if (a[0] > b[0]) {
    return 1L;
  }
  if (a[0] < b[0]) {
    return -1L;
  }

  for (i = length(a) - 1; i >= 1; i--) {
    if (a[i] < b[i]) {
      if (sign(a) == POS) {
        return -1L;
      }
      else {
        return 1L;
      }
    }
    if (a[i] > b[i]) {
      if (sign(a) == NEG) {
        return -1L;
      }
      else {
        return 1L;
      }
    }
  }
  return 0L;
}

long mp_greater(lrs_mp a, lrs_mp b) /* tests if a > b and returns (TRUE=POS) */
{
  long i;

  if (a[0] > b[0]) {
    return (TRUE);
  }
  if (a[0] < b[0]) {
    return (FALSE);
  }

  for (i = length(a) - 1; i >= 1; i--) {
    if (a[i] < b[i]) {
      if (sign(a) == POS) {
        return (0);
      }
      else {
        return (1);
      }
    }
    if (a[i] > b[i]) {
      if (sign(a) == NEG) {
        return (0);
      }
      else {
        return (1);
      }
    }
  }
  return (0);
}
void itomp(long in, lrs_mp a)
/* convert integer i to multiple precision with base BASE */
{
  long i;
  a[0] = 2; /* initialize to zero */
  for (i = 1; i < lrs_digits; i++) {
    a[i] = 0;
  }
  if (in < 0) {
    storesign(a, NEG);
    in = in * (-1);
  }
  i = 0;
  while (in != 0) {
    i++;
    a[i] = in - BASE * (in / BASE);
    in = in / BASE;
    storelength(a, i + 1);
  }
} /* end of itomp */

void linint(lrs_mp a, long ka, lrs_mp b, long kb) /*compute a*ka+b*kb --> a */
/***Handbook of Algorithms and Data Structures P.239 ***/
{
  long i, la, lb;
  la = length(a);
  lb = length(b);
  for (i = 1; i < la; i++) {
    a[i] *= ka;
  }
  if (sign(a) != sign(b)) {
    kb = (-kb);
  }
  if (lb > la) {
    storelength(a, lb);
    for (i = la; i < lb; i++) {
      a[i] = 0;
    }
  }
  for (i = 1; i < lb; i++) {
    a[i] += kb * b[i];
  }
  normalize(a);
}
/***end of linint***/

void mptodouble(lrs_mp a, double *x) /* convert lrs_mp to double */
{
  long i, la;
  double y = 1.0;

  (*x) = 0;
  la = length(a);
  for (i = 1; i < la; i++) {
    (*x) = (*x) + y * a[i];
    y = y * BASE;
  }
  if (negative(a)) {
    (*x) = -(*x);
  }
}

void mulint(lrs_mp a, lrs_mp b, lrs_mp c) /* multiply two integers a*b --> c */

/***Handbook of Algorithms and Data Structures, p239  ***/
{
  long nlength, i, j, la, lb;
  /*** b and c may coincide ***/
  la = length(a);
  lb = length(b);
  nlength = la + lb - 2;
  if (nlength > lrs_digits) {
    digits_overflow();
  }

  for (i = 0; i < la - 2; i++) {
    c[lb + i] = 0;
  }
  for (i = lb - 1; i > 0; i--) {
    for (j = 2; j < la; j++) {
      if ((c[i + j - 1] += b[i] * a[j]) > MAXD - (BASE - 1) * (BASE - 1) - MAXD / BASE) {
        c[i + j - 1] -= (MAXD / BASE) * BASE;
        c[i + j] += MAXD / BASE;
      }
    }
    c[i] = b[i] * a[1];
  }
  storelength(c, nlength);
  storesign(c, sign(a) == sign(b) ? POS : NEG);
  normalize(c);
}
/***end of mulint ***/

void normalize(lrs_mp a)
{
  long cy, i, la;
  la = length(a);
start:
  cy = 0;
  for (i = 1; i < la; i++) {
    cy = (a[i] += cy) / BASE;
    a[i] -= cy * BASE;
    if (a[i] < 0) {
      a[i] += BASE;
      cy--;
    }
  }
  while (cy > 0) {
    a[i++] = cy % BASE;
    cy /= BASE;
  }
  if (cy < 0) {
    a[la - 1] += cy * BASE;
    for (i = 1; i < la; i++) {
      a[i] = (-a[i]);
    }
    storesign(a, sign(a) == POS ? NEG : POS);
    goto start;
  }
  while (a[i - 1] == 0 && i > 2) {
    i--;
  }
  if (i > lrs_record_digits) {
    if ((lrs_record_digits = i) > lrs_digits) {
      digits_overflow();
    }
  };
  storelength(a, i);
  if (i == 2 && a[1] == 0) {
    storesign(a, POS);
  }
} /* end of normalize */

long length(lrs_mp a)
{
  /* formerly a macro but conflicts with string length */
  return ((a[0] > 0) ? a[0] : -a[0]);
}

long mptoi(lrs_mp a) /* convert lrs_mp to long integer */
{
  long len = length(a);
  if (len == 2) {
    return sign(a) * a[1];
  }
  if (len == 3) {
    return sign(a) * (a[1] + BASE * a[2]);
  }
  notimpl("mp to large for conversion to long");
  return 0; /* never executed */
}

#ifdef PLRS
string prat(const char name[], lrs_mp Nin, lrs_mp Din) /*reduce and print Nin/Din  */
{

  lrs_mp Nt, Dt;
  long i;
  // create stream to collect output
  stringstream ss;
  string str;

  ss << name;

  /* reduce fraction */
  copy(Nt, Nin);
  copy(Dt, Din);
  reduce(Nt, Dt);
  /* pipe output to stream */
  if (sign(Nin) * sign(Din) == NEG) {
    ss << "-";
  }
  else {
    ss << " ";
  }

  ss << Nt[length(Nt) - 1];

  for (i = length(Nt) - 2; i >= 1; i--) {
    ss << Nt[i];
  }
  if (!(Dt[0] == 2 && Dt[1] == 1)) {
    /* rational */
    ss << "/";
    ss << Dt[length(Dt) - 1];
    for (i = length(Dt) - 2; i >= 1; i--) {
      ss << Dt[i];
    }
  }
  ss << " ";
  // pipe stream to single string
  str = ss.str();
  return str;
}

char *cprat(const char name[], lrs_mp Nin, lrs_mp Din)
{
  char *ret;
  unsigned long len;
  int i, offset = 0;
  string s;
  const char *cstr;

  s = prat(name, Nin, Din);
  cstr = s.c_str();
  len = strlen(cstr);
  ret = (char *)malloc(sizeof(char) * (len + 1));

  for (i = 0; i + offset < len + 1;) {
    if (cstr[i + offset] != ' ') {
      ret[i] = cstr[i + offset];
      i++;
    }
    else { /* skip whitespace */
      offset++;
    }
  }

  return ret;
}

string pmp(char name[], lrs_mp a) /*print the long precision integer a */
{

  long i;
  // create stream to collect output
  stringstream ss;
  string str;

  ss << name;
  if (sign(a) == NEG) {
    ss << "-";
  }
  else {
    ss << " ";
  }

  ss << a[length(a) - 1];
  for (i = length(a) - 2; i >= 1; i--) {
    ss << a[i];
  }

  ss << " ";

  // pipe stream to single string
  str = ss.str();
  return str;
}
#else
void prat(const char name[], lrs_mp Nin, lrs_mp Din) /*reduce and print Nin/Din  */
{
  lrs_mp Nt, Dt;
  long i;
  fprintf(lrs_ofp, "%s", name);
  /* reduce fraction */
  copy(Nt, Nin);
  copy(Dt, Din);
  reduce(Nt, Dt);
  /* print out       */
  if (sign(Nin) * sign(Din) == NEG) {
    fprintf(lrs_ofp, "-");
  }
  else {
    fprintf(lrs_ofp, " ");
  }
  fprintf(lrs_ofp, "%lu", Nt[length(Nt) - 1]);
  for (i = length(Nt) - 2; i >= 1; i--) {
    fprintf(lrs_ofp, FORMAT, Nt[i]);
  }
  if (!(Dt[0] == 2 && Dt[1] == 1)) /* rational */
  {
    fprintf(lrs_ofp, "/");
    fprintf(lrs_ofp, "%lu", Dt[length(Dt) - 1]);
    for (i = length(Dt) - 2; i >= 1; i--) {
      fprintf(lrs_ofp, FORMAT, Dt[i]);
    }
  }
  fprintf(lrs_ofp, " ");
}

void pmp(char name[], lrs_mp a) /*print the long precision integer a */
{

  long i;
  fprintf(lrs_ofp, "%s", name);
  if (sign(a) == NEG) {
    fprintf(lrs_ofp, "-");
  }
  else {
    fprintf(lrs_ofp, " ");
  }
  fprintf(lrs_ofp, "%lu", a[length(a) - 1]);
  for (i = length(a) - 2; i >= 1; i--) {
    fprintf(lrs_ofp, FORMAT, a[i]);
  }
  fprintf(lrs_ofp, " ");
}
#endif

long readrat(lrs_mp Na, lrs_mp Da)
/* read a rational or integer and convert to lrs_mp with base BASE */
/* returns true if denominator is not one                      */
/* returns 999 if premature end of file                        */
{
  char in[MAXINPUT], num[MAXINPUT], den[MAXINPUT];
  if (fscanf(lrs_ifp, "%s", in) == EOF) {
    fprintf(lrs_ofp, "\nInvalid input: check you have entered enough data!\n");
    exit(1);
  }
  if (!strcmp(in, "end")) /*premature end of input file */
  {
    return (999L);
  }
  atoaa(in, num, den); /*convert rational to num/dem strings */
  atomp(num, Na);
  if (den[0] == '\0') {
    itomp(1L, Da);
    return (FALSE);
  }
  atomp(den, Da);
  return (TRUE);
}

void addint(lrs_mp a, lrs_mp b, lrs_mp c) /* compute c=a+b */
{
  copy(c, a);
  linint(c, 1, b, 1);
}

void atomp(char s[], lrs_mp a) /*convert string to lrs_mp integer */
{
  lrs_mp mpone;
  long diff, ten, i, sig;
  itomp(1L, mpone);
  ten = 10L;
  for (i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
    ;
  /*skip white space */
  sig = POS;
  if (s[i] == '+' || s[i] == '-') { /* sign */
    sig = (s[i++] == '+') ? POS : NEG;
  }
  itomp(0L, a);
  while (s[i] >= '0' && s[i] <= '9') {
    diff = s[i] - '0';
    linint(a, ten, mpone, diff);
    i++;
  }
  storesign(a, sig);
  if (s[i]) {
    fprintf(stderr, "\nIllegal character in number: '%s'\n", s + i);
    exit(1);
  }

} /* end of atomp */

void subint(lrs_mp a, lrs_mp b, lrs_mp c) /* compute c=a-b */

{
  copy(c, a);
  linint(a, 1, b, -1);
}

void decint(lrs_mp a, lrs_mp b) /* compute a=a-b */ { linint(a, 1, b, -1); }

long myrandom(long num, long nrange)
/* return a random number in range 0..nrange-1 */

{
  long i;
  i = (num * 401 + 673) % nrange;
  return (i);
}

long atos(char s[]) /* convert s to integer */
{
  long i, j;
  j = 0;
  for (i = 0; s[i] >= '0' && s[i] <= '9'; ++i) {
    j = 10 * j + s[i] - '0';
  }
  return (j);
}

void stringcpy(char *s, char *t) /*copy t to s pointer version */
{
  while (((*s++) = (*t++)) != '\0')
    ;
}

void rattodouble(lrs_mp a, lrs_mp b, double *x) /* convert lrs_mp rational to double */

{
  double y;
  mptodouble(a, &y);
  mptodouble(b, x);
  *x = y / (*x);
}

void atoaa(char in[], char num[], char den[])
/* convert rational string in to num/den strings */
{
  long i, j;
  for (i = 0; in[i] != '\0' && in[i] != '/'; i++) {
    num[i] = in[i];
  }
  num[i] = '\0';
  den[0] = '\0';
  if (in[i] == '/') {
    for (j = 0; in[j + i + 1] != '\0'; j++) {
      den[j] = in[i + j + 1];
    }
    den[j] = '\0';
  }
} /* end of atoaa */

void lcm(lrs_mp a, lrs_mp b)
/* a = least common multiple of a, b; b is preserved */
{
  lrs_mp u, v;
  copy(u, a);
  copy(v, b);
  gcd(u, v);
  exactdivint(a, u, v); /* v=a/u   no remainder*/
  mulint(v, b, a);
} /* end of lcm */

void reducearray(lrs_mp_vector p, long n)
/* find largest gcd of p[0]..p[n-1] and divide through */
{
  lrs_mp divisor;
  lrs_mp Temp;
  long i = 0L;

  while ((i < n) && zero(p[i])) {
    i++;
  }
  if (i == n) {
    return;
  }

  copy(divisor, p[i]);
  storesign(divisor, POS);
  i++;

  while (i < n) {
    if (!zero(p[i])) {
      copy(Temp, p[i]);
      storesign(Temp, POS);
      gcd(divisor, Temp);
    }
    i++;
  }

  /* reduce by divisor */
  for (i = 0; i < n; i++) {
    if (!zero(p[i])) {
      reduceint(p[i], divisor);
    }
  }
} /* end of reducearray */

void reduceint(lrs_mp Na, lrs_mp Da) /* divide Na by Da and return */
{
  lrs_mp Temp;
  copy(Temp, Na);
  exactdivint(Temp, Da, Na);
}

void reduce(lrs_mp Na, lrs_mp Da) /* reduces Na Da by gcd(Na,Da) */
{
  lrs_mp Nb, Db, Nc, Dc;
  copy(Nb, Na);
  copy(Db, Da);
  storesign(Nb, POS);
  storesign(Db, POS);
  copy(Nc, Na);
  copy(Dc, Da);
  gcd(Nb, Db); /* Nb is the gcd(Na,Da) */
  exactdivint(Nc, Nb, Na);
  exactdivint(Dc, Nb, Da);
}

long comprod(lrs_mp Na, lrs_mp Nb, lrs_mp Nc, lrs_mp Nd) /* +1 if Na*Nb > Nc*Nd  */
                                                         /* -1 if Na*Nb < Nc*Nd  */
                                                         /*  0 if Na*Nb = Nc*Nd  */
{
  lrs_mp mc, md;
  mulint(Na, Nb, mc);
  mulint(Nc, Nd, md);
  linint(mc, ONE, md, -ONE);
  if (positive(mc)) {
    return (1);
  }
  if (negative(mc)) {
    return (-1);
  }
  return (0);
}

void notimpl(char s[])
{
  fflush(stdout);
  fprintf(stderr, "\nAbnormal Termination  %s\n", s);
  exit(1);
}

void getfactorial(lrs_mp factorial, long k) /* compute k factorial in lrs_mp */
{
  lrs_mp temp;
  long i;
  itomp(ONE, factorial);
  for (i = 2; i <= k; i++) {
    itomp(i, temp);
    mulint(temp, factorial, factorial);
  }
} /* end of getfactorial */
/***************************************************************/
/*     Package of routines for rational arithmetic             */
/***************************************************************/

void scalerat(lrs_mp Na, lrs_mp Da, long ka) /* scales rational by ka */
{
  lrs_mp Nt;
  copy(Nt, Na);
  itomp(ZERO, Na);
  linint(Na, ZERO, Nt, ka);
  reduce(Na, Da);
}

void linrat(lrs_mp Na, lrs_mp Da, long ka, lrs_mp Nb, lrs_mp Db, long kb, lrs_mp Nc, lrs_mp Dc)
/* computes Nc/Dc = ka*Na/Da  +kb* Nb/Db
   and reduces answer by gcd(Nc,Dc) */
{
  lrs_mp c;
  mulint(Na, Db, Nc);
  mulint(Da, Nb, c);
  linint(Nc, ka, c, kb); /* Nc = (ka*Na*Db)+(kb*Da*Nb)  */
  mulint(Da, Db, Dc);    /* Dc =  Da*Db           */
  reduce(Nc, Dc);
}

void divrat(lrs_mp Na, lrs_mp Da, lrs_mp Nb, lrs_mp Db, lrs_mp Nc, lrs_mp Dc)
/* computes Nc/Dc = (Na/Da)  / ( Nb/Db )
   and reduces answer by gcd(Nc,Dc) */
{
  mulint(Na, Db, Nc);
  mulint(Da, Nb, Dc);
  reduce(Nc, Dc);
}

void mulrat(lrs_mp Na, lrs_mp Da, lrs_mp Nb, lrs_mp Db, lrs_mp Nc, lrs_mp Dc)
/* computes Nc/Dc = Na/Da  * Nb/Db and reduces by gcd(Nc,Dc) */
{
  mulint(Na, Nb, Nc);
  mulint(Da, Db, Dc);
  reduce(Nc, Dc);
}

/*     End package of routines for rational arithmetic         */

/***************************************************************/
/*                                                             */
/*  End of package for multiple precision arithmetic           */
/*                                                             */
/***************************************************************/

void *xcalloc(long n, long s, long l, char *f)
{
  void *tmp;

  tmp = calloc(n, s);
  if (tmp == 0) {
    char buf[200];

    sprintf(buf, "\n\nFatal error on line %ld of %s", l, f);
    perror(buf);
    exit(1);
  }
  return tmp;
}

void lrs_getdigits(long *a, long *b)
{
  /* send digit information to user */
  *a = DIG2DEC(lrs_digits);
  *b = DIG2DEC(lrs_record_digits);
  return;
}

void lrs_default_digits_overflow()
{
  fprintf(stdout, "\nOverflow at digits=%ld", DIG2DEC(lrs_digits));
  fprintf(stdout, "\nInitialize lrs_mp_init with  n > %ldL\n", DIG2DEC(lrs_digits));

  exit(1);
}

#ifdef PLRS

/* read a rational or integer and convert to lrs_mp with base BASE */
/* returns true if denominator is not one                      */
/* returns 999 if premature end of file                        */
long plrs_readrat(lrs_mp Na, lrs_mp Da, const char *rat)
{
  char in[MAXINPUT], num[MAXINPUT], den[MAXINPUT];
  strcpy(in, rat);
  atoaa(in, num, den); /*convert rational to num/dem strings */
  atomp(num, Na);
  if (den[0] == '\0') {
    itomp(1L, Da);
    return (FALSE);
  }
  atomp(den, Da);
  return (TRUE);
}

#endif

/* end of lrsmp.c */
