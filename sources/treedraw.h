// File: treedraw.h -- contains the definition of the configuration class 
// for the extensive form
// @(#)treedraw.h	1.7 7/18/95
#ifndef TREEDRAW_H
#define TREEDRAW_H
#include "gblock.h"
#include "treecons.h"
#include "gambdraw.h"

class TreeDrawSettings:public GambitDrawSettings
{
	private:
	// These are user-definable
	// Size info for different elements of the tree
		int branch_length, node_length, fork_length, y_spacing,x_origin, y_origin;
		int outcome_length;
	// Total size
  	int max_x,max_y;
  // Colors
		int chance_color,cursor_color;
  // Labeling info
		Bool show_infosets;
		int	 	node_above_label,node_below_label,branch_above_label,
					branch_below_label,node_terminal_label,node_right_label;
		Bool color_coded_outcomes;
	// Fonts for labels. Note, the system does not make a copy. DO NOT delete the font!
		wxFont *node_above_font,*node_below_font,*node_right_font,
					 *branch_above_font,*branch_below_font,*node_terminal_font;
	// Cursor stuff
		Bool	flashing_cursor;
	// Zoom mode;
		float zoom_factor;
	// These are used internally
		int x_scroll,y_scroll,xsteps,ysteps;
	// Private functions
		typedef struct {int what_font;TreeDrawSettings *draw_settings;} draw_params_legend_struct;
		static void draw_params_legends_func(wxButton &ob,wxCommandEvent &ev);
	public:
	// Constructor
		TreeDrawSettings(void);
	// Control the coordinates of upper left corner
		void 	SetXOrigin(int xo)        { x_origin = xo; }
		int 	XOrigin(void) const       { return x_origin; }
		void 	SetYOrigin(int yo)        { y_origin = yo; }
		int 	YOrigin(void) const       { return y_origin; }
	// Size of the various tree parts
		int 	BranchLength(void) const  { return branch_length; }
		void 	SetBranchLength(int l)	 	{ branch_length=l;}
		int		ForkLength(void) const		{ return fork_length;}
		void	SetForkLength(int l)			{ fork_length=l;}
		int 	NodeLength(void) const    { return node_length; }
		void 	SetNodeLength(int l)		 	{ node_length=l;}
		int 	YSpacing(void) const      { return y_spacing; }
		void 	SetYSpacing(int l)			 	{ y_spacing=l;}
	// Total size of the tree (calculated in ::Render)
		int		MaxX(void) const					{ return max_x;}
		void	SetMaxX(int m)						{ max_x=m;}
		int		MaxY(void) const					{ return max_y;}
		void	SetMaxY(int m)						{ max_y=m;}
	// Controls the display of infoset lines (turn of to speed up)
		void	SetShowInfosets(Bool _s) 	{show_infosets=_s;}
		int		ShowInfosets(void) const  {return show_infosets;}
		void	SetOutcomeLength(int l) 	{outcome_length=l;}
		int		OutcomeLength(void) const	{return outcome_length;}
		void	SetOutcomeColor(Bool c)		{color_coded_outcomes=c;}
		Bool	ColorCodedOutcomes(void) const	{return color_coded_outcomes;}
	// Legends
		void	SetLabelNodeAbove(int l)	{node_above_label=l;}
		int		LabelNodeAbove(void) const{return node_above_label;}
		void	SetLabelNodeBelow(int l)	{node_below_label=l;}
		int		LabelNodeBelow(void) const{return node_below_label;}
		void	SetLabelNodeRight(int l)	{node_right_label=l;}
		int		LabelNodeRight(void) const{return node_right_label;}
		void	SetLabelBranchAbove(int l)	{branch_above_label=l;}
		int		LabelBranchAbove(void) const{return branch_above_label;}
		void	SetLabelBranchBelow(int l)	{branch_below_label=l;}
		int		LabelBranchBelow(void) const{return branch_below_label;}
		void	SetLabelNodeTerminal(int l) {node_terminal_label=l;}
		int		LabelNodeTerminal(void) const{return node_terminal_label;}
	// Controls the fonts
		void		SetNodeAboveFont(wxFont *f)
			{if (node_above_font) delete node_above_font;node_above_font=f;}
		wxFont 	*NodeAboveFont(void) const {return node_above_font;}
		void		SetNodeRightFont(wxFont *f)
			{if (node_right_font) delete node_right_font;node_right_font=f;}
		wxFont 	*NodeRightFont(void) const {return node_right_font;}
		void		SetNodeBelowFont(wxFont *f)
			{if (node_below_font) delete node_below_font;node_below_font=f;}
		wxFont 	*NodeBelowFont(void) const {return node_below_font;}
		void		SetBranchAboveFont(wxFont *f)
			{if (branch_above_font) delete branch_above_font;branch_above_font=f;}
		wxFont 	*BranchAboveFont(void) const {return branch_above_font;}
		void		SetBranchBelowFont(wxFont *f)
			{if (branch_below_font) delete branch_below_font;branch_below_font=f;}
		wxFont 	*BranchBelowFont(void) const {return branch_below_font;}
		void		SetNodeTerminalFont(wxFont *f)
			{if (node_terminal_font) delete node_terminal_font; node_terminal_font=f;}
		wxFont	*NodeTerminalFont(void) const {return node_terminal_font;}
	// Controls the cursor style [flashing or non-flashing]
		void		SetFlashingCursor(Bool f)	{flashing_cursor=f;}
		Bool		FlashingCursor(void) const {return flashing_cursor;}
	// Control the color of the cursor
		int 	CursorColor(void) const	  { return cursor_color; }
		void 	SetCursorColor(int _cl)	 	{ cursor_color=_cl;}
	// Set the zoom
		void		SetZoom(float z,bool force=false);
		float   Zoom(void) const		{return zoom_factor;}

	// Functions to set the various options
		void	SetOptions(void);
		void	SetLegends(void);
		void	SaveOptions(char *f=0);
		void	LoadOptions(char *f);
	// Functions for internal use by gambit
		void set_x_scroll(int xs) {x_scroll=xs;}
		int get_x_scroll(void) const {return x_scroll;}
		void set_y_scroll(int ys) {y_scroll=ys;}
		int get_y_scroll(void) const {return y_scroll;}
		int	x_steps(void)	const {return xsteps;}
		void set_x_steps(int xs)	{xsteps=xs;}
		int	y_steps(void)	const {return ysteps;}
		void	set_y_steps(int ys)	{ysteps=ys;}
};
#endif
