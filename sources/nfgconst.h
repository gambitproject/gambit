// File: nfgconst.h -- defines various constants for NormShow menu events
// @(#)nfgconst.h	1.1 7/4/95

#ifndef NFGCONST_H
#define NFGCONST_H
// must make sure it none of the menus are a power of two
#define NFG_SOLVE_SOLVE_MENU				100
#define NFG_SOLVE_INSPECT_MENU			101
#define NFG_SOLVE_PAYOFF_MENU				102 // this is not a real menu=DoubleClick
#define	NFG_SOLVE_REMOVE_MENU				103
#define NFG_SOLVE_COMPRESS_MENU			104
#define NFG_SOLVE_SUPPORTS_MENU			105

#define NFG_DISPLAY_COLORS					200
#define NFG_DISPLAY_OPTIONS					201
#define NFG_DISPLAY_ACCELS					202

#define NFG_LABEL_GAME							300
#define NFG_LABEL_STRATS						301
#define NFG_LABEL_PLAYERS						302

#define NFG_FILE_SAVE								250

// Help system
#define NFG_GUI_HELP							"Normal Form GUI"
#define NFG_FEATURES_HELP					"Normal Form Features"
#define NFG_SOLVE_HELP						"Normal Form Solutions"
#define NFG_INSPECT_HELP					"Normal Solutions Inspect"
#define NFG_ELIMDOM_INSPECT_HELP	"Elimdom Solutions Inspect"
#define NFG_SOLVE_INSPECT_HELP		"Elimdom Solutions"
#define NFG_SOLVE_INSPECT_OPTIONS_HELP	"Normal Form Inspect Options"
#define NFG_LABEL_HELP						"Normal Form Labels"
#define NFG_ELIMDOM_HELP		"Elimination of Dominated Strategies"
#define NFG_SUPPORTS_HELP		"Examining NF Supports"
#define NFG_MAKE_SUPPORT_HELP	"Creating NF Supports"

// help names for actual solution algorithms
#define ENUMPURE_HELP						"EnumPure"
#define ENUMMIXED_HELP          "EnumMixed"
#define LP_HELP									"LP"
#define LCP_HELP								"LCP"
#define SIMPDIV_HELP						"SimpDiv"
#define LIAP_HELP								"Liap"
#define GOBIT_HELP							"Gobit"
#endif
