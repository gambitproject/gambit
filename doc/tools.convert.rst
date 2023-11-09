:program:`gambit-convert`: Convert games among various representations
======================================================================

:program:`gambit-convert` reads a game on standard input in any supported format
and converts it to another text representation.  Currently, this tool supports
outputting the strategic form of the game in one of these formats:

* A standard HTML table.
* A LaTeX fragment in the format of Martin Osborne's `sgame` macros
  (see http://www.economics.utoronto.ca/osborne/latex/index.html).


.. program:: gambit-convert

.. cmdoption:: -O FORMAT

   Required.  Specifies the output format.  Supported options for
   `FORMAT` are `html` or `sgame`.

.. cmdoption:: -r PLAYER

   Specifies the player number to place on the rows of the tables.
   The default if not specified is to place player 1 on the rows.

.. cmdoption:: -c PLAYER

   Specifies the player number to place on the columns of the tables.
   The default if not specified is to place player 2 on the columns.

.. cmdoption:: -h

   Prints a help message listing the available options.

.. cmdoption:: -q

   Suppresses printing of the banner at program launch.


Example invocation for HTML output::

   $ gambit-convert -O html 2x2.nfg
   Convert games among various file formats
   Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
   This is free software, distributed under the GNU GPL

   <center><h1>Two person 2 x 2 game with unique mixed equilibrium</h1></center>
   <table><tr><td></td><td align=center><b>1</b></td><td
   align=center><b>2</b></td></tr><tr><td align=center><b>1</b></td><td
   align=center>2,0</td><td align=center>0,1</td></tr><tr><td
   align=center><b>2</b></td><td align=center>0,1</td><td
   align=center>1,0</td></tr></table>


Example invocation for LaTeX output::

   $ gambit-convert -O sgame 2x2.nfg
   Convert games among various file formats
   Gambit version 16.1.0, Copyright (C) 1994-2023, The Gambit Project
   This is free software, distributed under the GNU GPL

   \begin{game}{2}{2}[Player 1][Player 2]
   &1 & 2\\
   1 &  $2,0$  &  $0,1$ \\
   2 &  $0,1$  &  $1,0$
   \end{game}

