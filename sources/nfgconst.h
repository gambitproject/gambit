//
// FILE: nfgconst.h -- defines various constants for NormShow menu events
//
// $Id$
//

#ifndef NFGCONST_H
#define NFGCONST_H

// must make sure that none of the menus are a power of two
#define NFG_SOLVE_SOLVE_MENU            100
#define NFG_SOLVE_INSPECT_MENU          101
#define NFG_SOLVE_REMOVE_MENU           103
#define NFG_SOLVE_COMPRESS_MENU         104
#define NFG_SOLVE_SUPPORTS_MENU         105
#define NFG_SOLVE_FEATURES_MENU         110
#define NFG_SOLVE_ALGORITHM_MENU        120
#define NFG_SOLVE_DOMINANCE_MENU        130
#define NFG_SOLVE_SETTINGS_MENU         135
#define NFG_SOLVE_S_ALLNASH_MENU        140
#define NFG_SOLVE_STANDARD_MENU         145
#define NFG_SOLVE_GAMEINFO_MENU         155

#define NFG_PREFS_OUTCOMES_MENU         350

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

#define NFG_DISPLAY_COLORS              200
#define NFG_DISPLAY_ACCELS              202

#define NFG_EDIT_GAME                   300
#define NFG_EDIT_STRATS                 301
#define NFG_EDIT_PLAYERS                302
#define NFG_EDIT_OUTCOMES               303
#define NFG_EDIT_OUTCOMES_ATTACH        304
#define NFG_EDIT_OUTCOMES_DETACH        305
#define NFG_EDIT_OUTCOMES_NAME          306

#define NFG_FILE_SAVE                   250

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

