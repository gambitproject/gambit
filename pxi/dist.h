#include "gambitio.h"
#include "gtuple.h"
/********************** HEADER FILE FOR DIST.CC ****************/
#define ROW						0
#define COL						1
#define	LARGE_NUMBER	100.00
#define RECT_SIZE 		0.1

typedef	gTuple<float> PointNd;
gOutput &operator<<(gOutput &op,const PointNd &p);

/*-----------------functions in dist.cc--------------------------------*/
float Distance(PointNd first,PointNd second);
int Closest(gTuple<PointNd> equis,PointNd this_point);
