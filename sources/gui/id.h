//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of constants used for menu items and so forth
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef ID_H
#define ID_H


//
// The following anonymous enum lists all menu item identifiers.
// When new ones are added, each group should be kept sorted alphabetically,
// and numbers reissued to remain consecutive.
// The identifiers may have slightly different interpretations in
// different contexts, depending on the event handler(s) in use for the
// window.
//
typedef enum {
  GBT_MENU_FILE_EXPORT = 100,
  GBT_MENU_FILE_EXPORT_BMP = 101,
  GBT_MENU_FILE_EXPORT_COMLAB = 102,
  GBT_MENU_FILE_EXPORT_HTML = 103,
  GBT_MENU_FILE_EXPORT_JPEG = 104,
  GBT_MENU_FILE_EXPORT_PNG = 105,
  GBT_MENU_FILE_EXPORT_POSTSCRIPT = 106,

  GBT_MENU_EDIT_CONTINGENCY = 200,
  GBT_MENU_EDIT_DELETE = 201,
  GBT_MENU_EDIT_GAME = 202,
  GBT_MENU_EDIT_INSERT = 203,
  GBT_MENU_EDIT_MARK_SUBGAME_TREE = 204,
  GBT_MENU_EDIT_MOVE = 205,
  GBT_MENU_EDIT_NODE = 206,
  GBT_MENU_EDIT_REVEAL = 207,
  GBT_MENU_EDIT_STRATS = 208,
  GBT_MENU_EDIT_TOGGLE_SUBGAME = 209,
  GBT_MENU_EDIT_UNMARK_SUBGAME_TREE = 210,

  GBT_MENU_VIEW_DOMINANCE = 300,
  GBT_MENU_VIEW_NAVIGATION = 301,
  GBT_MENU_VIEW_NFG_REDUCED = 302,
  GBT_MENU_VIEW_OUTCOMES = 303,
  GBT_MENU_VIEW_OUTCOME_LABELS = 304,
  GBT_MENU_VIEW_PROBABILITIES = 305,
  GBT_MENU_VIEW_PROFILES = 306,
  GBT_MENU_VIEW_SUPPORTS = 307,
  GBT_MENU_VIEW_SUPPORT_REACHABLE = 308,
  GBT_MENU_VIEW_VALUES = 309,
  GBT_MENU_VIEW_ZOOMIN = 310,
  GBT_MENU_VIEW_ZOOMOUT = 311,

  GBT_MENU_FORMAT_DISPLAY = 400,
  GBT_MENU_FORMAT_DISPLAY_COLORS = 401,
  GBT_MENU_FORMAT_DISPLAY_DECIMALS = 402,
  GBT_MENU_FORMAT_DISPLAY_LAYOUT = 403,
  GBT_MENU_FORMAT_DISPLAY_LEGEND = 404, 
  GBT_MENU_FORMAT_FONTS = 405,
  GBT_MENU_FORMAT_FONTS_ABOVEBRANCH = 406,
  GBT_MENU_FORMAT_FONTS_ABOVENODE = 407,
  GBT_MENU_FORMAT_FONTS_AFTERNODE = 408,
  GBT_MENU_FORMAT_FONTS_BELOWBRANCH = 409,
  GBT_MENU_FORMAT_FONTS_BELOWNODE = 410,
  GBT_MENU_FORMAT_FONTS_DATA = 411,
  GBT_MENU_FORMAT_FONTS_LABELS = 412,
  GBT_MENU_FORMAT_AUTOSIZE = 413,

  GBT_MENU_TOOLS_CH = 500,
  GBT_MENU_TOOLS_DOMINANCE = 501,
  GBT_MENU_TOOLS_EQUILIBRIUM = 502,
  GBT_MENU_TOOLS_QRE = 506,

  GBT_MENU_PROFILES_BEHAV = 600,
  GBT_MENU_PROFILES_DELETE = 601,
  GBT_MENU_PROFILES_DUPLICATE = 602,
  GBT_MENU_PROFILES_GRID = 603,
  GBT_MENU_PROFILES_INFO = 604,
  GBT_MENU_PROFILES_MIXED = 605,
  GBT_MENU_PROFILES_MYERSON = 606,
  GBT_MENU_PROFILES_NEW = 607,
  GBT_MENU_PROFILES_PROPERTIES = 608,
  GBT_MENU_PROFILES_REPORT = 609,
  GBT_MENU_PROFILES_VECTOR = 610,

  GBT_MENU_SUPPORTS_DELETE = 700,
  GBT_MENU_SUPPORTS_DUPLICATE = 701,
  
  GBT_MENU_OUTCOMES_ATTACH = 800,
  GBT_MENU_OUTCOMES_DELETE = 801,
  GBT_MENU_OUTCOMES_DETACH = 802,
  GBT_MENU_OUTCOMES_NEW = 803,

  GBT_MENU_QRE_EXPORT_PXI = 900,
  GBT_MENU_QRE_PLOT = 901
} gbtMenuIds;

#endif // ID_H









