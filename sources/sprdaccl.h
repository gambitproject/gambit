//
// FILE: sprdaccl.h -- gives names to various SpreadSheet3D events.
//
// $Id$
//

#ifndef SPRDACCL_H
#define SPRDACCL_H

#include "accels.h"
#include "spreadconfig.h"

#define 	NUM_SPREAD_EVENTS	5
AccelEvent spread_events[NUM_SPREAD_EVENTS]=
{
AccelEvent("Print",OUTPUT_MENU),
AccelEvent("Close",CLOSE_MENU),
AccelEvent("Options",OPTIONS_MENU),
AccelEvent("Help",HELP_MENU_CONTENTS),
AccelEvent("HelpAbout",HELP_MENU_ABOUT)
};

#endif
