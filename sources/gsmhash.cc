//#
//# FILE: gsmhash.h -- definitions of descendent HashTables used by
//#                    GSM and its accompanying modules
//#                    companion to GSM
//#
//# $Id$
//#




#include "gsmhash.h"


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__



#include "hash.imp"

TEMPLATE class HashTable< gString, Portion* >;
TEMPLATE class HashTable< gString, FuncDescObj* >;




