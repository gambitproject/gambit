// File: spreadim.cc -- template instantiations for classes that for some
// reason did not fit anywhere.
// $Id$
#include "wx.h"
#include "wxmisc.h"
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
TEMPLATE class gNode<SpreadSheet> ;
TEMPLATE class gList<SpreadSheet> ;

class NFSupport;
TEMPLATE class gList<NFSupport *>;
TEMPLATE class gNode<NFSupport *>;

class EFSupport;
TEMPLATE class gList<EFSupport *>;
TEMPLATE class gNode<EFSupport *>;


#include "grarray.imp"
TEMPLATE class gRectArray<SpreadDataCell> ;
#include "grblock.imp"
TEMPLATE class gRectBlock<SpreadDataCell> ;
#include "garray.imp"
TEMPLATE class gArray<SpreadDataCell> ;
class wxChoice;
TEMPLATE class gArray<wxChoice *>;




