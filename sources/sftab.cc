//
// FILE: sftab.cc -- Sequence form tableau
//
// $Id$
//

#include "sftab.imp"

#ifdef __GNUG__
template int NumSequences(int, const ExtForm<double> &);
template int NumSequences(int, const ExtForm<gRational> &);
template int NumInfosets(int, const ExtForm<double> &);
template int NumInfosets(int, const ExtForm<gRational> &);
template class SFTableau<double>;
template class SFTableau<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
int NumSequences(int, const ExtForm<double> &);
int NumSequences(int, const ExtForm<gRational> &);
int NumInfosets(int, const ExtForm<double> &);
int NumInfosets(int, const ExtForm<gRational> &);
class SFTableau<double>;
class SFTableau<gRational>;
#pragma option -Jgx
#endif   // __GNUG__, __BORLANDC__





