//
// FILE: grblock.h -- resizable 2-D array class
//
// $Id$
//

#ifndef GRBLOCK_H
#define GRBLOCK_H

#include "base/grarray.h"

template <class T> class gRectBlock: public gRectArray<T>  { 
 public:
  gRectBlock(void);
  gRectBlock(unsigned int nrows, unsigned int ncols);
  gRectBlock(int minr, int maxr, int minc, int maxc);
  gRectBlock(const gRectBlock<T> &);
  virtual ~gRectBlock();

  gRectBlock<T> &operator=(const gRectBlock<T> &);

  void AddRow(const gArray<T> &);
  void RemoveRow(int);
  void InsertRow(int, const gArray<T> &);
 
  void AddColumn(const gArray<T> &);
  void RemoveColumn(int);
  void InsertColumn(int, const gArray<T> &);
};

#endif   // GRBLOCK_H
