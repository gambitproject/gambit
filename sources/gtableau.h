//#
//# FILE: gtableau.h -- Implementation of tableau type
//#
//# @(#)gtableau.h	1.19 8/13/95
//#


#ifndef GTABLEAU_H
#define GTABLEAU_H

#include <assert.h>
#include "gmisc.h"
#include "gstream.h"
#include "gmatrix.h"
#include "gblock.h"
#include "gmap.h"
#include "bfs.h"

#if defined(__BORLANDC__)
#include <stdio.h>
#include <conio.h>
#endif

template <class T> class gTableau  {
  protected:
    gMatrix<T> Tableau;
    gBlock<int> Col_Labels, Row_Labels;

  public:
    gTableau(void);
    gTableau(int row_min, int row_max, int row_labels,
	     int col_min, int col_max, int col_labels);
    gTableau(const gTableau<T> &T);
    gTableau(gInput &f);
    ~gTableau();

    gTableau<T> &operator=(const gTableau<T> &);
	
    T &Entry(int row, int col);
    T Entry(int row, int col) const;

    void DoPivot(int row, int col);

    void Input(gInput &In_File);

    void Dump(gOutput &Out_File);
};

#endif   // GTABLEAU_H



