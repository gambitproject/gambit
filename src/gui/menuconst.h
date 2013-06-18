//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/gui/menuconst.h
// Declaration of constants used for menu items
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef MENUCONST_H
#define MENUCONST_H

typedef enum {
  GBT_MENU_FILE_NEW_EFG = 1098,
  GBT_MENU_FILE_NEW_NFG = 1099,
  GBT_MENU_FILE_EXPORT = 1102,
  GBT_MENU_FILE_EXPORT_BMP = 1103,
  GBT_MENU_FILE_EXPORT_JPEG = 1104,
  GBT_MENU_FILE_EXPORT_PNG = 1105,
  GBT_MENU_FILE_EXPORT_POSTSCRIPT = 1106,
  GBT_MENU_FILE_EXPORT_SVG = 1109,
  GBT_MENU_FILE_EXPORT_EFG = 1107,
  GBT_MENU_FILE_EXPORT_NFG = 1108,

  GBT_MENU_EDIT_INSERT_MOVE = 1200,
  GBT_MENU_EDIT_INSERT_ACTION = 1201,
  GBT_MENU_EDIT_DELETE_TREE = 1202,
  GBT_MENU_EDIT_DELETE_PARENT = 1203,
  GBT_MENU_EDIT_REMOVE_OUTCOME = 1204,
  GBT_MENU_EDIT_REVEAL = 1407,
  GBT_MENU_EDIT_GAME = 1419,
  GBT_MENU_EDIT_NODE = 1420,
  GBT_MENU_EDIT_MOVE = 1421,

  GBT_MENU_EDIT_NEWPLAYER = 1500,

  GBT_MENU_VIEW_PROFILES = 1850,
  GBT_MENU_VIEW_ZOOMIN = 1601,
  GBT_MENU_VIEW_ZOOMOUT = 1602,
  GBT_MENU_VIEW_ZOOM100 = 1900,
  GBT_MENU_VIEW_ZOOMFIT = 1603,
  GBT_MENU_VIEW_STRATEGIC = 1851,

  GBT_MENU_FORMAT_LAYOUT = 1620,
  GBT_MENU_FORMAT_LABELS = 1604,
  GBT_MENU_FORMAT_FONTS = 1612,
  GBT_MENU_FORMAT_DECIMALS_ADD = 1622,
  GBT_MENU_FORMAT_DECIMALS_DELETE = 1623,

  GBT_MENU_TOOLS_DOMINANCE = 1800,
  GBT_MENU_TOOLS_EQUILIBRIUM = 1801,
  GBT_MENU_TOOLS_QRE = 1802
} gbtMenuItems;


#endif // MENUCONST_H









