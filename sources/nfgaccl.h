//
// FILE: nfgaccl.h -- gives names to various NfgShow events
//
// $Id$
//

#include "accels.h"
#include "nfgconst.h"

const int NUM_NFG_EVENTS = 33;

AccelEvent nfg_events[NUM_NFG_EVENTS]=
{
  AccelEvent("FileSave", NFG_FILE_SAVE),
  AccelEvent("FilePrint", NFG_FILE_OUTPUT),
  AccelEvent("FileClose", NFG_FILE_CLOSE),
  AccelEvent("EditLabel", NFG_EDIT_LABEL),
  AccelEvent("EditStrategies", NFG_EDIT_STRATS),
  AccelEvent("EditPlayers", NFG_EDIT_PLAYERS),
  AccelEvent("EditOutcomes", NFG_EDIT_OUTCOMES),
  AccelEvent("OutcomeAttach", NFG_EDIT_OUTCOMES_ATTACH),
  AccelEvent("OutcomeDetach", NFG_EDIT_OUTCOMES_DETACH),
  AccelEvent("OutcomePayoffs", NFG_EDIT_OUTCOMES_PAYOFFS),
  AccelEvent("SupportUndominated", NFG_SUPPORT_UNDOMINATED),
  AccelEvent("SupportSelect", NFG_SUPPORT_SELECT),
  AccelEvent("SupportSelectPrevious", NFG_SUPPORT_SELECT_PREVIOUS),
  AccelEvent("SupportSelectNext", NFG_SUPPORT_SELECT_NEXT),
  AccelEvent("SolveStandard", NFG_SOLVE_STANDARD),
  AccelEvent("SolveCustomEnumPure", NFG_SOLVE_CUSTOM_ENUMPURE),
  AccelEvent("SolveCustomEnumMixed", NFG_SOLVE_CUSTOM_ENUMMIXED),
  AccelEvent("SolveCustomLcp", NFG_SOLVE_CUSTOM_LCP),
  AccelEvent("SolveCustomLp", NFG_SOLVE_CUSTOM_LP),
  AccelEvent("SolveCustomLiap", NFG_SOLVE_CUSTOM_LIAP),
  AccelEvent("SolveCustomSimpdiv", NFG_SOLVE_CUSTOM_SIMPDIV),
  AccelEvent("SolveCustomPolEnum", NFG_SOLVE_CUSTOM_POLENUM),
  AccelEvent("SolveCustomQre", NFG_SOLVE_CUSTOM_QRE),
  AccelEvent("SolveCustomQreGrid", NFG_SOLVE_CUSTOM_QREGRID),
  AccelEvent("ViewSolutions", NFG_VIEW_SOLUTIONS),
  AccelEvent("ViewGameInfo", NFG_VIEW_GAMEINFO),
  AccelEvent("ViewOutcomes", NFG_VIEW_OUTCOMES),
  AccelEvent("PrefsColors", NFG_PREFS_COLORS),
  AccelEvent("PrefsAccels", NFG_PREFS_ACCELS),
  AccelEvent("HelpAbout", NFG_HELP_ABOUT),
  AccelEvent("HelpContents", NFG_HELP_CONTENTS),
  AccelEvent("EditPayoff", NFG_ACCL_PAYOFF),
  AccelEvent("EditNextPayoff", NFG_ACCL_NEXT_PAYOFF)
};


