// File: efgaccl.h -- gives names to various Extensive form GUI options
// @(#)efgaccl.h	1.7 12/24/95

#ifndef EFGACCL_H
#define EFGACCL_H

#include "accels.h"
#define 	NUM_EFG_EVENTS	41
AccelEvent efg_events[NUM_EFG_EVENTS]=
{
AccelEvent("AddNode",NODE_ADD),
AccelEvent("DeleteNode",NODE_DELETE),
AccelEvent("LabelNode",NODE_LABEL),
AccelEvent("Probs4Node",ACTION_PROBS),
AccelEvent("SetMark",NODE_SET_MARK),
AccelEvent("GoToMark",NODE_GOTO_MARK),
AccelEvent("AddBranch",ACTION_INSERT),
AccelEvent("LabelBranch",ACTION_LABEL),
AccelEvent("LabelTree",TREE_LABEL_LABEL),
AccelEvent("Players",TREE_PLAYERS),
AccelEvent("CopyTree",TREE_COPY),
AccelEvent("PruneTree",TREE_DELETE),
AccelEvent("MoveTree",TREE_MOVE),
AccelEvent("Outcomes",TREE_OUTCOMES),
AccelEvent("JoinIset",INFOSET_JOIN),
AccelEvent("BreakIset",INFOSET_BREAK),
AccelEvent("MergeIset",INFOSET_MERGE),
AccelEvent("SwitchPlayer",INFOSET_SWITCH_PLAYER),
AccelEvent("RevealIset",INFOSET_REVEAL),
AccelEvent("ZoomIn",DISPLAY_INC_ZOOM),
AccelEvent("ZoomOut",DISPLAY_DEC_ZOOM),
AccelEvent("Redraw",DISPLAY_REDRAW),
AccelEvent("HelpAbout",GAMBIT_HELP_ABOUT),
AccelEvent("Help",GAMBIT_HELP_CONTENTS),
AccelEvent("Save",FILE_SAVE),
AccelEvent("Close",FILE_CLOSE),
AccelEvent("Print",FILE_OUTPUT),
AccelEvent("Solve",SOLVE_SOLVE),
AccelEvent("Supports",SOLVE_SUPPORTS),
AccelEvent("MakeNF",SOLVE_SOLVE_NORMAL),
AccelEvent("Inspect Soln",SOLVE_INSPECT),
AccelEvent("Remove Soln",SOLVE_REMOVE),
AccelEvent("Mark Subgames",SUBGAME_SOLVE),
AccelEvent("Unmark All Subgames",SUBGAME_CLEARALL),
AccelEvent("Unmark Node Subgame",SUBGAME_CLEARONE),
AccelEvent("Collapse All Subgames",SUBGAME_COLLAPSEALL),
AccelEvent("Collapse Node Subgame",SUBGAME_COLLAPSEONE),
AccelEvent("Expand All Subgames",SUBGAME_EXPANDALL),
AccelEvent("Expand Node Subgame",SUBGAME_EXPANDONE),
AccelEvent("Toggle Subgame",SUBGAME_TOGGLE),
AccelEvent("Mark Subgame",SUBGAME_SET)
};

#endif
