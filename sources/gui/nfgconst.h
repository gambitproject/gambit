//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of menu constants for normal form window
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

#ifndef NFGCONST_H
#define NFGCONST_H

typedef enum {
  GBT_NFG_MENU_FILE_IMPORT = 198,
  GBT_NFG_MENU_FILE_IMPORT_COMLAB = 199,
  GBT_NFG_MENU_FILE_EXPORT = 200,
  GBT_NFG_MENU_FILE_EXPORT_HTML = 201,
  GBT_NFG_MENU_FILE_EXPORT_COMLAB = 202,

  GBT_NFG_MENU_EDIT_STRATS = 301,
  GBT_NFG_MENU_EDIT_CONTINGENCY = 309,
  GBT_NFG_MENU_EDIT_GAME = 310,

  GBT_NFG_MENU_VIEW_PROFILES = 101,
  GBT_NFG_MENU_VIEW_NAVIGATION = 102,
  GBT_NFG_MENU_VIEW_OUTCOMES = 103,
  GBT_NFG_MENU_VIEW_SUPPORTS = 104,
  GBT_NFG_MENU_VIEW_DOMINANCE = 110,
  GBT_NFG_MENU_VIEW_PROBABILITIES = 111,
  GBT_NFG_MENU_VIEW_VALUES = 112,
  GBT_NFG_MENU_VIEW_OUTCOME_LABELS = 120,

  GBT_NFG_MENU_FORMAT_DISPLAY = 204,
  GBT_NFG_MENU_FORMAT_DISPLAY_COLORS = 206,
  GBT_NFG_MENU_FORMAT_DISPLAY_DECIMALS = 207,
  GBT_NFG_MENU_FORMAT_FONT = 212,
  GBT_NFG_MENU_FORMAT_FONT_DATA = 208,
  GBT_NFG_MENU_FORMAT_FONT_LABELS = 203,

  GBT_NFG_MENU_TOOLS_DOMINANCE = 160,
  GBT_NFG_MENU_TOOLS_EQUILIBRIUM = 498,
  GBT_NFG_MENU_TOOLS_QRE = 499,
  GBT_NFG_MENU_TOOLS_CH = 497,

  GBT_NFG_MENU_SUPPORT_DUPLICATE = 710,
  GBT_NFG_MENU_SUPPORT_DELETE = 711,

  GBT_NFG_MENU_PROFILES_NEW = 701,
  GBT_NFG_MENU_PROFILES_DUPLICATE = 702,
  GBT_NFG_MENU_PROFILES_DELETE = 703,
  GBT_NFG_MENU_PROFILES_PROPERTIES = 704,
  GBT_NFG_MENU_PROFILES_REPORT = 705
};

#endif // NFGCONST_H

