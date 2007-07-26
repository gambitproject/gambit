README.txt for wxPlotLib

-------------------------------------------------------------------------------

wxPlotLib is 2D plot window widget, data/function curve classes, and some data
processing functions.

It has NOTHING to do with the wxWidgets's wxPlot, except that it was originally
based on it. I doubt that it contains more than a line or two of identical code
anymore. 

-------------------------------------------------------------------------------

Written by John Labenski, except as noted below and in headers
Copyright 2003, John Labenski, except as noted below and in headers

License wxWindows license, except as noted below and in headers.

fourier.h/c
wxPlotLib includes code for FFT from Don Cross (fourier.h/c) that is to the best
of my knowledge from the book "Numerical Recipes in FORTRAN" and is in the 
public domain. 

fparser.h/hh/cpp:
The function parser library was written by 'Warp', the file fparser.txt 
explains the license. Basicly the code is free to use for commercial and 
noncommercial programs, but the original author makes some simple requests 
if used Commercially.
 
lm_lsqr.h/cpp
The Levenberg-Marquart nonlinear least squares 2-D curve fitting class is from 
the public domain Fortran version of Argonne National Laboratories MINPACK, 
written in C by Steve Moshier.

-------------------------------------------------------------------------------
Depends:

wxWidgets - >= 2.5.x, untested in 2.4.x, but would probably work.
wxThings - it uses range, medsort, genergdi. 

-------------------------------------------------------------------------------
Provides:

plotwin.h
wxPlotWin: a 2D plotting widget, mainly for interactive plotting. It does not
generate pie charts or other "fun" plots, just 2D x-y plots. The mouse can
mainpulate the display in real time. It's all about being interactive.

plotcurve.h
wxPlotCurve: a base class for a curve, given an x value return a y. You have to
subclass this to have it do anything.

wxPlotFunction: a function curve, you send in a wxString "2*sin(x)" and then
call GetY(x) to get the value. This is based on the fparser library, please
read src/fparser.txt for details.

wxPlotData: A data curve (like wxImage) that stores x,y and yImag values.
It can do all sorts of stuff, Load/Save, FFT, FFT filter, sort, shift...

lm_lsqr.h
LM_LeastSquare: a Levenberg-Marquart nonlinear least squares 2-D curve fitting
class for use with wxPlotFunction and wxPlotData. From public domain Fortran 
version of Argonne National Laboratories MINPACK, written in C by Steve Moshier
and subsequently turned into a C++ class by John Labenski.

Compilation:

Only a Makefile for *unix using wxGTK currently. It generates a library in the 
wxWidgets directory specified by the output of $wx-config --prefix.

I have a define in plotwin.cpp called wxPLOT_FAST_GRAPHICS that tries to draw 
in GTK and MSW using native methods for higher perfomance. If you don't wish to
use that then just rem out #define wxPLOT_FAST_GRAPHICS and you'll use 
wxWidgets dc rendering code.

If you have problems compiling in GTK take a look at the Makefile, I try to 
guess what version of gtk you're using 1.2x or 2.x, search for GTK_CFLAGS.
