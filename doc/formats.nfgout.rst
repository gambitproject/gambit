The strategic game (.nfg) file format, outcome version
======================================================

This file format defines a strategic N-player game. In this version,
the payoffs are defined by means of outcomes, which may appear more
than one place in the game table. This may give a more compact means
of representing a game where many different strategy combinations map
to the same consequences for the players. For a version of this format
in which payoffs are listed explicitly, without identification by
outcomes, see the previous section.



A sample file
-------------

This is a sample file illustrating the general format of the file.
This file defines the same game as the example in the previous
section::

    NFG 1 R "Selten (IJGT, 75), Figure 2, normal form" { "Player 1" "Player 2" }

    {
    { "1" "2" "3" }
    { "1" "2" }
    }

    {
    { "" 1, 1 }
    { "" 0, 2 }
    { "" 0, 2 }
    { "" 1, 1 }
    { "" 0, 3 }
    { "" 2, 0 }
    }
    1 2 3 4 5 6




Structure of the prologue
-------------------------

The prologue is constructed as follows. The file begins with the token
NFG , identifying it as a strategic gamefile. Next is the digit 1 ;
this digit is a version number. Since only version 1 files have been
supported for more than a decade, all files have a 1 in this position.
Next comes the letter R . The letter R used to distinguish files which
had rational numbers for numerical data; this distinction is obsolete,
so all new files should have R in this position.

The prologue continues with the title of the game. Following the title
is a list of the names of the players defined in the game. This list
follows the convention found elsewhere in the file of being surrounded
by curly braces and delimited by whitespace (but not commas,
semicolons, or any other character). The order of the players is
significant; the first entry in the list will be numbered as player 1,
the second entry as player 2, and so forth.

Following the list of players is a list of strategies. This is a
nested list; each player's strategies are given as a list of text
labels, surrounded by curly braces.

The nested strategy list is followed by an optional text comment
field.

The prologue closes with a list of outcomes. This is also a nested
list. Each outcome is specified by a text string, followed by a list
of numerical payoffs, one for each player defined. The payoffs may
optionally be separated by commas, as in the example file. The
outcomes are implicitly numbered in the order they appear; the first
outcome is given the number 1, the second 2, and so forth.


Structure of the body (list of outcomes)
----------------------------------------

The body of the file is a list of outcome indices. These are presented
in the same lexicographic order as the payoffs in the payoff file
format; please see the documentation of that format for the
description of the ordering. For each entry in the table, a
nonnegative integer is given, corresponding to the outcome number
assigned as described in the prologue section. The special outcome
number 0 is reserved for the "null" outcome, which is defined as a
payoff of zero to all players. The number of entries in this list,
then, should be the same as the product of the number of strategies
for all players in the game.
