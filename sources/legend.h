typedef struct {char *l_name;int l_id;} l_struct;
l_struct node_above_src[]={{"Nothing",		NODE_ABOVE_NOTHING},
													 {"Label",			NODE_ABOVE_LABEL},
													 {"Player",			NODE_ABOVE_PLAYER},
													 {"Infoset",		NODE_ABOVE_INFOSET},
													 {"Outcome",		NODE_ABOVE_OUTCOME},
													 {"",						LEGEND_END}};
l_struct node_below_src[]={{"Nothing",		NODE_BELOW_NOTHING},
													 {"Label",			NODE_BELOW_LABEL},
													 {"Player",			NODE_BELOW_PLAYER},
													 {"Infoset",		NODE_BELOW_INFOSET},
                           {"Outcome",		NODE_BELOW_OUTCOME},
													 {"",						LEGEND_END}};
l_struct branch_above_src[]={{"Nothing",	BRANCH_ABOVE_NOTHING},
													 {"Label",			BRANCH_ABOVE_LABEL},
													 {"Player",			BRANCH_ABOVE_PLAYER},
													 {"Probs",			BRANCH_ABOVE_PROBS},
													 {"",						LEGEND_END}};
l_struct branch_below_src[]={{"Nothing",	BRANCH_BELOW_NOTHING},
													 {"Label",			BRANCH_BELOW_LABEL},
													 {"Player",			BRANCH_BELOW_PLAYER},
													 {"Probs",			BRANCH_BELOW_PROBS},
													 {"",						LEGEND_END}};
l_struct node_terminal_src[]={{"Nothing",	NODE_TERMINAL_NOTHING},
													 {"Outcome",		NODE_TERMINAL_OUTCOME},
													 {"Name",				NODE_TERMINAL_NAME},
													 {"",						LEGEND_END}};
l_struct node_right_src[]={{"Nothing",		NODE_RIGHT_NOTHING},
													 {"Outcome",		NODE_RIGHT_OUTCOME},
													 {"Name",				NODE_RIGHT_NAME},
													 {"",						LEGEND_END}};
