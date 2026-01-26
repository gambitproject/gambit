.. _file-formats-efg:

The extensive game (.efg) file format
=====================================

The extensive game (.efg) file format has been used by Gambit, with
minor variations, to represent extensive games since circa 1994. It
replaced an earlier format, which had no particular name but which had
the conventional extension .dt1.


A sample file
-------------

This is a sample file illustrating the general format of the file::

    EFG 2 R "General Bayes game, one stage" { "Player 1" "Player 2" }
    c "ROOT" 1 "(0,1)" { "1G" 0.500000 "1B" 0.500000 } 0
    c "" 2 "(0,2)" { "2g" 0.500000 "2b" 0.500000 } 0
    p "" 1 1 "(1,1)" { "H" "L" } 0
    p "" 2 1 "(2,1)" { "h" "l" } 0
    t "" 1 "Outcome 1" { 10.000000 2.000000 }
    t "" 2 "Outcome 2" { 0.000000 10.000000 }
    p "" 2 1 "(2,1)" { "h" "l" } 0
    t "" 3 "Outcome 3" { 2.000000 4.000000 }
    t "" 4 "Outcome 4" { 4.000000 0.000000 }
    p "" 1 1 "(1,1)" { "H" "L" } 0
    p "" 2 2 "(2,2)" { "h" "l" } 0
    t "" 5 "Outcome 5" { 10.000000 2.000000 }
    t "" 6 "Outcome 6" { 0.000000 10.000000 }
    p "" 2 2 "(2,2)" { "h" "l" } 0
    t "" 7 "Outcome 7" { 2.000000 4.000000 }
    t "" 8 "Outcome 8" { 4.000000 0.000000 }
    c "" 3 "(0,3)" { "2g" 0.500000 "2b" 0.500000 } 0
    p "" 1 2 "(1,2)" { "H" "L" } 0
    p "" 2 1 "(2,1)" { "h" "l" } 0
    t "" 9 "Outcome 9" { 4.000000 2.000000 }
    t "" 10 "Outcome 10" { 2.000000 10.000000 }
    p "" 2 1 "(2,1)" { "h" "l" } 0
    t "" 11 "Outcome 11" { 0.000000 4.000000 }
    t "" 12 "Outcome 12" { 10.000000 2.000000 }
    p "" 1 2 "(1,2)" { "H" "L" } 0
    p "" 2 2 "(2,2)" { "h" "l" } 0
    t "" 13 "Outcome 13" { 4.000000 2.000000 }
    t "" 14 "Outcome 14" { 2.000000 10.000000 }
    p "" 2 2 "(2,2)" { "h" "l" } 0
    t "" 15 "Outcome 15" { 0.000000 4.000000 }
    t "" 16 "Outcome 16" { 10.000000 0.000000 }


Structure of the prologue
-------------------------

The extensive gamefile consists of two parts: the prologue, or header,
and the list of nodes, or body. In the example file, the prologue is
the first line. (Again, this is just a consequence of the formatting
we have chosen and is not a requirement of the file structure itself.)

The prologue is constructed as follows. The file begins with the token
EFG , identifying it as an extensive gamefile. Next is the digit 2 ;
this digit is a version number. Since only version 2 files have been
supported for more than a decade, all files have a 2 in this position.
Next comes the letter R . The letter R used to distinguish files which
had rational numbers for numerical data; this distinction is obsolete,
so all new files should have R in this position.

The prologue continues with the title of the game. Following the title
is a list of the names of the players defined in the game. This list
follows the convention found elsewhere in the file of being surrounded
by curly braces and delimited by whitespace (but not commas,
semicolons, or any other character). The order of the players is
significant; the first entry in the list will be numbered as player 1,
the second entry as player 2, and so forth.  At the end of the prologue
is an optional text comment field.



Structure of the body (list of nodes)
-------------------------------------

The body of the file lists the nodes which comprise the game tree.
These nodes are listed in the prefix traversal of the tree. The prefix
traversal for a subtree is defined as being the root node of the
subtree, followed by the prefix traversal of the subtree rooted by
each child, in order from first to last. Thus, for the whole tree, the
root node appears first, followed by the prefix traversals of its
child subtrees. For convenience, the game above follows the convention
of one line per node.

Each node entry begins with an unquoted character indicating the type
of the node. There are three node types:

+ `c` for a chance node
+ `p` for a personal player node
+ `t` for a terminal node

Each node type will be discussed individually below. There are three
numbering conventions which are used to identify the information
structure of the tree. Wherever a player number is called for, the
integer specified corresponds to the index of the player in the player
list from the prologue. The first player in the list is numbered 1,
the second 2, and so on. Information sets are identified by an
arbitrary positive integer which is unique within the player. Gambit
generates these numbers as 1, 2, etc. as they appear first in the
file, but there are no requirements other than uniqueness. The same
integer may be used to specify information sets for different players;
this is not ambiguous since the player number appears as well.
Finally, outcomes are also arbitrarily numbered in the file format in
the same way in which information sets are, except for the special
number 0 which is reserved to indicate the null outcome.
Outcome 0 must not have a name or payoffs specified.

Information sets and outcomes may (and frequently will) appear
multiple times within a game. By convention, the second and subsequent
times an information set or outcome appears, the file may omit the
descriptive information for that information set or outcome.
Alternatively, the file may specify the descriptive information again;
however, it must precisely match the original declaration of the
information set or outcome. Any mismatch in repeated declarations
is an error, and the file is not valid.
If any part of the description is omitted, the whole description must be omitted.

Outcomes may appear at nonterminal nodes. In these cases, payoffs are
interpreted as incremental payoffs; the payoff to a player for a
given path through the tree is interpreted as the sum of the payoffs
at the outcomes encountered on that path (including at the terminal
node). This is ideal for the representation of games with well-
defined"stages"; see, for example, the file bayes2a.efg in the Gambit
distribution for a two-stage example of the Bayesian game represented
previously.

In the following lists, fields which are omittable according to the
above rules are indicated by the label (optional).

**Format of chance (nature) nodes.** Entries for chance nodes begin
with the character c . Following this, in order, are



+ a text string, giving the name of the node
+ a positive integer specifying the information set number
+ (optional) the name of the information set and a list of actions at the information set with their
  corresponding probabilities
+ a nonnegative integer specifying the outcome
+ (optional) the name of the outcome and the payoffs to each player for the outcome

**Format of personal (player) nodes.** Entries for personal player
decision nodes begin with the character p . Following this, in order,
are:

+ a text string, giving the name of the node
+ a positive integer specifying the player who owns the node
+ a positive integer specifying the information set
+ (optional) the name of the information set and a list of action names for the information set
+ a nonnegative integer specifying the outcome
+ (optional) the name of the outcome and the payoffs to each player for the outcome


**Format of terminal nodes.** Entries for terminal nodes begin with
the character t . Following this, in order, are:

+ a text string, giving the name of the node
+ a nonnegative integer specifying the outcome
+ (optional) the name of the outcome and the payoffs to each player for the outcome

There is no explicit end-of-file delimiter for the file.
