#include "wx.h"
#pragma		hdr_stop


#include "gblock.h"
#include "gtuple.h"
#include "gstring.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gBlock<int>;
class gBlock<gString>;
class gTuple<gString>;
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

#include "spread.h"
#include "glist.h"
#include "glist.imp"
class SpreadSheet;
TEMPLATE class gNode<SpreadSheet> ;
TEMPLATE class gList<SpreadSheet> ;
#include "gblock.h"
#include "gblock.imp"
TEMPLATE class gBlock<SpreadDataCell> ;
TEMPLATE class gBlock<gBlock<SpreadDataCell> >;
TEMPLATE gOutput &operator<<(gOutput &op,const gBlock<SpreadDataCell> &c);

template <class T> class SolnShow;
TEMPLATE class gBlock<SolnShow<double> *>;
#include "rational.h"
TEMPLATE class gBlock<SolnShow<gRational> *>;

#include "gtuplem.imp"
TEMPLATE class gMatrix1<SpreadDataCell> ;






