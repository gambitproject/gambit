// File: spreadim.cc -- template instantiations for classes that for some
// reason did not fit anywhere.
// $Id$

#include "wx.h"
#include "wxmisc.h"

#include "grblock.h"
#include "garray.h"

#include "spread.h"
#include "rational.h"
#include "glist.h"
#include "glist.imp"
template class gNode<SpreadSheet> ;
template class gList<SpreadSheet> ;

class NFSupport;
template class gList<NFSupport *>;
template class gNode<NFSupport *>;

class EFSupport;
template class gList<EFSupport *>;
template class gNode<EFSupport *>;


#include "grarray.imp"
template class gRectArray<SpreadDataCell> ;
#include "grblock.imp"
template class gRectBlock<SpreadDataCell> ;
#include "garray.imp"
template class gArray<SpreadDataCell> ;
class wxChoice;
template class gArray<wxChoice *>;




