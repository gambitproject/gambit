Printing and exporting games
============================

Gambit supports (almost) WYSIWYG (what you see is what you get) output
of both extensive and strategic games, both to a printer and to
several graphical formats. For all of these operations, the game is
drawn exactly as currently displayed on the screen, including whether
the extensive or strategic representation is used, the layout, colors
for players, dominance and probability indicators, and so forth.



Printing a game
---------------

To print the game, press :kbd:`Ctrl`-:kbd:`P`, select
:menuselection:`File --> Print`, or click
the printer icon on the toolbar. The game is scaled so that the
printout fits on one page, while maintaining the same ratio of
horizontal to vertical size; that is, the scaling factor is the same
in both horizontal and vertical dimensions.

Note that especially for extensive games, one dimension of the tree is
much larger than the other. Typically, the extent of the tree
vertically is much greater than its horizontal extent. Because the
printout is scaled to fit on one page, printing such a tree will
generally result in what appears to be a thin line running vertically
down the center of the page. This is in fact the tree, shrunk so the
large vertical dimension fits on the page, meaning that the horizontal
dimension, scaled at the same ratio, becomes very tiny.



Saving to a graphics file
-------------------------

Gambit supports export to five graphical file formats:


+ Windows bitmaps ( .bmp )
+ JPEG, a lossy compressed format ( .jpg , .jpeg )
+ PNG, a lossless compressed format ( .png ); these are similar to
  GIFs
+ Encapsulated PostScript ( .ps )
+ Scalable vector graphics ( .svg )

To export a game to one of these formats, select
:menuselection:`File --> Export`, and
select the corresponding menu entry.

The Windows bitmap and PNG formats are generally recommended for
export, as they both are lossless formats, which will reproduce the
game image exactly as in Gambit. PNG files use a lossless compression
algorithm, so they are typically much smaller than the Windows bitmap
for the same game. Not all image viewing and manipulation tools handle
PNG files; in those cases, use the Windows bitmap output instead. JPEG
files use a compression algorithm that only approximates the original
version, which often makes it ill-suited for use in saving game
images, since it often leads to "blocking" in the image file.

For all three of these formats, the dimensions of the exported graphic
are determined by the dimensions of the game as drawn on screen. Image
export is only supported for games which are less than about 65000
pixels in either the horizontal or vertical dimensions. This is
unlikely to be a practical problem, since such games are so large they
usually cannot be drawn in such a way that a human can make sense of
them.

Encapsulated PostScript output is generally useful for inclusion in
LaTeX and other scientific document preparation systems. This is a
vector-based output, and thus can be rescaled much more effectively
than the other output formats.


