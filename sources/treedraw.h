#ifndef TREEDRAW_H
#define TREEDRAW_H
#include "treecons.h"
class TreeDrawParams   {
friend class DisplayOptionsForm;
	private:
	// These are user-definable
  // Size info for different elements of the tree
		int branch_length, node_length, y_spacing,x_origin, y_origin;
		int outcome_length;
  // Colors
		int chance_color,cursor_color;
  // Labeling info
		Bool show_infosets;
		int	 	node_above_label,node_below_label,branch_above_label,
					branch_below_label,node_terminal_label;
	// Fonts for labels. Note, the system does not make a copy. DO NOT delete the font!
		wxFont *node_above_font,*node_below_font,
					 *branch_above_font,*branch_below_font,*node_terminal_font;
	// Cursor stuff
		Bool	flashing_cursor;
	// Zoom mode;
		float zoom_factor;
  // These are used internally
		int x_scroll,y_scroll;
		typedef struct {int pl;int cl;} player_colors_struct;
		wxList *player_colors;
  public:
    TreeDrawParams(void);

		void 	SetXOrigin(int xo)        { x_origin = xo; }
    int 	XOrigin(void) const        { return x_origin; }

    void 	SetYOrigin(int yo)        { y_origin = yo; }
    int 	YOrigin(void) const        { return y_origin; }

		int 	BranchLength(void) 			   { return branch_length; }
    void 	SetBranchLength(int _l)	 { branch_length=_l;}		
		int 	NodeLength(void) 			     { return node_length; }
    void 	SetNodeLength(int _l)		 { node_length=_l;}
		int 	YSpacing(void) 			       { return y_spacing; }
    void 	SetYSpacing(int _l)			 { y_spacing=_l;}
		int 	CursorColor(void) 			   { return cursor_color; }
    void 	SetCursorColor(int _cl)	 { cursor_color=_cl;}

		void	SetShowInfosets(Bool _s) {show_infosets=_s;}
		int		ShowInfosets(void) {return show_infosets;}
		void	SetOutcomeLength(int _l) {outcome_length=_l;}
		int		OutcomeLength(void) {return outcome_length;}
		void	SetPlayerColor(int pl,int cl);
		int		GetPlayerColor(int pl);

		void	SetLabelNodeAbove(int _l)	{node_above_label=_l;}
		int		LabelNodeAbove(void)			{return node_above_label;}
		void	SetLabelNodeBelow(int _l)	{node_below_label=_l;}
		int		LabelNodeBelow(void)			{return node_below_label;}
		void	SetLabelBranchAbove(int _l)	{branch_above_label=_l;}
		int		LabelBranchAbove(void)			{return branch_above_label;}
		void	SetLabelBranchBelow(int _l)	{branch_below_label=_l;}
		int		LabelBranchBelow(void)			{return branch_below_label;}
		void	SetLabelNodeTerminal(int _l) {node_terminal_label=_l;}
		int		LabelNodeTerminal(void) {return node_terminal_label;}

		void		SetNodeAboveFont(wxFont *_f)
			{if (node_above_font) delete node_above_font;node_above_font=_f;}
		wxFont 	*NodeAboveFont(void) {return node_above_font;}
		void		SetNodeBelowFont(wxFont *_f)
			{if (node_below_font) delete node_below_font;node_below_font=_f;}
		wxFont 	*NodeBelowFont(void) {return node_below_font;}
		void		SetBranchAboveFont(wxFont *_f)
			{if (branch_above_font) delete branch_above_font;branch_above_font=_f;}
		wxFont 	*BranchAboveFont(void) {return branch_above_font;}
		void		SetBranchBelowFont(wxFont *_f)
			{if (branch_below_font) delete branch_below_font;branch_below_font=_f;}
		wxFont 	*BranchBelowFont(void) {return branch_below_font;}
		void		SetNodeTerminalFont(wxFont *_f)
			{if (node_terminal_font) delete node_terminal_font; node_terminal_font=_f;}
		wxFont	*NodeTerminalFont(void) {return node_terminal_font;}

		void		SetFlashingCursor(Bool _f)	{flashing_cursor=_f;}
		Bool		FlashingCursor(void) {return flashing_cursor;}

		void		SetZoom(float _z);
		float   Zoom(void)				{return zoom_factor;}

		// Functions to set the various options
		void	SetOptions(void);
		void	SetLegends(void);
		void	SaveOptions(char *f=NULL);	// pass a filename to read from that file
		void	LoadOptions(char *f=NULL);
		// Functions for internal use by gambit
		void set_x_scroll(int xs) {x_scroll=xs;}
		int get_x_scroll(void) {return x_scroll;}
		void set_y_scroll(int ys) {y_scroll=ys;}
    int get_y_scroll(void) {return y_scroll;}
};
#endif

