//
// FILE: nfgaccl.h -- gives names to various NfgShow events
//
// $Id$
//

#include "accels.h"
#include "nfgconst.h"

#define 	NUM_NFG_EVENTS	33
AccelEvent nfg_events[NUM_NFG_EVENTS]=
{
AccelEvent("FileSave",NFG_FILE_SAVE),
AccelEvent("FileOutput",OUTPUT_MENU),
AccelEvent("FileClose",CLOSE_MENU),
AccelEvent("EditGame",NFG_EDIT_GAME),
AccelEvent("EditStrats",NFG_EDIT_STRATS),
AccelEvent("EditPlayers",NFG_EDIT_PLAYERS),
AccelEvent("EditOutcomes",NFG_EDIT_OUTCOMES),
AccelEvent("OutcomeAttach",NFG_EDIT_OUTCOMES_ATTACH),
AccelEvent("OutcomeDetach",NFG_EDIT_OUTCOMES_DETACH),
AccelEvent("OutcomeLabel",NFG_EDIT_OUTCOMES_LABEL),
AccelEvent("OutcomePayoffs",NFG_EDIT_OUTCOMES_PAYOFFS),
AccelEvent("SupportsElimDom",NFG_SUPPORT_UNDOMINATED),
AccelEvent("SupportsSelect",NFG_SUPPORT_SELECT),
AccelEvent("SolveStandard",NFG_SOLVE_STANDARD),
AccelEvent("SolveCustom",NFG_SOLVE_CUSTOM),
AccelEvent("EnumPure",NFG_SOLVE_CUSTOM_ENUMPURE),
AccelEvent("EnumMixed",NFG_SOLVE_CUSTOM_ENUMMIXED),
AccelEvent("LCP",NFG_SOLVE_CUSTOM_LCP),
AccelEvent("LP",NFG_SOLVE_CUSTOM_LP),
AccelEvent("Liap",NFG_SOLVE_CUSTOM_LIAP),
AccelEvent("Simpdiv",NFG_SOLVE_CUSTOM_SIMPDIV),
AccelEvent("PolEnum",NFG_SOLVE_CUSTOM_POLENUM),
AccelEvent("QRE",NFG_SOLVE_CUSTOM_QRE),
AccelEvent("QREGrid",NFG_SOLVE_CUSTOM_QREGRID),
AccelEvent("ViewSolutions",NFG_VIEW_SOLUTIONS),
AccelEvent("ViewGameInfo",NFG_VIEW_GAMEINFO),
AccelEvent("ViewOutcomes",NFG_VIEW_OUTCOMES),
AccelEvent("PrefsColors",NFG_PREFS_COLORS),
AccelEvent("PrefsAccels",NFG_PREFS_ACCELS),
AccelEvent("HelpAbout",HELP_MENU_ABOUT),
AccelEvent("HelpContents",HELP_MENU_CONTENTS),
AccelEvent("EditPayoff",NFG_ACCL_PAYOFF),
AccelEvent("EditNextPayoff",NFG_ACCL_NEXT_PAYOFF),
};


