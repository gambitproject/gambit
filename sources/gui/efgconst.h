//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of constants used for extensive form menu items
//

#ifndef EFGCONST_H
#define EFGCONST_H

typedef enum {
  efgmenuEDIT_INSERT = 1100,
  efgmenuEDIT_DELETE = 1101,
  efgmenuEDIT_REVEAL = 1407,
  efgmenuEDIT_TOGGLE_SUBGAME = 1410,
  efgmenuEDIT_MARK_SUBGAME_TREE = 1411,
  efgmenuEDIT_UNMARK_SUBGAME_TREE = 1412,
  efgmenuEDIT_GAME = 1419,
  efgmenuEDIT_NODE = 1420,
  efgmenuEDIT_MOVE = 1421,

  efgmenuVIEW_PROFILES = 1850,
  efgmenuVIEW_NAVIGATION = 1860,
  efgmenuVIEW_OUTCOMES = 1861,
  efgmenuVIEW_SUPPORTS = 1862,
  efgmenuVIEW_ZOOMIN = 1601,
  efgmenuVIEW_ZOOMOUT = 1602,
  efgmenuVIEW_SUPPORT_REACHABLE = 1814,

  efgmenuFORMAT_LEGEND = 1604,
  efgmenuFORMAT_SAVE = 1605,
  efgmenuFORMAT_LOAD = 1606,
  efgmenuFORMAT_FONTS = 1612,
  efgmenuFORMAT_FONTS_ABOVENODE = 1613,
  efgmenuFORMAT_FONTS_BELOWNODE = 1614,
  efgmenuFORMAT_FONTS_AFTERNODE = 1615,
  efgmenuFORMAT_FONTS_ABOVEBRANCH = 1616,
  efgmenuFORMAT_FONTS_BELOWBRANCH = 1617,
  efgmenuFORMAT_DISPLAY = 1603,
  efgmenuFORMAT_DISPLAY_DECIMALS = 1618,
  efgmenuFORMAT_DISPLAY_LAYOUT = 1620,

  efgmenuTOOLS_DOMINANCE = 1800,
  efgmenuTOOLS_EQUILIBRIUM = 1801,

  efgmenuTOOLS_NFG = 1521,
  efgmenuTOOLS_NFG_REDUCED = 1504,
  efgmenuTOOLS_NFG_AGENT = 1519,

  // Menu items for support window
  efgmenuSUPPORT_DUPLICATE = 2020,
  efgmenuSUPPORT_DELETE = 2021,
  
  // Menu items for profiles window
  efgmenuPROFILES_NEW = 2001,
  efgmenuPROFILES_DUPLICATE = 2002,
  efgmenuPROFILES_DELETE = 2003,
  efgmenuPROFILES_PROPERTIES = 2004
} guiEfgMenuItems;


#endif // EFGCONST_H









