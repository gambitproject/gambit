#include "wx.h"
#pragma		hdr_stop

#include "grblock.h"
#include "garray.h"
#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gArray<int>;
class gString;                         
class gArray<gString>;
bool operator==(const gArray<int> &, const gArray<int> &);

#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

#include "spread.h"
#include "rational.h"
#include "glist.h"
#include "glist.imp"
TEMPLATE class gNode<int>;
TEMPLATE class gList<int>;
TEMPLATE class gNode<SpreadSheet> ;
TEMPLATE class gList<SpreadSheet> ;


template <class T> class SolnShow;
TEMPLATE class gNode<SolnShow<double> *>;
TEMPLATE class gList<SolnShow<double> *>;
#ifdef GRATIONAL
	TEMPLATE class gNode<SolnShow<gRational> *>;
	TEMPLATE class gList<SolnShow<gRational> *>;
#endif

#include "grarray.imp"
TEMPLATE class gRectArray<SpreadDataCell> ;
#include "grblock.imp"
TEMPLATE class gRectBlock<SpreadDataCell> ;
#include "garray.imp"
TEMPLATE class gArray<SpreadDataCell> ;




                                               
