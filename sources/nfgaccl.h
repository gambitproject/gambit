// File: nfgaccl.h -- gives names to various NormShow events
// $Id$

#include "accels.h"
#include "nfgconst.h"

#define 	NUM_NFG_EVENTS	9
AccelEvent nfg_events[NUM_NFG_EVENTS]=
{
AccelEvent("Save",NFG_FILE_SAVE),
AccelEvent("Solve",NFG_SOLVE_SOLVE_MENU),
AccelEvent("Inspect",NFG_SOLVE_INSPECT_MENU),
AccelEvent("Compress",NFG_SOLVE_COMPRESS_MENU),
AccelEvent("EditPayoff",NFG_ACCL_PAYOFF),
AccelEvent("EditNextPayoff",NFG_ACCL_NEXT_PAYOFF),
AccelEvent("RemoveSoln",NFG_SOLVE_REMOVE_MENU),
AccelEvent("SetColors",NFG_DISPLAY_COLORS),
AccelEvent("EditAccels",NFG_DISPLAY_ACCELS)
};
