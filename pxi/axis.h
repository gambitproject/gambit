/******************** HEADER FILE FOR AXIS.CC *******************************/
#define	DATA_TYPE_ARITH         0  // these MUST be the same as in normequs.h
#define	DATA_TYPE_LOG           1
#define XOFF                   30
#define GRID_H                  5
#define	XGRIDS                 10
#define YGRIDS                 10

#define DRAW_AXIS       1
#define DRAW_TICKS      2
#define	DRAW_NUMS       4
#define	DRAW_LABELS     8
#define	DRAW_SQUARE    16

// these are used in PlotAxis_3
#define	TAN60   1.732        // tan(60) used as a constant to save time...
#define	F2OR3   1.1547005    // 2/sqrt(3)
#define	F1OR3   0.5773503    // 1/sqrt(3)
#define	PXI_3_HEIGHT  cw*TAN60/2

void PlotAxis_X(wxDC& dc,float x_start,float x_end, float y_start,float y_end,
		  int x0, int y0, int cw,int ch,
		  int plot_type=DATA_TYPE_ARITH,unsigned int features=0,float log_step=0.0);

void PlotAxis_3(wxDC& dc, 
		  int x0, int y0, int cw,int ch, 
		  unsigned int features,wxString labels[]);

void PlotAxis_2(wxDC& dc, float x_start,float x_end,float y_start,float y_end,
		  int x0, int y0, int cw,int ch, 
		  unsigned int features);

