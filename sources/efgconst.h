//
// FILE: efgconst.h -- constants used in extform gui for each menu item, and
// some extra features.
//
//  $Id$
//

#ifndef EFGCONST_H
#define EFGCONST_H

#define FILE_SAVE                           1020
#define FILE_OUTPUT                         1040
#define FILE_CLOSE                          1050
#define FILE_LOG                            1055

#define EDIT_NODE                           900
#define EDIT_ACTIONS                        910
#define EDIT_INFOSET                        920
#define EDIT_TREE                           930

#define NODE_OUTCOME_OUTCOME                1100
#define NODE_OUTCOME_DIALOG                 1101
#define NODE_INSERT_PLYRNUM                 1102
#define NODE_INSERT_NUMBRCH                 1103
#define NODE_INSERT_DIALOG                  1104
#define NODE_ADD                            1105
#define NODE_GAME                           1107
#define NODE_DELETE                         1108
#define NODE_INSERT                         1109
#define NODE_LABEL                          1110
#define NODE_SET_MARK                       1112
#define NODE_GOTO_MARK                      1113
#define NODE_SUBGAME                        1114

#define ACTION_DELETE                       1205
#define ACTION_INSERT                       1206
#define ACTION_APPEND                       1207
#define ACTION_LABEL                        1208
#define ACTION_PROBS                        1209

#define TREE_LABEL_LABEL                    1300
#define TREE_LABEL_DIALOG                   1301
#define TREE_DELETE                         1302
#define TREE_COPY                           1303
#define TREE_INSERT                         1304
#define TREE_LABEL                          1305
#define TREE_PLAYERS                        1306
#define TREE_MOVE                           1307
#define TREE_SUBGAMES                       1309
#define TREE_INFOSETS                       1310

#define TREE_OUTCOMES                       1320
#define TREE_OUTCOMES_ATTACH                1321
#define TREE_OUTCOMES_DETACH                1322
#define TREE_OUTCOMES_LABEL                 1323
#define TREE_OUTCOMES_PAYOFFS               1324
#define TREE_OUTCOMES_DELETE                1325
#define TREE_OUTCOMES_NEW                   1326

#define SUBGAME_MARKALL                     1350
#define SUBGAME_MARK                        1351
#define SUBGAME_UNMARKALL                   1352
#define SUBGAME_UNMARK                      1354
#define SUBGAME_COLLAPSE                    1356
#define SUBGAME_COLLAPSEALL                 1358
#define SUBGAME_EXPAND                      1360
#define SUBGAME_EXPANDBRANCH                1361
#define SUBGAME_EXPANDALL                   1362

#define INFOSET_MERGE                       1400
#define INFOSET_BREAK                       1401
#define INFOSET_JOIN                        1402
#define INFOSET_LABEL                       1403
#define INFOSET_SWITCH_PLAYER               1405
#define INFOSET_REVEAL                      1407
#define INFOSET_SPLIT                       1410

#define SOLVE_CUSTOM                        1502
#define SOLVE_CUSTOM_EFG                    1503
#define SOLVE_CUSTOM_NFG                    1505
#define SOLVE_CUSTOM_EFG_ENUMPURE           1506
#define SOLVE_CUSTOM_EFG_LCP                1507
#define SOLVE_CUSTOM_EFG_LP                 1508
#define SOLVE_CUSTOM_EFG_LIAP               1509
#define SOLVE_CUSTOM_EFG_POLENUM            1538
#define SOLVE_CUSTOM_EFG_QRE              1510
#define SOLVE_CUSTOM_NFG_ENUMPURE           1511
#define SOLVE_CUSTOM_NFG_ENUMMIXED          1512
#define SOLVE_CUSTOM_NFG_LCP                1513
#define SOLVE_CUSTOM_NFG_LP                 1514
#define SOLVE_CUSTOM_NFG_LIAP               1515
#define SOLVE_CUSTOM_NFG_SIMPDIV            1516
#define SOLVE_CUSTOM_NFG_POLENUM            1539
#define SOLVE_CUSTOM_NFG_QRE              1517
#define SOLVE_CUSTOM_NFG_QREGRID          1518

#define SOLVE_NFG_REDUCED                   1504
#define SOLVE_NFG_AGENT                     1519
#define SOLVE_NFG                           1521

#define SOLVE_STANDARD                      1540

#define DISPLAY_SET_ZOOM                    1600
#define DISPLAY_INC_ZOOM                    1601
#define DISPLAY_DEC_ZOOM                    1602
#define DISPLAY_SET_OPTIONS                 1603
#define DISPLAY_LEGENDS                     1604
#define DISPLAY_COLORS                      1609
#define DISPLAY_ACCELS                      1610
#define DISPLAY_SAVE_DEFAULT                1605
#define DISPLAY_LOAD_DEFAULT                1606
#define DISPLAY_SAVE_CUSTOM                 1607
#define DISPLAY_LOAD_CUSTOM                 1608
#define DISPLAY_REDRAW                      1611

#define SUPPORTS_SUPPORTS                   1800
#define SUPPORTS_ELIMDOM                    1810

#define INSPECT_SOLUTIONS                   1850
#define INSPECT_CURSOR                      1860
#define INSPECT_INFOSETS                    1865
#define INSPECT_GAMEINFO                    1870
#define INSPECT_ZOOM_WIN                    1880


#define GAMBIT_HELP_CONTENTS                1700
#define GAMBIT_HELP_ABOUT                   1701

// Help system
#define EFG_GUI_HELP                    "Extensive Form GUI"
#define EFG_OUTCOME_HELP                "Outcomes GUI"
#define EFG_ACTION_HELP                 "Action Menu"
#define EFG_NODE_HELP                   "Node Menu"
#define EFG_INFOSET_HELP                "Infoset Menu"
#define EFG_TREE_HELP                   "Tree Menu"
#define EFG_INSPECT_HELP                "Extensive Solutions Inspect"
#define EFG_SOLVE_INSPECT_OPTIONS_HELP  "Extensive Form Inspect Options"
#define EFG_SOLVE_OPTIONS_HELP          "Extensive Form Solution Options"
#define EFG_MAKE_SUPPORT_HELP           "Creating EF Supports"
#define EFG_SUPPORTS_HELP               "Examining EF Supports"
#define EFG_STANDARD_HELP               "EFG Standard Solutions"
#define EFG_CUSTOM_HELP                 "EFG Custom Solutions"
#define EFG_SUBGAMESOLN_HELP            "EFG Solutions and Subgames"
#define EFG_SOLN_SORT_HELP              "Sorting and Filtering Solutions"
#define EFG_SOLNSTART_HELP              "EFG Custom Solutions"
#define EFG_DATATYPE_HELP               "Data Types"
#define EFG_GAMEINFO_HELP               "EFG Information"

// sections in the defaults file(s)
#define     SOLN_SECT                   "Soln-Defaults"
#define     BSOLN_SHOW_SECT             "Behav-Soln-Show"

// Different type-dependent values that need to be displayable in the
// tree window.  These are accessed through the BaseTreeWindow->
// TreeWindow->ExtensiveShow.  Also used in NodeInspect window.

typedef enum 
{
    tRealizProb, tIsetProb, tBeliefProb, tNodeValue,
    tIsetValue, tBranchProb, tBranchVal
} TypedSolnValues;


#endif // EFGCONST_H


