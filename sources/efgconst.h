//
// FILE: efgconst.h -- constants used in extensive form windows 
//                     for menu items and various features
//
//  $Id$
//

#ifndef EFGCONST_H
#define EFGCONST_H

typedef enum {
  efgmenuFILE_SAVE = 1020, 
  efgmenuFILE_PAGE_SETUP = 1030,
  efgmenuFILE_PRINT_PREVIEW = 1040,
  efgmenuFILE_PRINT = 1045,
  efgmenuFILE_CLOSE = 1050,

  efgmenuEDIT_NODE = 900,
  efgmenuEDIT_NODE_ADD = 1105,
  efgmenuEDIT_NODE_DELETE = 1108,
  efgmenuEDIT_NODE_INSERT = 1109,
  efgmenuEDIT_NODE_LABEL = 1110,
  efgmenuEDIT_NODE_SET_MARK = 1112,
  efgmenuEDIT_NODE_GOTO_MARK = 1113,

  efgmenuEDIT_ACTIONS = 910,
  efgmenuEDIT_ACTION_DELETE = 1205,
  efgmenuEDIT_ACTION_INSERT = 1206,
  efgmenuEDIT_ACTION_APPEND = 1207,
  efgmenuEDIT_ACTION_LABEL = 1208,
  efgmenuEDIT_ACTION_PROBS = 1209,

  efgmenuEDIT_INFOSET = 920,
  efgmenuEDIT_INFOSET_MERGE = 1400,
  efgmenuEDIT_INFOSET_BREAK = 1401,
  efgmenuEDIT_INFOSET_JOIN = 1402,
  efgmenuEDIT_INFOSET_LABEL = 1403,
  efgmenuEDIT_INFOSET_PLAYER = 1405,
  efgmenuEDIT_INFOSET_REVEAL = 1407,
  efgmenuEDIT_INFOSET_SPLIT = 1410,

  efgmenuEDIT_OUTCOMES = 1319,
  efgmenuEDIT_OUTCOMES_ATTACH = 1321,
  efgmenuEDIT_OUTCOMES_DETACH = 1322,
  efgmenuEDIT_OUTCOMES_LABEL = 1323,
  efgmenuEDIT_OUTCOMES_PAYOFFS = 1324,
  efgmenuEDIT_OUTCOMES_DELETE = 1325,
  efgmenuEDIT_OUTCOMES_NEW = 1326,

  efgmenuEDIT_TREE = 930,
  efgmenuEDIT_TREE_DELETE = 1302,
  efgmenuEDIT_TREE_COPY = 1303,
  efgmenuEDIT_TREE_INSERT = 1304,
  efgmenuEDIT_TREE_LABEL = 1305,
  efgmenuEDIT_TREE_PLAYERS = 1306,
  efgmenuEDIT_TREE_MOVE = 1307,
  efgmenuEDIT_TREE_INFOSETS = 1310,

  efgmenuSUBGAME_MARKALL = 1350,
  efgmenuSUBGAME_MARK = 1351,
  efgmenuSUBGAME_UNMARKALL = 1352,
  efgmenuSUBGAME_UNMARK = 1354,
  efgmenuSUBGAME_COLLAPSE = 1356,
  efgmenuSUBGAME_COLLAPSEALL = 1358,
  efgmenuSUBGAME_EXPAND = 1360,
  efgmenuSUBGAME_EXPANDBRANCH = 1361,
  efgmenuSUBGAME_EXPANDALL = 1362,

  efgmenuSUPPORT_UNDOMINATED = 1800,
  efgmenuSUPPORT_NEW = 1801,
  efgmenuSUPPORT_EDIT = 1802,
  efgmenuSUPPORT_DELETE = 1803,
  efgmenuSUPPORT_SELECT = 1810,
  efgmenuSUPPORT_SELECT_FROMLIST = 1811,
  efgmenuSUPPORT_SELECT_PREVIOUS = 1812,
  efgmenuSUPPORT_SELECT_NEXT = 1813,
  efgmenuSUPPORT_REACHABLE = 1814,

  efgmenuSOLVE_STANDARD = 1540,
  efgmenuSOLVE_CUSTOM = 1502,
  efgmenuSOLVE_CUSTOM_EFG = 1503,
  efgmenuSOLVE_CUSTOM_EFG_ENUMPURE = 1506,
  efgmenuSOLVE_CUSTOM_EFG_LCP = 1507,
  efgmenuSOLVE_CUSTOM_EFG_LP = 1508,
  efgmenuSOLVE_CUSTOM_EFG_LIAP = 1509,
  efgmenuSOLVE_CUSTOM_EFG_POLENUM = 1538,
  efgmenuSOLVE_CUSTOM_EFG_QRE = 1510,
  efgmenuSOLVE_CUSTOM_NFG = 1505,
  efgmenuSOLVE_CUSTOM_NFG_ENUMPURE = 1511,
  efgmenuSOLVE_CUSTOM_NFG_ENUMMIXED = 1512,
  efgmenuSOLVE_CUSTOM_NFG_LCP = 1513,
  efgmenuSOLVE_CUSTOM_NFG_LP = 1514,
  efgmenuSOLVE_CUSTOM_NFG_LIAP = 1515,
  efgmenuSOLVE_CUSTOM_NFG_SIMPDIV = 1516,
  efgmenuSOLVE_CUSTOM_NFG_POLENUM = 1539,
  efgmenuSOLVE_CUSTOM_NFG_QRE = 1517,
  efgmenuSOLVE_CUSTOM_NFG_QREGRID = 1518,

  efgmenuSOLVE_NFG = 1521,
  efgmenuSOLVE_NFG_REDUCED = 1504,
  efgmenuSOLVE_NFG_AGENT = 1519,
  
  efgmenuSOLVE_WIZARD = 1525,

  efgmenuINSPECT_SOLUTIONS = 1850,
  efgmenuINSPECT_CURSOR = 1860,
  efgmenuINSPECT_INFOSETS = 1865,
  efgmenuINSPECT_ZOOM_WIN = 1880,
  efgmenuINSPECT_GAMEINFO = 1870,

  efgmenuPREFS_INC_ZOOM = 1601,
  efgmenuPREFS_DEC_ZOOM = 1602,
  efgmenuPREFS_LEGEND = 1604,
  efgmenuPREFS_COLORS = 1609,
  efgmenuPREFS_ACCELS = 1610,
  efgmenuPREFS_SAVE = 1605,
  efgmenuPREFS_LOAD = 1606,
  efgmenuPREFS_REDRAW = 1611,  // accelerator only (?)
  efgmenuPREFS_FONTS = 1612,
  efgmenuPREFS_FONTS_ABOVENODE = 1613,
  efgmenuPREFS_FONTS_BELOWNODE = 1614,
  efgmenuPREFS_FONTS_AFTERNODE = 1615,
  efgmenuPREFS_FONTS_ABOVEBRANCH = 1616,
  efgmenuPREFS_FONTS_BELOWBRANCH = 1617,
  efgmenuPREFS_DISPLAY = 1603,
  efgmenuPREFS_DISPLAY_DECIMALS = 1618,
  efgmenuPREFS_DISPLAY_LAYOUT = 1620,

  // Menu items for profiles window
  efgmenuPROFILES_FILTER = 2000,
  efgmenuPROFILES_NEW = 2001,
  efgmenuPROFILES_CLONE = 2002,
  efgmenuPROFILES_RENAME = 2003,
  efgmenuPROFILES_EDIT = 2004,
  efgmenuPROFILES_DELETE = 2005

} guiEfgMenuItems;


// Different type-dependent values that need to be displayable in the
// tree window.

typedef enum {
  tRealizProb, tIsetProb, tBeliefProb, tNodeValue,
  tIsetValue, tBranchProb, tBranchVal
} TypedSolnValues;


#endif // EFGCONST_H


