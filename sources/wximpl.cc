#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

#include "gblock.h"
#include "gblock.imp"
class wxChoice;
TEMPLATE class gBlock<wxChoice *>;
