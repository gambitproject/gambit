//
// FILE: legend.h -- Defines verbal names of items that can be displayed on
//                   an extensive form tree.
// $Id$
//

#ifndef LEGEND_H
#define LEGEND_H

#include "legendc.h"

typedef struct {
  char *l_name;
  int   l_id;
} l_struct;

const l_struct node_above_src[] = 
{
  {"Nothing",        NODE_ABOVE_NOTHING},
  {"Node Label",     NODE_ABOVE_LABEL},
  {"Player",         NODE_ABOVE_PLAYER},
  {"Infoset Label",  NODE_ABOVE_ISETLABEL},
  {"Infoset ID",     NODE_ABOVE_ISETID},
  {"Outcome",        NODE_ABOVE_OUTCOME},
  {"Realiz Prob",    NODE_ABOVE_REALIZPROB},
  {"Belief Prob",    NODE_ABOVE_BELIEFPROB},
  {"Value",          NODE_ABOVE_VALUE},
  {"",               LEGEND_END}
};

const l_struct node_below_src[] = 
{
  {"Nothing",        NODE_BELOW_NOTHING},
  {"Node Label",     NODE_BELOW_LABEL},
  {"Player",         NODE_BELOW_PLAYER},
  {"Infoset Label",  NODE_BELOW_ISETLABEL},
  {"Infoset ID",     NODE_BELOW_ISETID},
  {"Outcome",        NODE_BELOW_OUTCOME},
  {"Realiz Prob",    NODE_BELOW_REALIZPROB},
  {"Belief Prob",    NODE_BELOW_BELIEFPROB},
  {"Value",          NODE_BELOW_VALUE},
  {"",               LEGEND_END}
};

const l_struct branch_above_src[] = 
{
  {"Nothing",        BRANCH_ABOVE_NOTHING},
  {"Label",          BRANCH_ABOVE_LABEL},
  {"Player",         BRANCH_ABOVE_PLAYER},
  {"Probs",          BRANCH_ABOVE_PROBS},
  {"Value",          BRANCH_ABOVE_VALUE},
  {"",               LEGEND_END}
};

const l_struct branch_below_src[] = 
{
  {"Nothing",        BRANCH_BELOW_NOTHING},
  {"Label",          BRANCH_BELOW_LABEL},
  {"Player",         BRANCH_BELOW_PLAYER},
  {"Probs",          BRANCH_BELOW_PROBS},
  {"Value",          BRANCH_BELOW_VALUE},
  {"",               LEGEND_END}
};

const l_struct node_right_src[] = 
{
  {"Nothing",        NODE_RIGHT_NOTHING},
  {"Outcome",        NODE_RIGHT_OUTCOME},
  {"Name",           NODE_RIGHT_NAME},
  {"",               LEGEND_END}
};

#endif  // LEGEND_H
