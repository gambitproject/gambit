//
// File: flasher.h,flasher.cc: implementation of a flashing node cursor
//
// @(#)flasher.h	1.1 6/6/94

// Note: this may not be the best way of doing this since timers are a
//       limited resource
#include "wx_timer.h"
typedef enum {myFlasher,myCursor} FlashType;
class	TreeNodeCursor
{
private:
	FlashType		__mytype;
	Bool				flashing;
	// since this class makes sense only with interactive (screen) devices,
  // it't ok to init it with physical device coords: _s-start,_e-end
  int	x_s,x_e,y_s,y_e,old_x_s,old_x_e,old_y_s,old_y_e;
	wxDC *dc;
public:
	TreeNodeCursor(wxDC *_dc);
virtual  ~TreeNodeCursor(void);
	void	SetDC(wxDC *_dc) {dc=_dc;}
	void	SetFlashNode(int x_s,int x_e,int y_s,int y_e);
	void	SetFlashing(Bool _flashing) {flashing=_flashing;}
	Bool	Flashing(void) {return flashing;}
	void	Flash(void);		// used to force a flash after moving
	void	SetType(FlashType _t) {__mytype=_t;}
  FlashType Type(void)	{return __mytype;}
};

class TreeNodeFlasher: public wxTimer,public TreeNodeCursor
{
public:
	TreeNodeFlasher(wxDC *_dc);
	~TreeNodeFlasher(void);
	void	Notify(void) {Flash();}
	void	SetFlashing(Bool _flashing);
};
