// File: efgaccl.h -- gives names to various Extensive form GUI options
// $Id$

#ifndef EFGACCL_H
#define EFGACCL_H

#include "accels.h"
#define 	NUM_EFG_EVENTS	31
AccelEvent efg_events[NUM_EFG_EVENTS]=
{
AccelEvent("AddNode",NODE_ADD),
AccelEvent("DeleteNode",NODE_DELETE),
AccelEvent("LabelNode",NODE_LABEL),
AccelEvent("Probs4Node",NODE_PROBS),
AccelEvent("SetMark",NODE_SET_MARK),
AccelEvent("GoToMark",NODE_GOTO_MARK),
AccelEvent("SwitchPlayerN",NODE_SWITCH_PLAYER),
AccelEvent("AddBranch",BRANCH_INSERT),
AccelEvent("LabelBranch",BRANCH_LABEL),
AccelEvent("LabelTree",TREE_LABEL_LABEL),
AccelEvent("Players",TREE_PLAYERS),
AccelEvent("CopyTree",TREE_COPY),
AccelEvent("PruneTree",TREE_DELETE),
AccelEvent("MoveTree",TREE_MOVE),
AccelEvent("Outcomes",TREE_OUTCOMES),
AccelEvent("JoinIset",INFOSET_JOIN),
AccelEvent("BreakIset",INFOSET_BREAK),
AccelEvent("MergeIset",INFOSET_MERGE),
AccelEvent("SwitchPlayerI",INFOSET_SWITCH_PLAYER),
AccelEvent("ZoomIn",DISPLAY_INC_ZOOM),
AccelEvent("ZoomOut",DISPLAY_DEC_ZOOM),
AccelEvent("Redraw",DISPLAY_REDRAW),
AccelEvent("HelpAbout",HELP_ABOUT),
AccelEvent("Help",HELP_GAMBIT),
AccelEvent("Save",FILE_SAVE),
AccelEvent("Close",FILE_CLOSE),
AccelEvent("Print",FILE_OUTPUT),
AccelEvent("Solve",SOLVE_SOLVE),
AccelEvent("MakeNF",SOLVE_SOLVE_NORMAL),
AccelEvent("Inspect Soln",SOLVE_INSPECT),
AccelEvent("Remove Soln",SOLVE_REMOVE)
};

#endif
