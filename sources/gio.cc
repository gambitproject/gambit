//
// FILE: gio.cc -- Provide global standard stream instances
//
// $Id$
//

#include "gstream.h"

gFileInput _gin(stdin);
gInput &gin = _gin;

gStandardOutput _gout(stdout);
gOutput &gout = _gout;

gStandardOutput _gerr(stderr);
gOutput &gerr = _gerr;


