//
// FILE: treedraw.h -- Display configuration class for the extensive form
//
// $Id$
//

#ifndef TREEDRAW_H
#define TREEDRAW_H

#include "wx/font.h"
#include "wx/button.h"

#include "base/base.h"
#include "treecons.h"
#include "gambdraw.h"

class TreeDrawSettings : public GambitDrawSettings {
private:
  int branch_length, node_length, fork_length, y_spacing;
  int outcome_length;

  // Colors
  int chance_color, cursor_color;

  // Labeling info
  int show_infosets;
  int node_above_label, node_below_label, branch_above_label;
  int  branch_below_label, node_right_label;
  bool color_coded_outcomes;
  int num_prec;
  
  // Fonts for labels.
  wxFont node_above_font, node_below_font, node_right_font;
  wxFont branch_above_font, branch_below_font;

  // Cursor stuff.
  bool    flashing_cursor;

  // Display only those nodes in the support that are reachable from root.
  bool root_reachable;

public:
  TreeDrawSettings(void);
  
  // Size of the various tree parts.
  int     BranchLength(void) const  { return branch_length; }
  void    SetBranchLength(int l)    { branch_length = l;    }
  int     ForkLength(void) const    { return fork_length;   }
  void    SetForkLength(int l)      { fork_length = l;      }
  int     NodeLength(void) const    { return node_length;   }
  void    SetNodeLength(int l)      { node_length = l;      }
  int     YSpacing(void) const      { return y_spacing;     }
  void    SetYSpacing(int l)        { y_spacing = l;        }

  // Controls the display of infoset lines (turn of to speed up).
  void    SetShowInfosets(int _s)         { show_infosets = _s;          }
  int     ShowInfosets(void) const        { return show_infosets;        }
  void    SetOutcomeLength(int l)         { outcome_length = l;          }
  int     OutcomeLength(void) const       { return outcome_length;       }
  void    SetOutcomeColor(bool c)         { color_coded_outcomes = c;    }
  bool    ColorCodedOutcomes(void) const  { return color_coded_outcomes; }
  int NumDecimals(void) const { return num_prec; }
  void SetNumDecimals(int p_decimals) { num_prec = p_decimals; } 

  // Legends
  void    SetLabelNodeAbove(int l)      { node_above_label = l;       }
  int     LabelNodeAbove(void) const    { return node_above_label;    }
  void    SetLabelNodeBelow(int l)      { node_below_label = l;       }
  int     LabelNodeBelow(void) const    { return node_below_label;    }
  void    SetLabelNodeRight(int l)      { node_right_label = l;       }
  int     LabelNodeRight(void) const    { return node_right_label;    }
  void    SetLabelBranchAbove(int l)    { branch_above_label = l;     }
  int     LabelBranchAbove(void) const  { return branch_above_label;  }
  void    SetLabelBranchBelow(int l)    { branch_below_label = l;     }
  int     LabelBranchBelow(void) const  { return branch_below_label;  }

  // Fonts 
  void SetNodeAboveFont(const wxFont &f)
    { node_above_font = f; }
  const wxFont &NodeAboveFont(void) const { return node_above_font; }
  void SetNodeRightFont(const wxFont &f)
    { node_right_font = f; }
  const wxFont &NodeRightFont(void) const { return node_right_font; }
  void SetNodeBelowFont(const wxFont &f)
    { node_below_font = f; }
  const wxFont &NodeBelowFont(void) const { return node_below_font; }
  void SetBranchAboveFont(const wxFont &f)
    { branch_above_font = f; }
  const wxFont &BranchAboveFont(void) const { return branch_above_font; }
  void SetBranchBelowFont(const wxFont &f)
    { branch_below_font = f; }
  const wxFont &BranchBelowFont(void) const { return branch_below_font; }

  // Control the cursor style [flashing or non-flashing].
  void    SetFlashingCursor(bool f)  { flashing_cursor = f;    }
  bool    FlashingCursor(void) const { return flashing_cursor; }

  // Control the color of the cursor.
  int     CursorColor(void) const { return cursor_color; }
  void    SetCursorColor(int _cl) { cursor_color = _cl;  }

  // Control what nodes are displayed in the support.
  bool    RootReachable(void) const { return root_reachable; }
  void    SetRootReachable(bool r)  { root_reachable = r;    }

  // Functions to set the various options.
  void    SaveOptions(void);
  void    LoadOptions(void);
};

#define SHOWISET_NONE   0
#define SHOWISET_SAME   1
#define SHOWISET_ALL    2

#endif  // TREEDRAW_H
