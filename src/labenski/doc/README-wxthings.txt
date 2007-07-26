readme.txt for wxThings

block.h/cpp - wxBlockInt is a wxGridSelection type class based on
wxRect2DInt to store selected 2-D rectangular areas. It knows how to
combine with other blocks. wxBlockIntSelection consists of an array
of wxBlockInts. It minimizes them if possible so they never overlap
and can sort them from each corner.

range.h/cpp - wxRangeInt min/max class for storing a 1-D range of
values and knows how to combine with other wxRangeInts.
wxRangeIntSelection contains an array of wxRangeInts and minimizes them
if possible, they never overlap, and always stay in order from low to
high. wxRangeDbl and wxRangeDblSelection is the same, but for double
valued ranges.

filebrws.h/cpp & filebrws_icons_wdr.h - wxFileBrowser is an Explorer
type wiget with a tree view of dirs and a list/icon view of files with
optional image preview. Not fully completed, but works. You need wxWidgets
CVS HEAD, >= 2.5.

geometry.h/cpp - wxCircleInt, wxCircleDouble, wxRay2DDouble, wxRay2DInt
wxEllipseInt, wxEllipseDouble, etc... not fully finished.

genergdi.h/cpp - wxGenericColour, wxGenericPen. A color and pen that store
the values themselves and have identical functions as wxColour and wxPen.
They do not require a GUI and are suited to instances where you may want to
store a considerable number of different ones.

optvalue.h/cpp - wxOptionValue class contains a string type and two
arrays of options and values. It works on the same format as the
wxFileConfig class and can load and save to a string/stream.
[opt valuetype]
option1 = value1
option2 = value2

spinctld.h/cpp - wxSpinCtrlDbl is a drop in replacement for a
wxSpinCtrl that uses doubles with extra features such as, ESC sets
'default' value, SPACE sets the control to the last valid value, using
SHIFT,CTRL,ALT work as multipliers for the increment when using the
arrow keys or PgUp/PgDn. It can display in decimal or exponential
format and can 'guess' the number of appropriate digits from the
increment or you can set it yourself.

toggle.h/cpp - wxCustomButton a toggle type button that can act like a
wxButton, wxBitmapButton, wxToggleButton (w/ image or not). It is a
drop in replacement for each of these controls. It can display the icon
along with the text (on any side) and can create a "greyed"
out image to use when the button is disabled. It can also do weird
things like act like a button on a single click and a  toggle on double
click. Note: to differentiate between single and double clicks a timer
is used, if you want it to be just a regular toggle button I suggest
you remove the timer code.

dropdown.h/cpp - DropDownBase a base class for a combobox type widget. It
manages a wxPopupTransientWindow as a child. See wxBitmapComboBox.

bmpcombo.h/cpp - wxBitmapComboBox is a wxComboBox widget for bitmaps. You
Append some bitmaps either individually or with an array. Since bitmaps are
refed this should be a fast process and you don't have to keep them around.
It acts just like a wxComboBox otherwise, sends a EVT_COMBOBOX when selections
are made with either the mouse on the pulldown list or by pressing the up/down
arrows. There is a problem in wxGTK when this is used in a modal dialog, the
mouse events do not go to the popup transient window's child and so you cannot
select items, the keys still work though.

menubtn.h/cpp - wxMenuButton is a button with a label and/or bitmap
and a combobox type dropbown to the right of it. You can attach a menu
to it and the dropdown button shows the menu. If you click on the label
button the next wxITEM_RADIO in the attached menu is selected in a
round robin fashion. If there are no radio items then the label button
does nothing.

medsort.h - Various Public Domain median finding or sorting
macros, a quick select routine, wirth's kth smallest, torben median,
pixel qsort, and a pixel qsort that sorts two arrays together (only one
is used for the comparison, the  other just follows). They can be
DECLARED in headers, DEFINED in you  cpp files, or just used inline.
Since they're macros you can set if they're for arrays of doubles,
ints, chars...
