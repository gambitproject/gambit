//
// FILE: treedraw.h -- Display configuration class for the extensive form
//
// $Id$
//

#ifndef TREEDRAW_H
#define TREEDRAW_H

#include "wx/font.h"
#include "wx/button.h"
#include "wx/config.h"

#include "base/base.h"

class TreeDrawSettings {
private:
  int node_length, m_branchLength, y_spacing;
  int outcome_length;

  // Colors
  int chance_color;

  // Labeling info
  int show_infosets;
  int node_above_label, node_below_label, branch_above_label;
  int  branch_below_label, node_right_label;
  bool color_coded_outcomes;
  int num_prec;
  
  // Fonts for labels.
  wxFont m_nodeAboveFont, m_nodeBelowFont, m_nodeRightFont;
  wxFont m_branchAboveFont, m_branchBelowFont;

  // Display only those nodes in the support that are reachable from root.
  bool root_reachable;

  static void LoadFont(const wxString &, const wxConfig &, wxFont &);
  static void SaveFont(const wxString &, wxConfig &, const wxFont &);
  
public:
  TreeDrawSettings(void);
  
  int BranchLength(void) const { return m_branchLength; }
  void SetBranchLength(int p_length) { m_branchLength = p_length; }
  int     NodeLength(void) const    { return node_length; }
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
  void SetNodeAboveFont(const wxFont &p_font) { m_nodeAboveFont = p_font; }
  const wxFont &NodeAboveFont(void) const { return m_nodeAboveFont; }
  void SetNodeRightFont(const wxFont &p_font) { m_nodeRightFont = p_font; }
  const wxFont &NodeRightFont(void) const { return m_nodeRightFont; }
  void SetNodeBelowFont(const wxFont &p_font) { m_nodeBelowFont = p_font; }
  const wxFont &NodeBelowFont(void) const { return m_nodeBelowFont; }
  void SetBranchAboveFont(const wxFont &p_font) { m_branchAboveFont = p_font; }
  const wxFont &BranchAboveFont(void) const { return m_branchAboveFont; }
  void SetBranchBelowFont(const wxFont &p_font) { m_branchBelowFont = p_font; }
  const wxFont &BranchBelowFont(void) const { return m_branchBelowFont; }

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
