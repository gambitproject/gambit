//
// FILE: twflash.h: implementation of a flashing node cursor
//
// $Id$

// Note: this may not be the best way of doing this since timers are a
//       limited resource

#ifndef TWFLASH_H
#define TWFLASH_H

class TreeRender;

typedef enum {
  myFlasher, myCursor
} FlashType;

// different shapes of cursor.
typedef enum {
  nodeCursor, subgameCursor
} CursorType;


class TreeNodeCursor {
private:
  TreeRender *m_parent;
  FlashType __mytype;
  CursorType cursor_type, old_cursor_type;
  bool flashing;
  int x_s, x_e, y_s, y_e, old_x_s, old_x_e, old_y_s, old_y_e;

  void DrawCursor(wxDC &, int xs, int ys, int xe, int ye, CursorType type);

public:
  TreeNodeCursor(TreeRender *);
  virtual ~TreeNodeCursor();

  void SetFlashNode(int x_s, int y_s, int x_e, int y_e, CursorType   type);
  void GetFlashNode(int & x_s, int & y_s, int & x_e, int & y_e) const;
  virtual void SetFlashing(bool _flashing) { flashing = _flashing; }
  bool Flashing(void) const   { return flashing; }
  void Flash(void);
  void Flash(wxDC &);  // used to force a flash after moving
  void SetType(FlashType _t) { __mytype = _t; }
  FlashType Type(void) const { return __mytype; }
};

class TreeNodeFlasher : public wxTimer, public TreeNodeCursor {
public:
  TreeNodeFlasher(TreeRender *);
  virtual ~TreeNodeFlasher();

  void  Notify(void) { Flash(); }
  void  SetFlashing(bool _flashing);
};

#endif  // TWFLASH_H
