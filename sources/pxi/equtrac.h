//
// File: equtrac.h -- definition of the equilibrium tracker class
//

#ifndef  EQUTRAC_H
#define	 EQUTRAC_H

#include <math.h>

#include "base/gstream.h"
#include "base/gblock.h"
#include "pxifile.h"

#define	LARGE_NUMBER	100.00
#define RECT_SIZE 		0.1

class EquTracker {
private:
  gbtBlock<DataLine> equs;
  double merror; // margin of error (i.e. how close is close enough)
  double Distance(const gbtArray<double> &first, const gbtArray<double> &second);
  int Closest(const DataLine &this_line);
public:
  EquTracker(void);
  int Check_Equ(const DataLine &p,int *new_equ=NULL,DataLine *prev_point=NULL);
  double GetMError(void) {return merror;}
  void SetMError(double m) {merror=m;}
  int Length(void) {return equs.Length();}
};

#endif //  EQUTRAC_H
