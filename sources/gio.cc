//
// FILE: gio.cc -- Provide global standard stream instances
//
// $Id$
//

#include "base/gstream.h"

gStandardInput _gin;
gInput &gin = _gin;

gStandardOutput _gout;
gOutput &gout = _gout;

gStandardOutput _gerr;
gOutput &gerr = _gerr;


