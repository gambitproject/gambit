//
// FILE: spreadim.cc -- template instantiations for classes that for some
// reason did not fit anywhere.
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "grblock.h"
#include "garray.h"

#include "spread.h"
#include "rational.h"
#include "glist.h"
#include "glist.imp"

class SpreadSheet;
template class gList<SpreadSheet *> ;

class NFSupport;
template class gList<NFSupport *>;

class EFSupport;
template class gList<EFSupport *>;

#include "grarray.imp"
#include "spreadcanvas.h"
template class gRectArray<SpreadDataCell> ;
#include "grblock.imp"
template class gRectBlock<SpreadDataCell> ;
#include "garray.imp"
template class gArray<SpreadDataCell> ;
class wxChoice;
template class gArray<wxChoice *>;




