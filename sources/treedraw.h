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
		int branch_length, node_length, y_spacing,x_origin, y_origin;
		int outcome_length;
	// Total size
  	int max_x,max_y;
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
		int 	BranchLength(void) 			  { return branch_length; }
		void 	SetBranchLength(int l)	 	{ branch_length=l;}
		int 	NodeLength(void) 			    { return node_length; }
		void 	SetNodeLength(int l)		 	{ node_length=l;}
		int 	YSpacing(void) 			      { return y_spacing; }
		void 	SetYSpacing(int l)			 	{ y_spacing=l;}
	// Total size of the tree (calculated in ::Render)
		int		MaxX(void)								{ return max_x;}
		void	SetMaxX(int m)						{ max_x=m;}
		int		MaxY(void)								{ return max_y;}
    void	SetMaxY(int m)						{ max_y=m;}
	// Controls the display of infoset lines (turn of to speed up)
		void	SetShowInfosets(Bool _s) 	{show_infosets=_s;}
		int		ShowInfosets(void) {return show_infosets;}
		void	SetOutcomeLength(int l) 	{outcome_length=l;}
		int		OutcomeLength(void) 			{return outcome_length;}
	// Legends
		void	SetLabelNodeAbove(int l)	{node_above_label=l;}
		int		LabelNodeAbove(void)			{return node_above_label;}
		void	SetLabelNodeBelow(int l)	{node_below_label=l;}
		int		LabelNodeBelow(void)			{return node_below_label;}
		void	SetLabelBranchAbove(int l)	{branch_above_label=l;}
		int		LabelBranchAbove(void)			{return branch_above_label;}
		void	SetLabelBranchBelow(int l)	{branch_below_label=l;}
		int		LabelBranchBelow(void)			{return branch_below_label;}
		void	SetLabelNodeTerminal(int l) {node_terminal_label=l;}
		int		LabelNodeTerminal(void) {return node_terminal_label;}
	// Controls the fonts
		void		SetNodeAboveFont(wxFont *f)
			{if (node_above_font) delete node_above_font;node_above_font=f;}
		wxFont 	*NodeAboveFont(void) {return node_above_font;}
		void		SetNodeBelowFont(wxFont *f)
			{if (node_below_font) delete node_below_font;node_below_font=f;}
		wxFont 	*NodeBelowFont(void) {return node_below_font;}
		void		SetBranchAboveFont(wxFont *f)
			{if (branch_above_font) delete branch_above_font;branch_above_font=f;}
		wxFont 	*BranchAboveFont(void) {return branch_above_font;}
		void		SetBranchBelowFont(wxFont *f)
			{if (branch_below_font) delete branch_below_font;branch_below_font=f;}
		wxFont 	*BranchBelowFont(void) {return branch_below_font;}
		void		SetNodeTerminalFont(wxFont *f)
			{if (node_terminal_font) delete node_terminal_font; node_terminal_font=f;}
		wxFont	*NodeTerminalFont(void) {return node_terminal_font;}
	// Controls the cursor style [flashing or non-flashing]
		void		SetFlashingCursor(Bool f)	{flashing_cursor=f;}
		Bool		FlashingCursor(void) {return flashing_cursor;}
	// Control the color of the cursor
		int 	CursorColor(void) 			  { return cursor_color; }
		void 	SetCursorColor(int _cl)	 	{ cursor_color=_cl;}
	// Set the zoom
		void		SetZoom(float z,bool force=false);
		float   Zoom(void)				{return zoom_factor;}

	// Functions to set the various options
		void	SetOptions(void);
		void	SetLegends(void);
		void	SaveOptions(char *f=0);
		void	LoadOptions(char *f);
	// Functions for internal use by gambit
		void set_x_scroll(int xs) {x_scroll=xs;}
		int get_x_scroll(void) {return x_scroll;}
		void set_y_scroll(int ys) {y_scroll=ys;}
		int get_y_scroll(void) {return y_scroll;}
		int	x_steps(void)	{return xsteps;}
		void set_x_steps(int xs)	{xsteps=xs;}
		int	y_steps(void)	{return ysteps;}
		void	set_y_steps(int ys)	{ysteps=ys;}
};
#endif
