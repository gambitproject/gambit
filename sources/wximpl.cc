#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

#include "garray.imp"
class wxChoice;
TEMPLATE class gArray<wxChoice *>;
#include "glist.imp"
class wxFrame;
TEMPLATE class gList<wxFrame *>;
TEMPLATE class gNode<wxFrame *>;








