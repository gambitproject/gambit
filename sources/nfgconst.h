//
// FILE: nfgconst.h -- defines various constants for NfgShow menu events
//
// $Id$
//

#ifndef NFGCONST_H
#define NFGCONST_H

// must make sure that none of the menus are a power of two
#define NFG_FILE_SAVE                   250

#define NFG_EDIT_GAME                   300
#define NFG_EDIT_STRATS                 301
#define NFG_EDIT_PLAYERS                302
#define NFG_EDIT_OUTCOMES               303
#define NFG_EDIT_OUTCOMES_ATTACH        304
#define NFG_EDIT_OUTCOMES_DETACH        305
#define NFG_EDIT_OUTCOMES_LABEL         306
#define NFG_EDIT_OUTCOMES_PAYOFFS       307
#define NFG_EDIT_OUTCOMES_NEW           308
#define NFG_EDIT_OUTCOMES_DELETE        309

#define NFG_SUPPORT_UNDOMINATED         160
#define NFG_SUPPORT_NEW                 161
#define NFG_SUPPORT_EDIT                162
#define NFG_SUPPORT_DELETE              163
#define NFG_SUPPORT_SELECT              164

#define NFG_SOLVE_STANDARD              145
#define NFG_SOLVE_CUSTOM                499
#define NFG_SOLVE_CUSTOM_ENUMPURE       500
#define NFG_SOLVE_CUSTOM_ENUMMIXED      501
#define NFG_SOLVE_CUSTOM_LCP            502
#define NFG_SOLVE_CUSTOM_LP             503
#define NFG_SOLVE_CUSTOM_LIAP           504
#define NFG_SOLVE_CUSTOM_SIMPDIV        505
#define NFG_SOLVE_CUSTOM_POLENUM        506
#define NFG_SOLVE_CUSTOM_QRE            507
#define NFG_SOLVE_CUSTOM_QREGRID        508

#define NFG_VIEW_SOLUTIONS              101
#define NFG_VIEW_DOMINANCE              103
#define NFG_VIEW_PROBABILITIES          104
#define NFG_VIEW_VALUES                 105
#define NFG_VIEW_OUTCOMES               350
#define NFG_VIEW_GAMEINFO               155

#define NFG_PREFS_DISPLAY               204
#define NFG_PREFS_DISPLAY_COLUMNS       205
#define NFG_PREFS_DISPLAY_ROWS          206
#define NFG_PREFS_DISPLAY_DECIMALS      207
#define NFG_PREFS_FONTS                 208
#define NFG_PREFS_FONTS_LABELS          209
#define NFG_PREFS_FONTS_CELLS           210
#define NFG_PREFS_COLORS                200
#define NFG_PREFS_ACCELS                202

#define NFG_ACCL_PAYOFF                 400     // these are not real menus, just accels
#define NFG_ACCL_NEXT_PAYOFF            410

// Help system
#define NFG_GUI_HELP                    "Normal Form GUI"
#define NFG_OUTCOME_HELP                "Outcomes GUI"
#define NFG_FEATURES_HELP               "Normal Form Features"
#define NFG_SOLVE_HELP                  "Normal Form Solutions"
#define NFG_INSPECT_HELP                "Normal Solutions Inspect"
#define NFG_ELIMDOM_INSPECT_HELP        "Elimdom Solutions Inspect"
#define NFG_SOLVE_INSPECT_HELP          "Elimdom Solutions"
#define NFG_SOLVE_INSPECT_OPTIONS_HELP  "Normal Form Inspect Options"
#define NFG_EDIT_HELP                   "Normal Form Edit"
#define NFG_SUPPORTS_HELP               "Examining NF Supports"
#define NFG_MAKE_SUPPORT_HELP           "Creating NF Supports"
#define NFG_STANDARD_HELP               "NFG Standard Solutions"
#define NFG_CUSTOM_HELP                 "NFG Custom Solutions"
#define NFG_SOLN_SORT_HELP              "Sorting and Filtering Solutions"
#define NFG_SOLNSTART_HELP              "NFG Custom Solutions"
#define NFG_DATATYPE_HELP               "Data Types"

// help names for actual solution algorithms
#define ENUMPURE_HELP                   "EnumPure"
#define ENUMMIXED_HELP                  "EnumMixed"
#define LP_HELP                         "LP"
#define LCP_HELP                        "LCP"
#define SIMPDIV_HELP                    "SimpDiv"
#define LIAP_HELP                       "Liap"
#define QRE_HELP                      "Qre"

// Section names for the defaults file
#define MSOLN_SHOW_SECT                 "Mixed-Soln-Show"

#endif // NFGCONST_H

