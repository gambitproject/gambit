// File: legend.h

// Defines verbal names of items that can be displayed on
// an extensive form tree.  If an item is added, the following should be
// modified: legendc.h, btreewn.cc: ProcessDClick, ProcessRClick, ProcessRDClick,
// RenderLabels, and possibly treedraw.cc: SetLegends

// $Id$

// g++ generates a spurious warning about legends_src[6] being unused;
// I don't know how to get around it.

char *legends_src[] = 
{
    "Above Node",
    "Below Node",
    "Above Branch",
    "Below Branch",
    "At Terminal Node",
    "Right of Node"
};


typedef struct 
{
    char *l_name;
    int   l_id;
} l_struct;


l_struct node_above_src[] = 
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


l_struct node_below_src[] = 
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


l_struct branch_above_src[] = 
{
    {"Nothing",        BRANCH_ABOVE_NOTHING},
    {"Label",          BRANCH_ABOVE_LABEL},
    {"Player",         BRANCH_ABOVE_PLAYER},
    {"Probs",          BRANCH_ABOVE_PROBS},
    {"Value",          BRANCH_ABOVE_VALUE},
    {"",               LEGEND_END}
};


l_struct branch_below_src[] = 
{
    {"Nothing",        BRANCH_BELOW_NOTHING},
    {"Label",          BRANCH_BELOW_LABEL},
    {"Player",         BRANCH_BELOW_PLAYER},
    {"Probs",          BRANCH_BELOW_PROBS},
    {"Value",          BRANCH_BELOW_VALUE},
    {"",               LEGEND_END}
};


l_struct node_terminal_src[] = 
{
    {"Nothing",        NODE_TERMINAL_NOTHING},
    {"Outcome",        NODE_TERMINAL_OUTCOME},
    {"Name",           NODE_TERMINAL_NAME},
    {"",               LEGEND_END}
};


l_struct node_right_src[] = 
{
    {"Nothing",        NODE_RIGHT_NOTHING},
    {"Outcome",        NODE_RIGHT_OUTCOME},
    {"Name",           NODE_RIGHT_NAME},
    {"",               LEGEND_END}
};

