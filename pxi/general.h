// File: general.h, header file for general.cc
// This file defines generally useful functions and constants that do not
// belong obviously anywhere else.  Just a grab-bag of random utilities
// Feel free to place these functions somewhere else if it makes more sense


#ifndef	GENERAL_H
#define	GENERAL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef	BOOL_DEFINED
#define Bool	int
#endif
#define	TRUE	1
#define	FALSE	0

void FindStringInFile(FILE *fp,const char *s);
void FindCharInFile(FILE *fp,char c,int rewind=0);
int FindIntArray(int *array,int num,int what);
Bool	IsCursor(int ch);
Bool	IsNumeric(int ch);
Bool	IsAlphaNum(int ch);
Bool	IsDelete(int ch);

#endif